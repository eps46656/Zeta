#include <zeta/core/disk_info.h>

int Zeta_Core_DiskInfo_CHS2LBA(Zeta_Core_DiskInfo* disk_info, size_t* dst_lba,
                               size_t c, size_t h, size_t s) {
    ZETA_Core_DebugAssert(disk_info != NULL);
    ZETA_Core_DebugAssert(dst_lba != NULL);

    if (disk_info->num_of_cylinders <= c) { return -1; }
    if (disk_info->heads_per_cylinder <= h) { return -1; }
    if (disk_info->secs_per_track <= s - 1) { return -1; }

    *dst_lba =
        disk_info->secs_per_track * (disk_info->heads_per_cylinder * c + h) +
        s - 1;

    return 0;
}

int Zeta_Core_DiskInfo_LBA2CHS(Zeta_Core_DiskInfo* disk_info, size_t* dst_chs,
                               size_t lba) {
    ZETA_Core_DebugAssert(disk_info != NULL);
    ZETA_Core_DebugAssert(dst_chs != NULL);

    size_t s = lba % disk_info->secs_per_track + 1;
    size_t tmp = lba / disk_info->secs_per_track;
    size_t h = tmp % disk_info->heads_per_cylinder;
    size_t c = tmp / disk_info->heads_per_cylinder;

    if (disk_info->num_of_cylinders <= c) { return -1; }

    dst_chs[0] = c;
    dst_chs[1] = h;
    dst_chs[2] = s;

    return 0;
}
