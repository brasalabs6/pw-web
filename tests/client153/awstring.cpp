#include "AWString.h"
#include <stdexcept>
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
int main() {
  AWString source(u"Lobo \U0001f43a"), copy(source);
  copy[0] = u'l';
  CHECK(source == u"Lobo \U0001f43a");
  CHECK(copy == u"lobo \U0001f43a");
  CHECK(source.GetLength() == 7);
  copy += u" 153";
  CHECK(copy.Find(u"153") == 8);
  auto *buffer = copy.GetBuffer(32);
  buffer[0] = u'L';
  copy.ReleaseBuffer();
  CHECK(copy == u"Lobo \U0001f43a 153");
  AWString formatted;
  formatted.Format(u"%s %d %.2f %I64d %%", u"Lobo", 42, 1.25, 1234567890123LL);
  CHECK(formatted == u"Lobo 42 1.25 1234567890123 %");
  AWString number(u"9223372036854775807");
  CHECK(number.ToInt64() == 9223372036854775807LL);
}
