#include "AFile.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
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
