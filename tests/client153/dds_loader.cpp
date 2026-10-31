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
  std::vector<unsigned char> rgba(136);
  rgba[0]='D'; rgba[1]='D'; rgba[2]='S'; rgba[3]=' ';
  put(rgba,4,124); put(rgba,12,1); put(rgba,16,2); put(rgba,28,1);
  put(rgba,76,32); put(rgba,80,0x41); put(rgba,88,32);
  put(rgba,92,0xff0000); put(rgba,96,0xff00); put(rgba,100,0xff); put(rgba,104,0xff000000);
  rgba[128]=1; rgba[129]=2; rgba[130]=3; rgba[131]=4;
  CHECK(pw153::load_dds(rgba.data(), rgba.size(), out, error));
  CHECK(out.encoding == pw153::DdsEncoding::bgra8);
  std::vector<unsigned char> pixels;
  CHECK(pw153::dds_bgra_to_rgba(rgba.data(), rgba.size(), out.levels[0], pixels));
  CHECK(pixels[0]==3 && pixels[1]==2 && pixels[2]==1 && pixels[3]==4);
  CHECK(!pw153::dds_bgra_to_rgba(rgba.data(), 130, out.levels[0], pixels));
  put(rgba,104,0); CHECK(!pw153::load_dds(rgba.data(), rgba.size(), out, error));
  std::cout
      << "PASS DDS DXT3 layout, mip size, truncation, unsupported format\n";
}
