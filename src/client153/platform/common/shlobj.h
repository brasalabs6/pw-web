/*
 * FILE: shlobj.h (platform seam)
 *
 * DESCRIPTION: The shell object surface the GT overlay reaches. The browser
 * target has no shell; the documented type surface lets the sources
 * compile, and the shell-dependent paths degrade through their existing
 * failure checks.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_SHLOBJ_H
#define PW153_PLATFORM_SHLOBJ_H

#include "Windows.h"

//	The shell item ID list (the documented opaque layout).
typedef struct _ITEMIDLIST {
  void *idList;
} ITEMIDLIST;

typedef ITEMIDLIST *LPITEMIDLIST;

//	The shell folder IDs (the documented CSIDL values).
#define CSIDL_DESKTOP 0x0000
#define CSIDL_APPDATA 0x001a
#define CSIDL_LOCAL_APPDATA 0x001c
#define CSIDL_COMMON_APPDATA 0x0023

typedef DWORD SHGFP_TYPE;
#define SHGFP_TYPE_CURRENT 0

//	The shell folder path: the browser target has no shell folders; the
//	call fails explicitly (the caller's fallback path handles it).
inline HRESULT SHGetFolderPathW(HWND owner, int folder, void *reserved,
                                SHGFP_TYPE type, wchar_t *path) {
  (void)owner;
  (void)folder;
  (void)reserved;
  (void)type;
  if (path)
    path[0] = L'\0';
  return E_FAIL;
}

inline HRESULT SHGetFolderPathA(HWND owner, int folder, void *reserved,
                                SHGFP_TYPE type, char *path) {
  (void)owner;
  (void)folder;
  (void)reserved;
  (void)type;
  if (path)
    path[0] = '\0';
  return E_FAIL;
}

#endif
