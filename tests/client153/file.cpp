#include "AFile.h"
#include "mox.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main(int argc, char **argv) {
  if (argc == 2) {
    AFile file;
    CHECK(file.Open(argv[1], AFILE_OPENEXIST | AFILE_BINARY));
    CHECK(file.IsBinary());
    CHECK(file.GetPos() == 4);
    char field[2048];
    DWORD count = 0;
    CHECK(file.ReadString(field, sizeof(field), &count));
    CHECK(std::strncmp(field, "FRAME: ", 7) == 0 ||
          std::strncmp(field, "VERSION: ", 9) == 0);
    const auto size = file.GetFileLength();
    CHECK(size >= 4);
    CHECK(file.ResetPointer());
    std::vector<unsigned char> bytes(size);
    CHECK(file.Read(bytes.data(), size, &count));
    CHECK(count == size);
    pw153::Model model;
    std::string error;
    CHECK(pw153::load_mox(bytes.data(), bytes.size(), model, error));
    std::cout << "PASS REAL AFile -> MOX: bytes=" << size
              << " frames=" << model.frames.size()
              << " meshes=" << model.meshes.size() << "\n";
    return 0;
  }

  const char *path = "client153-file-test.mox";
  AFile f;
  CHECK(f.Open(path, AFILE_CREATENEW | AFILE_BINARY));
  CHECK(f.WriteString(AString("original")));
  CHECK(f.Close());
  CHECK(f.Open(path, AFILE_OPENEXIST | AFILE_BINARY));
  CHECK(f.GetPos() == 4);
  AString text;
  CHECK(f.ReadString(text));
  CHECK(text == "original");
  CHECK(f.GetFileLength() == 16);
  CHECK(!f.ReadString(text));
  CHECK(f.ResetPointer());
  CHECK(f.GetPos() == 0);
  CHECK(f.Close());
  FILE *raw = std::fopen(path, "wb");
  CHECK(raw);
  std::fwrite("MOXB\x04\0\0\0ab", 1, 10, raw);
  std::fclose(raw);
  CHECK(f.Open(path, AFILE_OPENEXIST | AFILE_BINARY));
  text = "unchanged";
  CHECK(!f.ReadString(text));
  CHECK(text == "unchanged");
  CHECK(f.Close());
  raw = std::fopen(path, "wb");
  CHECK(raw);
  std::fwrite("MOXBabc", 1, 7, raw);
  std::fclose(raw);
  CHECK(f.Open(path, AFILE_OPENEXIST));
  char out[8];
  DWORD count = 99;
  CHECK(!f.ReadString(out, sizeof(out), &count));
  CHECK(count == 0);
  CHECK(f.Close());
  CHECK(!f.Open("missing-client153-file", AFILE_OPENEXIST));
  CHECK(!f.Seek(0, AFILE_SEEK_SET));
  std::remove(path);
  std::cout << "PASS AFile original binary strings, MOXB, rewind, truncation, "
               "missing-file and closed seek\n";
}
