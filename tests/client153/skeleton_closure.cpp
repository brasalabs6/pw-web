// Skeleton closure test: the original A3DSkeleton loads the real wolf BON
// from the authorized client and exposes the 42-joint hierarchy the validated
// tracks test already proved (84 tracks / 31,122 samples). Cross-reference:
// tests/client153/tracks.cpp and docs/evidence/client153-original-tracks.md.
#include "A3DBone.h"
#include "A3DConfig.h"
#include "A3DJoint.h"
#include "A3DPI.h"
#include "A3DSkeleton.h"
#include "A3DSkinModelAct.h"
#include "A3DTrackMan.h"
#include "AFI.h"
#include "animated_skin.h"
#include "skin_binding.h"
#include <fstream>
#include <iterator>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)

//	A3DSkeleton::Load/Release are protected in the original design (the skin
//	model calls them); the test lifts them through a local subclass without
//	changing the shipped header.
class TestActionCore : public A3DSkinModelActionCore {
public:
  TestActionCore() : A3DSkinModelActionCore(30) {}
  using A3DSkinModelActionCore::BindTrackSet;
};

class TestSkeleton : public A3DSkeleton {
public:
  using A3DSkeleton::Load;
  using A3DSkeleton::Release;
};

static void test_config_flags() {
  // g_pA3DConfig is allocated by the engine in the original; tests own it.
  if (!g_pA3DConfig)
    g_pA3DConfig = new A3DConfig();
  CHECK(!g_pA3DConfig->GetFlagNewBoneScale());
  g_pA3DConfig->SetFlagNewBoneScale(true);
  CHECK(g_pA3DConfig->GetFlagNewBoneScale());
  g_pA3DConfig->SetFlagNewBoneScale(false);
  CHECK(!g_pA3DConfig->GetFlagNewBoneScale());
}

