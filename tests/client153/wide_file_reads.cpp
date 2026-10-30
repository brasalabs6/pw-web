// B05: the wide file reads the serverlist parser uses (a_fgetc/a_fgets —
// EC_ServerList.cpp). The game's data files are UTF-16 LE; the system
// fgetwc/fgetws decode one byte per wchar in the C locale and misread
// them under -fshort-wchar, so the port owns 16-bit-unit readers. The
// test drives the REAL seam functions through the game's own macros over
// a fixture shaped exactly like the shipped serverlist.txt (BOM, group
// line, tab-separated server lines with \r\n endings).
#define UNICODE
#include "ABaseDef.h"
#include "AChar.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

int failures = 0;

void check(const char *label, bool ok) {
  if (!ok) {
    ++failures;
    std::printf("FAIL %s\n", label);
  } else {
    std::printf("ok %s\n", label);
  }
}

// UTF-16 LE bytes of a wide string (no BOM — the caller adds it).
void append_utf16(std::string &out, const wchar_t *text) {
  for (const wchar_t *p = text; *p; ++p) {
    out.push_back(static_cast<char>(*p & 0xff));
    out.push_back(static_cast<char>((*p >> 8) & 0xff));
  }
}

}  // namespace

int main() {
  // The shipped serverlist.txt shape: BOM, group line, blank line, then
  // tab-separated server lines "name\tport:address\tzoneid\r\n".
  std::string bytes;
  bytes.push_back(static_cast<char>(0xff));
  bytes.push_back(static_cast<char>(0xfe));
  append_utf16(bytes, L"Level Up\r\n");
  append_utf16(bytes, L"\r\n");
  append_utf16(bytes, L"Draco (PvE)\t29000:gateway7.perfectworld.com.br\t7\r\n");
  append_utf16(bytes, L"Pegasus (PvP)\t29000:gateway6.perfectworld.com.br\t6\r\n");

  const char *path = "u05_serverlist_shape.txt";
  FILE *write = std::fopen(path, "wb");
  if (!write || std::fwrite(bytes.data(), 1, bytes.size(), write) != bytes.size()) {
    std::printf("FAIL fixture write\n");
    return 1;
  }
  std::fclose(write);

  FILE *file = std::fopen(path, "rb");
  check("fixture opens", file != nullptr);
  if (!file)
    return 1;

  // The parser's first read consumes the BOM as one wide unit.
  const int bom = a_fgetc(file);
  check("BOM consumed as one wide unit", bom == 0xFEFF);

  wchar_t line[256];
  check("group line reads", a_fgets(line, 256, file) != nullptr);
  check("group line exact", wcscmp(line, L"Level Up\r\n") == 0);
  check("blank line reads", a_fgets(line, 256, file) != nullptr);
  check("blank line exact", wcscmp(line, L"\r\n") == 0);
  check("server line reads", a_fgets(line, 256, file) != nullptr);
  check("server line exact",
        wcscmp(line, L"Draco (PvE)\t29000:gateway7.perfectworld.com.br\t7\r\n") == 0);
  check("second server line reads", a_fgets(line, 256, file) != nullptr);
  check("second server line exact",
        wcscmp(line, L"Pegasus (PvP)\t29000:gateway6.perfectworld.com.br\t6\r\n") == 0);
  check("EOF returns null", a_fgets(line, 256, file) == nullptr);
  std::fclose(file);
  std::remove(path);

  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all wide file read checks passed\n");
  return 0;
}
