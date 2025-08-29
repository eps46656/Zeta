#include <zeta/core/integer.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

unsigned long long ULLHash(unsigned long long x, unsigned long long salt) {
    x ^= salt;

#if ZETA_Core_ullong_width == 32
    x = (x ^ (x >> 16)) * 0x45d9f3bULL;
    x = (x ^ (x >> 16)) * 0x45d9f3bULL;
    x = x ^ (x >> 16);
#elif ZETA_Core_ullong_width == 64
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x = x ^ (x >> 31);
#else
#error "Unsupported architecture."
#endif

    x ^= salt;

    return x;
}

unsigned long long SLLHash(long long x, unsigned long long salt) {
    return ULLHash(x, salt);
}

#if ZETA_Core_ullong_width == 32
#define LCG_MUL (0x1010101ULL)
#define LCG_INC (0x24924907ULL)
#elif ZETA_Core_ullong_width == 64
#define LCG_MUL (0x1010101ULL)
#define LCG_INC (0x2492492492492479ULL)
#else
#error "Unsupported architecture."
#endif

unsigned long long random_seed_{ ZETA_Core_PtrToAddr(&random_seed_) - 1 };

unsigned long long GetRandom() {
    random_seed_ =
        ((random_seed_ + __builtin_readcyclecounter()) * LCG_MUL + LCG_INC);

    return ULLHash(random_seed_, __builtin_readcyclecounter());
}

unsigned long long SimpleRandomRotate(unsigned long long* x) {
    return ULLHash(*x = (*x * LCG_MUL + LCG_INC), 0);
}

// -----------------------------------------------------------------------------

int Choose2(bool cond0, bool cond1, unsigned long long* random_seed) {
    ZETA_Core_DebugAssert(cond0 || cond1);

    switch (static_cast<int>(cond1) * 2 + static_cast<int>(cond0)) {
        case 0b01: return 0;
        case 0b10: return 1;
        default: return static_cast<int>(SimpleRandomRotate(random_seed) % 2);
    }
}

int Choose3(bool cond0, bool cond1, bool cond2,
            unsigned long long* random_seed) {
    ZETA_Core_DebugAssert(cond0 || cond1 || cond2);

    switch (static_cast<int>(cond2) * 4 + static_cast<int>(cond1) * 2 +
            static_cast<int>(cond0)) {
        case 0b001: return 0;
        case 0b010: return 1;
        case 0b100: return 2;
        case 0b011:
            return static_cast<int>(SimpleRandomRotate(random_seed) % 2);
        case 0b101:
            return static_cast<int>(SimpleRandomRotate(random_seed) % 2) * 2;
        case 0b110:
            return static_cast<int>(SimpleRandomRotate(random_seed) % 2) + 1;
        default: return static_cast<int>(SimpleRandomRotate(random_seed) % 3);
    }
}

}  // namespace zeta::core
