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
enum class DdsEncoding { compressed, bgra8 };
struct Dds {
  DdsEncoding encoding = DdsEncoding::compressed;
  std::uint32_t width = 0, height = 0, gl_format = 0;
  std::vector<DdsLevel> levels;
};
bool dds_bgra_to_rgba(const std::uint8_t *, std::size_t, const DdsLevel &,
                       std::vector<std::uint8_t> &);
bool load_dds(const std::uint8_t *, std::size_t, Dds &, std::string &);
} // namespace pw153
