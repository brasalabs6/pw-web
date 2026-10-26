#include "widestr.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace pw153::platform {
namespace {

std::size_t wide_length(const char16_t *text) {
  std::size_t length = 0;
  while (text[length])
    ++length;
  return length;
}

void widen_copy(char16_t *out, const char *narrow, std::size_t count) {
  for (std::size_t i = 0; i < count; ++i)
    out[i] = static_cast<char16_t>(static_cast<unsigned char>(narrow[i]));
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
char16_t parse_spec(const char16_t *&cursor, Spec &spec, va_list *args) {
  spec = Spec{false, false, false, false, false, 0, 0, false, 0};
  while (true) {
    const char16_t c = *cursor;
    if (c == u'-')
      spec.left = true;
    else if (c == u'+')
      spec.plus = true;
    else if (c == u' ')
      spec.space = true;
    else if (c == u'0')
      spec.zero = true;
    else if (c == u'#')
      spec.hash = true;
    else
      break;
    ++cursor;
  }
  if (*cursor == u'*') {
    spec.width = va_arg(*args, int);
    ++cursor;
  } else
    while (*cursor >= u'0' && *cursor <= u'9')
      spec.width = spec.width * 10 + (*cursor++ - u'0');
  if (*cursor == u'.') {
    ++cursor;
    spec.hasPrecision = true;
    if (*cursor == u'*') {
      spec.precision = va_arg(*args, int);
      ++cursor;
    } else
      while (*cursor >= u'0' && *cursor <= u'9')
        spec.precision = spec.precision * 10 + (*cursor++ - u'0');
  }
  if (*cursor == u'h') {
    spec.size = 1;
    ++cursor;
  } else if (*cursor == u'l' || *cursor == u'F' || *cursor == u'N') {
    spec.size = 2;
    ++cursor;
  } else if (*cursor == u'I') {
    ++cursor;
    if (cursor[0] == u'6' && cursor[1] == u'4') {
      spec.size = 3;
      cursor += 2;
    } else if (cursor[0] == u'3' && cursor[1] == u'2') {
      spec.size = 0;
      cursor += 2;
    }
  }
  return *cursor++;
}

// Composes "%<flags>*.*<conv>" and formats through the narrow CRT, which
// applies width/precision/padding; the ASCII result is then widened.
template <typename Value>
void emit_numeric(char16_t *&out, const Spec &spec, const char *conversion,
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

void pad_and_emit(char16_t *&out, const char16_t *body, std::size_t bodyLen,
                  const Spec &spec) {
  const int width = spec.width;
  const std::size_t padding =
      width > 0 && static_cast<std::size_t>(width) > bodyLen
          ? static_cast<std::size_t>(width) - bodyLen
          : 0;
  if (!spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      *out++ = u' ';
  for (std::size_t i = 0; i < bodyLen; ++i)
    *out++ = body[i];
  if (spec.left)
    for (std::size_t i = 0; i < padding; ++i)
      *out++ = u' ';
}

} // namespace

int wide_vformat(char16_t *out, const char16_t *format, va_list args) {
  va_list list;
  va_copy(list, args);
  char16_t *cursorOut = out;
  const char16_t *cursor = format;
  while (*cursor) {
    if (*cursor != u'%') {
      *cursorOut++ = *cursor++;
      continue;
    }
    ++cursor;
    if (*cursor == u'%') {
      *cursorOut++ = u'%';
      ++cursor;
      continue;
    }
    Spec spec;
    const char16_t conversion = parse_spec(cursor, spec, &list);
    char16_t body[160];
    std::size_t bodyLen = 0;
    switch (conversion) {
    case u'c':
    case u'C': {
      //	Characters arrive through the default argument promotion.
      body[0] = static_cast<char16_t>(va_arg(list, int));
      bodyLen = 1;
      break;
    }
    case u's': {
      const char16_t *value = va_arg(list, const char16_t *);
      if (!value) {
        const char16_t nullText[] = u"(null)";
        std::memcpy(body, nullText, sizeof(nullText) - sizeof(nullText[0]));
        bodyLen = 6;
      } else {
        bodyLen = wide_length(value);
        if (spec.hasPrecision && spec.precision >= 0 &&
            static_cast<std::size_t>(spec.precision) < bodyLen)
          bodyLen = static_cast<std::size_t>(spec.precision);
        std::memcpy(body, value, bodyLen * sizeof(char16_t));
      }
      break;
    }
    case u'S': {
      //	MSVC wide printf: %S is the narrow string in a UNICODE build.
      const char *value = va_arg(list, const char *);
      if (!value) {
        const char16_t nullText[] = u"(null)";
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
    case u'd':
    case u'i':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "lld", va_arg(list, std::int64_t));
      else
        emit_numeric(cursorOut, spec, "d", va_arg(list, int));
      continue;
    case u'u':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llu", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "u", va_arg(list, unsigned));
      continue;
    case u'x':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llx", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "x", va_arg(list, unsigned));
      continue;
    case u'X':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llX", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "X", va_arg(list, unsigned));
      continue;
    case u'o':
      if (spec.size == 3)
        emit_numeric(cursorOut, spec, "llo", va_arg(list, std::uint64_t));
      else
        emit_numeric(cursorOut, spec, "o", va_arg(list, unsigned));
      continue;
    case u'f':
    case u'e':
    case u'E':
    case u'g':
    case u'G': {
      const double value = va_arg(list, double);
      char conv[2] = {static_cast<char>(conversion), 0};
      emit_numeric(cursorOut, spec, conv, value);
      continue;
    }
    case u'p': {
      void *value = va_arg(list, void *);
      char narrow[32];
      std::snprintf(narrow, sizeof(narrow), "%p", value);
      const std::size_t len = std::strlen(narrow);
      widen_copy(cursorOut, narrow, len);
      cursorOut += len;
      continue;
    }
    case u'n': {
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
  *cursorOut = u'\0';
  va_end(list);
  return static_cast<int>(cursorOut - out);
}

namespace {
// Copies the numeric prefix (sign, digits, '.', exponent) into a narrow
// buffer for the narrow CRT; returns the copied length.
std::size_t copy_numeric_prefix(char *out, std::size_t capacity,
                                const char16_t *text, std::size_t *consumed) {
  std::size_t i = 0, n = 0;
  auto put = [&](char c) {
    if (n + 1 < capacity)
      out[n++] = c;
  };
  while (text[i] == u' ' || text[i] == u'\t' || text[i] == u'\n' ||
         text[i] == u'\r')
    ++i;
  if (text[i] == u'+' || text[i] == u'-')
    put(static_cast<char>(text[i++]));
  while (text[i] >= u'0' && text[i] <= u'9')
    put(static_cast<char>(text[i++]));
  if (text[i] == u'.') {
    put(static_cast<char>(text[i++]));
    while (text[i] >= u'0' && text[i] <= u'9')
      put(static_cast<char>(text[i++]));
  }
  if (text[i] == u'e' || text[i] == u'E') {
    put(static_cast<char>(text[i++]));
    if (text[i] == u'+' || text[i] == u'-')
      put(static_cast<char>(text[i++]));
    while (text[i] >= u'0' && text[i] <= u'9')
      put(static_cast<char>(text[i++]));
  }
  out[n] = '\0';
  *consumed = i;
  return n;
}
} // namespace

std::int64_t wide_toi64(const char16_t *text) {
  if (!text)
    return 0;
  bool negative = false;
  std::size_t i = 0;
  while (text[i] == u' ' || text[i] == u'\t' || text[i] == u'\n' ||
         text[i] == u'\r')
    ++i;
  if (text[i] == u'-' || text[i] == u'+') {
    negative = text[i] == u'-';
    ++i;
  }
  std::int64_t value = 0;
  while (text[i] >= u'0' && text[i] <= u'9') {
    value = value * 10 + (text[i] - u'0');
    ++i;
  }
  return negative ? -value : value;
}

float wide_tof(const char16_t *text) {
  char narrow[128];
  std::size_t consumed = 0;
  if (!text || copy_numeric_prefix(narrow, sizeof(narrow), text, &consumed) == 0)
    return 0.0f;
  return std::strtof(narrow, nullptr);
}

double wide_tod(const char16_t *text, const char16_t **endptr) {
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

int wide_format(char16_t *out, const char16_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = wide_vformat(out, format, args);
  va_end(args);
  return written;
}

int wide_toi(const char16_t *text) {
  if (!text)
    return 0;
  bool negative = false;
  std::size_t i = 0;
  while (text[i] == u' ' || text[i] == u'\t' || text[i] == u'\n' ||
         text[i] == u'\r')
    ++i;
  if (text[i] == u'-' || text[i] == u'+') {
    negative = text[i] == u'-';
    ++i;
  }
  long value = 0;
  while (text[i] >= u'0' && text[i] <= u'9') {
    value = value * 10 + (text[i] - u'0');
    ++i;
  }
  return static_cast<int>(negative ? -value : value);
}

} // namespace pw153::platform
