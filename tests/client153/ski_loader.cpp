#include "ski.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
using Bytes = std::vector<std::uint8_t>;
void u32(Bytes &b, unsigned n) {
  for (int i = 0; i < 4; i++)
    b.push_back(n >> (i * 8));
}
void f32(Bytes &b, float f) {
  unsigned u;
  std::memcpy(&u, &f, 4);
  u32(b, u);
}
void text(Bytes &b, const char *s) {
  u32(b, std::strlen(s));
  while (*s)
    b.push_back(*s++);
}
Bytes fixture() {
  Bytes b{'M', 'O', 'X', 'B'};
  u32(b, 0x41534b49);
  u32(b, 8);
  for (auto n : {1, 0, 0, 0, 1, 1, 4})
    u32(b, n);
  f32(b, 0);
  u32(b, 2);
  u32(b, 0);
  u32(b, 0);
  b.resize(108);
  text(b, "wolf.dds");
  const char *material = "MATERIAL: test";
  while (*material)
    b.push_back(*material++);
  b.push_back(0);
  for (int i = 0; i < 17; i++)
    f32(b, 1);
  b.push_back(1);
  text(b, "body");
  u32(b, 0);
  u32(b, 0);
  u32(b, 3);
  u32(b, 3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++)
      f32(b, i == j ? 1 : 0);
    f32(b, .75f);
    f32(b, .25f);
    f32(b, 0);
    u32(b, 0x100);
    for (int j = 0; j < 5; j++)
      f32(b, 0);
  }
  b.insert(b.end(), {0, 0, 1, 0, 2, 0});
  return b;
}
int main(int argc, char **argv) {
  pw153::Skin skin;
  std::string error;
  if (argc == 2) {
    std::ifstream f(argv[1], std::ios::binary);
    Bytes b((std::istreambuf_iterator<char>(f)), {});
    if (!pw153::load_ski(b.data(), b.size(), skin, error)) {
      std::cerr << error << '\n';
      return 1;
    }
    std::cout << "REAL SKI version=" << skin.version
              << " bones=" << skin.skeleton_bones
              << " meshes=" << skin.meshes.size() << '\n';
    return 0;
  }
  auto b = fixture();
  CHECK(pw153::load_ski(b.data(), b.size(), skin, error));
  CHECK(skin.meshes.size() == 1);
  CHECK(skin.textures[0] == "wolf.dds");
  CHECK(skin.meshes[0].vertices[0].weights[1] == .25f);
  CHECK(skin.meshes[0].vertices[0].bones[1] == 1);
  CHECK(skin.materials[0].two_sided);
  for (std::size_t i = 0; i < b.size(); i++) {
    CHECK(!pw153::load_ski(b.data(), i, skin, error));
    CHECK(skin.meshes.empty());
  }
  b.push_back(0);
  CHECK(!pw153::load_ski(b.data(), b.size(), skin, error));
  b.pop_back();
  b[b.size() - 1] = 255;
  CHECK(!pw153::load_ski(b.data(), b.size(), skin, error));
  b = fixture();
  b[20] = 1;
  CHECK(!pw153::load_ski(b.data(), b.size(), skin, error));
  std::cout << "PASS SKI weights, bone indices, materials, truncations, "
               "unsupported mesh and atomic failure\n";
}
