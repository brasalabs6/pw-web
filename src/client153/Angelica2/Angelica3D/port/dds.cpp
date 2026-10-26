// DDS legacy DXT layout: Microsoft Programming Guide for DDS / D3D compressed
// formats. Preserves original compressed blocks and mipmaps; no transcoding or
// quality loss.
#include "dds.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace pw153 {
bool load_dds(const std::uint8_t *data, std::size_t size, Dds &output,
              std::string &error) {
  output = Dds{};
  error.clear();
  try {
    if (!data || size < 128 || std::memcmp(data, "DDS ", 4))
      throw std::runtime_error("invalid DDS signature/header");
    auto u32 = [&](std::size_t p) {
      return std::uint32_t(data[p]) | (std::uint32_t(data[p + 1]) << 8) |
             (std::uint32_t(data[p + 2]) << 16) |
             (std::uint32_t(data[p + 3]) << 24);
    };
    if (u32(4) != 124 || u32(76) != 32 || !(u32(80) & 4))
      throw std::runtime_error("unsupported DDS header");
    if (u32(112) || u32(24) > 1)
      throw std::runtime_error("DDS cubemap/volume unsupported");
    Dds next;
    next.width = u32(16);
    next.height = u32(12);
    if (!next.width || !next.height || next.width > 8192 || next.height > 8192)
      throw std::runtime_error("DDS dimensions exceed limits");
    const auto fourcc = u32(84);
    std::size_t block_bytes = 16;
    if (fourcc == 0x31545844) {
      next.gl_format = 0x83F1;
      block_bytes = 8;
    } else if (fourcc == 0x33545844)
      next.gl_format = 0x83F2;
    else if (fourcc == 0x35545844)
      next.gl_format = 0x83F3;
    else
      throw std::runtime_error("unsupported DDS compression");
    auto width = next.width, height = next.height;
    std::size_t offset = 128;
    const auto count = std::max(1u, u32(28));
    unsigned max_levels = 1;
    for (auto extent = std::max(width, height); extent > 1; extent >>= 1)
      max_levels++;
    if (count > max_levels)
      throw std::runtime_error("invalid DDS mip count");
    for (unsigned level = 0; level < count; level++) {
      const std::size_t bytes =
          std::size_t((width + 3) / 4) * ((height + 3) / 4) * block_bytes;
      if (bytes > size - offset)
        throw std::runtime_error("truncated DDS mip chain");
      next.levels.push_back({width, height, offset, bytes});
      offset += bytes;
      width = std::max(1u, width / 2);
      height = std::max(1u, height / 2);
    }
    if (offset != size)
      throw std::runtime_error("unexpected trailing DDS data");
    output = std::move(next);
    return true;
  } catch (const std::exception &exc) {
    error = exc.what();
    return false;
  }
}
} // namespace pw153
