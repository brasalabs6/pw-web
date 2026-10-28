//	Case/platform variant forwarding to "widestr.h"	//	the wide seam replaces the TCHAR CRT..
#ifndef PW153_PLATFORM_TCHAR_H
#define PW153_PLATFORM_TCHAR_H
#include "widestr.h"	//	the wide seam replaces the TCHAR CRT.

//	MS CRT tchar.h also spells the character type with the leading underscore.
typedef TCHAR _TCHAR;

//	Generic-text macros: the seam's TCHAR is unconditionally the port's
//	16-bit wide type, so the generic-text surface maps to the wide seam
//	(never the libc wcs* CRT, which scans 32-bit units). MS CRT tchar.h
//	selects these by _UNICODE; the seam has a single wide mapping.
#define _T(text) L##text
#define _TEXT(text) L##text
inline int _stprintf(_TCHAR *out, const _TCHAR *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = pw153::platform::wide_vsprintf(out, format, args);
  va_end(args);
  return written;
}
inline _TCHAR *_tcscpy(_TCHAR *destination, const _TCHAR *source) {
  return pw153::platform::wide_strcpy(destination, source);
}
inline _TCHAR *_tcsncpy(_TCHAR *destination, const _TCHAR *source, int count) {
  return pw153::platform::wide_strncpy(destination, source, count);
}
inline int _tcscmp(const _TCHAR *a, const _TCHAR *b) {
  return pw153::platform::wide_strcmp(a, b);
}
inline int _tcsncmp(const _TCHAR *a, const _TCHAR *b, int count) {
  return pw153::platform::wide_strncmp(a, b, count);
}
inline int _tcslen(const _TCHAR *text) {
  return pw153::platform::wide_strlen(text);
}
inline int _ttoi(const _TCHAR *text) {
  return pw153::platform::wide_toi(text);
}
//	MS _sntprintf: the count-bounded generic-text formatter (tchar.h spells
//	it as a macro over _snwprintf in a UNICODE build). wide_snprintf matches
//	the CRT semantics (no terminator when truncated).
#define _sntprintf(out, count, format, ...) \
  pw153::platform::wide_snprintf((out), (count), (format), ##__VA_ARGS__)

#endif // PW153_PLATFORM_TCHAR_H
