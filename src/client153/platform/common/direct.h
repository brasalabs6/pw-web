/*
 * FILE: direct.h (platform seam)
 *
 * DESCRIPTION: The MSVC CRT directory-control surface over the real
 * POSIX calls (the browser target's filesystem).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_DIRECT_H
#define PW153_PLATFORM_DIRECT_H

#include "Windows.h"

#include <unistd.h>

//	_rmdir: the real directory removal over the UTF-8 path conversion.
inline int _rmdir(const char *path) {
  if (!path || !*path)
    return -1;
  std::string normalized(path);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  return rmdir(normalized.c_str());
}

//	_chdir: the real working-directory change.
inline int _chdir(const char *path) {
  if (!path || !*path)
    return -1;
  std::string normalized(path);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  return chdir(normalized.c_str());
}

#endif // PW153_PLATFORM_DIRECT_H
