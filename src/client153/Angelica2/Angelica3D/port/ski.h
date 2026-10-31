#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace pw153 {
struct SkinVertex {
  std::array<float, 3> position{}, weights{}, normal{};
  std::array<std::uint8_t, 4> bones{};
  std::array<float, 2> uv{};
};
struct SkinMesh {
  std::string name;
  std::int32_t texture = -1, material = -1;
  std::vector<SkinVertex> vertices;
  std::vector<std::uint16_t> indices;
};
struct SkinMaterial {
  std::string name;
  std::array<float, 17> values{};
  bool two_sided = false;
};
struct Skin {
  std::uint32_t version = 0, skeleton_bones = 0, skin_bones = 0;
  float min_weight = 0;
  std::vector<std::string> bone_names, textures;
  std::vector<SkinMaterial> materials;
  std::vector<SkinMesh> meshes;
};
bool load_ski(const std::uint8_t *data, std::size_t size, Skin &out,
              std::string &error);
} // namespace pw153
