#include "pw_core.h"

#include <cstdint>

extern "C" int pw_core_version() { return 1; }
extern "C" int pw_core_add(int a, int b) { return a + b; }
extern "C" std::uint32_t pw_core_read_u32le(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) {
    return static_cast<std::uint32_t>(a) |
           (static_cast<std::uint32_t>(b) << 8U) |
           (static_cast<std::uint32_t>(c) << 16U) |
           (static_cast<std::uint32_t>(d) << 24U);
}
