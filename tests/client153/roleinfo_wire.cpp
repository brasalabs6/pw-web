// Compiled separately against the client and server RoleInfo definitions.
#include "roleinfo"
#include <cstdio>

int main() {
  GNET::RoleInfo info;
  GNET::Marshal::OctetsStream wire;
  wire << info;
  const auto *bytes = static_cast<const unsigned char *>(wire.begin());
  for (std::size_t i = 0; i < wire.size(); ++i) std::printf("%02x", bytes[i]);
  std::puts("");
}
