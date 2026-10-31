/*
 * FILE: io.h (platform seam)
 *
 * DESCRIPTION: The MSVC CRT file-find surface (_findfirst/_findnext/
 * _findclose) over the same real directory scan FindFirstFileA uses
 * (findfiles.cpp): pattern matching, the attribute flags, and the
 * documented _finddata_t layout.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_IO_H
#define PW153_PLATFORM_IO_H

#include <cstdint>
#include <ctime>

//	The CRT file size type (the MSVC 32-bit width).
typedef unsigned long _fsize_t;

//	The CRT find data (the documented _finddata_t layout; POSIX has no
//	creation time, so time_create reports -1 like the CRT does when the
//	information is unavailable).
typedef struct _finddata_t {
  unsigned attrib;
  std::time_t time_create;
  std::time_t time_access;
  std::time_t time_write;
  _fsize_t size;
  char name[260];
} _finddata_t;

//	The attribute flags (the documented CRT values).
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

//	The find handle is the same real scan state FindFirstFileA uses;
//	_findfirst returns -1 when nothing matches (the documented CRT
//	contract).
std::intptr_t _findfirst(const char *pattern, _finddata_t *data);
int _findnext(std::intptr_t handle, _finddata_t *data);
int _findclose(std::intptr_t handle);

#endif // PW153_PLATFORM_IO_H
