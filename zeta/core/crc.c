#include <zeta/core/crc.h>
#include <zeta/core/debugger.h>

void Zeta_Core_CRCHasher_Init(void* hasher_, ubit64_t div, ubit64_t reg) {
    Zeta_Core_CRCHasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);

    hasher->div = div;
    hasher->reg = reg;
}

ubit64_t Zeta_Core_CRCHasher_GetResult(void* hasher_) {
    Zeta_Core_CRCHasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);
    return hasher->reg;
}

void Zeta_Core_CRCHasher_Rotate(void* hasher_, size_t cnt,
                                unsigned char const* data, int reverse) {
    Zeta_Core_CRCHasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);

    ZETA_Core_DebugAssert(data != NULL);

    ubit64_t div = hasher->div;
    ubit64_t reg = hasher->reg;

    for (; 0 < cnt--; ++data) {
        unsigned char x = data[0];
        ZETA_Core_DebugAssert(0 <= x);
        ZETA_Core_DebugAssert(x <= 255);

        if (reverse) {
            reg ^= __builtin_bitreverse8(x);
        } else {
            reg ^= x;
        }

        for (int i = 0; i < 8; ++i) {
            if ((reg & 1) == 0) {
                reg >>= 1;
            } else {
                reg = (reg >> 1) ^ div;
            }
        }
    }

    hasher->reg = reg;
}
