#include "skin_binding.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
  pw153::SkinVertex vertex;
  vertex.position = {1, 2, 3};
  vertex.normal = {0, 1, 0};
  vertex.weights = {.75f, .25f, 0};
  vertex.bones = {0, 1, 0, 0};
  std::vector<A3DMATRIX4> matrices(2, A3DMATRIX4::IDENTITY);
  matrices[1].Translate(4, 0, 0);
  A3DVECTOR3 pos, normal;
  CHECK(pw153::blend_skin_vertex(vertex, {0, 1}, matrices, pos, normal));
  CHECK(std::abs(pos.x - 2) < .0001f && std::abs(pos.y - 2) < .0001f);
  CHECK(std::abs(normal.y - 1) < .0001f);
  vertex.bones[1] = 5;
  CHECK(!pw153::blend_skin_vertex(vertex, {0, 1}, matrices, pos, normal));
  std::cout << "PASS Angelica skin weight blending and invalid bone index\n";
}
