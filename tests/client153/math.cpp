#include "A3DMatrix.h"
#include "A3DQuaternion.h"
#include "A3DVertex.h"
#include "d3dx8.h"
#include <cstddef>
#include <cmath>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
bool near(float a, float b) { return std::abs(a - b) < .0001f; }
int main() {
  D3DXMATRIX perspective;
  CHECK(D3DXMatrixPerspectiveFovLH(&perspective, 1.57079632679f, 2.f, 1.f, 11.f) == &perspective);
  CHECK(near(perspective._11, .5f) && near(perspective._22, 1.f));
  CHECK(near(perspective._33, 1.1f) && near(perspective._34, 1.f));
  CHECK(near(perspective._43, -1.1f) && near(perspective._44, 0.f));
  CHECK(near((A3DVECTOR3(0, 0, 1) * perspective).z, 0.f));
  CHECK(near((A3DVECTOR3(0, 0, 11) * perspective).z, 1.f));
  static_assert(sizeof(A3DBVERTEX3) == 48);
  static_assert(offsetof(A3DBVERTEX3, dwMatIndices) == 24);
  static_assert(offsetof(A3DBVERTEX3, vNormal) == 28);
  static_assert(offsetof(A3DBVERTEX3, tu) == 40);
  static_assert(sizeof(A3DVECTOR3) == 12);
  static_assert(sizeof(A3DMATRIX4) == 64);
  A3DMATRIX4 translate;
  translate.Translate(3, 4, 5);
  auto v = A3DVECTOR3(1, 2, 3) * translate;
  CHECK(near(v.x, 4) && near(v.y, 6) && near(v.z, 8));
  A3DMATRIX4 scale;
  scale.Scale(2, 3, 4);
  v = A3DVECTOR3(1, 2, 3) * (scale * translate);
  CHECK(near(v.x, 5) && near(v.y, 10) && near(v.z, 17));
  auto product = (scale * translate) * (scale * translate).GetInverseTM();
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      CHECK(near(product.m[r][c], r == c ? 1.f : 0.f));
  A3DMATRIX4 rotate;
  rotate.RotateZ(.7f);
  CHECK(near(rotate.Determinant(), 1));
  A3DMATRIX4 axis;
  axis.RotateAxis(A3DVECTOR3(0, 2, 0), .7f);
  rotate.RotateY(.7f);
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      CHECK(near(axis.m[r][c], rotate.m[r][c]));
  CHECK(near(scale.Determinant(), 24));
  A3DMATRIX4 project(A3DMATRIX4::IDENTITY);
  project._44 = 2;
  v = A3DVECTOR3(2, 4, 6) * project;
  CHECK(near(v.x, 1) && near(v.y, 2) && near(v.z, 3));
  A3DQUATERNION q(A3DVECTOR3(0, 0, 1), 1.57079632679f);
  A3DMATRIX4 qm;
  q.ConvertToMatrix(qm);
  v = A3DVECTOR3(1, 0, 0) * qm;
  CHECK(near(v.x, 0) && near(v.y, 1));
  A3DQUATERNION back(qm);
  CHECK(near(std::abs(DotProduct(q, back)), 1));
  A3DQUATERNION identity(1, 0, 0, 0);
  auto middle = SLERPQuad(identity, q, .5f);
  middle.ConvertToMatrix(qm);
  v = A3DVECTOR3(1, 0, 0) * qm;
  CHECK(near(v.x, .70710678f) && near(v.y, .70710678f));
  A3DQUATERNION euler(.3f, -.4f, .5f);
  float x = 99, y = 99, z = 99;
  euler.ConvertToEulerAngle(x, y, z);
  CHECK(near(x, .3f) && near(y, -.4f) && near(z, .5f));
  std::cout
      << "PASS original math ABI, row-vector composition, affine inverse, "
         "determinant, axis normalization and homogeneous coordinates\n";
}
