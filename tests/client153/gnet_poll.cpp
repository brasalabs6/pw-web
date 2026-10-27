#include "winsock2.h"
#include "gnpollio.h"
#include <stdexcept>
#include <sys/socket.h>

namespace GNET {
PollIO::IOMap PollIO::iomap;
PollIO::FDSet PollIO::fdset;
PollIO::IOMap PollIO::ionew;
Thread::Mutex PollIO::locker_ionew;
Thread::Mutex PollIO::locker_poll;
fd_set PollIO::rfds, PollIO::wfds, PollIO::efds;
SOCKET PollControl::writer;
bool PollIO::wakeup_scope = false;
bool PollIO::wakeup_flag = false;
}

class Reader : public GNET::PollIO {
public:
  explicit Reader(int socket) : PollIO(socket) {}
  bool called = false;
private:
  int UpdateEvent() override { return IO_POLLIN; }
  void PollIn() override { called = true; }
};

int main() {
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) throw std::runtime_error("socketpair");
  Reader reader(sockets[1]);
  if (send(sockets[0], "x", 1, 0) != 1) throw std::runtime_error("send");
  GNET::PollIO::Poll(10);
  if (!reader.called) throw std::runtime_error("read event was not delivered");
  closesocket(sockets[0]);
  closesocket(sockets[1]);
}
