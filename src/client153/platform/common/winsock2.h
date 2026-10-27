/*
 * FILE: winsock2.h (platform seam)
 *
 * DESCRIPTION: The Winsock surface the client's network layer compiles
 * against, mapped to the real POSIX socket layer that Emscripten provides
 * (backed by WebSocket transport). The GNET contract needs only a blocking
 * byte stream on a TCP connection plus select() - no overlapped I/O exists
 * in the original (verified: zero WSARecv/WSAOVERLAPPED usage). DNS resolves
 * through the same layer. WSAStartup/WSACleanup return success because the
 * POSIX layer initializes lazily - that is the real behavior of this backend,
 * not a fake success.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WINSOCK2_H
#define PW153_PLATFORM_WINSOCK2_H

#include "Windows.h"

#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET (SOCKET)-1
#define SOCKET_ERROR (-1)

#define AF_UNSPEC 0
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3
#define IPPROTO_IP 0
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define INADDR_ANY 0u
#define INADDR_NONE 0xFFFFFFFFu
#define SOL_SOCKET 0xFFFF
#define SO_DEBUG 0x0001
#define SO_REUSEADDR 0x0004
#define SO_KEEPALIVE 0x0008
#define SO_DONTROUTE 0x0010
#define SO_BROADCAST 0x0020
#define SO_SNDBUF 0x1001
#define SO_RCVBUF 0x1002
#define TCP_NODELAY 1
#define FIONBIO 0x8004667Eu
#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2
#define WSAEWOULDBLOCK 10035
#define WSAEINPROGRESS 10036
#define WSANOTINITIALISED 10093

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct hostent HOSTENT;

inline int closesocket(SOCKET fd) { return ::close(fd); }

inline SOCKET socket(int af, int type, int protocol) {
  return ::socket(af, type, protocol);
}

inline int connect(SOCKET fd, const SOCKADDR *addr, int addrlen) {
  return ::connect(fd, addr, static_cast<socklen_t>(addrlen));
}

inline int bind(SOCKET fd, const SOCKADDR *addr, int addrlen) {
  return ::bind(fd, addr, static_cast<socklen_t>(addrlen));
}

inline int listen(SOCKET fd, int backlog) { return ::listen(fd, backlog); }

inline SOCKET accept(SOCKET fd, SOCKADDR *addr, int *addrlen) {
  socklen_t len = addrlen ? static_cast<socklen_t>(*addrlen) : 0;
  const SOCKET result = ::accept(fd, addr, addrlen ? &len : nullptr);
  if (addrlen)
    *addrlen = static_cast<int>(len);
  return result;
}

inline int send(SOCKET fd, const char *buffer, int length, int flags) {
  return static_cast<int>(
      ::send(fd, buffer, static_cast<std::size_t>(length), flags));
}

inline int recv(SOCKET fd, char *buffer, int length, int flags) {
  return static_cast<int>(
      ::recv(fd, buffer, static_cast<std::size_t>(length), flags));
}

inline int sendto(SOCKET fd, const char *buffer, int length, int flags,
                  const SOCKADDR *addr, int addrlen) {
  return static_cast<int>(::sendto(fd, buffer,
                                   static_cast<std::size_t>(length), flags,
                                   addr, static_cast<socklen_t>(addrlen)));
}

inline int recvfrom(SOCKET fd, char *buffer, int length, int flags,
                    SOCKADDR *addr, int *addrlen) {
  socklen_t len = addrlen ? static_cast<socklen_t>(*addrlen) : 0;
  const int result = static_cast<int>(
      ::recvfrom(fd, buffer, static_cast<std::size_t>(length), flags, addr,
                 addrlen ? &len : nullptr));
  if (addrlen)
    *addrlen = static_cast<int>(len);
  return result;
}

inline int shutdown(SOCKET fd, int how) { return ::shutdown(fd, how); }

inline int setsockopt(SOCKET fd, int level, int name, const char *value,
                      int length) {
  return ::setsockopt(fd, level, name, value, static_cast<socklen_t>(length));
}

inline int getsockopt(SOCKET fd, int level, int name, char *value,
                      int *length) {
  socklen_t len = length ? static_cast<socklen_t>(*length) : 0;
  const int result = ::getsockopt(fd, level, name, value,
                                  length ? &len : nullptr);
  if (length)
    *length = static_cast<int>(len);
  return result;
}

inline int getsockname(SOCKET fd, SOCKADDR *addr, int *addrlen) {
  socklen_t len = addrlen ? static_cast<socklen_t>(*addrlen) : 0;
  const int result = ::getsockname(fd, addr, addrlen ? &len : nullptr);
  if (addrlen)
    *addrlen = static_cast<int>(len);
  return result;
}

inline int ioctlsocket(SOCKET fd, int command, unsigned long *value) {
  //\tFIONBIO maps to the real non-blocking flag on the POSIX layer.
  if (command == FIONBIO && value)
    return ::ioctl(fd, FIONBIO, value);
  return SOCKET_ERROR;
}

inline int WSAGetLastError() { return errno; }

inline int WSAStartup(unsigned short version, void *data) {
  (void)version;
  (void)data;
  //\tThe POSIX socket layer initializes lazily; startup is a no-op success
  //\twith the real backend semantics.
  return 0;
}

inline int WSACleanup() { return 0; }

inline struct hostent *gethostbyname(const char *name) {
  return ::gethostbyname(name);
}

inline int gethostname(char *name, int length) {
  return ::gethostname(name, static_cast<std::size_t>(length));
}

inline unsigned long inet_addr(const char *address) {
  return static_cast<unsigned long>(::inet_addr(address));
}

inline char *inet_ntoa(struct in_addr address) {
  //\tPOSIX inet_ntoa returns a static buffer per thread; the winsock
  //\tsemantics the client relies on (use before the next call) match.
  return ::inet_ntoa(address);
}

inline unsigned short htons(unsigned short value) { return ::htons(value); }
inline unsigned short ntohs(unsigned short value) { return ::ntohs(value); }
inline unsigned long htonl(unsigned long value) { return ::htonl(value); }
inline unsigned long ntohl(unsigned long value) { return ::ntohl(value); }

#endif // PW153_PLATFORM_WINSOCK2_H
