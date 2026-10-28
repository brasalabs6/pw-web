/*
 * FILE: mbctype.h (platform seam)
 *
 * DESCRIPTION: The multibyte code-page surface of the Windows CRT. The
 * browser target has no code pages - the process locale is the single-byte
 * C locale - so the _MB_CP_* constants keep the documented CRT values and
 * the setters fail explicitly instead of faking success: _setmbcp reports
 * failure (the callers treat the CRT default as the only state) exactly
 * like the toolhelp seam reports ERROR_CALL_NOT_IMPLEMENTED. _getmbcp
 * reports the honest state, the _MB_CP_ANSI default.
 *
 * API surface per the Windows CRT mbctype.h (Microsoft); this seam declares
 * the documented interface, it is not a copy of the SDK header.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_MBCTYPE_H
#define PW153_PLATFORM_MBCTYPE_H

#include <cerrno>

// The code-page selectors passed to _setmbcp (documented CRT values).
#define _MB_CP_ANSI 0
#define _MB_CP_OEM 1
#define _MB_CP_OLS 2
#define _MB_CP_SLCS 3
#define _MB_CP_ULS 4
#define _MB_CP_DBCS 5
#define _MB_CP_932 932
#define _MB_CP_936 936
#define _MB_CP_949 949
#define _MB_CP_950 950
#define _MB_CP_UTF8 65001

#ifdef __cplusplus
extern "C++" {

inline int _setmbcp(int codePage) {
  (void)codePage;
  // The browser target has no code pages; every request fails explicitly.
  errno = ENOSYS;
  return -1;
}

inline int _getmbcp() { return _MB_CP_ANSI; }

} // extern "C++"
#else
int _setmbcp(int codePage);
int _getmbcp(void);
#endif

#endif // PW153_PLATFORM_MBCTYPE_H
