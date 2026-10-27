#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
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
  GNET::Marshal::OctetsStream float_bytes;
  float_bytes << 1.0f;
  const unsigned char float_expected[] = {0x3f, 0x80, 0x00, 0x00};
  CHECK(float_bytes.size() == sizeof(float_expected));
  CHECK(std::memcmp(float_bytes.begin(), float_expected, sizeof(float_expected)) == 0);
  float float_decoded = 0;
  GNET::Marshal::OctetsStream float_input(float_bytes);
  float_input >> float_decoded;
  CHECK(float_decoded == 1.0f);
  GNET::Marshal::OctetsStream double_bytes;
  double_bytes << 1.0;
  const unsigned char double_expected[] = {0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  CHECK(double_bytes.size() == sizeof(double_expected));
  CHECK(std::memcmp(double_bytes.begin(), double_expected, sizeof(double_expected)) == 0);
  double double_decoded = 0;
  GNET::Marshal::OctetsStream double_input(double_bytes);
  double_input >> double_decoded;
  CHECK(double_decoded == 1.0);
  const std::string text = "perfect world";
  GNET::Marshal::OctetsStream text_bytes;
  text_bytes << text;
  std::string decoded_text;
  GNET::Marshal::OctetsStream text_input(text_bytes);
  text_input >> decoded_text;
  CHECK(decoded_text == text);
  const std::u16string wide_text = u"PW";
  GNET::Marshal::OctetsStream wide_bytes;
  wide_bytes << wide_text;
  std::u16string decoded_wide;
  GNET::Marshal::OctetsStream wide_input(wide_bytes);
  wide_input >> decoded_wide;
  CHECK(decoded_wide == wide_text);
  const GNET::Octets bytes(text.data(), text.size());
  GNET::Marshal::OctetsStream octets_bytes;
  octets_bytes << bytes;
  GNET::Octets decoded_octets;
  GNET::Marshal::OctetsStream octets_input(octets_bytes);
  octets_input >> decoded_octets;
  CHECK(decoded_octets == bytes);
  std::vector<unsigned int> values{1, 255, 65536};
  GNET::Marshal::OctetsStream sequence;
  sequence << values;
  std::vector<unsigned int> result;
  GNET::Marshal::OctetsStream input(sequence);
  input >> result;
  CHECK(result == values);
}
