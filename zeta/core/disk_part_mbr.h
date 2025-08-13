#pragma once

#include <zeta/core/disk_info.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DiskPartMBR_MBR);
ZETA_Core_DeclareStruct(Zeta_Core_DiskPartMBR_PartEntry);

#define ZETA_Core_DiskPartMBR_size_of_bootstrap (446)

struct Zeta_Core_DiskPartMBR_PartEntry {
    byte_t state;
    byte_t type;
    u64_t beg;
    u64_t end;
};

struct Zeta_Core_DiskPartMBR_MBR {
    byte_t bootstrap[ZETA_Core_DiskPartMBR_size_of_bootstrap];
    Zeta_Core_DiskPartMBR_PartEntry part_entries[4];
};

byte_t const* Zeta_Core_DiskPartMBR_ReadMBR(Zeta_Core_DiskPartMBR_MBR* dst,
                                            Zeta_Core_DiskInfo const* disk_info,
                                            byte_t const* data,
                                            size_t data_size);

byte_t* Zeta_Core_DiskPartMBR_WriteMBR(byte_t* dst, size_t dst_size,
                                       Zeta_Core_DiskInfo const* disk_info,
                                       Zeta_Core_DiskPartMBR_MBR* mbr);

ZETA_Core_ExternC_End;
