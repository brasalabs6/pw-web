/*
 * FILE: Iphlpapi.h (platform seam)
 *
 * DESCRIPTION: The IP Helper adapter surface the defence hardware-id
 * code compiles against. The browser target has no network adapters
 * addressable through this API; the queries fail explicitly (the hwid
 * code's fallbacks handle it).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_IPHLPAPI_H
#define PW153_PLATFORM_IPHLPAPI_H

#include "Windows.h"

#ifndef ERROR_NO_DATA
#define ERROR_NO_DATA 110
#endif

typedef ULONG *PULONG;

//	The adapter information record (the documented iptypes.h layout the
//	callers read).
typedef struct _IP_ADAPTER_INFO {
  struct _IP_ADAPTER_INFO *Next;	//	the documented linked list.
  char AdapterName[260];
  char Description[132];
  UINT AddressLength;
  BYTE Address[8];
  DWORD Index;
  UINT Type;
  UINT DhcpEnabled;
  char CurrentIpAddress[16];
  DWORD IpAddressListIndex;
  char IpAddress[16];
  char IpMask[16];
  DWORD GatewayListIndex;
  char Gateway[16];
  DWORD DhcpServerListIndex;
  char DhcpServer[16];
  BOOL HaveWins;
  DWORD PrimaryWinsServerListIndex;
  char PrimaryWinsServer[16];
  DWORD SecondaryWinsServerListIndex;
  char SecondaryWinsServer[16];
} IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;

//	The adapter enumeration: no adapters exist on the browser target;
//	the buffer is reported too small (the documented GetAdaptersInfo
//	contract for "no data" is a nonzero return; the callers bail).
inline DWORD GetAdaptersInfo(PIP_ADAPTER_INFO adapterInfo, PULONG sizePointer) {
  if (sizePointer)
    *sizePointer = 0;
  (void)adapterInfo;
  return ERROR_NO_DATA;	//	110: the documented no-data result.
}

#endif // PW153_PLATFORM_IPHLPAPI_H
