#include <zeta/core/debugger.h>
#include <zeta/core/utf16.h>
#include <zeta/core/utils.h>

static byte_t* EncodeChar_(byte_t* dst, unichar_t data, int endian) {
    if (data < 0) { return NULL; }

    if (data < 0xd800) {
        ZETA_Core_UIntToBytes(data, dst, 2, endian);
        return dst + 2;
    }

    if (data < 0xe000) { return NULL; }

    if (data < 0x10000) {
        ZETA_Core_UIntToBytes(data, dst, 2, endian);
        return dst + 2;
    }

    if (0x110000 <= data) { return NULL; }

    data -= 0x10000;

    ZETA_Core_UIntToBytes(data / 0x400 + 0xd800, dst, 2, endian);
    ZETA_Core_UIntToBytes(data % 0x400 + 0xdc00, dst + 2, 2, endian);

    return dst + 4;
}

Zeta_Core_UTF16_Result Zeta_Core_UTF16_Encode(byte_t* dst, size_t dst_size,
                                              unichar_t const* src,
                                              size_t src_size, int endian) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    ZETA_Core_DebugAssert(endian == ZETA_Core_LittleEndian ||
                          endian == ZETA_Core_BigEndian);

    byte_t tmp[8];

    size_t dst_cnt = 0;
    unichar_t const* origin_src = src;

    for (; dst_cnt < dst_size && 0 < src_size; ++src, --src_size) {
        byte_t* tmp_p = EncodeChar_(tmp, *src, endian);

        if (tmp_p == NULL) {
            return (Zeta_Core_UTF16_Result){
                .success = FALSE,
                .dst_cnt = dst_cnt,
                .src_cnt = src - origin_src,
            };
        }

        size_t tmp_size = tmp_p - tmp;

        if (dst_size - dst_cnt < tmp_size) {
            return (Zeta_Core_UTF16_Result){
                .success = TRUE,
                .dst_cnt = dst_cnt,
                .src_cnt = src - origin_src,
            };
        }

        if (dst != NULL) {
            Zeta_Core_MemCopy(dst, tmp, tmp_size);
            dst += tmp_size;
        }

        dst_cnt += tmp_size;
    }

    return (Zeta_Core_UTF16_Result){
        .success = TRUE,
        .dst_cnt = dst_cnt,
        .src_cnt = src - origin_src,
    };
}

static byte_t const* DecodeChar_(unichar_t* dst, byte_t const* src,
                                 size_t src_size, int endian) {
    if (src_size < 2) { return src; }

    unichar_t x = ZETA_Core_BytesToUInt(unichar_t, src, 2, endian);

    if (0xffff < x) { return NULL; }

    if (x < 0xd800 || 0xdfff < x) {
        *dst = x;
        return src + 2;
    }

    if (0xdbff < x) { return NULL; }

    if (src_size < 4) { return src; }

    unichar_t y = ZETA_Core_BytesToUInt(unichar_t, src + 2, 2, endian);

    if (y < 0xdc00 || 0xdfff < y) { return NULL; }

    *dst = (x - 0xd800) * 0x400 + (y - 0xdc00) + 0x10000;

    return src + 4;
}

Zeta_Core_UTF16_Result Zeta_Core_UTF16_Decode(unichar_t* dst, size_t dst_size,
                                              byte_t const* src,
                                              size_t src_size, int endian) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    ZETA_Core_DebugAssert(endian == ZETA_Core_LittleEndian ||
                          endian == ZETA_Core_BigEndian);

    size_t dst_cnt = 0;
    byte_t const* origin_src = src;

    while (dst_cnt < dst_size && 0 < src_size) {
        unichar_t tmp;

        byte_t const* nxt_src = DecodeChar_(&tmp, src, src_size, endian);

        if (nxt_src == NULL) {
            return (Zeta_Core_UTF16_Result){
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

    return (Zeta_Core_UTF16_Result){
        .success = TRUE,
        .dst_cnt = dst_cnt,
        .src_cnt = src - origin_src,
    };
}
