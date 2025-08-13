#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_StaticAssert(32 <= __CHAR_BIT__ * sizeof(size_t));

ZETA_Core_DeclareStruct(Zeta_Core_LCGRandomGenerator);

struct Zeta_Core_LCGRandomGenerator {
    size_t a;
    size_t c;
    size_t m;
    size_t seed;
};

void Zeta_Core_LCGRandomGenerator_Set(void* lcgrg, size_t seed);

void Zeta_Core_LCGRandomGenerator_Rotate(void* lcgrg);

size_t Zeta_Core_LCGRandomGenerator_Fetch(void* lcgrg);

ZETA_Core_ExternC_End;
