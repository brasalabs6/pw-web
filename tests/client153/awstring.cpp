#include "AWString.h"
#include <stdexcept>
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)
int main() {
  AWString source(L"Lobo \U0001f43a"), copy(source);
  copy[0] = u'l';
  CHECK(source == L"Lobo \U0001f43a");
  CHECK(copy == L"lobo \U0001f43a");
  CHECK(source.GetLength() == 7);
  copy += L" 153";
  CHECK(copy.Find(L"153") == 8);
  auto *buffer = copy.GetBuffer(32);
  buffer[0] = u'L';
  copy.ReleaseBuffer();
  CHECK(copy == L"Lobo \U0001f43a 153");
  AWString formatted;
  formatted.Format(L"%s %d %.2f %I64d %%", L"Lobo", 42, 1.25, 1234567890123LL);
  CHECK(formatted == L"Lobo 42 1.25 1234567890123 %");
  AWString number(L"9223372036854775807");
  CHECK(number.ToInt64() == 9223372036854775807LL);
}
