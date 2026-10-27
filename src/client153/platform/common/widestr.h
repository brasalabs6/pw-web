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

#endif // PW153_PLATFORM_WIDESTR_H
