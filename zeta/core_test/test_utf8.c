
#include <zeta/core/debugger.h>
#include <zeta/core/utf8.h>

#define BIN_PATTERN "%c%c%c%c%c%c%c%c"

#define BIN_VAL(x)                                          \
    ((x) & 0x80 ? '1' : '0'), ((x) & 0x40 ? '1' : '0'),     \
        ((x) & 0x20 ? '1' : '0'), ((x) & 0x10 ? '1' : '0'), \
        ((x) & 0x8 ? '1' : '0'), ((x) & 0x4 ? '1' : '0'),   \
        ((x) & 0x2 ? '1' : '0'), ((x) & 0x1 ? '1' : '0')

typedef unsigned long long ULL;

void Print(unsigned char* b, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        unsigned char x = b[i];
        printf(BIN_PATTERN " ", BIN_VAL(x));
    }

    printf("\n");
}

int main() {
    ZETA_Core_PrintVar("UTF-8 test\n");

    size_t uni_size = 512 * 1024 * 1024;

    size_t beg = 1610612736;
    size_t end = beg + uni_size;

    size_t max = 0x7fffffff;

    ZETA_Core_PrintVar(max);

    if (max + 1 < end) { end = max + 1; }

    uni_size = end - beg;

    ZETA_Core_PrintVar((ULL)end);

    unichar_t* data_uni = malloc(sizeof(unichar_t) * uni_size);

    unichar_t* data_uni_re = malloc(sizeof(unichar_t) * uni_size);

    byte_t* data_utf8 = malloc(sizeof(byte_t) * (uni_size * 6 + 1));

    for (size_t i = 0; i < uni_size; ++i) { data_uni[i] = beg + i; }

    Zeta_Core_UTF8_Result en_ret =
        Zeta_Core_UTF8_Encode(data_utf8, uni_size * 6 + 1, data_uni, uni_size);

    ZETA_Core_DebugAssert(en_ret.success);
    ZETA_Core_DebugAssert(en_ret.src_cnt == uni_size);

    size_t utf8_size = en_ret.dst_cnt;

    Zeta_Core_UTF8_Result de_ret =
        Zeta_Core_UTF8_Decode(data_uni_re, uni_size, data_utf8, utf8_size);

    ZETA_Core_DebugAssert(de_ret.success);
    ZETA_Core_DebugAssert(de_ret.dst_cnt == uni_size);
    ZETA_Core_DebugAssert(de_ret.src_cnt == utf8_size);

    for (size_t i = 0; i < uni_size; ++i) {
        ZETA_Core_DebugAssert(data_uni[i] == data_uni_re[i]);
    }

    printf("ok\n");

    return 0;
}
