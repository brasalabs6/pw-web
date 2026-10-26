// Shared printf ABI adaptation for original Angelica APIs.
#include "format.h"
#include <cstring>
#include <stdexcept>
namespace pw153::platform {
std::string printf_format(const char *format) {
  if (!format)
    throw std::invalid_argument("null AString format");
  std::string out;
  for (std::size_t i = 0; format[i]; ++i) {
    out += format[i];
    if (format[i] != '%')
      continue;
    ++i;
    if (format[i] == '%') {
      out += '%';
      continue;
    }
    // Flags, width and precision precede the length modifier.
    while (format[i] && std::strchr("-+ #0*.0123456789", format[i]))
      out += format[i++];
    if (std::strncmp(format + i, "I64", 3) == 0) {
      out += "ll";
      i += 3;
    } else if (std::strncmp(format + i, "I32", 3) == 0)
      i += 3;
    if (!format[i])
      throw std::invalid_argument("incomplete AString format");
    out += format[i];
  }
  return out;
}
} // namespace pw153::platform
