#include "winsock2.h"
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
int main() {
  int fds[2];
  CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
  CHECK(send(fds[0], "PW", 2, 0) == 2);
  char buffer[3]{};
  CHECK(recv(fds[1], buffer, 2, 0) == 2);
  CHECK(std::strcmp(buffer, "PW") == 0);
  int socket_type = 0;
  int option_length = sizeof(socket_type);
  CHECK(getsockopt(fds[1], SOL_SOCKET, SO_TYPE,
                   reinterpret_cast<char *>(&socket_type), &option_length) == 0);
  CHECK(socket_type == SOCK_STREAM);
  unsigned long nonblocking = 1;
  CHECK(ioctlsocket(fds[1], FIONBIO, &nonblocking) == 0);
  CHECK(recv(fds[1], buffer, 1, 0) == SOCKET_ERROR);
  CHECK(WSAGetLastError() == WSAEWOULDBLOCK);
  CHECK(closesocket(fds[0]) == 0 && closesocket(fds[1]) == 0);
}
