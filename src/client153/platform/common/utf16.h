#pragma once
#include <string>
#include <string_view>
namespace pw153::platform {
// Strict conversions; malformed input returns false and clears the output.
bool utf8_to_utf16(std::string_view input, std::u16string &output);
bool utf16_to_utf8(std::u16string_view input, std::string &output);
} // namespace pw153::platform
