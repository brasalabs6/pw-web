/* Adapted from A3DSkin::Load, A3DSkinMesh::LoadData and A3DMaterial::Load,
 * Angelica client revision e3d6d0075d0a745c9f9859f2cb25b01d71dec045.
 * Original Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 * This first loader supports skin meshes; other mesh classes fail explicitly.
 */
#include "ski.h"
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>
namespace pw153 {
namespace {
class Reader {
  const std::uint8_t *data;
  std::size_t size, pos = 0;

public:
  Reader(const std::uint8_t *p, std::size_t n) : data(p), size(n) {}
  std::size_t remaining() const { return size - pos; }
  const std::uint8_t *take(std::size_t n) {
    if (n > remaining())
      throw std::runtime_error("truncated SKI");
    auto *p = data + pos;
    pos += n;
    return p;
  }
  std::uint32_t u32() {
    auto p = take(4);
    return std::uint32_t(p[0]) | std::uint32_t(p[1]) << 8 |
           std::uint32_t(p[2]) << 16 | std::uint32_t(p[3]) << 24;
  }
  std::uint16_t u16() {
    auto p = take(2);
    return std::uint16_t(p[0]) | std::uint16_t(p[1]) << 8;
  }
  float f32() {
    auto n = u32();
    float f;
    std::memcpy(&f, &n, 4);
    if (!std::isfinite(f))
      throw std::runtime_error("nonfinite SKI float");
    return f;
  }
  std::string string() {
    auto n = u32();
    if (n > 4096)
      throw std::runtime_error("oversized SKI string");
    auto p = take(n);
    return {reinterpret_cast<const char *>(p), n};
  }
  std::string line() {
    std::string s;
    while (true) {
      auto c = *take(1);
      if (!c)
        return s;
      if (s.size() >= 2047)
        throw std::runtime_error("oversized SKI material");
      s.push_back(char(c));
    }
  }
  std::uint32_t count(std::uint32_t limit) {
    auto n = u32();
    if (n > limit)
      throw std::runtime_error("SKI count exceeds limit");
    return n;
  }
};
} // namespace
bool load_ski(const std::uint8_t *data, std::size_t size, Skin &out,
              std::string &error) {
  out = {};
  error.clear();
  try {
    if (!data || size < 108)
      throw std::runtime_error("truncated SKI header");
    Reader r(data, size);
    if (r.u32() != 0x42584f4d || r.u32() != 0x41534b49)
      throw std::runtime_error("invalid SKI signature");
    Skin result;
    result.version = r.u32();
    if (result.version != 8 && result.version != 9)
      throw std::runtime_error("unsupported SKI version");
    auto nmesh = r.count(4096), rigid = r.count(4096),
         morphskin = r.count(4096), morphrigid = r.count(4096);
    auto ntex = r.count(4096), nmaterial = r.count(4096);
    result.skin_bones = r.count(4096);
    result.min_weight = r.f32();
    result.skeleton_bones = r.count(4096);
    auto supple = r.count(4096), muscle = r.count(4096);
    r.take(52);
    if (rigid || morphskin || morphrigid || supple || muscle)
      throw std::runtime_error("unsupported SKI mesh class");
    if (result.version >= 9)
      for (std::uint32_t i = 0; i < result.skin_bones; i++)
        result.bone_names.push_back(r.string());
    for (std::uint32_t i = 0; i < ntex; i++)
      result.textures.push_back(r.string());
    for (std::uint32_t i = 0; i < nmaterial; i++) {
      SkinMaterial m;
      m.name = r.line();
      if (m.name.compare(0, 10, "MATERIAL: "))
        throw std::runtime_error("invalid SKI material");
      for (auto &v : m.values)
        v = r.f32();
      auto b = *r.take(1);
      if (b > 1)
        throw std::runtime_error("invalid SKI material flag");
      m.two_sided = b != 0;
      result.materials.push_back(std::move(m));
    }
    for (std::uint32_t i = 0; i < nmesh; i++) {
      SkinMesh m;
      m.name = r.string();
      m.texture = static_cast<std::int32_t>(r.u32());
      m.material = static_cast<std::int32_t>(r.u32());
      auto nv = r.count(1000000), ni = r.count(3000000);
      if (nv > r.remaining() / 48)
        throw std::runtime_error("truncated SKI vertices");
      m.vertices.reserve(nv);
      for (std::uint32_t j = 0; j < nv; j++) {
        SkinVertex v;
        for (auto &x : v.position)
          x = r.f32();
        for (auto &x : v.weights)
          x = r.f32();
        auto indices = r.u32();
        for (unsigned k = 0; k < 4; k++)
          v.bones[k] = std::uint8_t(indices >> (8 * k));
        for (auto &x : v.normal)
          x = r.f32();
        for (auto &x : v.uv)
          x = r.f32();
        m.vertices.push_back(v);
      }
      if (ni > r.remaining() / 2)
        throw std::runtime_error("truncated SKI indices");
      m.indices.reserve(ni);
      for (std::uint32_t j = 0; j < ni; j++) {
        auto n = r.u16();
        if (n >= nv)
          throw std::runtime_error("invalid SKI index");
        m.indices.push_back(n);
      }
      result.meshes.push_back(std::move(m));
    }
    if (r.remaining())
      throw std::runtime_error("unexpected trailing SKI data");
    out = std::move(result);
    return true;
  } catch (const std::exception &e) {
    error = e.what();
    return false;
  }
}
} // namespace pw153
