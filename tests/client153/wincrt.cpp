#include "Windows.h"
#include <stdexcept>

int main() {
  if (stricmp("Login", "lOgIn") != 0) throw std::runtime_error("case-insensitive equality");
  if (stricmp("alpha", "BETA") >= 0 || stricmp("zeta", "Beta") <= 0)
    throw std::runtime_error("case-insensitive ordering");
}
