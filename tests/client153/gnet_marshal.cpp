#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "gnmarshal.h"
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
int main() {
  CHECK(GNET::byteorder_32(0x12345678u) == 0x78563412u);
  CHECK(GNET::byteorder_64(UINT64_C(0x0123456789abcdef)) == UINT64_C(0xefcdab8967452301));
  const unsigned int value = 0x4000;
  GNET::Marshal::OctetsStream out;
  out << GNET::CompactUINT(value);
  const unsigned char expected[] = {0xc0, 0x00, 0x40, 0x00};
  CHECK(out.size() == sizeof(expected));
  CHECK(std::memcmp(out.begin(), expected, sizeof(expected)) == 0);
  unsigned int decoded = 0;
  GNET::Marshal::OctetsStream in(out);
  in >> GNET::CompactUINT(decoded);
  CHECK(decoded == value);
  std::vector<unsigned int> values{1, 255, 65536};
  GNET::Marshal::OctetsStream sequence;
  sequence << values;
  std::vector<unsigned int> result;
  GNET::Marshal::OctetsStream input(sequence);
  input >> result;
  CHECK(result == values);
}
