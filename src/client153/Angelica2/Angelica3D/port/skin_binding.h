#pragma once
#include "A3DMatrix.h"
#include "ski.h"
#include <vector>
class A3DSkeleton;
namespace pw153 {
// Reproduce A3DSkin::BindSkeleton's mapping, including its virtual bone.
bool bind_skin(const Skin &skin, A3DSkeleton &skeleton,
               std::vector<int> &mapping);
// Apply A3DSkinMesh::GetBlendedVertices weights to supplied blend matrices.
bool blend_skin_vertex(const SkinVertex &vertex,
                       const std::vector<int> &mapping,
                       const std::vector<A3DMATRIX4> &matrices,
                       A3DVECTOR3 &position, A3DVECTOR3 &normal);
} // namespace pw153
