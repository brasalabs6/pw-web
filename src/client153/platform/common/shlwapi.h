/*
 * FILE: shlwapi.h (platform seam)
 *
 * DESCRIPTION: The shell lightweight utility surface. The browser target has
 * no shell; the path utilities map to the portable implementations.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_SHLWAPI_H
#define PW153_PLATFORM_SHLWAPI_H

#include "Windows.h"
#include <cstring>
#include <sys/stat.h>

inline LPSTR PathFindFileNameA(const char *path) {
  if (!path)
    return const_cast<LPSTR>(path);
  const char *last = path;
  for (const char *p = path; *p; ++p)
    if (*p == '\\' || *p == '/')
      last = p + 1;
  return const_cast<LPSTR>(last);
}

inline void PathStripPathA(char *path) {
  if (!path)
    return;
  LPSTR name = PathFindFileNameA(path);
  if (name != path)
    std::memmove(path, name, std::strlen(name) + 1);
}

//	File existence: the real filesystem check (the documented semantics).
inline BOOL PathFileExistsA(const char *path) {
  if (!path || !*path) return FALSE;
  struct stat info;
  return ::stat(path, &info) == 0 ? TRUE : FALSE;
}

inline BOOL PathFileExistsW(const wchar_t *path) {
  if (!path)
    return FALSE;
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(path, narrow))
    return FALSE;
  FILE *probe = std::fopen(narrow.c_str(), "rb");
  if (probe) {
    std::fclose(probe);
    return TRUE;
  }
  return FALSE;
}

inline BOOL PathAppendW(wchar_t *path, const wchar_t *more) {
  if (!path || !more)
    return FALSE;
  std::size_t length = 0;
  while (path[length])
    ++length;
  std::size_t extra = 0;
  while (more[extra])
    ++extra;
  if (length > 0 && path[length - 1] != L'\\' && path[length - 1] != L'/') {
    if (length + 1 >= MAX_PATH)
      return FALSE;
    path[length++] = L'\\';
  }
  if (length + extra >= MAX_PATH)
    return FALSE;
  std::memcpy(path + length, more, (extra + 1) * sizeof(wchar_t));
  return TRUE;
}

#endif // PW153_PLATFORM_SHLWAPI_H
