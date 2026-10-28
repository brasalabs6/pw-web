#include "widestr.h"
#include <cstddef>
int main() {
  volatile wchar_t text[] = {L'A', L'B', L'C', 0};
  auto input = const_cast<const wchar_t*>(text);
  auto *volatile fn = &wcslen;
  if (fn(input) != 3) return 1;
  return pw153::platform::wide_strlen(input) == 3 ? 0 : 2;
}
