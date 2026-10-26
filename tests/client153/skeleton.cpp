// Tests for the client153 skeleton support batch: assist IDs, point/rect,
// color values, scalar math helpers, geometry bounds, coordinate hierarchy,
// controllers, IK stubs, counter and random generator. Original behavior is
// pinned; adaptations are documented in the source headers and evidence file.
#include "A3DController.h"
#include "A3DIKData.h"
#include "A3DIKSolver.h"
#include "A3DCoordinate.h"
#include "A3DFuncs.h"
#include "A3DGeometry.h"
#include "A3DMacros.h"
#include "A3DPI.h"
#include "A3DTypes.h"
#include "AAssist.h"
#include "ACounter.h"
#include "APoint.h"
#include "ARect.h"
#include "AMemory.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)

static bool nearf(float a, float b, float eps = 1e-5f) {
  return std::fabs(a - b) <= eps * std::fmax(1.0f, std::fmax(std::fabs(a), std::fabs(b)));
}

static bool near_mat(const A3DMATRIX4 &a, const A3DMATRIX4 &b, float eps = 1e-4f) {
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      if (!nearf(a.m[i][j], b.m[i][j], eps))
        return false;
  return true;
}

static void test_assist_ids() {
  // CRC-32/ISO-HDLC values cross-checked with an independent implementation.
  CHECK(a_MakeIDFromString("test") == 0xd87f7e0c);
  CHECK(a_MakeIDFromString("wolf.bon") == 0x94dcca54);
  CHECK(a_MakeIDFromString("ASKE") == 0x792d537c);
  // Lower-case variant folds ASCII letters before hashing.
  CHECK(a_MakeIDFromLowString("Wolf.BON") == a_MakeIDFromString("wolf.bon"));
  CHECK(a_MakeIDFromString("Wolf.BON") != a_MakeIDFromString("wolf.bon"));

  char out[64];
  CHECK(a_GetStringAfter("key=value", "key=", out) && std::string(out) == "value");
  CHECK(!a_GetStringAfter("nope", "key=", out));

  const DWORD page = a_GetCharCodePage();
  a_SetCharCodePage(936);
  CHECK(a_GetCharCodePage() == 936);
  a_SetCharCodePage(page);

  // Original LCG: multiplier 16807, modulus 2^31-1, fixed initial seed.
  CHECK(a_Random() == 2136458046);
  CHECK(a_Random() == 1523801282);
  const float fr = a_Random(0.25f, 0.75f);
  CHECK(fr >= 0.25f && fr <= 0.75f);
  int ir = a_Random(-3, 3);
  CHECK(ir >= -3 && ir <= 3);

  int x = 5;
  a_Clamp(x, 1, 3);
  CHECK(x == 3);
  a_Swap(x, ir);
}

static void test_point_rect() {
  APointI p(2, 3);
  p.Offset(1, -1);
  CHECK(p == APointI(3, 2));
  CHECK(APointI(1, 1) + APointI(2, 3) == APointI(3, 4));
  CHECK(APointI(5, 5) - APointI(2, 1) == APointI(3, 4));
  CHECK(-APointI(2, 3) == APointI(-2, -3));

  ARectI rc(1, 2, 5, 6);
  CHECK(rc.Width() == 4 && rc.Height() == 4);
  CHECK(rc.CenterPoint() == APointI(3, 4));
  CHECK(rc.PtInRect(1, 2) && !rc.PtInRect(5, 2) && !rc.PtInRect(3, 6));
  CHECK(rc + APointI(1, 1) == ARectI(2, 3, 6, 7));

  ARectI a(0, 0, 4, 4), b(2, 2, 6, 6);
  ARectI inter = a & b;
  CHECK(inter == ARectI(2, 2, 4, 4));
  ARectI uni = a | b;
  CHECK(uni == ARectI(0, 0, 6, 6));
  ARectI empty(0, 0, 0, 0);
  CHECK((a & empty).IsRectNull());
  CHECK((a | empty) == a);

  ARectI denorm(5, 6, 1, 2);
  denorm.Normalize();
  CHECK(denorm == ARectI(1, 2, 5, 6));
  denorm.Inflate(1, 1);
  denorm.Deflate(1, 1);
  CHECK(denorm == ARectI(1, 2, 5, 6));
  denorm.Clear();
  CHECK(denorm.IsRectNull());
}

