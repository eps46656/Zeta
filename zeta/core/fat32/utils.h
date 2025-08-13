#pragma cone

#include <zeta/core/define.h>

ZETA_Core_DeclareStruct(ZETA_Core_FAT32_Header);

struct Zeta_Core_FAT32_Header {
    u64_t base_blk_num;

    byte_t jmp_boot[3];

    byte_t oem_name[8];

    u32_t bytes_per_sec;

    u32_t secs_per_clus;

    u32_t reserved_sec_cnt;

    u32_t fat_cnt;

    u32_t sec_cnt;

    byte_t media;

    u32_t fat_size;

    u32_t sec_per_trk;

    u32_t head_cnt;

    u32_t hidden_sec_cnt;

    u32_t ext_flags;

    u32_t fs_ver;

    u64_t root_clus;

    u32_t fs_info;

    u32_t bk_boot_sec;

    u32_t drv_num;

    u32_t vol_id;

    byte_t vol_lab[11];

    byte_t fs_type[8];
};

void Zeta_Core_FAT32_ReadHeader(Zeta_Core_FAT32_Header* dst);
