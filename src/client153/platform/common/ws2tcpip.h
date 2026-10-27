/*
 * FILE: ws2tcpip.h (platform seam)
 *
 * DESCRIPTION: The IP helper surface the defence layer uses, mapped to the
 * POSIX equivalents.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#ifndef PW153_PLATFORM_WS2TCPIP_H
#define PW153_PLATFORM_WS2TCPIP_H
#include "winsock2.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#define IP_TTL 2
#define IP_HDRINCL 2
inline int inet_pton(int af, const char *src, void *dst) {
  return ::inet_pton(af, src, dst);
}
#endif
