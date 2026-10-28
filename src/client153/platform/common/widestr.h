// Wide (UTF-16) formatting primitives for the original client code.
// MSVC's vswprintf/swprintf/_wtoi have no wchar_t standard equivalents, so
// the port provides real implementations here with the MSVC semantics the
// original sources rely on: %s reads a wide string, %S reads a narrow string
// (UNICODE build), %c reads a wide character, %I64 reads a 64-bit integer
// and %ld/%lu keep the MSVC 32-bit long width so the varargs stream stays
// aligned with the original callers on every target.
#ifndef PW153_PLATFORM_WIDESTR_H
#define PW153_PLATFORM_WIDESTR_H

#include <cstdarg>
#include <cstdint>
#include <cstddef>
#include <cstdio>

namespace pw153::platform {

// MSVC-style vswprintf: writes a nul-terminated result into out (the caller
// guarantees capacity, exactly like the original call sites) and returns the
// number of characters written, excluding the terminator.
int wide_vformat(wchar_t *out, const wchar_t *format, va_list args);
int wide_snprintf(wchar_t *out, std::size_t count, const wchar_t *format, ...);
int wide_vsnprintf(wchar_t *out, std::size_t count, const wchar_t *format,
                   va_list args);
int wide_fprintf_utf16le(std::FILE *out, const wchar_t *format, ...);

// _wtoi: parses an optionally signed decimal prefix; parsing stops at the
// first non-digit, matching the CRT behavior.
int wide_toi(const wchar_t *text);

// _wtoi64: 64-bit variant of wide_toi.
std::int64_t wide_toi64(const wchar_t *text);

// swscanf("%f") equivalent: parses a decimal float, 0.0f when nothing parses.
float wide_tof(const wchar_t *text);

// wcstod equivalent; endptr (optional) receives the first unconsumed char16.
double wide_tod(const wchar_t *text, const wchar_t **endptr);

// swprintf equivalent for the original call sites.
int wide_format(wchar_t *out, const wchar_t *format, ...);

//	The system wchar_t CRT is compiled for the platform's 32-bit wchar_t
//	and is wrong under -fshort-wchar; the port owns these 16-bit versions.
int wide_strlen(const wchar_t *text);
int wide_strcmp(const wchar_t *a, const wchar_t *b);
int wide_strncmp(const wchar_t *a, const wchar_t *b, int n);
const wchar_t *wide_strstr(const wchar_t *hay, const wchar_t *needle);
const wchar_t *wide_strrchr(const wchar_t *text, wchar_t ch);
int wide_strcspn(const wchar_t *text, const wchar_t *set);
int wide_casecmp(const wchar_t *a, const wchar_t *b);

// In-place case conversion (the _wcsupr/_wcslwr and strupr/strlwr surface).
void wide_strupr(wchar_t *text);
void wide_strlwr(wchar_t *text);
void narrow_strupr(char *text);
void narrow_strlwr(char *text);

// The copy/concatenate/scan primitives the wcs* names below wrap (the
// standard CRT semantics over 16-bit wchar_t).
wchar_t *wide_strcpy(wchar_t *destination, const wchar_t *source);
wchar_t *wide_strncpy(wchar_t *destination, const wchar_t *source, int count);
wchar_t *wide_strcat(wchar_t *destination, const wchar_t *source);
wchar_t *wide_strncat(wchar_t *destination, const wchar_t *source, int count);
wchar_t *wide_strchr(const wchar_t *text, wchar_t character);

// MSVC swprintf/vswprintf aliases over the formatter.
int wide_sprintf(wchar_t *out, const wchar_t *format, ...);
int wide_vsprintf(wchar_t *out, const wchar_t *format, va_list args);

// swscanf for the spec set the client uses: %d %i %u %x %f %s %c with
// optional width and literal separators; returns the converted count.
int wide_sscanf(const wchar_t *text, const wchar_t *format, ...);

// _itow/_i64tow: wide int-to-string conversions (radix supported).
wchar_t *wide_itow(int value, wchar_t *buffer, int radix);
wchar_t *wide_i64tow(long long value, wchar_t *buffer, int radix);

} // namespace pw153::platform

// The wide CRT names: wchar_t is 16-bit here (-fshort-wchar) and the
// system libc's wcs* scan 32-bit units, so the port owns the names. The
// standard names are extern "C" inline so the libc declarations (any
// <wchar.h> pulled later) are compatible redeclarations and every call
// site binds to these 16-bit implementations. The MSVC-only names
// (wcsicmp, wcslwr, wcsupr) have no libc counterpart and are plain
// inline. Defined here because Windows.h includes this header before
// any libc wide header can be pulled in.
//	The exception-specification follows the host libc's declarations
//	(glibc declares the names __THROW/noexcept; musl declares them
//	unspecified) so the redeclarations stay compatible on both.
#ifdef __THROW
#define PW153_WCS_THROW __THROW
#else
#define PW153_WCS_THROW
#endif

