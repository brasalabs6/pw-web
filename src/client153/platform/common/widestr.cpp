#include "widestr.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

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
void emit_numeric(wchar_t *&out, const Spec &spec, const char *conversion,
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
  widen_copy(out, narrow, len);
  out += len;
}

void pad_and_emit(wchar_t *&out, const wchar_t *body, std::size_t bodyLen,
                  const Spec &spec) {
  const int width = spec.width;
  const std::size_t padding =
      width > 0 && static_cast<std::size_t>(width) > bodyLen
          ? static_cast<std::size_t>(width) - bodyLen
          : 0;
  if (!spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      *out++ = L' ';
  for (std::size_t i = 0; i < bodyLen; ++i)
    *out++ = body[i];
  if (spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      *out++ = L' ';
}

} // namespace

int wide_vformat(wchar_t *out, const wchar_t *format, va_list args) {
  va_list list;
  va_copy(list, args);
  wchar_t *cursorOut = out;
  const wchar_t *cursor = format;
  while (*cursor) {
    if (*cursor != L'%') {
      *cursorOut++ = *cursor++;
      continue;
    }
    ++cursor;
    if (*cursor == L'%') {
      *cursorOut++ = L'%';
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
      if (!value) {
        const wchar_t nullText[] = L"(null)";
        std::memcpy(body, nullText, sizeof(nullText) - sizeof(nullText[0]));
        bodyLen = 6;
      } else {
        bodyLen = wide_length(value);
        if (spec.hasPrecision && spec.precision >= 0 &&
            static_cast<std::size_t>(spec.precision) < bodyLen)
          bodyLen = static_cast<std::size_t>(spec.precision);
        std::memcpy(body, value, bodyLen * sizeof(wchar_t));
      }
      break;
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
        widen_copy(body, value, len);
        bodyLen = len;
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
      widen_copy(cursorOut, narrow, len);
      cursorOut += len;
      continue;
    }
    case L'n': {
      int *target = va_arg(list, int *);
      if (target)
        *target = static_cast<int>(cursorOut - out);
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
  *cursorOut = L'\0';
  va_end(list);
  return static_cast<int>(cursorOut - out);
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

} // namespace pw153::platform
