#include "AString.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
  AString source("original"), copy(source);
  copy[0] = 'O';
  CHECK(source == "original");
  CHECK(copy == "Original");
  copy += " client";
  CHECK(copy == "Original client");
  AString n("9223372036854775807");
  CHECK(n.ToInt64() == 9223372036854775807LL);
  AString formatted;
  formatted.Format("%s %c %.*f %I64d %%", "wolf", 'A', 2, 1.25,
                   1234567890123LL);
  CHECK(formatted == "wolf A 1.25 1234567890123 %");
  AString shared(formatted);
  formatted.Format("%s!", static_cast<const char *>(formatted));
  CHECK(shared == "wolf A 1.25 1234567890123 %");
  CHECK(formatted == "wolf A 1.25 1234567890123 %!");
  std::string large(9000, 'x');
  formatted.Format("%s:%d", large.c_str(), 42);
  CHECK(formatted.GetLength() == 9003);
  AString caseTest("Wolf_153");
  caseTest.MakeUpper();
  CHECK(caseTest == "WOLF_153");
  caseTest.MakeLower();
  CHECK(caseTest == "wolf_153");
  CHECK(caseTest.CompareNoCase("WOLF_153") == 0);
  AString padded("  bone  ");
  padded.TrimLeft();
  padded.TrimRight();
  CHECK(padded == "bone");
  CHECK(padded.Mid(1, 2) == "on");
  CHECK(padded.Find("ne") == 2);
  char *buffer = padded.GetBuffer(20);
  std::strcpy(buffer, "skeleton");
  padded.ReleaseBuffer();
  CHECK(padded.GetLength() == 8);
  std::cout << "PASS original AString COW, conversions, Windows I64 format, "
               "aliasing, large formatting and buffer lifecycle\n";
}
