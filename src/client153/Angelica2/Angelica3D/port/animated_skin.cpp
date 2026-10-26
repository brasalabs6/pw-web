#include "animated_skin.h"
#include "A3DBone.h"
#include "A3DConfig.h"
#include "A3DSkeleton.h"
#include "A3DSkinModelAct.h"
#include "A3DTrackMan.h"
#include "skin_binding.h"
#include <fstream>
#include <iterator>
namespace pw153 {
namespace {
class Skeleton : public A3DSkeleton {
public:
  using A3DSkeleton::Load;
  using A3DSkeleton::Release;
};
class Action : public A3DSkinModelActionCore {
public:
  using A3DSkinModelActionCore::BindTrackSet;
};
} // namespace
struct AnimatedSkin::Impl {
  Skeleton skeleton;
  Action action;
  Skin skin;
  std::vector<int> mapping;
  std::vector<std::vector<A3DVECTOR3>> positions;
  std::string error;
  bool loaded = false, has_skeleton = false;
};
AnimatedSkin::AnimatedSkin() : impl(new Impl) {}
AnimatedSkin::~AnimatedSkin() {
  if (impl->has_skeleton)
    impl->skeleton.Release();
}
void AnimatedSkin::close() {
  if (impl->has_skeleton)
    impl->skeleton.Release();
  impl.reset(new Impl);
}
bool AnimatedSkin::load(const char *bon, const char *ski) {
  close();
  if (!bon || !ski || !g_pA3DConfig) {
    impl->error = "missing model path or engine configuration";
    return false;
  }
  std::ifstream input(ski, std::ios::binary);
  if (!input) {
    impl->error = "skin file unavailable";
    return false;
  }
  input.seekg(0, std::ios::end);
  const auto size = input.tellg();
  if (size <= 0 || size > 64 * 1024 * 1024) {
    impl->error = "invalid skin file size";
    return false;
  }
  input.seekg(0, std::ios::beg);
  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
  if (!input.read(reinterpret_cast<char *>(bytes.data()), size)) {
    impl->error = "skin file read failed";
    return false;
  }
  if (!load_ski(bytes.data(), bytes.size(), impl->skin, impl->error))
    return false;
  impl->has_skeleton = true;
  if (!impl->skeleton.Load(bon)) {
    impl->error = "skeleton load failed";
    return false;
  }
  if (!bind_skin(impl->skin, impl->skeleton, impl->mapping)) {
    impl->error = "skin does not match skeleton";
    return false;
  }
  impl->action.SetTrackSetFileName(impl->skeleton.GetFileName(), false);
  if (!impl->action.BindTrackSet()) {
    impl->error = "skeleton tracks unavailable";
    return false;
  }
  impl->positions.resize(impl->skin.meshes.size());
  impl->loaded = true;
  return true;
}
bool AnimatedSkin::update(int time_ms) {
  if (!impl->loaded || time_ms < 0) {
    impl->error = "invalid animation state or time";
    return false;
  }
  auto *tracks = impl->action.GetTrackSet();
  for (int i = 0; i < impl->skeleton.GetBoneNum(); ++i) {
    auto *bone = impl->skeleton.GetBone(i);
    int joint = bone->GetFirstJoint();
    if (!tracks->GetBoneTrack(joint))
      continue;
    A3DVECTOR3 p;
    A3DQUATERNION q;
    if (!impl->action.GetBoneStateOnTrack(joint, time_ms, p, q)) {
      impl->error = "track sampling failed";
      return false;
    }
    bone->AddBlendState(q, p, 1, A3DBone::BM_EXCLUSIVE);
  }
  if (!impl->skeleton.Update(0)) {
    impl->error = "skeleton update failed";
    return false;
  }
  std::vector<A3DMATRIX4> matrices;
  for (int i = 0; i < impl->skeleton.GetBoneNum(); ++i) {
    auto *b = impl->skeleton.GetBone(i);
    matrices.push_back(b->GetBoneInitTM() * b->GetAbsoluteTM());
  }
  matrices.push_back(A3DMATRIX4::IDENTITY);
  std::vector<std::vector<A3DVECTOR3>> next(impl->skin.meshes.size());
  for (std::size_t m = 0; m < impl->skin.meshes.size(); ++m)
    for (const auto &v : impl->skin.meshes[m].vertices) {
      A3DVECTOR3 p, n;
      if (!blend_skin_vertex(v, impl->mapping, matrices, p, n)) {
        impl->error = "invalid skin bone index";
        return false;
      }
      next[m].push_back(p);
    }
  impl->positions = std::move(next);
  impl->error.clear();
  return true;
}
std::size_t AnimatedSkin::vertex_count() const {
  std::size_t n = 0;
  for (const auto &m : impl->skin.meshes)
    n += m.vertices.size();
  return n;
}
const Skin &AnimatedSkin::skin() const { return impl->skin; }
const std::vector<std::vector<A3DVECTOR3>> &AnimatedSkin::positions() const {
  return impl->positions;
}
const std::string &AnimatedSkin::error() const { return impl->error; }
} // namespace pw153
