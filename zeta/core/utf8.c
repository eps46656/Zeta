#include <zeta/core/debugger.h>
#include <zeta/core/utf8.h>
#include <zeta/core/utils.h>

static byte_t* EncodeChar_(byte_t* dst, unichar_t data) {
    if (data < 0 || 0x7fffffff < data) { return NULL; }

    if (data <= 0b01111111) {
        *dst = data;
        return dst + 1;
    }

    unichar_t ub = 0b00011111;
    unichar_t lead = 0b111111000000;

    byte_t tmp[6];
    byte_t* tmp_p = tmp;

    for (;; ub /= 2, lead /= 2) {
        *(tmp_p++) = 0b10000000 + data % 0b01000000;
        data /= 0b01000000;

        if (data <= ub) { break; }
    }

    *(dst++) = lead % 0b100000000 + data;

    while (tmp < tmp_p) { *(dst++) = *(--tmp_p); }

    return dst;
}

Zeta_Core_UTF8_Result Zeta_Core_UTF8_Encode(byte_t* dst, size_t dst_size,
                                            unichar_t const* src,
                                            size_t src_size) {
    ZETA_Core_DebugAssert(src != NULL);

    byte_t tmp[6];

    size_t dst_cnt = 0;
    unichar_t const* origin_src = src;

    for (; dst_cnt < dst_size && 0 < src_size; ++src, --src_size) {
        byte_t* tmp_p = EncodeChar_(tmp, *src);

        if (tmp_p == NULL) {
            return (Zeta_Core_UTF8_Result){
                .success = FALSE,
                .dst_cnt = dst_cnt,
                .src_cnt = src - origin_src,
            };
        }

        size_t tmp_size = tmp_p - tmp;

        if (dst_size - dst_cnt < tmp_size) { break; }

        if (dst != NULL) {
            Zeta_Core_MemCopy(dst, tmp, tmp_size);
            dst += tmp_size;
        }

        dst_cnt += tmp_size;
    }

    return (Zeta_Core_UTF8_Result){
        .success = TRUE,
        .dst_cnt = dst_cnt,
        .src_cnt = src - origin_src,
    };
}

static byte_t const* DecodeChar_(unichar_t* dst, byte_t const* src,
                                 size_t src_size) {
    unichar_t x = *src;

    if (x < 0) { return NULL; }

    if (x < 0b10000000) {
        *dst = x;
        return src + 1;
    }

    if (x < 0b11000000 || 0b11111101 < x) { return NULL; }

    unsigned width;
    unichar_t ret;

    if (0b11111100 <= x) {
        width = 6;
        ret = x % 0b00000010;
    } else if (0b11111000 <= x) {
        width = 5;
        ret = x % 0b00000100;
    } else if (0b11110000 <= x) {
        width = 4;
        ret = x % 0b00001000;
    } else if (0b11100000 <= x) {
        width = 3;
        ret = x % 0b00010000;
    } else {  // 0b11000000 <= x
        width = 2;
        ret = x % 0b00100000;
    }

    if (src_size < width) { return src; }

    ++src;

    for (unsigned i = 1; i < width; ++i, ++src) {
        unichar_t k = *src;

        ZETA_Core_DebugAssert(0 <= k);
        ZETA_Core_DebugAssert(k / 0b01000000 == 0b10);

        if (k < 0 || k / 0b01000000 != 0b10) { return NULL; }

        ret = ret * 0b01000000 + k % 0b01000000;
    }

    *dst = ret;

    return src;
}

Zeta_Core_UTF8_Result Zeta_Core_UTF8_Decode(unichar_t* dst, size_t dst_size,
                                            byte_t const* src,
                                            size_t src_size) {
    ZETA_Core_DebugAssert(src != NULL);

    size_t dst_cnt = 0;
    byte_t const* origin_src = src;

    while (dst_cnt < dst_size && 0 < src_size) {
        unichar_t tmp;

        byte_t const* nxt_src = DecodeChar_(&tmp, src, src_size);

        if (nxt_src == NULL) {
            return (Zeta_Core_UTF8_Result){
                .success = FALSE,
                .dst_cnt = dst_cnt,
                .src_cnt = src - origin_src,
            };
        }

        if (src == nxt_src) { break; }

        if (dst != NULL) { *(dst++) = tmp; }

        ++dst_cnt;

        src_size -= nxt_src - src;
        src = nxt_src;
    }

    return (Zeta_Core_UTF8_Result){
        .success = TRUE,
        .dst_cnt = dst_cnt,
        .src_cnt = src - origin_src,
    };
}