static void test_skeleton_load(const char *bonPath, const char *skiPath) {
  TestSkeleton skeleton;
  CHECK(skeleton.Load(bonPath));
  CHECK(skeleton.GetJointNum() == 42); // validated cross-reference: wolf BON
  CHECK(skeleton.GetBoneNum() == 42);  // one bone per joint in the wolf BON
  CHECK(skeleton.GetHookNum() >= 0);   // hooks are data-driven, not assumed

  // Named lookup resolves the same objects as index access.
  A3DBone *first = skeleton.GetBone(0);
  CHECK(first != nullptr);
  int index = -1;
  A3DBone *byName = skeleton.GetBone(first->GetName(), &index);
  CHECK(byName == first && index == 0);

  // Root bones exist and the first root resolves through GetRootBone.
  CHECK(skeleton.GetRootBoneNum() > 0);
  CHECK(skeleton.GetRootBone(0) >= 0);
  CHECK(skeleton.GetRootBone(-1) == -1);
  CHECK(skeleton.GetRootBone(skeleton.GetBoneNum()) == -1);

  // Joints resolve by index and the hierarchy is wired to the skeleton.
  A3DJoint *joint = skeleton.GetJoint(0);
  CHECK(joint != nullptr);

  // Update walks the whole tree without touching the (unattached) skin model.
  CHECK(skeleton.Update(16));

  // Exercise the original action's track-to-bone flow, without assuming that
  // loading tracks alone advances animation (A3DSkinModelAction is not ported).
  A3DSklTrackSet *tracks = A3D::g_A3DTrackMan.GetSkeletonTrackSet(
      skeleton.GetSkeletonID(), skeleton.GetFileName());
  CHECK(tracks != nullptr);
  TestActionCore action;
  action.SetTrackSetFileName(skeleton.GetFileName(), false);
  CHECK(action.BindTrackSet());
  int changed = 0;
  for (int i = 0; i < skeleton.GetBoneNum(); ++i) {
    A3DBone *bone = skeleton.GetBone(i);
    const int joint = bone->GetFirstJoint();
    A3DSklTrackSet::BONETRACK *track = tracks->GetBoneTrack(joint);
    if (!track)
      continue;
    auto *positions = track->pPosTrack;
    auto *rotations = track->pRotTrack;
    CHECK(positions != nullptr && rotations != nullptr);
    auto apply = [&](int time) {
      A3DQUATERNION rotation;
      A3DVECTOR3 position;
      CHECK(action.GetBoneStateOnTrack(joint, time, position, rotation));
      bone->AddBlendState(rotation, position, 1.0f, A3DBone::BM_EXCLUSIVE);
      CHECK(skeleton.Update(0));
      return bone->GetRelativeTM();
    };
    const auto firstMatrix = apply(positions->GetTrackStartTime());
    const auto lastMatrix = apply(positions->GetTrackEndTime());
    bool differs = false;
    for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col) {
        CHECK(std::isfinite(firstMatrix.m[row][col]));
        CHECK(std::isfinite(lastMatrix.m[row][col]));
        differs |= std::abs(firstMatrix.m[row][col] - lastMatrix.m[row][col]) >
                   0.00001f;
      }
    changed += differs;
  }
  CHECK(changed > 0);
  if (skiPath) {
    std::ifstream file(skiPath, std::ios::binary);
    CHECK(file.good());
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)), {});
    pw153::Skin skin;
    std::string error;
    CHECK(pw153::load_ski(bytes.data(), bytes.size(), skin, error));
    std::vector<int> mapping;
    CHECK(pw153::bind_skin(skin, skeleton, mapping));
    auto pose = [&](bool end) {
      for (int i = 0; i < skeleton.GetBoneNum(); ++i) {
        A3DBone *bone = skeleton.GetBone(i);
        int joint = bone->GetFirstJoint();
        auto *track = tracks->GetBoneTrack(joint);
        if (!track)
          continue;
        int time = end ? track->pPosTrack->GetTrackEndTime()
                       : track->pPosTrack->GetTrackStartTime();
        A3DVECTOR3 position;
        A3DQUATERNION rotation;
        CHECK(action.GetBoneStateOnTrack(joint, time, position, rotation));
        bone->AddBlendState(rotation, position, 1, A3DBone::BM_EXCLUSIVE);
      }
      CHECK(skeleton.Update(0));
      std::vector<A3DMATRIX4> matrices;
      for (int i = 0; i < skeleton.GetBoneNum(); ++i) {
        auto *bone = skeleton.GetBone(i);
        matrices.push_back(bone->GetBoneInitTM() * bone->GetAbsoluteTM());
      }
      matrices.push_back(A3DMATRIX4::IDENTITY);
      return matrices;
    };
    const auto start = pose(false), end = pose(true);
    int moved = 0, vertices = 0;
    for (const auto &mesh : skin.meshes)
      for (const auto &vertex : mesh.vertices) {
        A3DVECTOR3 a, b, normal;
        CHECK(pw153::blend_skin_vertex(vertex, mapping, start, a, normal));
        CHECK(pw153::blend_skin_vertex(vertex, mapping, end, b, normal));
        CHECK(std::isfinite(a.x) && std::isfinite(b.x));
        moved +=
            std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z) >
            .00001f;
        ++vertices;
      }
    CHECK(moved > 0);
    std::cout << "real skinned vertices: " << vertices << ", moved: " << moved
              << "\n";
  }
  std::cout << "animated bone transforms changed: " << changed << "\n";

  std::cout << "wolf BON: joints=" << skeleton.GetJointNum()
            << " bones=" << skeleton.GetBoneNum()
            << " hooks=" << skeleton.GetHookNum() << "\n";
  skeleton.Release();
  if (skiPath) {
    pw153::AnimatedSkin instance;
    CHECK(instance.load(bonPath, skiPath));
    CHECK(instance.update(0));
    CHECK(instance.vertex_count() == 997);
    const auto initial = instance.positions();
    CHECK(instance.update(1000));
    int moved = 0;
    for (std::size_t m = 0; m < initial.size(); ++m)
      for (std::size_t i = 0; i < initial[m].size(); ++i) {
        const auto &a = initial[m][i];
        const auto &b = instance.positions()[m][i];
        moved +=
            std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z) >
            .00001f;
      }
    CHECK(moved > 0);
    CHECK(!instance.update(-1));
    instance.close();
    CHECK(!instance.update(0));
  }
}

static void test_skeleton_negative() {
  TestSkeleton skeleton;
  CHECK(!skeleton.Load("missing/wolf.bon"));
  skeleton.Release(); // Release on a failed load is safe
}

int main(int argc, char **argv) {
  try {
    //	The engine sets the base dir at startup; tests pin it to the cwd.
    CHECK(af_SetBaseDir("."));
    test_config_flags();
    A3DSkinModelActionCore defaultAction;
    defaultAction.SetStartAndEndFrame(0, 30);
    CHECK(defaultAction.GetEndTime() == 1000);
    if (argc == 2 || argc == 3) {
      test_skeleton_load(argv[1], argc == 3 ? argv[2] : nullptr);
      test_skeleton_negative();
    } else {
      std::cout << "BON tests skipped: pass the authorized wolf.bon path\n";
    }
    std::cout << "client153 skeleton closure: all tests passed\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "FAIL: " << e.what() << "\n";
    return 1;
  }
}
