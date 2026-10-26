#include "ABaseDef.h"
#include <cstdint>
#include <cstdio>
#include <stdexcept>
int main() {
  if (sizeof(ACHAR) != 2) throw std::runtime_error("Unicode client characters must be 16-bit UTF-16 code units");
  const ACHAR sample[] = _AL("A\u00e9");
  if (sample[0] != 0x41 || sample[1] != 0xe9 || sample[2] != 0)
    throw std::runtime_error("Unicode literals must preserve UTF-16 code units");
  std::puts("PASS original client UTF-16 code-unit width and literals");
}
