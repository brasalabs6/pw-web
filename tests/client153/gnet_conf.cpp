#include "winsock2.h"
#include "gnconf.h"
#include <cstdio>
#include <stdexcept>
#include <unistd.h>

Thread::RWLock GNET::Conf::locker;
GNET::Conf GNET::Conf::instance;

int main() {
  char path[] = "/tmp/pw153-conf-XXXXXX";
  int fd = mkstemp(path);
  if (fd < 0) throw std::runtime_error("mkstemp");
  FILE *file = fdopen(fd, "w");
  if (!file) { close(fd); unlink(path); throw std::runtime_error("fdopen"); }
  std::fputs("[Network]\nHost = example.test\n", file);
  std::fclose(file);
  auto *config = GNET::Conf::GetInstance(path);
  auto value = config->find("network", "host");
  unlink(path);
  if (value != "example.test") throw std::runtime_error("case-insensitive config lookup");
}
