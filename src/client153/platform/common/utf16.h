#pragma once
#include <string>
#include <string_view>
#include <cstdint>
static_assert(sizeof(wchar_t) == 2,
              "the client port requires -fshort-wchar (original UTF-16 width)");

namespace pw153::platform {
// Strict conversions; malformed input returns false and clears the output.
bool utf8_to_utf16(std::string_view input, std::u16string &output);
bool utf16_to_utf8(std::u16string_view input, std::string &output);

//	wchar_t overloads: with -fshort-wchar the wide type is the original
//	16-bit width, so these share the same conversion core.
bool utf8_to_utf16(std::string_view input, std::wstring &output);
bool utf16_to_utf8(const wchar_t *input, std::string &output);
} // namespace pw153::platform
