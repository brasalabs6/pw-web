#pragma once
#include "A3DVector.h"
#include "ski.h"
#include <memory>
#include <string>
#include <vector>
namespace pw153 {
class AnimatedSkin {
public:
  AnimatedSkin();
  ~AnimatedSkin();
  AnimatedSkin(const AnimatedSkin &) = delete;
  AnimatedSkin &operator=(const AnimatedSkin &) = delete;
  bool load(const char *bon, const char *ski);
  bool update(int time_ms);
  void close();
  std::size_t vertex_count() const;
  const Skin &skin() const;
  const std::vector<std::vector<A3DVECTOR3>> &positions() const;
  const std::string &error() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace pw153
