
#include <stdio.h>
#include <zeta/core/crc.h>
#include <zeta/core/utils.h>

typedef unsigned long long ull;

ull Reverse(ull x) {
    ull ret = 0;

    for (int i = 0; i < 32; ++i, x /= 2) {
        if (x % 2 == 0) {
            ret = ret * 2;
        } else {
            ret = ret * 2 + 1;
        }
    }

    return ret;
}

int main() {
    Zeta_Core_CRCHasher hasher;
    // Zeta_Core_CRCHasher_Init(&hasher, 0xEDB88320, 0);
    // Zeta_Core_CRCHasher_Init(&hasher, 0xEDB88320, 0xFFFFFFFF);
    Zeta_Core_CRCHasher_Init(&hasher, 0x04C11DB7, 0xFFFFFFFF);

    byte_t tmp;

    if (0) {
        FILE* f = fopen("test_2.elf", "rb");

        for (;;) {
            tmp = fgetc(f);
            if (feof(f)) { break; }

            Zeta_Core_CRCHasher_Rotate(&hasher, &tmp, 1, 1);
        }

        fclose(f);
    } else {
        tmp = 0x53;
        Zeta_Core_CRCHasher_Rotate(&hasher, &tmp, 1, 1);
    }

    ZETA_Core_PrintVar("%08x", (ull)hasher.reg);
    ZETA_Core_PrintVar("%08x", 0xFFFFFFFF ^ (ull)hasher.reg);
    ZETA_Core_PrintVar("%08x", Reverse((ull)hasher.reg));
    ZETA_Core_PrintVar("%08x", Reverse(0xFFFFFFFF ^ (ull)hasher.reg));

    // b5123644174b7ba1d1101bb2157a5802acc664c037d52c2852aadf3f1dea905a
    // b5123644174b7ba1d1101bb2157a5802acc664c037d52c2852aadf3f1dea905a

    return 0;
}
