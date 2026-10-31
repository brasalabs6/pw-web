#include "utf16.h"
#include <stdexcept>
#include <string>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
  std::u16string wide;
  std::string bytes;
  CHECK(pw153::platform::utf8_to_utf16("Ol\xc3\xa1 \xf0\x9f\x90\xba", wide));
  CHECK(wide == u"Olá \U0001f43a");
  CHECK(pw153::platform::utf16_to_utf8(wide, bytes));
  CHECK(bytes == "Ol\xc3\xa1 \xf0\x9f\x90\xba");
  const std::string embedded("a\0b", 3);
  CHECK(pw153::platform::utf8_to_utf16(embedded, wide));
  CHECK(wide == std::u16string(u"a\0b", 3));
  CHECK(pw153::platform::utf16_to_utf8(wide, bytes) && bytes == embedded);
  CHECK(!pw153::platform::utf8_to_utf16("\xc0\xaf", wide));
  CHECK(!pw153::platform::utf8_to_utf16("\xf4\x90\x80\x80", wide));
  CHECK(!pw153::platform::utf8_to_utf16("\xed\xa0\x80", wide));
  CHECK(wide.empty());
  CHECK(!pw153::platform::utf16_to_utf8(std::u16string(1, 0xd800), bytes));
  CHECK(bytes.empty());
}
