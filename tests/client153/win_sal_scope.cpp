#include "Windows.h"

#if defined(__in) || defined(__out)
#error DirectShow annotations must not leak from Windows.h
#endif

#include <iostream>

int main() {
  std::cout << "Windows.h does not leak DirectShow annotations\n";
  return 0;
}
