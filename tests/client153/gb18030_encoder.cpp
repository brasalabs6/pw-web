// GB18030 encoder regression test. a_Utf8ToGb18030 must reproduce the
// official GB18030 mapping (the encoding the original .pck package indexes
// use) because musl's iconv cannot encode TO GB18030. Expected byte
// sequences generated from Python's official gb18030 codec (see
// tools/gen-gb18030-encoder.py for the table provenance).
#include "AAssist.h"
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const char* label, const char* utf8, const std::vector<unsigned char>& expected) {
  char out[512];
  const int n = a_Utf8ToGb18030(utf8, out, sizeof(out));
  const bool ok = n == static_cast<int>(expected.size()) &&
                  std::memcmp(out, expected.data(), expected.size()) == 0 && out[n] == '\0';
  if (!ok) {
    ++failures;
    std::printf("FAIL %s: got %d bytes", label, n);
    for (int i = 0; i < n && i < 64; ++i) std::printf(" %02x", (unsigned char)out[i]);
    std::printf(", expected %zu bytes", expected.size());
    for (unsigned char b : expected) std::printf(" %02x", b);
    std::printf("\n");
  } else {
    std::printf("ok %s\n", label);
  }
}

}  // namespace

int main() {
  // ASCII passthrough (package path with no Chinese characters).
  check("ascii passthrough", "surfaces\\ingame\\hp_warn.tga",
        {'s','u','r','f','a','c','e','s','\\','i','n','g','a','m','e','\\','h','p','_','w','a','r','n','.','t','g','a'});
  // GBK two-byte range: the UI texture whose package lookup failed before
  // the encoder existed (EC_ImageRes.cpp "InGame\\升级了.tga").
  check("gbk filename", "InGame\\\xe5\x8d\x87\xe7\xba\xa7\xe4\xba\x86.tga",
        {'I','n','G','a','m','e','\\',0xc9,0xfd,0xbc,0xb6,0xc1,0xcb,'.','t','g','a'});
  // The fireworks effect file whose package lookup failed the same way.
  check("gbk path", "gfx\\\xe7\x83\x9f\xe8\x8a\xb1\\\xe6\x95\x88\xe6\x9e\x9c.txt",
        {'g','f','x','\\',0xd1,0xcc,0xbb,0xa8,'\\',0xd0,0xa7,0xb9,0xfb,'.','t','x','t'});
  // The login loading screen image name.
  check("loading image", "\xe8\xbd\xbd\xe5\x85\xa5""1024-1.jpg",
        {0xd4,0xd8,0xc8,0xeb,'1','0','2','4','-','1','.','j','p','g'});
  // Four-byte range: BMP codepoint outside GBK (U+017F LATIN SMALL LETTER
  // LONG S -> 81 30 97 37).
  check("four-byte bmp", "\xc5\xbf", {0x81,0x30,0x97,0x37});
  // Four-byte range: supplementary plane (U+10000 -> 90 30 81 30).
  check("four-byte supplementary", "\xf0\x90\x80\x80", {0x90,0x30,0x81,0x30});
  // Mixed ASCII + Chinese.
  check("mixed", "Data\\\xe4\xbb\xbb\xe5\x8a\xa1\\\xe4\xbb\xbb\xe5\x8a\xa1""1.txt",
        {'D','a','t','a','\\',0xc8,0xce,0xce,0xf1,'\\',0xc8,0xce,0xce,0xf1,'1','.','t','x','t'});
  // Invalid UTF-8 must fail (return 0), never emit a wrong hash input.
  {
    char out[512];
    if (a_Utf8ToGb18030("\xff\xfe bad", out, sizeof(out)) != 0 || out[0] != '\0') {
      ++failures;
      std::printf("FAIL invalid utf-8 rejected\n");
    } else {
      std::printf("ok invalid utf-8 rejected\n");
    }
  }
  // Truncated output buffer must fail, not truncate silently.
  {
    char out[8];
    if (a_Utf8ToGb18030("InGame\\\xe5\x8d\x87\xe7\xba\xa7\xe4\xba\x86.tga", out, 8) != 0) {
      ++failures;
      std::printf("FAIL small buffer rejected\n");
    } else {
      std::printf("ok small buffer rejected\n");
    }
  }
  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all gb18030 encoder checks passed\n");
  return 0;
}
