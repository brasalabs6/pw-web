// Wide (UTF-16) formatting primitives for the original client code.
// MSVC's vswprintf/swprintf/_wtoi have no char16_t standard equivalents, so
// the port provides real implementations here with the MSVC semantics the
// original sources rely on: %s reads a wide string, %S reads a narrow string
// (UNICODE build), %c reads a wide character, %I64 reads a 64-bit integer
// and %ld/%lu keep the MSVC 32-bit long width so the varargs stream stays
// aligned with the original callers on every target.
#ifndef PW153_PLATFORM_WIDESTR_H
#define PW153_PLATFORM_WIDESTR_H

#include <cstdarg>
#include <cstdint>

namespace pw153::platform {

// MSVC-style vswprintf: writes a nul-terminated result into out (the caller
// guarantees capacity, exactly like the original call sites) and returns the
// number of characters written, excluding the terminator.
int wide_vformat(char16_t *out, const char16_t *format, va_list args);

// _wtoi: parses an optionally signed decimal prefix; parsing stops at the
// first non-digit, matching the CRT behavior.
int wide_toi(const char16_t *text);

// _wtoi64: 64-bit variant of wide_toi.
std::int64_t wide_toi64(const char16_t *text);

// swscanf("%f") equivalent: parses a decimal float, 0.0f when nothing parses.
float wide_tof(const char16_t *text);

// wcstod equivalent; endptr (optional) receives the first unconsumed char16.
double wide_tod(const char16_t *text, const char16_t **endptr);

// swprintf equivalent for the original call sites.
int wide_format(char16_t *out, const char16_t *format, ...);

} // namespace pw153::platform

#endif // PW153_PLATFORM_WIDESTR_H
