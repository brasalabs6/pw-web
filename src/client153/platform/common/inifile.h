/*
 * FILE: inifile.h (platform seam)
 *
 * DESCRIPTION: The Win32 private-profile (INI) reader with the documented
 * semantics: case-insensitive section/key lookup, surrounding-space
 * trimming, optional surrounding quotes, `;` comments, the caller's
 * default on miss, and the key/section listing forms. The browser
 * target reads the same .ini files over the adapter's filesystem.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_INIFILE_H
#define PW153_PLATFORM_INIFILE_H

#include "widestr.h"	//	the UTF-16/UTF-8 path conversion.
#include "utf16.h"	//	the converters.

//	The narrow reader (byte passthrough over the file's own encoding).
unsigned GetPrivateProfileStringA(const char *section, const char *key,
                                  const char *defaultValue, char *buffer,
                                  unsigned size, const char *file);
unsigned GetPrivateProfileIntA(const char *section, const char *key,
                               unsigned defaultValue, const char *file);

//	The wide reader (UTF-16 keys/values over the same parser).
unsigned GetPrivateProfileStringW(const wchar_t *section, const wchar_t *key,
                                  const wchar_t *defaultValue,
                                  wchar_t *buffer, unsigned size,
                                  const wchar_t *file);
unsigned GetPrivateProfileIntW(const wchar_t *section, const wchar_t *key,
                               unsigned defaultValue, const wchar_t *file);

//	The UNICODE defaults (the client builds with -DUNICODE).
inline unsigned GetPrivateProfileString(const wchar_t *section,
                                        const wchar_t *key,
                                        const wchar_t *defaultValue,
                                        wchar_t *buffer, unsigned size,
                                        const wchar_t *file) {
  return GetPrivateProfileStringW(section, key, defaultValue, buffer, size,
                                   file);
}
inline unsigned GetPrivateProfileInt(const wchar_t *section,
                                     const wchar_t *key, unsigned defaultValue,
                                     const wchar_t *file) {
  return GetPrivateProfileIntW(section, key, defaultValue, file);
}

#endif // PW153_PLATFORM_INIFILE_H
