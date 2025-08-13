#pragma once

#include <zeta/core/circular_array.h>

ZETA_Core_ExternC_Beg;

void SegShoveL(Zeta_Core_CircularArray* l_ca, Zeta_Core_CircularArray* r_ca,
               size_t rl_cnt, size_t ins_cnt, size_t shove_cnt);

void SegShoveR(Zeta_Core_CircularArray* l_ca, Zeta_Core_CircularArray* r_ca,
               size_t lr_cnt, size_t ins_cnt, size_t shove_cnt);

ZETA_Core_ExternC_End;
