#include <disk_part_gpt.h>
#include <zeta/core/debugger.h>
#include <zeta/core/utils.h>

#define READ_(tmp_ret, tmp_len, src, len)                              \
    ({                                                                 \
        ZETA_Core_AutoVar(tmp_len, (len));                             \
                                                                       \
        for (size_t tmp_i = 0; tmp_i < tmp_len; ++tmp_i) {             \
            if (src[tmp_i] < 0 || 255 < src[tmp_i]) { goto ERR_RET; }  \
        }                                                              \
                                                                       \
        u64_t tmp_ret = ZETA_Core_BytesToUInt(u64_t, (src), tmp_len,   \
                                              ZETA_Core_LittleEndian); \
                                                                       \
        (src) += tmp_len;                                              \
                                                                       \
        tmp_ret;                                                       \
    })

#define READ(src, len) \
    READ_(ZETA_Core_TmpName, ZETA_Core_TmpName, ZETA_Core_TmpName, (src), (len))

#define WRITE_(tmp_len, val, dst, len)                                    \
    ZETA_Core_AutoVar(tmp_len, (len));                                    \
    ZETA_Core_UIntToBytes((val), (dst), tmp_len, ZETA_Core_LittleEndian); \
    (dst) += tmp_len;                                                     \
    ZETA_Core_StaticAssert(TRUE)

#define WRITE(val, dst, len) WRITE_(ZETA_Core_TmpName, (val), (dst), (len))

byte_t const* Zeta_Core_DiskPartGPT_ReadHeader(
    Zeta_Core_DiskPartGPT_Header* dst, byte_t const* data, size_t data_size) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(data != NULL);

    if (data_size < 512) { return NULL; }

    char const sign[] = "EFI PART";

    for (int i = 0; i < 8; ++i, ++data) {
        if (*data != sign[i]) { return NULL; }
    }

    dst->revision_num = READ(data, 4);
    dst->header_size = READ(data, 4);
    dst->header_crc32 = READ(data, 4);

    for (int i = 0; i < 4; ++i, ++data) {
        if (*data != 0) { return NULL; }
    }

    dst->cur_lba = READ(data, 8);
    dst->bk_lba = READ(data, 8);

    dst->first_usable = READ(data, 8);
    dst->last_usable = READ(data, 8);

    for (int i = 0; i < 16; ++i, ++data) { dst->disk_guid[i] = *data; }

    dst->beg_of_part_entries = READ(data, 8);
    dst->num_of_part_entries = READ(data, 4);
    dst->size_of_part_entry = READ(data, 4);
    dst->crc32_of_part_entries = READ(data, 4);

    return data;

ERR_RET:

    return NULL;
}

byte_t* Zeta_Core_DiskPartGPT_WriteHeader(
    byte_t* dst, size_t dst_size, Zeta_Core_DiskPartGPT_Header* header) {
    ZETA_Core_DebugAssert(dst != NULL);

    ZETA_Core_DebugAssert(header != NULL);

    if (dst_size < 512) { return NULL; }

    char const sign[] = "EFI PART";

    for (int i = 0; i < 8; ++i, ++dst) { *dst = sign[i]; }

    WRITE(header->revision_num, dst, 4);
    WRITE(header->header_size, dst, 4);
    WRITE(header->header_crc32, dst, 4);

    for (int i = 0; i < 8; ++i, ++dst) { *dst = 0; }

    WRITE(header->cur_lba, dst, 8);
    WRITE(header->bk_lba, dst, 8);

    WRITE(header->first_usable, dst, 8);
    WRITE(header->last_usable, dst, 8);

    for (int i = 0; i < 16; ++i, ++dst) { *dst = header->disk_guid[i]; }

    WRITE(header->beg_of_part_entries, dst, 8);
    WRITE(header->num_of_part_entries, dst, 4);
    WRITE(header->size_of_part_entry, dst, 4);
    WRITE(header->crc32_of_part_entries, dst, 4);

    return dst;
}

byte_t const* Zeta_Core_DiskPartGPT_ReadPartEntry(
    Zeta_Core_DiskPartGPT_PartEntry* dst, byte_t const* data,
    size_t data_size) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(data != NULL);

    if (data_size < 128) { return NULL; }

    for (int i = 0; i < 16; ++i, ++data) { dst->type_guid[i] = *data; }
    for (int i = 0; i < 16; ++i, ++data) { dst->part_guid[i] = *data; }

    dst->beg = READ(data, 8);
    dst->end = READ(data, 8) + 1;

    dst->flags = READ(data, 8);

    for (int i = 0; i < 72; ++i, ++data) { dst->name[i] = *data; }

    return data;
}

byte_t* Zeta_Core_DiskPartGPT_WritePartEntry(
    byte_t* dst, size_t dst_size, Zeta_Core_DiskPartGPT_PartEntry* part_entry) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(part_entry != NULL);

    if (dst_size < 128) { return NULL; }

    for (int i = 0; i < 16; ++i, ++dst) { *dst = part_entry->type_guid[i]; }
    for (int i = 0; i < 16; ++i, ++dst) { *dst = part_entry->part_guid[i]; }

    WRITE(part_entry->beg, dst, 8);
    WRITE(part_entry->end - 1, dst, 8);

    WRITE(part_entry->flags, dst, 8);

    for (int i = 0; i < 72; ++i, ++dst) { *dst = part_entry->name[i]; }

    return dst;
}
