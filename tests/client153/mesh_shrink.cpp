#include "Triangle.h"
#include <AArray.h>
#include <A3DVector.h>
#include <A3DMatrix.h>
#define private public
#include "Mesh.h"
#undef private
#include "FWTextMesh.h"

int main() {
  CMesh mesh;
  A3DVECTOR3 points[] = {{0,0,0}, {1,0,0}, {0,1,0}, {0,0,0}};
  for (auto &point : points) mesh.m_aryVertices.Add(point);
  CTriangle triangle(3, 1, 2);
  mesh.m_aryTriangles.Add(triangle);
  mesh.ShrinkVertices(mesh.m_aryTriangles, mesh.m_aryVertices);
  if (mesh.m_aryVertices.GetSize() != 3 || mesh.m_aryTriangles[0].Index0 != 0) return 1;
  if (!(mesh.m_aryVertices[0] == points[0]) || !(mesh.m_aryVertices[1] == points[1]) || !(mesh.m_aryVertices[2] == points[2])) return 2;
  MeshParam param{};
  if (FillMeshParam(&param, &mesh) != 0 || param.nVerticesCount != 3 || param.nIndicesCount != 3) return 3;
  if (!(param.pVertices[1] == points[1]) || param.pIndices[0] != 0 || param.pIndices[1] != 1 || param.pIndices[2] != 2) return 4;
  delete[] param.pVertices;
  delete[] param.pIndices;
  return 0;
}
