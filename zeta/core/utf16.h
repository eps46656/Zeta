#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_UTF16_Result);

struct Zeta_Core_UTF16_Result {
    bool_t success;

    size_t dst_cnt;
    size_t src_cnt;
};

Zeta_Core_UTF16_Result Zeta_Core_UTF16_Encode(byte_t* dst, size_t dst_size,
                                              unichar_t const* src,
                                              size_t src_size, int endian);

Zeta_Core_UTF16_Result Zeta_Core_UTF16_Decode(unichar_t* dst, size_t dst_size,
                                              byte_t const* src,
                                              size_t src_size, int endian);

ZETA_Core_ExternC_End;
