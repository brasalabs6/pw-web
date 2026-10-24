#include "pw_core.h"
#include <cassert>
#include <cstdint>

int main() {
    assert(pw_core_version() == 1);
    assert(pw_core_add(7, 5) == 12);
    assert(pw_core_add(-7, 5) == -2);
    assert(pw_core_read_u32le(0x78, 0x56, 0x34, 0x12) == UINT32_C(0x12345678));
    assert(pw_core_read_u32le(0, 0, 0, 0) == 0U);
    return 0;
}
