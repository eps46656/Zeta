#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_UTF8_Result);

/**
 * @brief Indicate the encode result.
 */
struct Zeta_Core_UTF8_Result {
    bool_t success;

    size_t dst_cnt;
    size_t src_cnt;
};

Zeta_Core_UTF8_Result Zeta_Core_UTF8_Encode(byte_t* dst, size_t dst_size,
                                            unichar_t const* src,
                                            size_t src_size);

Zeta_Core_UTF8_Result Zeta_Core_UTF8_Decode(unichar_t* dst, size_t dst_size,
                                            byte_t const* src, size_t src_size);

ZETA_Core_ExternC_End;
