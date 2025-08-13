#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_CRCHasher);

struct Zeta_Core_CRCHasher {
    ubit64_t div;
    ubit64_t reg;
};

void Zeta_Core_CRCHasher_Init(void* hasher, ubit64_t div, ubit64_t reg);

ubit64_t Zeta_Core_CRCHasher_GetResult(void* hasher);

void Zeta_Core_CRCHasher_Rotate(void* hasher, size_t cnt,
                                unsigned char const* src, int reverse);

ZETA_Core_ExternC_End;
