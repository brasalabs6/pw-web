#include "AWString.h"
#include "widestr.h"
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
  AWString font(L"Fonte");
  wchar_t font_record[256];
  pw153::platform::wide_sprintf(font_record, L"%s###%d###%d###%d###%d",
                                static_cast<const wchar_t*>(font), 1, 2, 0, 1);
  CHECK(AWString(font_record) == L"Fonte###1###2###0###1");
  wchar_t long_text[301];
  for (int i = 0; i < 300; ++i) long_text[i] = L'x';
  long_text[300] = 0;
  wchar_t long_result[400];
  CHECK(pw153::platform::wide_sprintf(long_result, L"%s", long_text) == 300);
  CHECK(AWString(long_result) == long_text);
  char long_narrow[301];
  for (int i = 0; i < 300; ++i) long_narrow[i] = 'x';
  long_narrow[300] = 0;
  CHECK(pw153::platform::wide_sprintf(long_result, L"%S", long_narrow) == 300);
  CHECK(AWString(long_result) == long_text);
  AWString number(L"9223372036854775807");
  CHECK(number.ToInt64() == 9223372036854775807LL);
}
