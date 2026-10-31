/*
 * FILE: urlmon.h (platform seam)
 *
 * DESCRIPTION: The URL moniker download-to-file surface the client's
 * browser-dialog code compiles against. The browser target has no COM
 * monikers: the download maps to the fetch adapter in the network cycle;
 * until it registers, the calls fail explicitly (the callers' error paths
 * handle it).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_URLMON_H
#define PW153_PLATFORM_URLMON_H

#include "Windows.h"
#include "unknwn.h"	//	LPUNKNOWN.
#include "exdisp.h"	//	IBindStatusCallback (the real interface the
		//	browser-dialog callback implements).

typedef IBindStatusCallback *LPBINDSTATUSCALLBACK;
typedef void *IBindCtx_FWD;

//\tThe download-to-file entry point: no COM monikers exist on the browser
//\ttarget; the call fails explicitly (the caller reports the failure).
inline HRESULT URLDownloadToFileA(LPUNKNOWN caller, const char *url,
                                 const char *file, DWORD reserved,
                                 LPBINDSTATUSCALLBACK callback) {
  (void)caller;
  (void)url;
  (void)file;
  (void)reserved;
  (void)callback;
  return E_FAIL;
}

inline HRESULT URLDownloadToFileW(LPUNKNOWN caller, const wchar_t *url,
                                 const wchar_t *file, DWORD reserved,
                                 LPBINDSTATUSCALLBACK callback) {
  (void)caller;
  (void)url;
  (void)file;
  (void)reserved;
  (void)callback;
  return E_FAIL;
}

#endif // PW153_PLATFORM_URLMON_H
