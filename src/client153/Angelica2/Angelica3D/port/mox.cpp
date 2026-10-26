/* Portable binary load path adapted from A3DFrame::Load, A3DMesh::Load,
 * A3DMaterial::Load and AFile::ReadString at client-153
 * e3d6d0075d0a745c9f9859f2cb25b01d71dec045.
 * Original Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 * Bounds, fixed-width decoding and transactional output replace Win32/file
 * APIs.
 */
#include "mox.h"
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>
namespace pw153 {
namespace {
class Reader {
  const std::uint8_t *data_;
  std::size_t size_, pos_ = 0;

public:
  Reader(const std::uint8_t *data, std::size_t size)
      : data_(data), size_(size) {}
  std::size_t remaining() const { return size_ - pos_; }
  const std::uint8_t *take(std::size_t n) {
    if (n > remaining())
      throw std::runtime_error("truncated MOX at byte " + std::to_string(pos_));
    const auto *out = data_ + pos_;
    pos_ += n;
    return out;
  }
  std::vector<std::uint8_t> bytes(std::size_t n) {
    const auto *p = take(n);
    return {p, p + n};
  }
  std::uint32_t u32() {
    auto b = take(4);
    return std::uint32_t(b[0]) | (std::uint32_t(b[1]) << 8) |
           (std::uint32_t(b[2]) << 16) | (std::uint32_t(b[3]) << 24);
  }
  std::uint16_t u16() {
    auto b = take(2);
    return std::uint16_t(b[0]) | (std::uint16_t(b[1]) << 8);
  }
  bool boolean() {
    auto v = take(1)[0];
    if (v > 1)
      throw std::runtime_error("invalid MOX bool");
    return v != 0;
  }
  float f32() {
    auto u = u32();
    float f;
    std::memcpy(&f, &u, 4);
    if (!std::isfinite(f))
      throw std::runtime_error("nonfinite MOX float");
    return f;
  }
  std::string string() {
    std::string s;
    while (true) {
      if (!remaining() || s.size() >= 2047)
        throw std::runtime_error("truncated or oversized MOX string");
      auto c = take(1)[0];
      if (!c)
        return s;
      s.push_back(char(c));
    }
  }
  std::uint32_t count(std::uint32_t max, const char *what) {
    auto n = u32();
    if (n > max)
      throw std::runtime_error(std::string("MOX count exceeds limit: ") + what);
    return n;
  }
  Matrix matrix() {
    Matrix m;
    for (auto &f : m)
      f = f32();
    return m;
  }
};
std::string field(const std::string &text, const char *prefix) {
  const std::string p(prefix);
  if (text.compare(0, p.size(), p))
    throw std::runtime_error("expected MOX field " + p);
  return text.substr(p.size());
}
unsigned versioned(Reader &r, std::string &label) {
  label = r.string();
  if (label.compare(0, 9, "VERSION: "))
    return 1;
  auto v = field(label, "VERSION: ");
  if (v != "1" && v != "2" && v != "100")
    throw std::runtime_error("unsupported MOX record version " + v);
  label = r.string();
  return unsigned(std::stoul(v));
}
void mesh(Reader &r, Model &model, std::size_t owner) {
  if (model.meshes.size() >= 4096)
    throw std::runtime_error("too many MOX meshes");
  Mesh m;
  m.frame = owner;
  std::string label;
  auto version = versioned(r, label);
  m.name = field(label, "MESH: ");
  auto nv = r.count(65536, "vertices"), ni = r.count(196608, "indices"),
       nf = r.count(4096, "mesh frames");
  if (!nv || !ni || !nf)
    throw std::runtime_error("empty MOX mesh");
  const bool lod = r.boolean();
  m.wire = r.boolean();
  if (ni % (m.wire ? 2 : 3))
    throw std::runtime_error("incomplete MOX primitive");
  if (version == 1)
    m.properties = r.bytes(r.count(40, "version 1 mesh properties"));
  else
    m.properties = r.bytes(8);
  m.indices.reserve(ni);
  for (unsigned i = 0; i < ni; i++) {
    auto index = r.u16();
    if (index >= nv)
      throw std::runtime_error("MOX index outside vertex range");
    m.indices.push_back(index);
  }
  if (lod) {
    m.lod_min = r.f32();
    m.lod_max = r.f32();
    m.lod_limit = r.u32();
    for (unsigned i = 0; i < nv; i++)
      m.lod_map.push_back(r.u16());
  }
  if (std::uint64_t(nv) * nf * 32 > r.remaining())
    throw std::runtime_error("truncated MOX vertex frames");
  m.vertices.resize(nf);
  for (auto &frame : m.vertices) {
    frame.resize(nv);
    for (auto &v : frame) {
      v.x = r.f32();
      v.y = r.f32();
      v.z = r.f32();
      v.nx = r.f32();
      v.ny = r.f32();
      v.nz = r.f32();
      v.u = r.f32();
      v.v = r.f32();
    }
  }
  label = r.string();
  if (label.compare(0, 15, "DETAILTEXTURE: ") == 0) {
    m.detail_texture = field(label, "DETAILTEXTURE: ");
    m.detail_matrix = r.matrix();
    label = r.string();
  }
  m.texture = field(label, "TEXTURE: ");
  field(r.string(), "MATERIAL: ");
  for (auto &value : m.material.values) {
    value = r.f32();
  }
  m.material.two_sided = r.boolean();
  if (version == 100) {
    m.lightmap.resize(nf);
    for (auto &frame : m.lightmap) {
      frame.resize(nv);
      for (auto &uv : frame) {
        uv[0] = r.f32();
        uv[1] = r.f32();
      }
    }
  }
  model.meshes.push_back(std::move(m));
}
void frame(Reader &r, Model &model, int parent, unsigned depth) {
  if (depth > 64 || model.frames.size() >= 4096)
    throw std::runtime_error("MOX frame hierarchy limit");
  Frame f;
  f.parent = parent;
  std::string label;
  auto version = versioned(r, label);
  if (version > 2)
    throw std::runtime_error("unsupported MOX frame version");
  f.name = field(label, "FRAME: ");
  auto nf = r.count(4096, "frame transforms");
  if (!nf || std::uint64_t(nf) * 64 > r.remaining())
    throw std::runtime_error("invalid MOX transform count");
  f.transforms.reserve(nf);
  for (unsigned i = 0; i < nf; i++)
    f.transforms.push_back(r.matrix());
  auto boxes = r.count(4096, "frame boxes");
  for (unsigned i = 0; i < boxes; i++)
    f.boxes.push_back(r.bytes(version == 1 ? 168 : 136));
  const auto owner = model.frames.size();
  model.frames.push_back(std::move(f));
  auto meshes = r.count(4096, "meshes");
  for (unsigned i = 0; i < meshes; i++)
    mesh(r, model, owner);
  auto children = r.count(4096, "child frames");
  for (unsigned i = 0; i < children; i++)
    frame(r, model, int(owner), depth + 1);
}
} // namespace
bool load_mox(const std::uint8_t *data, std::size_t size, Model &output,
              std::string &error) {
  output = Model{};
  error.clear();
  try {
    if (!data || size < 4 || size > 64 * 1024 * 1024)
      throw std::runtime_error("invalid MOX input size");
    Reader r(data, size);
    auto magic = r.bytes(4);
    if (std::memcmp(magic.data(), "MOXB", 4))
      throw std::runtime_error("unsupported MOX signature");
    Model next;
    frame(r, next, -1, 0);
    if (r.remaining())
      throw std::runtime_error("unexpected trailing MOX bytes");
    output = std::move(next);
    return true;
  } catch (const std::exception &exc) {
    error = exc.what();
    return false;
  }
}
} // namespace pw153
