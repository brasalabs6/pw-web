#include "winsock2.h"
#include "gnnetio.h"
#include <stdexcept>
#include <string>
#include <sys/socket.h>

namespace GNET {
Security::Map& Security::GetMap() { static Map map; return map; }
NullSecurity null_security(NULLSECURITY);
PollIO::IOMap PollIO::iomap;
PollIO::FDSet PollIO::fdset;
PollIO::IOMap PollIO::ionew;
Thread::Mutex PollIO::locker_ionew;
Thread::Mutex PollIO::locker_poll;
fd_set PollIO::rfds, PollIO::wfds, PollIO::efds;
SOCKET PollControl::writer;
bool PollIO::wakeup_scope = false;
bool PollIO::wakeup_flag = false;
char errormsg[ERRSIZE] = {};
}

class TestSession : public GNET::NetSession {
public:
  explicit TestSession(std::string &received) : received_(received) {}
  void QueueReply() {
    GNET::Octets data("OK", 2);
    if (!Output(data)) throw std::runtime_error("output buffer full");
  }
  std::string Identification() const override { return "test"; }
  void OnRecv() override {
    GNET::Octets &data = Input();
    received_.assign(static_cast<const char *>(data.begin()), data.size());
    data.clear();
  }
  bool NoMoreData() const override { return true; }
  void OnSend() override {}
  GNET::NetSession *Clone() const override { return nullptr; }
private:
  std::string &received_;
};

int main() {
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0) throw std::runtime_error("socketpair");
  std::string received;
  auto *session = new TestSession(received);
  new GNET::StreamIO(sockets[1], session);
  if (send(sockets[0], "PW", 2, 0) != 2) throw std::runtime_error("send");
  GNET::PollIO::Poll(10);
  GNET::PollIO::Poll(0);
  if (received != "PW") throw std::runtime_error("session did not receive bytes");
  session->QueueReply();
  GNET::PollIO::Poll(10);
  char reply[2];
  if (recv(sockets[0], reply, 2, 0) != 2 || std::string(reply, 2) != "OK")
    throw std::runtime_error("session did not send bytes");
  session->Close();
  GNET::PollIO::Poll(0);
  closesocket(sockets[0]);
}
