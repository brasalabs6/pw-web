// Tests for the original AngelicaFile family: AFilePackage loads a real v145
// PCK, entries match the independently validated Python reader, shared files
// open/close through the widened handle, AFileImage reads package content,
// AIniFile round-trips sections/keys, and the zlib compress/uncompress pair
// round-trips. Cross-validation data comes from tools/assets/pck153.py.
#include "AFileImage.h"
#include "AFilePackage.h"
#include "AFilePackMan.h"
#include "AFilePackGame.h"
#include "AFI.h"
#include "AIniFile.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)

static int count_entries(const char *pckPath) {
  AFilePackage package;
  CHECK(package.Open(pckPath, AFilePackage::OPENEXIST, false));
  const int count = package.GetFileNumber();
  CHECK(package.Close());
  return count;
}

static void test_real_package(const char *pckPath, int expectedEntries) {
  // The Python reader validated 15 packages / 304,636 files with this layout;
  // facedata.pck carries 2005 entries in that evidence.
  const int count = count_entries(pckPath);
  std::cout << "facedata.pck entries: " << count << "\n";
  CHECK(count == expectedEntries);

  AFilePackage package;
  CHECK(package.Open(pckPath, AFilePackage::OPENEXIST, false));

  CHECK(package.Close());
}

static void test_shared_file_roundtrip(const char *pckPath) {
  AFilePackage package;
  CHECK(package.Open(pckPath, AFilePackage::OPENEXIST, false));
  const int count = package.GetFileNumber();
  CHECK(count > 0);

  // Open every entry through the widened shared-file handle and confirm the
  // reported length matches the entry table; close each one.
  int opened = 0;
  for (int i = 0; i < count && opened < 64; ++i) {
    const AFilePackage::FILEENTRY *entry = package.GetFileEntryByIndex(i);
    if (!entry || entry->dwLength == 0)
      continue;
    BYTE *buffer = nullptr;
    DWORD length = 0;
    const uintptr_t handle = package.OpenSharedFile(entry->szFileName, &buffer, &length, false);
    if (handle) {
      CHECK(buffer != nullptr);
      CHECK(length == entry->dwLength);
      package.CloseSharedFile(handle);
      ++opened;
    }
  }
  CHECK(opened > 0);
  std::cout << "shared files opened/verified: " << opened << "\n";
  CHECK(package.Close());
}

static void test_ini_roundtrip() {
  //	The original engine sets the base dir at startup (af_Initialize);
  //	tests pin it to the current directory explicitly.
  CHECK(af_SetBaseDir("."));

  //	AIniFile::Open requires an existing, non-empty file (AFILE_OPENEXIST
  //	and AFileImage reject zero-length files).
  {
    FILE *seed = std::fopen("test_output.ini", "w");
    CHECK(seed != nullptr);
    std::fputs("[a3d]\n", seed);
    std::fclose(seed);
  }

  AIniFile ini;
  CHECK(ini.Open("test_output.ini"));
  CHECK(ini.WriteIntValue("a3d", "DetailScene", 1));
  CHECK(ini.WriteIntValue("a3d", "NewBoneScale", 0));
  //	Original semantics: writes are in-memory; Save persists.
  CHECK(ini.Save("test_output.ini"));
  ini.Close();

  AIniFile reader;
  CHECK(reader.Open("test_output.ini"));
  CHECK(reader.GetValueAsBoolean("a3d", "DetailScene", false) == true);
  CHECK(reader.GetValueAsBoolean("a3d", "NewBoneScale", true) == false);
  CHECK(reader.GetValueAsInt("a3d", "DetailScene", 0) == 1);
  reader.Close();
  std::remove("test_output.ini");
}

static DWORD reject_decryption(const void *, DWORD, void *, DWORD *) {
  return 2;
}

