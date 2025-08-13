
#include <zeta/core/debugger.h>
#include <zeta/core/utf16.h>

#define BIN_PATTERN "%c%c%c%c%c%c%c%c"

#define BIN_VAL(x)                                          \
    ((x) & 0x80 ? '1' : '0'), ((x) & 0x40 ? '1' : '0'),     \
        ((x) & 0x20 ? '1' : '0'), ((x) & 0x10 ? '1' : '0'), \
        ((x) & 0x8 ? '1' : '0'), ((x) & 0x4 ? '1' : '0'),   \
        ((x) & 0x2 ? '1' : '0'), ((x) & 0x1 ? '1' : '0')

typedef unsigned long long ull;

void Print(unsigned char* b, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        unsigned char x = b[i];
        printf(BIN_PATTERN " ", BIN_VAL(x));
    }

    printf("\n");
}

int main() {
    size_t uni_size = 512 * 1024 * 1024;

    size_t beg = 0x10000;
    size_t end = 0x110000;

    int endian = ZETA_Core_LittleEndian;

    size_t max = 0x10ffff;

    ZETA_Core_PrintVar(max);

    if (max + 1 < end) { end = max + 1; }

    uni_size = end - beg;

    ZETA_Core_PrintVar(beg);
    ZETA_Core_PrintVar(end);
    ZETA_Core_PrintVar(uni_size);

    unichar_t* data_uni = malloc(sizeof(unichar_t) * uni_size);

    unichar_t* data_uni_re = malloc(sizeof(unichar_t) * uni_size);

    byte_t* data_utf16 = malloc(sizeof(byte_t) * (uni_size * 6 + 1));

    for (size_t i = 0; i < uni_size; ++i) { data_uni[i] = beg + i; }

    Zeta_Core_UTF16_Result en_ret = Zeta_Core_UTF16_Encode(
        data_utf16, uni_size * 6 + 1, data_uni, uni_size, endian);

    ZETA_Core_DebugAssert(en_ret.success);
    ZETA_Core_DebugAssert(en_ret.src_cnt == uni_size);

    size_t utf16_size = en_ret.dst_cnt;

    Zeta_Core_UTF16_Result de_ret = Zeta_Core_UTF16_Decode(
        data_uni_re, uni_size, data_utf16, utf16_size, endian);

    ZETA_Core_DebugAssert(de_ret.success);
    ZETA_Core_DebugAssert(de_ret.dst_cnt == uni_size);
    ZETA_Core_DebugAssert(de_ret.src_cnt == utf16_size);

    for (size_t i = 0; i < uni_size; ++i) {
        ZETA_Core_DebugAssert(data_uni[i] == data_uni_re[i]);
    }

    printf("ok\n");

    return 0;
}
