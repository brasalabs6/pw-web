// Portable binary loader adapted from Angelica 1.5.3. See source-manifest.json.
// Original Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace pw153 {
using Matrix = std::array<float, 16>;
struct Vertex {
  float x, y, z, nx, ny, nz, u, v;
};
static_assert(sizeof(Vertex) == 32, "Original A3DVERTEX disk layout");
struct Material {
  std::array<float, 17> values{};
  bool two_sided = false;
};
struct Frame {
  std::string name;
  int parent = -1;
  std::vector<Matrix> transforms;
  std::vector<std::vector<std::uint8_t>> boxes;
};
struct Mesh {
  std::string name, texture, detail_texture;
  std::size_t frame = 0;
  std::vector<std::vector<Vertex>> vertices;
  std::vector<std::uint16_t> indices, lod_map;
  std::vector<std::uint8_t> properties;
  bool wire = false;
  float lod_min = 0, lod_max = 0;
  std::uint32_t lod_limit = 0;
  Matrix detail_matrix{};
  Material material;
  std::vector<std::vector<std::array<float, 2>>> lightmap;
};
struct Model {
  std::vector<Frame> frames;
  std::vector<Mesh> meshes;
};
bool load_mox(const std::uint8_t *, std::size_t, Model &, std::string &error);
} // namespace pw153
