// Research harness only. The upstream implementation is compiled in place;
// no Angelica implementation or game data is copied into the application.
#include "ARandomGen.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>

#ifdef __EMSCRIPTEN__
static_assert(sizeof(DWORD) == 4, "The original DWORD contract needs 32 bits");
static_assert(sizeof(void*) == 4, "This probe targets wasm32");
#endif

int main() {
    constexpr std::uint32_t seeds[] = {0, 1, 42, UINT32_MAX};
    std::printf("{\"schema\":1,\"abi\":{\"longBytes\":%zu,\"pointerBytes\":%zu},\"cases\":[\n",
                sizeof(DWORD), sizeof(void*));
    for (unsigned case_index = 0; case_index < 4; ++case_index) {
        const auto seed = seeds[case_index];
        ARandomGen random, replay, ranges;
        random.Init(seed);
        replay.Init(seed);
        ranges.Init(seed);
        std::printf("%s{\"seed\":%u,\"uniform\":[", case_index ? "," : "", seed);
        for (unsigned i = 0; i < 1024; ++i) {
            const double sample = random.RandomUniform();
            assert(std::isfinite(sample) && sample >= 0.0 && sample <= 1.0);
            assert(sample == replay.RandomUniform());
            const int integer = ranges.RandomInt(-10, 10);
            const float real = ranges.RandomFloat(-2.0f, 3.0f);
            assert(integer >= -10 && integer <= 10);
            assert(std::isfinite(real) && real >= -2.0f && real <= 3.0f);
            std::printf("%s%.17g", i ? "," : "", sample);
        }
        std::printf("],\"gaussian\":[");
        for (unsigned i = 0; i < 64; ++i) {
            const double sample = random.RandomGaussian(0.0, 1.0);
            assert(std::isfinite(sample));
            assert(sample == replay.RandomGaussian(0.0, 1.0));
            std::printf("%s%.17g", i ? "," : "", sample);
        }
        std::printf("]}\n");
    }
    std::printf("]}\n");
    return 0;
}
