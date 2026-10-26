#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace pw153 {
struct DdsLevel {
  std::uint32_t width, height;
  std::size_t offset, bytes;
};
struct Dds {
  std::uint32_t width = 0, height = 0, gl_format = 0;
  std::vector<DdsLevel> levels;
};
bool load_dds(const std::uint8_t *, std::size_t, Dds &, std::string &);
} // namespace pw153