static void test_color_values() {
  const A3DCOLOR packed = A3DCOLORRGBA(200, 100, 50, 255);
  A3DCOLORVALUE v(packed);
  CHECK(nearf(v.r, 200.0f / 255.0f) && nearf(v.g, 100.0f / 255.0f) &&
        nearf(v.b, 50.0f / 255.0f) && nearf(v.a, 1.0f));
  CHECK(A3DCOLOR_GETRED(packed) == 200 && A3DCOLOR_GETGREEN(packed) == 100 &&
        A3DCOLOR_GETBLUE(packed) == 50 && A3DCOLOR_GETALPHA(packed) == 255);
  const A3DCOLOR back = v.ToRGBAColor();
  CHECK(A3DCOLOR_GETRED(back) == 200 && A3DCOLOR_GETGREEN(back) == 100 &&
        A3DCOLOR_GETBLUE(back) == 50 && A3DCOLOR_GETALPHA(back) == 255);
  v.Clamp();
  v.Set(2.0f, -1.0f, 0.5f, 0.5f);
  v.Clamp();
  CHECK(nearf(v.r, 1.0f) && nearf(v.g, 0.0f));
  CHECK(a3d_ColorRGBAToColorValue(packed).ToRGBAColor() == packed);
}

static void test_scalar_helpers() {
  const A3DMATRIX4 id = a3d_IdentityMatrix();
  CHECK(near_mat(id, A3D::g_matIdentity));
  CHECK(A3D::g_vAxisX == A3DVECTOR3(1, 0, 0) && A3D::g_vAxisY == A3DVECTOR3(0, 1, 0) &&
        A3D::g_vAxisZ == A3DVECTOR3(0, 0, 1) && A3D::g_vOrigin == A3DVECTOR3(0, 0, 0));

  A3DMATRIX4 zero = a3d_ZeroMatrix();
  CHECK(nearf(zero.m[3][3], 0.0f));

  const A3DMATRIX4 scale = a3d_Scaling(2.0f, 3.0f, 4.0f);
  CHECK(nearf(scale._11, 2.0f) && nearf(scale._22, 3.0f) && nearf(scale._33, 4.0f));

  const A3DVECTOR3 v(1, 2, 3);
  const A3DVECTOR3 scaled = v * scale;
  CHECK(scaled == A3DVECTOR3(2, 6, 12));

  const float pi = 3.1415926535f;
  const A3DVECTOR3 rx = a3d_RotatePosAroundX(A3DVECTOR3(0, 1, 0), pi / 2);
  CHECK(nearf(rx.y, 0.0f, 1e-4f) && nearf(rx.z, 1.0f, 1e-4f));
  const A3DVECTOR3 ry = a3d_RotatePosAroundY(A3DVECTOR3(1, 0, 0), pi / 2);
  CHECK(nearf(ry.x, 0.0f, 1e-4f) && nearf(ry.z, -1.0f, 1e-4f));
  const A3DVECTOR3 rz = a3d_RotatePosAroundZ(A3DVECTOR3(0, 1, 0), pi / 2);
  CHECK(nearf(rz.x, -1.0f, 1e-4f) && nearf(rz.y, 0.0f, 1e-4f));

  const A3DMATRIX4 rot = a3d_RotateZ(pi / 2);
  const A3DVECTOR3 rotated = a3d_VectorMatrix3x3(A3DVECTOR3(1, 0, 0), rot);
  CHECK(nearf(rotated.x, 0.0f, 1e-4f) && nearf(rotated.y, 1.0f, 1e-4f));

  // Transpose and inverse of an orthonormal rotation.
  const A3DMATRIX4 transposed = a3d_Transpose(rot);
  const A3DMATRIX4 inverse = a3d_InverseTM(rot);
  CHECK(near_mat(transposed, inverse));

  // Decompose a composed matrix back into its scale, rotation and position.
  const A3DVECTOR3 pos(1, 2, 3);
  const A3DMATRIX4 composed = scale * rot;
  A3DMATRIX4 withPos = composed;
  withPos._41 = pos.x;
  withPos._42 = pos.y;
  withPos._43 = pos.z;
  A3DVECTOR3 outScale, outPos;
  A3DQUATERNION outOrient;
  a3d_DecomposeMatrix(withPos, outScale, outOrient, outPos);
  CHECK(nearf(outScale.x, 2.0f) && nearf(outScale.y, 3.0f) && nearf(outScale.z, 4.0f));
  CHECK(outPos == pos || (nearf(outPos.x, pos.x) && nearf(outPos.y, pos.y) && nearf(outPos.z, pos.z)));

  // Look-at and view matrix describe the same camera with the same layout.
  const A3DVECTOR3 from(0, 1, -5), to(0, 1, 0), up(0, 1, 0);
  const A3DMATRIX4 look = a3d_LookAtMatrix(from, to, up, 0.0f);
  const A3DMATRIX4 view = a3d_ViewMatrix(from, to - from, up, 0.0f);
  CHECK(near_mat(look, view));

  const A3DMATRIX3 id3 = a3d_IdentityMatrix3();
  CHECK(nearf(id3._11, 1.0f) && nearf(id3._12, 0.0f) && nearf(id3._33, 1.0f));

  CHECK(nearf(a3d_Magnitude(A3DVECTOR3(3, 4, 0)), 5.0f));
  const A3DVECTOR3 norm = a3d_Normalize(A3DVECTOR3(0, 0, 7));
  CHECK(nearf(norm.z, 1.0f));
  const A3DVECTOR3 zeroV = a3d_Normalize(A3DVECTOR3(0, 0, 0));
  CHECK(zeroV == A3DVECTOR3(0, 0, 0));
  CHECK(a3d_VecMin(A3DVECTOR3(1, 5, 3), A3DVECTOR3(4, 2, 3)) == A3DVECTOR3(1, 2, 3));
  CHECK(a3d_VecMax(A3DVECTOR3(1, 5, 3), A3DVECTOR3(4, 2, 3)) == A3DVECTOR3(4, 5, 3));
}

