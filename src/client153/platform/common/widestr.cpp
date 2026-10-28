#include "widestr.h"

#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <cstring>
#include <vector>

namespace pw153::platform {
namespace {

std::size_t wide_length(const wchar_t *text) {
  std::size_t length = 0;
  while (text[length])
    ++length;
  return length;
}

void widen_copy(wchar_t *out, const char *narrow, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i)
    out[i] = static_cast<wchar_t>(static_cast<unsigned char>(narrow[i]));
}

struct Spec {
  bool left;
  bool plus;
  bool space;
  bool zero;
  bool hash;
  int width;
  int precision;
  bool hasPrecision;
  int size; // 0 default, 1 = h, 2 = l (MSVC 32-bit long), 3 = I64
};

// Parses flags, width, precision and size modifiers; returns the conversion
// character and advances the format cursor past it. The va_list pointer keeps
// the argument stream shared with the caller.
wchar_t parse_spec(const wchar_t *&cursor, Spec &spec, va_list *args) {
  spec = Spec{false, false, false, false, false, 0, 0, false, 0};
  while (true) {
    const wchar_t c = *cursor;
    if (c == L'-')
      spec.left = true;
    else if (c == L'+')
      spec.plus = true;
    else if (c == L' ')
      spec.space = true;
    else if (c == L'0')
      spec.zero = true;
    else if (c == L'#')
      spec.hash = true;
    else
      break;
    ++cursor;
  }
  if (*cursor == L'*') {
    spec.width = va_arg(*args, int);
    ++cursor;
  } else
    while (*cursor >= L'0' && *cursor <= L'9')
      spec.width = spec.width * 10 + (*cursor++ - L'0');
  if (*cursor == L'.') {
    ++cursor;
    spec.hasPrecision = true;
    if (*cursor == L'*') {
      spec.precision = va_arg(*args, int);
      ++cursor;
    } else
      while (*cursor >= L'0' && *cursor <= L'9')
        spec.precision = spec.precision * 10 + (*cursor++ - L'0');
  }
  if (*cursor == L'h') {
    spec.size = 1;
    ++cursor;
  } else if (*cursor == L'l' || *cursor == L'F' || *cursor == L'N') {
    spec.size = 2;
    ++cursor;
  } else if (*cursor == L'I') {
    ++cursor;
    if (cursor[0] == L'6' && cursor[1] == L'4') {
      spec.size = 3;
      cursor += 2;
    } else if (cursor[0] == L'3' && cursor[1] == L'2') {
      spec.size = 0;
      cursor += 2;
    }
  }
  return *cursor++;
}

// Composes "%<flags>*.*<conv>" and formats through the narrow CRT, which
// applies width/precision/padding; the ASCII result is then widened.
template <typename Value>
void emit_numeric(std::vector<wchar_t> &out, const Spec &spec, const char *conversion,
                  Value value) {
  char flags[8] = {0};
  std::size_t f = 0;
  if (spec.left)
    flags[f++] = '-';
  if (spec.plus)
    flags[f++] = '+';
  if (spec.space)
    flags[f++] = ' ';
  if (spec.zero)
    flags[f++] = '0';
  if (spec.hash)
    flags[f++] = '#';
  char format[16];
  std::snprintf(format, sizeof(format), "%%%s*.*%s", flags, conversion);
  char narrow[96];
  std::snprintf(narrow, sizeof(narrow), format, spec.width,
                spec.hasPrecision ? spec.precision : -1, value);
  const std::size_t len = std::strlen(narrow);
  for (std::size_t i = 0; i < len; ++i)
    out.push_back(static_cast<unsigned char>(narrow[i]));
}

void pad_and_emit(std::vector<wchar_t> &out, const wchar_t *body, std::size_t bodyLen,
                  const Spec &spec) {
  const int width = spec.width;
  const std::size_t padding =
      width > 0 && static_cast<std::size_t>(width) > bodyLen
          ? static_cast<std::size_t>(width) - bodyLen
          : 0;
  if (!spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      out.push_back(L' ');
  for (std::size_t i = 0; i < bodyLen; ++i)
    out.push_back(body[i]);
  if (spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      out.push_back(L' ');
}

} // namespace

static std::vector<wchar_t> wide_format_buffer(const wchar_t *format, va_list args) {
  va_list list;
  va_copy(list, args);
  std::vector<wchar_t> cursorOut;
  const wchar_t *cursor = format;
  while (*cursor) {
    if (*cursor != L'%') {
      cursorOut.push_back(*cursor++);
      continue;
    }
    ++cursor;
    if (*cursor == L'%') {
      cursorOut.push_back(L'%');
      ++cursor;
      continue;
    }
    Spec spec;
    const wchar_t conversion = parse_spec(cursor, spec, &list);
    wchar_t body[160];
    std::size_t bodyLen = 0;
    switch (conversion) {
    case L'c':
    case L'C': {
      //	Characters arrive through the default argument promotion.
      body[0] = static_cast<wchar_t>(va_arg(list, int));
      bodyLen = 1;
      break;
    }
    case L's': {
      const wchar_t *value = va_arg(list, const wchar_t *);
      if (!value)
        value = L"(null)";
      bodyLen = wide_length(value);
      if (spec.hasPrecision && spec.precision >= 0 &&
          static_cast<std::size_t>(spec.precision) < bodyLen)
        bodyLen = static_cast<std::size_t>(spec.precision);
      pad_and_emit(cursorOut, value, bodyLen, spec);
      continue;
    }
    case L'S': {
      //	MSVC wide printf: %S is the narrow string in a UNICODE build.
      const char *value = va_arg(list, const char *);
      if (!value) {
        const wchar_t nullText[] = L"(null)";
        std::memcpy(body, nullText, sizeof(nullText) - sizeof(nullText[0]));
        bodyLen = 6;
      } else {
        std::size_t len = std::strlen(value);
        if (spec.hasPrecision && spec.precision >= 0 &&
            static_cast<std::size_t>(spec.precision) < len)
          len = static_cast<std::size_t>(spec.precision);
        std::vector<wchar_t> widened(len);
        widen_copy(widened.data(), value, len);
        pad_and_emit(cursorOut, widened.data(), len, spec);
        continue;
      }
      break;
    }
    case L'd':
    case L'i':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "lld", va_arg(list, std::int64_t));
      else
        emit_numeric(cursorOut, spec, "d", va_arg(list, int));
      continue;
    case L'L':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llL", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "L", va_arg(list, unsigned));
      continue;
    case L'x':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llx", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "x", va_arg(list, unsigned));
      continue;
    case L'X':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llX", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "X", va_arg(list, unsigned));
      continue;
    case L'o':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llo", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "o", va_arg(list, unsigned));
      continue;
    case L'f':
    case L'e':
    case L'E':
    case L'g':
    case L'G': {
      const double value = va_arg(list, double);
      char conv[2] = {static_cast<char>(conversion), 0};
      emit_numeric(cursorOut, spec, conv, value);
      continue;
    }
    case L'p': {
      void *value = va_arg(list, void *);
      char narrow[32];
      std::snprintf(narrow, sizeof(narrow), "%p", value);
      const std::size_t len = std::strlen(narrow);
      for (std::size_t i = 0; i < len; ++i)
        cursorOut.push_back(static_cast<unsigned char>(narrow[i]));
      continue;
    }
    case L'n': {
      int *target = va_arg(list, int *);
      if (target)
        *target = static_cast<int>(cursorOut.size());
      continue;
    }
    default:
      //	Unknown conversion: emit it verbatim, like the original CRT.
      body[0] = conversion;
      bodyLen = 1;
      break;
    }
    pad_and_emit(cursorOut, body, bodyLen, spec);
  }
  va_end(list);
  return cursorOut;
}

int wide_vformat(wchar_t *out, const wchar_t *format, va_list args) {
  const auto result = wide_format_buffer(format, args);
  for (std::size_t i = 0; i < result.size(); ++i) out[i] = result[i];
  out[result.size()] = 0;
  return static_cast<int>(result.size());
}

int wide_snprintf(wchar_t *out, std::size_t count, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const auto result = wide_format_buffer(format, args);
  va_end(args);
  const std::size_t copied = result.size() < count ? result.size() : count;
  for (std::size_t i = 0; i < copied; ++i) out[i] = result[i];
  if (result.size() < count) out[result.size()] = 0;
  return result.size() > count ? -1 : static_cast<int>(result.size());
}

int wide_fprintf_utf16le(std::FILE *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const auto result = wide_format_buffer(format, args);
  va_end(args);
  for (wchar_t unit : result) {
    const auto value = static_cast<std::uint16_t>(unit);
    const unsigned char bytes[] = {static_cast<unsigned char>(value),
                                   static_cast<unsigned char>(value >> 8)};
    if (std::fwrite(bytes, 1, sizeof(bytes), out) != sizeof(bytes)) return -1;
  }
  return static_cast<int>(result.size());
}

namespace {
// Copies the numeric prefix (sign, digits, '.', exponent) into a narrow
// buffer for the narrow CRT; returns the copied length.
std::size_t copy_numeric_prefix(char *out, std::size_t capacity,
                                const wchar_t *text, std::size_t *consumed) {
  std::size_t i = 0, n = 0;
  auto put = [&](char c) {
    if (n + 1 < capacity)
      out[n++] = c;
  };
  while (text[i] == L' ' || text[i] == L'\t' || text[i] == L'\n' ||
         text[i] == L'\r')
    ++i;
  if (text[i] == L'+' || text[i] == L'-')
    put(static_cast<char>(text[i++]));
  while (text[i] >= L'0' && text[i] <= L'9')
    put(static_cast<char>(text[i++]));
  if (text[i] == L'.') {
    put(static_cast<char>(text[i++]));
    while (text[i] >= L'0' && text[i] <= L'9')
      put(static_cast<char>(text[i++]));
  }
  if (text[i] == L'e' || text[i] == L'E') {
    put(static_cast<char>(text[i++]));
    if (text[i] == L'+' || text[i] == L'-')
      put(static_cast<char>(text[i++]));
    while (text[i] >= L'0' && text[i] <= L'9')
      put(static_cast<char>(text[i++]));
  }
  out[n] = '\0';
  *consumed = i;
  return n;
}
} // namespace

std::int64_t wide_toi64(const wchar_t *text) {
  if (!text)
    return 0;
  bool negative = false;
  std::size_t i = 0;
  while (text[i] == L' ' || text[i] == L'\t' || text[i] == L'\n' ||
         text[i] == L'\r')
    ++i;
  if (text[i] == L'-' || text[i] == L'+') {
    negative = text[i] == L'-';
    ++i;
  }
  std::int64_t value = 0;
  while (text[i] >= L'0' && text[i] <= L'9') {
    value = value * 10 + (text[i] - L'0');
    ++i;
  }
  return negative ? -value : value;
}

float wide_tof(const wchar_t *text) {
  char narrow[128];
  std::size_t consumed = 0;
  if (!text || copy_numeric_prefix(narrow, sizeof(narrow), text, &consumed) == 0)
    return 0.0f;
  return std::strtof(narrow, nullptr);
}

double wide_tod(const wchar_t *text, const wchar_t **endptr) {
  char narrow[128];
  std::size_t consumed = 0;
  if (!text) {
    if (endptr)
      *endptr = nullptr;
    return 0.0;
  }
  if (copy_numeric_prefix(narrow, sizeof(narrow), text, &consumed) == 0) {
    if (endptr)
      *endptr = text;
    return 0.0;
  }
  const double value = std::strtod(narrow, nullptr);
  if (endptr)
    *endptr = text + consumed;
  return value;
}

int wide_format(wchar_t *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = wide_vformat(out, format, args);
  va_end(args);
  return written;
}

int wide_strlen(const wchar_t *text) {
  int length = 0;
  while (text[length])
    ++length;
  return length;
}

int wide_strcmp(const wchar_t *a, const wchar_t *b) {
  int i = 0;
  while (a[i] && a[i] == b[i])
    ++i;
  return static_cast<int>(a[i]) - static_cast<int>(b[i]);
}

int wide_strncmp(const wchar_t *a, const wchar_t *b, int n) {
  for (int i = 0; i < n; ++i) {
    if (a[i] != b[i])
      return static_cast<int>(a[i]) - static_cast<int>(b[i]);
    if (!a[i])
      return 0;
  }
  return 0;
}

const wchar_t *wide_strstr(const wchar_t *hay, const wchar_t *needle) {
  if (!needle[0])
    return hay;
  for (int i = 0; hay[i]; ++i) {
    int j = 0;
    while (needle[j] && hay[i + j] == needle[j])
      ++j;
    if (!needle[j])
      return hay + i;
  }
  return nullptr;
}

const wchar_t *wide_strrchr(const wchar_t *text, wchar_t ch) {
  const wchar_t *last = nullptr;
  for (int i = 0; text[i]; ++i)
    if (text[i] == ch)
      last = text + i;
  return last;
}

int wide_strcspn(const wchar_t *text, const wchar_t *set) {
  int span = 0;
  while (text[span]) {
    int i = 0;
    while (set[i] && set[i] != text[span])
      ++i;
    if (set[i])
      break;
    ++span;
  }
  return span;
}

int wide_casecmp(const wchar_t *a, const wchar_t *b) {
  int i = 0;
  while (a[i] && b[i]) {
    wchar_t ca = a[i] >= L'a' && a[i] <= L'z' ? static_cast<wchar_t>(a[i] - 32) : a[i];
    wchar_t cb = b[i] >= L'a' && b[i] <= L'z' ? static_cast<wchar_t>(b[i] - 32) : b[i];
    if (ca != cb)
      return static_cast<int>(ca) - static_cast<int>(cb);
    ++i;
  }
  return static_cast<int>(a[i]) - static_cast<int>(b[i]);
}

int wide_toi(const wchar_t *text) {
  if (!text)
    return 0;
  bool negative = false;
  std::size_t i = 0;
  while (text[i] == L' ' || text[i] == L'\t' || text[i] == L'\n' ||
         text[i] == L'\r')
    ++i;
  if (text[i] == L'-' || text[i] == L'+') {
    negative = text[i] == L'-';
    ++i;
  }
  long value = 0;
  while (text[i] >= L'0' && text[i] <= L'9') {
    value = value * 10 + (text[i] - L'0');
    ++i;
  }
  return static_cast<int>(negative ? -value : value);
}


void wide_strupr(wchar_t *text) {
  for (wchar_t *p = text; *p; ++p)
    if (*p >= L'a' && *p <= L'z')
      *p = static_cast<wchar_t>(*p - 32);
}

void wide_strlwr(wchar_t *text) {
  for (wchar_t *p = text; *p; ++p)
    if (*p >= L'A' && *p <= L'Z')
      *p = static_cast<wchar_t>(*p + 32);
}

void narrow_strupr(char *text) {
  for (char *p = text; *p; ++p)
    if (*p >= 'a' && *p <= 'z')
      *p = static_cast<char>(*p - 32);
}

void narrow_strlwr(char *text) {
  for (char *p = text; *p; ++p)
    if (*p >= 'A' && *p <= 'Z')
      *p = static_cast<char>(*p + 32);
}

int wide_sprintf(wchar_t *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = wide_vformat(out, format, args);
  va_end(args);
  return written;
}

int wide_vsprintf(wchar_t *out, const wchar_t *format, va_list args) {
  return wide_vformat(out, format, args);
}

namespace {
// wide_sscanf core: walks format and input, converting the supported specs.
int wide_scan_core(const wchar_t *&text, const wchar_t *format, va_list args,
                   bool &matching) {
  int converted = 0;
  while (*format && matching) {
    if (iswspace(*format)) {
      while (iswspace(*text))
        ++text;
      ++format;
      continue;
    }
    if (*format != L'%') {
      if (*text != *format) {
        matching = false;
        return converted;
      }
      ++text;
      ++format;
      continue;
    }
    ++format;
    int width = 0;
    while (*format >= L'0' && *format <= L'9')
      width = width * 10 + (*format++ - L'0');
    bool skip = false;
    if (*format == L'*') {
      skip = true;
      ++format;
    }
    const wchar_t spec = *format++;
    switch (spec) {
    case L'd':
    case L'i':
    case L'u':
    case L'x': {
      while (iswspace(*text))
        ++text;
      bool negative = false;
      if (spec != L'x' && (*text == L'-' || *text == L'+')) {
        negative = *text == L'-';
        ++text;
      }
      long long value = 0;
      if (spec == L'x') {
        while (true) {
          wchar_t c = *text;
          int digit = -1;
          if (c >= L'0' && c <= L'9')
            digit = static_cast<int>(c - L'0');
          else if (c >= L'a' && c <= L'f')
            digit = static_cast<int>(c - L'a' + 10);
          else if (c >= L'A' && c <= L'F')
            digit = static_cast<int>(c - L'A' + 10);
          if (digit < 0)
            break;
          value = value * 16 + digit;
          ++text;
        }
      } else {
        while (*text >= L'0' && *text <= L'9') {
          value = value * 10 + (*text - L'0');
          ++text;
        }
      }
      if (negative)
        value = -value;
      if (!skip) {
        if (spec == L'u')
          *va_arg(args, unsigned *) = static_cast<unsigned>(value);
        else
          *va_arg(args, int *) = static_cast<int>(value);
        ++converted;
      }
      break;
    }
    case L'f': {
      while (iswspace(*text))
        ++text;
      double value = 0.0;
      bool any = false;
      bool negative = false;
      if (*text == L'-' || *text == L'+') {
        negative = *text == L'-';
        ++text;
      }
      while (*text >= L'0' && *text <= L'9') {
        value = value * 10 + (*text - L'0');
        ++text;
        any = true;
      }
      if (*text == L'.') {
        ++text;
        double scale = 0.1;
        while (*text >= L'0' && *text <= L'9') {
          value += (*text - L'0') * scale;
          scale *= 0.1;
          ++text;
          any = true;
        }
      }
      if (!any) {
        matching = false;
        return converted;
      }
      if (negative)
        value = -value;
      if (!skip) {
        *va_arg(args, float *) = static_cast<float>(value);
        ++converted;
      }
      break;
    }
    case L's': {
      while (iswspace(*text))
        ++text;
      if (!*text) {
        matching = false;
        return converted;
      }
      if (!skip) {
        wchar_t *out = va_arg(args, wchar_t *);
        int count = 0;
        const int limit = width > 0 ? width : 0x7FFFFFFF;
        while (*text && !iswspace(*text) && count < limit) {
          *out++ = *text++;
          ++count;
        }
        *out = L'\0';
        ++converted;
      } else {
        while (*text && !iswspace(*text))
          ++text;
      }
      break;
    }
    case L'c': {
      if (!*text) {
        matching = false;
        return converted;
      }
      if (!skip) {
        *va_arg(args, wchar_t *) = *text++;
        ++converted;
      } else {
        ++text;
      }
      break;
    }
    case L'%': {
      if (*text != L'%') {
        matching = false;
        return converted;
      }
      ++text;
      break;
    }
    default:
      matching = false;
      return converted;
    }
  }
  return converted;
}
} // namespace

wchar_t *wide_itow(int value, wchar_t *buffer, int radix) {
  if (radix < 2 || radix > 36 || !buffer)
    return buffer;
  unsigned long long magnitude =
      value < 0 ? 0ull - static_cast<unsigned long long>(value)
                : static_cast<unsigned long long>(value);
  wchar_t temp[65];
  int length = 0;
  do {
    const int digit = static_cast<int>(magnitude % static_cast<unsigned long long>(radix));
    temp[length++] = digit < 10 ? static_cast<wchar_t>(L'0' + digit)
                                : static_cast<wchar_t>(L'a' + digit - 10);
    magnitude /= static_cast<unsigned long long>(radix);
  } while (magnitude);
  int out = 0;
  if (value < 0)
    buffer[out++] = L'-';
  while (length > 0)
    buffer[out++] = temp[--length];
  buffer[out] = L'\0';
  return buffer;
}

wchar_t *wide_i64tow(long long value, wchar_t *buffer, int radix) {
  return wide_itow(value, buffer, radix);
}

int wide_sscanf(const wchar_t *text, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  bool matching = true;
  const int converted = wide_scan_core(text, format, args, matching);
  va_end(args);
  return converted;
}

wchar_t *wide_strcpy(wchar_t *destination, const wchar_t *source) {
  wchar_t *out = destination;
  while ((*out++ = *source++) != L'\0') {
  }
  return destination;
}

wchar_t *wide_strncpy(wchar_t *destination, const wchar_t *source,
                      int count) {
  wchar_t *out = destination;
  int copied = 0;
  while (copied < count && (*out++ = *source++) != L'\0')
    ++copied;
  // The wcsncpy contract: a short source is padded with terminators.
  while (copied++ < count)
    *out++ = L'\0';
  return destination;
}

wchar_t *wide_strcat(wchar_t *destination, const wchar_t *source) {
  wchar_t *out = destination;
  while (*out)
    ++out;
  while ((*out++ = *source++) != L'\0') {
  }
  return destination;
}

wchar_t *wide_strncat(wchar_t *destination, const wchar_t *source, int count) {
  wchar_t *out = destination;
  while (*out)
    ++out;
  int copied = 0;
  while (copied < count && *source) {
    *out++ = *source++;
    ++copied;
  }
  *out = L'\0';
  return destination;
}

wchar_t *wide_strchr(const wchar_t *text, wchar_t character) {
  while (*text) {
    if (*text == character)
      return const_cast<wchar_t *>(text);
    ++text;
  }
  // The wcschr contract: the terminator itself is findable.
  if (character == L'\0')
    return const_cast<wchar_t *>(text);
  return nullptr;
}

} // namespace pw153::platform
