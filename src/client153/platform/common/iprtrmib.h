/*
 * iprtrmib.h — the MIB IP-address table types (platform seam).
 *
 * defence/socket_helper.cpp includes <iprtrmib.h> for the documented
 * MIB_IPADDRTABLE layout; the functions themselves (GetIpAddrTable,
 * GetBestInterface) are loaded dynamically from iphlpapi.dll by the caller,
 * and on the browser target LoadLibrary fails explicitly, so the caller's
 * own fallback path runs. Only the type surface is needed here.
 *
 * Struct layouts follow the documented Win32 ABI (any-size trailing array).
 */

#ifndef PW153_PLATFORM_IPRTRMIB_H
#define PW153_PLATFORM_IPRTRMIB_H

#include "Windows.h"

typedef struct _MIB_IPADDRROW {
  DWORD dwAddr;
  DWORD dwIndex;
  DWORD dwMask;
  DWORD dwBCastAddr;
  DWORD dwReasmSize;
  unsigned short unused1;
  unsigned short unused2;
} MIB_IPADDRROW, *PMIB_IPADDRROW;

#ifndef ANY_SIZE
#define ANY_SIZE 1
#endif

typedef struct _MIB_IPADDRTABLE {
  DWORD dwNumEntries;
  MIB_IPADDRROW table[ANY_SIZE];
} MIB_IPADDRTABLE, *PMIB_IPADDRTABLE;

#endif  // PW153_PLATFORM_IPRTRMIB_H
