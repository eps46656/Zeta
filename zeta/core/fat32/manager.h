#pragma once

#include <zeta/core/cache_manager.h>
#include <zeta/core/datetime.h>
#include <zeta/core/file_sys.h>
#include <zeta/core/seg_vector.h>

ZETA_Core_DeclareStruct(Zeta_Core_FAT32_Header);
ZETA_Core_DeclareStruct(Zeta_Core_FAT32_DirEntry);
ZETA_Core_DeclareStruct(Zeta_Core_FAT32_Dir);
ZETA_Core_DeclareStruct(Zeta_Core_FAT32_Manager);
ZETA_Core_DeclareStruct(Zeta_Core_FAT32_SNode);

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

struct Zeta_Core_FAT32_DirEntry {
    Zeta_Core_SegVector name;

    u8_t attr;

    u64_t first_clus_num;

    Zeta_Core_DateTime crt_datetime;
    Zeta_Core_DateTime acc_datetime;
    Zeta_Core_DateTime wrt_datetime;
};

struct Zeta_Core_FAT32_Dir {
    Zeta_Core_SegVector entries;
};

struct Zeta_Core_Eat32_NodeInfo {
    bool_t is_file;

    //
};

struct Zeta_Core_FAT32_SNode {
    void* cm_sd;

    void* buffer;
};

struct Zeta_Core_FAT32_Manager {
    Zeta_Core_FAT32_Header header;

    Zeta_CacheManager* cm;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void* Zeta_Core_FAT32_Manager_OpenSession(void* fat32_manager);

void Zeta_Core_FAT32_Manager_CloseSession(void* fat32_manager, void* session);

void Zeta_Core_FAT32_Manager_GetInfo(void* fat32_manager, void* session,
                                     size_t li,
                                     Zeta_Core_FileSys_NodeAttr* dst);

void Zeta_Core_FAT32_Manager_Open(void* fat32_manager, void* session,
                                  size_t li);

void Zeta_Core_FAT32_Manager_Close(void* fat32_manager, void* session);

void Zeta_Core_FAT32_Manager_Check(void* fat32_manager);

void Zeta_Core_FAT32_Manager_Check_Session(void* fat32_manager, void* session);

// --

/*

Some basic file system operations

GetDirectoryInfo
    Return the info of working directory.

GetInfo(filename)
    Return the info of file(dir) with filename under working directory.
    If there is not a file(dir) with filename, return a null info.

ChangeDirectory(path)
    cd command

Make(filename, type, mod)
    Make a file(dir) under working directory.

    type indicates it is a file or dir.
    mod indicated its permission.

Remove


*/
