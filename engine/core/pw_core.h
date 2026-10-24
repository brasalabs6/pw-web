#ifndef PW_WEB_CORE_H
#define PW_WEB_CORE_H

#include <cstdint>

// Independent, newly written reference functions for validating C++ -> WASM.
// NOT the original Perfect World client or an implementation of its protocol.
extern "C" {
int pw_core_version();
int pw_core_add(int a, int b);
std::uint32_t pw_core_read_u32le(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d);
}

#endif