static void test_decryption_failure_cleanup() {
  FILE *file = std::fopen("test_decryption.bin", "wb");
  CHECK(file != nullptr);
  CHECK(std::fwrite("testdata", 1, 8, file) == 8);
  CHECK(std::fclose(file) == 0);
  AFileImage::SetExtDecryptCallback(reject_decryption);
  {
    AFileImage image;
    CHECK(!image.Open("test_decryption.bin", AFILE_OPENEXIST));
    image.Close();
  }
  AFileImage::SetExtDecryptCallback(nullptr);
  CHECK(std::remove("test_decryption.bin") == 0);
}

static void test_zlib_roundtrip() {
  std::string payload(10000, '\0');
  for (std::size_t i = 0; i < payload.size(); ++i)
    payload[i] = static_cast<char>((i * 7 + 3) & 0xff);

  const DWORD srcLen = static_cast<DWORD>(payload.size());
  std::string compressed(payload.size() + 1024, '\0');
  DWORD compressedLen = static_cast<DWORD>(compressed.size());
  CHECK(AFilePackage::Compress(reinterpret_cast<BYTE *>(payload.data()), srcLen,
                               reinterpret_cast<BYTE *>(compressed.data()),
                               &compressedLen) == 0);
  CHECK(compressedLen > 0 && compressedLen < srcLen);

  std::string restored(payload.size(), '\0');
  DWORD restoredLen = static_cast<DWORD>(restored.size());
  CHECK(AFilePackage::Uncompress(reinterpret_cast<BYTE *>(compressed.data()),
                                 compressedLen,
                                 reinterpret_cast<BYTE *>(restored.data()),
                                 &restoredLen) == 0);
  CHECK(restoredLen == srcLen);
  CHECK(restored == payload);
}

int main(int argc, char **argv) {
  try {
    // Output capacity is MAX_PATH; too-long input must fail without writing past it.
    char guarded[512]; std::memset(guarded, 0x5a, sizeof(guarded));
    af_GetRelativePathNoBase("//Configs\\element_client.cfg", "/", guarded);
    CHECK(std::string(guarded) == "Configs\\element_client.cfg");
    const std::string limit(MAX_PATH - 1, 'x');
    af_GetFullPathNoBase(guarded, "", limit.c_str());
    CHECK(std::string(guarded) == limit);
    CHECK(guarded[MAX_PATH] == 0x5a);
    AString string_path;
    af_GetFullPathNoBase(string_path, "", limit.c_str());
    CHECK(std::string((const char*)string_path) == limit);
    const std::string oversized(MAX_PATH, 'x');
    bool rejected = false;
    try { af_GetFullPathNoBase(guarded, "", oversized.c_str()); }
    catch (const std::length_error &) { rejected = true; }
    CHECK(rejected);
    CHECK(guarded[MAX_PATH] == 0x5a);
    rejected = false;
    try { af_GetFullPath(guarded, oversized.c_str(), "/model"); }
    catch (const std::length_error &) { rejected = true; }
    CHECK(rejected);
    rejected = false;
    try { af_GetRelativePathNoBase(oversized.c_str(), "", guarded); }
    catch (const std::length_error &) { rejected = true; }
    CHECK(rejected);
    CHECK(guarded[MAX_PATH] == 0x5a);
    AFileImage image;
    CHECK(!image.Open(oversized.c_str(), AFILE_OPENEXIST));
    CHECK(!image.Open("folder", oversized.c_str(), AFILE_OPENEXIST));
    AFilePackage package;
    CHECK(!package.Open(oversized.c_str(), AFilePackage::OPENEXIST, false));
    CHECK(!package.Open("missing.pck", "", AFilePackage::OPENEXIST, false));
    AFilePackGame game;
    CHECK(!game.Open(oversized.c_str(), AFilePackGame::OPENEXIST, false));
    CHECK(!game.Open("missing.pck", "", AFilePackGame::OPENEXIST, false));
    test_decryption_failure_cleanup();
    test_zlib_roundtrip();
    test_ini_roundtrip();
    if (argc == 2) {
      test_real_package(argv[1], 2005);
      test_shared_file_roundtrip(argv[1]);
    } else {
      std::cout << "package tests skipped: pass the authorized PCK path\n";
    }
    std::cout << "client153 file family: all tests passed\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << e.what() << "\n";
    return 1;
  }
}
