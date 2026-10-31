/* Adapted from A3DSkin::BindSkeleton and A3DSkinMesh::GetBlendedVertices,
 * Angelica client revision e3d6d0075d0a745c9f9859f2cb25b01d71dec045.
 * Original Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#include "skin_binding.h"
#include "A3DBone.h"
#include "A3DFuncs.h"
#include "A3DSkeleton.h"
namespace pw153 {
bool bind_skin(const Skin &skin, A3DSkeleton &skeleton,
               std::vector<int> &mapping) {
  mapping.clear();
  const int count = skeleton.GetBoneNum();
  if (skin.version >= 9 &&
      skin.bone_names.size() > static_cast<std::size_t>(count))
    return false;
  if (skin.version < 9) {
    if (skin.skeleton_bones > static_cast<unsigned>(count))
      return false;
    for (unsigned i = 0; i < skin.skeleton_bones; ++i)
      mapping.push_back(i);
  } else {
    for (const auto &name : skin.bone_names) {
      int index = -1;
      if (!skeleton.GetBone(name.c_str(), &index)) {
        mapping.clear();
        return false;
      }
      mapping.push_back(index);
    }
  }
  mapping.push_back(count);
  return true;
}
bool blend_skin_vertex(const SkinVertex &v, const std::vector<int> &map,
                       const std::vector<A3DMATRIX4> &matrices,
                       A3DVECTOR3 &position, A3DVECTOR3 &normal) {
  A3DVECTOR3 p(0, 0, 0), n(0, 0, 0);
  const A3DVECTOR3 source(v.position[0], v.position[1], v.position[2]);
  const A3DVECTOR3 sourceNormal(v.normal[0], v.normal[1], v.normal[2]);
  auto apply = [&](int slot, float weight) {
    const auto index = v.bones[slot];
    if (index >= map.size() || map[index] < 0 ||
        static_cast<std::size_t>(map[index]) >= matrices.size())
      return false;
    const auto &matrix = matrices[map[index]];
    p += (matrix * source) * weight;
    n += a3d_VectorMatrix3x3(sourceNormal, matrix) * weight;
    return true;
  };
  float total = 0;
  int i = 0;
  for (; i < 3; ++i) {
    if (!v.weights[i])
      break;
    if (!apply(i, v.weights[i]))
      return false;
    total += v.weights[i];
  }
  if (i == 3 && 1.0f - total > 0.0f && !apply(3, 1.0f - total))
    return false;
  n.Normalize();
  position = p;
  normal = n;
  return true;
}
} // namespace pw153
