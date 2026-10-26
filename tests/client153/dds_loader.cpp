#include "dds.h"
#include <stdexcept>
#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition))                                                          \
      throw std::runtime_error("check failed: " #condition);                   \
  } while (false)
#include <iostream>
#include <vector>
void put(std::vector<unsigned char> &b, unsigned offset, unsigned value) {
  for (unsigned i = 0; i < 4; i++)
    b[offset + i] = (value >> (8 * i)) & 255;
}
int main() {
  std::vector<unsigned char> b(144);
  b[0] = 'D';
  b[1] = 'D';
  b[2] = 'S';
  b[3] = ' ';
  put(b, 4, 124);
  put(b, 12, 4);
  put(b, 16, 4);
  put(b, 28, 1);
  put(b, 76, 32);
  put(b, 80, 4);
  put(b, 84, 0x33545844);
  pw153::Dds out;
  std::string error;
  CHECK(pw153::load_dds(b.data(), b.size(), out, error));
  CHECK(out.width == 4 && out.levels.size() == 1 && out.levels[0].bytes == 16);
  for (unsigned i = 0; i < b.size(); i++)
    CHECK(!pw153::load_dds(b.data(), i, out, error));
  put(b, 84, 0x30315844);
  CHECK(!pw153::load_dds(b.data(), b.size(), out, error));
  std::cout
      << "PASS DDS DXT3 layout, mip size, truncation, unsupported format\n";
}
