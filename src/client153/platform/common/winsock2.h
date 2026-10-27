/*
 * FILE: winsock2.h (platform seam)
 *
 * DESCRIPTION: The Winsock surface the client's network layer compiles
 * against, mapped to POSIX sockets. Browser transport requires a compatible
 * WebSocket gateway and has not yet been validated end to end. GNET's select
 * and wakeup behavior also require adaptation for the browser.
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
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET (SOCKET)-1
#define SOCKET_ERROR (-1)

#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2
#define WSAEWOULDBLOCK 10035
#define WSAEINPROGRESS 10036
#define WSANOTINITIALISED 10093

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef SOCKADDR *LPSOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct hostent HOSTENT;

inline int closesocket(SOCKET fd) { return ::close(fd); }

inline int connect(SOCKET fd, const SOCKADDR *addr, int addrlen) {
  return ::connect(fd, addr, static_cast<socklen_t>(addrlen));
}

inline int bind(SOCKET fd, const SOCKADDR *addr, int addrlen) {
  return ::bind(fd, addr, static_cast<socklen_t>(addrlen));
}

inline SOCKET accept(SOCKET fd, SOCKADDR *addr, int *addrlen) {
  socklen_t len = addrlen ? static_cast<socklen_t>(*addrlen) : 0;
  const SOCKET result = ::accept(fd, addr, addrlen ? &len : nullptr);
  if (addrlen)
    *addrlen = static_cast<int>(len);
  return result;
}

inline int send(SOCKET fd, const char *buffer, int length, int flags) {
  return static_cast<int>(
      ::send(fd, static_cast<const void *>(buffer), static_cast<std::size_t>(length), flags));
}

inline int recv(SOCKET fd, char *buffer, int length, int flags) {
  return static_cast<int>(
      ::recv(fd, static_cast<void *>(buffer), static_cast<std::size_t>(length), flags));
}

inline int sendto(SOCKET fd, const char *buffer, int length, int flags,
                  const SOCKADDR *addr, int addrlen) {
  return static_cast<int>(::sendto(fd, static_cast<const void *>(buffer),
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

inline int setsockopt(SOCKET fd, int level, int name, const char *value,
                      int length) {
  return ::setsockopt(fd, level, name, static_cast<const void *>(value), static_cast<socklen_t>(length));
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
  if (static_cast<unsigned long>(command) == static_cast<unsigned long>(FIONBIO) && value) {
    int enabled = *value ? 1 : 0;
    return ::ioctl(fd, FIONBIO, &enabled);
  }
  return SOCKET_ERROR;
}

inline int WSAGetLastError() {
  if (errno == EWOULDBLOCK) return WSAEWOULDBLOCK;
  if (errno == EINPROGRESS) return WSAEINPROGRESS;
  return errno;
}

inline int WSAStartup(unsigned short version, void *data) {
  (void)version;
  (void)data;
  //\tThe POSIX socket layer initializes lazily; startup is a no-op success
  //\twith the real backend semantics.
  return 0;
}

inline int WSACleanup() { return 0; }

#endif // PW153_PLATFORM_WINSOCK2_H