static void test_geometry_bounds() {
  A3DAABB aabb(A3DVECTOR3(-1, -1, -1), A3DVECTOR3(1, 1, 1));
  CHECK(aabb.Center == A3DVECTOR3(0, 0, 0));
  //	Original semantics: Extents is the half-extent (Maxs - Center).
  CHECK(aabb.Extents == A3DVECTOR3(1, 1, 1));
  CHECK(aabb.Mins == A3DVECTOR3(-1, -1, -1) && aabb.Maxs == A3DVECTOR3(1, 1, 1));
  CHECK(aabb.IsPointIn(A3DVECTOR3(0, 0, 0)));
  CHECK(!aabb.IsPointIn(A3DVECTOR3(2, 0, 0)));

  A3DAABB other(A3DVECTOR3(0, 0, 0), A3DVECTOR3(2, 2, 2));
  //	Default construction leaves members uninitialized (original semantics);
  //	start the merge from a defined box.
  A3DAABB merged = aabb;
  merged.Merge(other);
  CHECK(merged.Mins == A3DVECTOR3(-1, -1, -1) && merged.Maxs == A3DVECTOR3(2, 2, 2));

  A3DOBB obb;
  obb.Clear();
  CHECK(obb.Center == A3DVECTOR3(0, 0, 0) && obb.Extents == A3DVECTOR3(0, 0, 0));
  obb.Center = A3DVECTOR3(1, 0, 0);
  obb.XAxis = A3D::g_vAxisX;
  obb.YAxis = A3D::g_vAxisY;
  obb.ZAxis = A3D::g_vAxisZ;
  obb.Extents = A3DVECTOR3(1, 1, 1);
  obb.CompleteExtAxis();
  CHECK(obb.ExtX == A3DVECTOR3(1, 0, 0) && obb.ExtY == A3DVECTOR3(0, 1, 0) &&
        obb.ExtZ == A3DVECTOR3(0, 0, 1));
  A3DAABB fromObb;
  a3d_BuildAABB(&fromObb, obb);
  CHECK(fromObb.Mins == A3DVECTOR3(0, -1, -1) && fromObb.Maxs == A3DVECTOR3(2, 1, 1));

  A3DVECTOR3 mins, maxs;
  a3d_ClearAABB(mins, maxs);
  A3DVECTOR3 p1(1, 2, 3), p2(-1, 0, 5);
  a3d_AddVertexToAABB(mins, maxs, p1);
  a3d_AddVertexToAABB(mins, maxs, p2);
  CHECK(mins == A3DVECTOR3(-1, 0, 3) && maxs == A3DVECTOR3(1, 2, 5));
}

