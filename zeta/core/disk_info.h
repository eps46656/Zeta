#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DiskInfo);

struct Zeta_Core_DiskInfo {
    size_t sec_size;
    size_t num_of_secs;

    size_t num_of_cylinders;
    size_t heads_per_cylinder;
    size_t secs_per_track;
};

ZETA_Core_ExternC_End;