extern "C" {
inline std::size_t wcslen(const wchar_t *text) PW153_WCS_THROW {
  return static_cast<std::size_t>(pw153::platform::wide_strlen(text));
}
inline wchar_t *wcscpy(wchar_t *destination, const wchar_t *source)
    PW153_WCS_THROW {
  return pw153::platform::wide_strcpy(destination, source);
}
inline wchar_t *wcsncpy(wchar_t *destination, const wchar_t *source,
                        std::size_t count) PW153_WCS_THROW {
  return pw153::platform::wide_strncpy(destination, source,
                                       static_cast<int>(count));
}
inline wchar_t *wcscat(wchar_t *destination, const wchar_t *source)
    PW153_WCS_THROW {
  return pw153::platform::wide_strcat(destination, source);
}
inline wchar_t *wcsncat(wchar_t *destination, const wchar_t *source,
                        std::size_t count) PW153_WCS_THROW {
  return pw153::platform::wide_strncat(destination, source,
                                       static_cast<int>(count));
}
inline int wcscmp(const wchar_t *a, const wchar_t *b) PW153_WCS_THROW {
  return pw153::platform::wide_strcmp(a, b);
}
inline int wcsncmp(const wchar_t *a, const wchar_t *b, std::size_t count)
    PW153_WCS_THROW {
  return pw153::platform::wide_strncmp(a, b, static_cast<int>(count));
}
inline std::size_t wcscspn(const wchar_t *text, const wchar_t *set)
    PW153_WCS_THROW {
  return static_cast<std::size_t>(pw153::platform::wide_strcspn(text, set));
}
} // extern "C"

//	The scan trio (wcschr/wcsrchr/wcsstr) is NOT aliased here: glibc's
//	C++ <wchar.h> declares a const-preserving overload pair sharing
//	one asm name, which the port cannot redefine without a duplicate
//	symbol. The game's call sites use wide_strchr/wide_strrchr/
//	wide_strstr directly (the 16-bit scanners).

// The MSVC-only wide CRT names.
inline int wcsicmp(const wchar_t *a, const wchar_t *b) {
  return pw153::platform::wide_casecmp(a, b);
}
inline wchar_t *wcslwr(wchar_t *text) {
  pw153::platform::wide_strlwr(text);
  return text;
}
inline wchar_t *wcsupr(wchar_t *text) {
  pw153::platform::wide_strupr(text);
  return text;
}

//	MSVC _vsnwprintf: the count-bounded wide formatter (the game's
//	glb_vsnprintf macro maps here). Always nul-terminates within the
//	count (the callers read the result as a string; MSVC could leave it
//	unterminated at exactly count - the callers never rely on that).
int _vsnwprintf(wchar_t *out, std::size_t count, const wchar_t *format,
                va_list args);

//	MSVC _vsnprintf: the count-bounded narrow formatter over the
//	standard vsnprintf.
inline int _vsnprintf(char *out, std::size_t count, const char *format,
                      va_list args) {
  return std::vsnprintf(out, count, format, args);
}

//	MSVC itoa/_itoa: the radix int-to-string conversion (the CRT
//	contract; the buffer receives the nul-terminated digits).
inline char *itoa(int value, char *buffer, int radix) {
  if (!buffer || radix < 2 || radix > 36)
    return buffer;
  char digits[36];
  int length = 0;
  unsigned long magnitude =
      value < 0 ? static_cast<unsigned long>(-static_cast<long>(value))
                : static_cast<unsigned long>(value);
  do {
    digits[length++] = static_cast<char>("0123456789abcdefghijklmnopqrstuvwxyz"[magnitude % radix]);
    magnitude /= static_cast<unsigned long>(radix);
  } while (magnitude);
  int out = 0;
  if (value < 0 && radix == 10)
    buffer[out++] = '-';
  while (length > 0)
    buffer[out++] = digits[--length];
  buffer[out] = '\0';
  return buffer;
}
inline char *_itoa(int value, char *buffer, int radix) {
  return itoa(value, buffer, radix);
}

//	swprintf: the port owns both arities. The C99 count-bounded form
//	matches the libc declaration (extern "C", the host's exception
//	spec) so every call binds to the 16-bit implementation - the
//	system swprintf scans 32-bit units under -fshort-wchar. The MSVC
//	2-arg form has no libc counterpart.
extern "C" {
inline int swprintf(wchar_t *out, std::size_t count, const wchar_t *format,
                    ...) PW153_WCS_THROW {
  va_list args;
  va_start(args, format);
  const int written = pw153::platform::wide_vsnprintf(out, count, format, args);
  va_end(args);
  return written;
}
}
inline int swprintf(wchar_t *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = pw153::platform::wide_vsprintf(out, format, args);
  va_end(args);
  return written;
}

#endif // PW153_PLATFORM_WIDESTR_H