static void test_coordinate_hierarchy() {
  A3DCoordinate root;
  A3DCoordinate child;
  A3DCoordinate grandChild;

  CHECK(root.AddChildCoord(&child));
  CHECK(child.AddChildCoord(&grandChild));
  CHECK(!root.AddChildCoord(nullptr));

  // Absolute TM of the root propagates down the hierarchy.
  root.SetAbsoluteTM(a3d_Translate(10.0f, 0.0f, 0.0f));
  CHECK(nearf(child.GetAbsoluteTM()._41, 10.0f));
  CHECK(nearf(grandChild.GetAbsoluteTM()._41, 10.0f));

  // A relative translation on the child stacks on top of the parent.
  child.SetRelativeTM(a3d_Translate(1.0f, 0.0f, 0.0f));
  CHECK(nearf(child.GetAbsoluteTM()._41, 11.0f));
  CHECK(nearf(grandChild.GetAbsoluteTM()._41, 11.0f));

  // Position and direction helpers keep the absolute TM consistent.
  root.SetPosAndDir(A3DVECTOR3(0, 0, 0), A3D::g_vAxisZ, A3D::g_vAxisY);
  CHECK(root.GetPos() == A3DVECTOR3(0, 0, 0));
  CHECK(nearf(child.GetAbsoluteTM()._41, 1.0f));

  root.Move(A3DVECTOR3(5, 0, 0));
  CHECK(nearf(root.GetPos().x, 5.0f));
  CHECK(nearf(child.GetAbsoluteTM()._41, 6.0f));

  // Removing a child detaches it without touching the remaining tree.
  child.RemoveChildCoord(&grandChild);
  root.Move(A3DVECTOR3(1, 0, 0));
  CHECK(nearf(grandChild.GetAbsoluteTM()._41, 6.0f));
  CHECK(nearf(child.GetAbsoluteTM()._41, 7.0f));

  child.RemoveAllChildrenCoords();
  root.RemoveAllChildrenCoords();
  // Detached tree: later root moves no longer touch the child.
  root.Move(A3DVECTOR3(100, 0, 0));
  CHECK(nearf(child.GetAbsoluteTM()._41, 7.0f));
}

static void test_controller_and_ik() {
  A3DPosController pos;
  A3DRotController rot;
  CHECK(pos.Update(16) && rot.Update(16));

  // Original 1.5.3 IK chain methods are stubs that report success.
  A3DIKChain chain;
  CHECK(chain.Init(4));
  CHECK(chain.AddBone(0, nullptr));
  CHECK(chain.RemoveBone(nullptr));
  CHECK(chain.Release());

  A3DIKGoal goal;
  A3DIKSwivelConstraint swivel;
}

static void test_counter() {
  ACounter counter;
  CHECK(counter.Init());
  //	Platform clock counts elapsed microseconds since first use (ATime slice
  //	design), so only monotonicity is asserted, not absolute magnitude.
  const std::int64_t us1 = ACounter::GetMicroSecondNow();
  const DWORD ms1 = ACounter::GetMilliSecondNow();
  const auto spin = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - spin < std::chrono::milliseconds(3)) {
  }
  CHECK(ACounter::GetMicroSecondNow() > us1);
  CHECK(ACounter::GetMilliSecondNow() >= ms1);
  // The portable CPU-cycle API is explicitly pending; it must not pretend.
  CHECK(ACounter::GetCPUCycle() == 0 && ACounter::GetCPUFrequency() == 0);

  bool changed = true;
  counter.UpdateFPSCounter(&changed);
  CHECK(!changed);
  CHECK(counter.ResetFPSCounter());
  CHECK(counter.Release());
}

int main() {
  try {
    test_assist_ids();
    test_point_rect();
    test_color_values();
    test_scalar_helpers();
    test_geometry_bounds();
    test_coordinate_hierarchy();
    test_controller_and_ik();
    test_counter();
    std::cout << "client153 skeleton support: all tests passed\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << e.what() << "\n";
    return 1;
  }
}
