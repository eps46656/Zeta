
#include <stdio.h>
#include <zeta/core/sha256.h>
#include <zeta/core/utils.h>

typedef unsigned long long ull;

int main() {
    Zeta_Core_SHA256Hasher hasher;
    Zeta_Core_SHA256Hasher_Init(&hasher);

    _BitInt(32) k = 0x59966565679;

    byte_t k_tmp[4];

    Zeta_Core_WriteBigEndian(k_tmp, k, 4);

    _BitInt(32) k_ = Zeta_Core_ReadBigEndian(k_tmp, 4);

    ZETA_Core_PrintVar((ull)k);
    ZETA_Core_PrintVar((ull)k_);

    byte_t tmp;

    if (1) {
        FILE* f = fopen("test_2.elf", "rb");

        for (;;) {
            tmp = fgetc(f);
            if (feof(f)) { break; }

            Zeta_Core_SHA256Hasher_Rotate(&hasher, &tmp, 1);
        }

        fclose(f);
    } else {
        // tmp = 0x53;
        // Zeta_Core_SHA256Hasher_Rotate(&hasher, &tmp, 1);
    }

    ZETA_Core_PrintVar(hasher.size);

    byte_t digits[32];

    Zeta_Core_SHA256Hasher_GetDigits(&hasher, digits);

    for (int i = 0; i < 32; ++i) { printf("%02x", digits[i]); }

    printf("\n");

    // b5123644174b7ba1d1101bb2157a5802acc664c037d52c2852aadf3f1dea905a
    // b5123644174b7ba1d1101bb2157a5802acc664c037d52c2852aadf3f1dea905a

    return 0;
}
