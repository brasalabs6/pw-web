#include "mox.h"
#include <stdexcept>
#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition))                                                          \
      throw std::runtime_error("check failed: " #condition);                   \
  } while (false)
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
using namespace pw153;
void u32(std::vector<unsigned char> &b, unsigned v) {
  for (int i = 0; i < 4; ++i)
    b.push_back((v >> (8 * i)) & 255);
}
void f32(std::vector<unsigned char> &b, float f) {
  unsigned u;
  std::memcpy(&u, &f, 4);
  u32(b, u);
}
void str(std::vector<unsigned char> &b, const char *s) {
  while (*s)
    b.push_back(*s++);
  b.push_back(0);
}
std::vector<unsigned char> fixture() {
  std::vector<unsigned char> b{'M', 'O', 'X', 'B'};
  str(b, "FRAME: fixture");
  u32(b, 1);
  for (int i = 0; i < 16; i++)
    f32(b, i % 5 == 0 ? 1.f : 0.f);
  u32(b, 0);
  u32(b, 1);
  str(b, "MESH: triangle");
  u32(b, 3);
  u32(b, 3);
  u32(b, 1);
  b.push_back(0);
  b.push_back(0);
  u32(b, 40);
  b.resize(b.size() + 40);
  b.insert(b.end(), {0, 0, 1, 0, 2, 0});
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 8; j++)
      f32(b, (j == 0) ? float(i) : 0.f);
  str(b, "TEXTURE: test.dds");
  str(b, "MATERIAL: test");
  for (int i = 0; i < 17; i++)
    f32(b, 1);
  b.push_back(0);
  u32(b, 0);
  return b;
}
int main(int argc, char **argv) {
  if (argc == 2) {
    std::ifstream input(argv[1], std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),
                                     {});
    Model real;
    std::string error;
    if (!load_mox(bytes.data(), bytes.size(), real, error)) {
      std::cerr << error << "\n";
      return 1;
    }
    std::cout << "REAL MOX frames=" << real.frames.size()
              << " meshes=" << real.meshes.size()
              << " vertices=" << real.meshes.at(0).vertices.at(0).size()
              << " indices=" << real.meshes.at(0).indices.size() << "\n";
    return 0;
  }
  auto data = fixture();
  Model model;
  std::string error;
  CHECK(load_mox(data.data(), data.size(), model, error));
  CHECK(model.meshes.size() == 1);
  CHECK(model.meshes[0].vertices[0].size() == 3);
  CHECK(model.meshes[0].indices[2] == 2);
  CHECK(model.meshes[0].texture == "test.dds");
  for (std::size_t i = 0; i < data.size(); i++) {
    Model out;
    CHECK(!load_mox(data.data(), i, out, error));
    CHECK(out.meshes.empty());
  }
  data[0] = 'X';
  CHECK(!load_mox(data.data(), data.size(), model, error));
  CHECK(model.meshes.empty());
  std::cout << "PASS MOX original-layout, geometry, texture, truncations and "
               "failure atomicity\n";
}
