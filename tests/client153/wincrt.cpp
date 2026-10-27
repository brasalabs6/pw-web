#include "Windows.h"
#include "dsound.h"
#include <cstddef>
#include <stdexcept>

static_assert(sizeof(WAVEFORMAT) == 14);
static_assert(sizeof(PCMWAVEFORMAT) == 16);
static_assert(offsetof(PCMWAVEFORMAT, wBitsPerSample) == 14);
static_assert(sizeof(WAVEFORMATEX) == 18);

int main() {
  if (stricmp("Login", "lOgIn") != 0) throw std::runtime_error("case-insensitive equality");
  if (stricmp("alpha", "BETA") >= 0 || stricmp("zeta", "Beta") <= 0)
    throw std::runtime_error("case-insensitive ordering");
  if (strcmpi("Align", "aLIGN") != 0 || strcmpi("Alpha", "beta") >= 0)
    throw std::runtime_error("strcmpi case-insensitive comparison");
}
