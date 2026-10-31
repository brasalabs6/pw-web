#include "utf16.h"
#include <cstdint>
#include <utility>
namespace pw153::platform {
bool utf8_to_utf16(std::string_view input, std::u16string &output) {
  output.clear();
  std::u16string result;
  for (std::size_t i = 0; i < input.size();) {
    const auto first = static_cast<std::uint8_t>(input[i++]);
    std::uint32_t cp;
    unsigned remaining;
    std::uint32_t minimum;
    if (first < 0x80) {
      cp = first;
      remaining = 0;
      minimum = 0;
    } else if (first >= 0xc2 && first <= 0xdf) {
      cp = first & 0x1f;
      remaining = 1;
      minimum = 0x80;
    } else if (first >= 0xe0 && first <= 0xef) {
      cp = first & 0x0f;
      remaining = 2;
      minimum = 0x800;
    } else if (first >= 0xf0 && first <= 0xf4) {
      cp = first & 0x07;
      remaining = 3;
      minimum = 0x10000;
    } else
      return false;
    if (remaining > input.size() - i)
      return false;
    while (remaining--) {
      const auto c = static_cast<std::uint8_t>(input[i++]);
      if ((c & 0xc0) != 0x80)
        return false;
      cp = (cp << 6) | (c & 0x3f);
    }
    if (cp < minimum || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
      return false;
    if (cp < 0x10000)
      result.push_back(static_cast<char16_t>(cp));
    else {
      cp -= 0x10000;
      result.push_back(static_cast<char16_t>(0xd800 + (cp >> 10)));
      result.push_back(static_cast<char16_t>(0xdc00 + (cp & 0x3ff)));
    }
  }
  output = std::move(result);
  return true;
}
bool utf16_to_utf8(std::u16string_view input, std::string &output) {
  output.clear();
  std::string result;
  for (std::size_t i = 0; i < input.size(); ++i) {
    std::uint32_t cp = input[i];
    if (cp >= 0xd800 && cp <= 0xdbff) {
      if (++i >= input.size() || input[i] < 0xdc00 || input[i] > 0xdfff)
        return false;
      cp = 0x10000 + ((cp - 0xd800) << 10) + (input[i] - 0xdc00);
    } else if (cp >= 0xdc00 && cp <= 0xdfff)
      return false;
    if (cp < 0x80)
      result.push_back(static_cast<char>(cp));
    else if (cp < 0x800) {
      result.push_back(static_cast<char>(0xc0 | (cp >> 6)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp < 0x10000) {
      result.push_back(static_cast<char>(0xe0 | (cp >> 12)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
      result.push_back(static_cast<char>(0xf0 | (cp >> 18)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
  }
  output = std::move(result);
  return true;
}
} // namespace pw153::platform


namespace pw153::platform {
bool utf8_to_utf16(std::string_view input, std::wstring &output) {
  std::u16string wide;
  if (!utf8_to_utf16(input, wide))
    return false;
  output.assign(reinterpret_cast<const wchar_t *>(wide.data()), wide.size());
  return true;
}

bool utf16_to_utf8(const wchar_t *input, std::string &output) {
  if (!input)
    return false;
  std::size_t length = 0;
  while (input[length])
    ++length;
  return utf16_to_utf8(std::u16string_view(reinterpret_cast<const char16_t *>(input), length), output);
}
} // namespace pw153::platform
