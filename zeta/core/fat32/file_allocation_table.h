#pragma once

#include <zeta/core/define.h>

struct Zeta_Core_FAT32_FileAllocationTable {
    //
};

unsigned long long Zeta_Core_FAT32_FileAllocationTable_GetEntry(
    void* fat, unsigned long long clus_num);

unsigned long long Zeta_Core_FAT32_FileAllocationTable_GetNxt(
    void* fat, unsigned long long clus_num);
