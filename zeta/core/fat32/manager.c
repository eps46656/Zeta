#include <zeta/core/debugger.h>
#include <zeta/core/dynamic_vector.h>
#include <zeta/core/fat32/manager.h>
#include <zeta/core/multi_level_table.h>
#include <zeta/core/staging_vector.h>
#include <zeta/core/utils.h>

/*

FAT32Manager

maintain current opened nodes (file/dir) as sturct Node

struct Node offers basic read and write function.
    Read: Read a

A:

maintain


file:
    seg num vector

    stage vector

    write stage vector's data back

dir
    seg num vector

    map of children nodes

    write children nodes back

*/

#if ZETA_Core_EnableDebug

#define CheckNodeVector_(nv) NodeVector_Check((nv))
#define CheckNodeVectorCursor_(nv, cursor) \
    NodeVector_Cursor_Check((nv), (cursor))

#define CheckManager_(manager) Zeta_Core_FAT32_Manager_Check((manager))

#else

#define CheckNodeVector_(nv) ZETA_Core_Unused((nv))

#define CheckNodeVectorCursor_(nv, cursor) \
    {                                      \
        ZETA_Core_Unused((nv));            \
        ZETA_Core_Unused((cursor));        \
    }                                      \
    ZETA_Core_StaticAssert(TRUE)

#define CheckManager_(manager) ZETA_Core_Unused((manager))

#endif

ZETA_Core_DeclareStruct(NodeVector);
ZETA_Core_DeclareStruct(DirRawEntry);
ZETA_Core_DeclareStruct(DirEntry);

#define UNUSED_CLUSTER (0)
#define BAD_CLUSTER (0x0ffffff7)
#define EOF_CLUSTER (0x0fffffff)

#define ASSERT_RET(cond)           \
    if (!(cond)) { goto ERR_RET; } \
    ZETA_Core_StaticAssert(TRUE)

#define READ(data, length)                                                   \
    ({                                                                       \
        ZETA_Core_AutoVar(tmp, Zeta_Core_ReadLittleEndianStd(data, length)); \
        data += length;                                                      \
        tmp;                                                                 \
    })

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define IsBadCluster_(x) ((x) == UNUSED_CLUSTER)

#define IsBadCluster_(x) ((x) == BAD_CLUSTER)

#define IsEOFCluster_(x)                       \
    ({                                         \
        ZETA_Core_AutoVar(tmp, x);             \
        0x0ffffff8 <= tmp&& tmp <= 0x0fffffff; \
    })

static size_t GetClusOffset_(Zeta_Core_FAT32_Header* header, size_t clus_num) {
    ZETA_Core_DebugAssert(header != NULL);

    //
}

/*
 * Read cnt bytes begining from beg (begining block number).
 * Use snode's cache session
 */
static void Read_(Zeta_Core_FAT32_Manager* manager,
                  Zeta_Core_FAT32_SNode* snode, void* dst_, size_t beg,
                  size_t cnt) {
    Zeta_CacheManager* cm = manager->cm;

    if (cnt == 0) { return; }

    size_t blk_size = cm->GetBlockSize(cm->context);

    byte_t* dst = dst_;
    ZETA_Core_DebugAssert(dst != NULL);

    size_t blk_num = manager->header.base_blk_num + beg / blk_size;
    size_t ele_idx = beg % blk_size;

    while (0 < cnt) {
        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, blk_size - ele_idx);

        byte_t const* src = cm->ReadBlock(cm->context, snode->cm_sd, blk_num);

        Zeta_Core_MemCopy(dst, src + ele_idx, cur_cnt);

        ele_idx += cur_cnt;
        cnt -= cur_cnt;
        dst += cur_cnt;

        if (ele_idx == blk_size) {
            ++blk_num;
            ele_idx = 0;
        }
    }
}

static void Write_(Zeta_Core_FAT32_Manager* manager,
                   Zeta_Core_FAT32_SNode* snode, void* src_, size_t beg,
                   size_t cnt) {
    Zeta_CacheManager* cm = manager->cm;

    if (cnt == 0) { return; }

    size_t blk_size = cm->GetBlockSize(cm->context);

    byte_t* src = src_;
    ZETA_Core_DebugAssert(src != NULL);

    size_t blk_num = manager->header.base_blk_num + beg / blk_size;
    size_t ele_idx = beg % blk_size;

    while (0 < cnt) {
        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, blk_size - ele_idx);

        if (ele_idx == 0 && cur_cnt == blk_size) {
            cm->WriteBlock(cm->context, snode->cm_sd, blk_num, src);
        } else {
            byte_t const* data =
                cm->ReadBlock(cm->context, snode->cm_sd, blk_num);

            size_t cur_beg = ele_idx;
            size_t cur_end = cur_beg + cur_cnt;

            Zeta_Core_MemCopy(snode->buffer, data, cur_beg);

            Zeta_Core_MemCopy(snode->buffer + cur_beg, src, cur_cnt);

            Zeta_Core_MemCopy(snode->buffer + cur_end, data + cur_end,
                              blk_size - cur_end);

            cm->WriteBlock(cm->context, snode->cm_sd, blk_num, snode->buffer);
        }

        ele_idx += cur_cnt;
        cnt -= cur_cnt;
        src += cur_cnt;

        if (ele_idx == blk_size) {
            ++blk_num;
            ele_idx = 0;
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static size_t GetSecOffsetOfDataRegion_(Zeta_Core_FAT32_Header* header) {
    return header->reserved_sec_cnt + header->fat_size * header->fat_cnt;
}

#define SecOffsetOfDataRegion_(header_)                                \
    ({                                                                 \
        Zeta_Core_FAT32_Header* header = (header_);                    \
        header->reserved_sec_cnt + header->fat_size * header->fat_cnt; \
    })

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ZETA_Core_DeclareStruct(Node);

struct Node {
    Zeta_Core_FAT32_Manager* manager;
    void* cache_manager_sd;

    size_t origin_size;
    // the original size of node

    Zeta_Core_SeqCntr clus_num_vec_dummy_vec_seq_cntr;
    Zeta_Core_StagingVector clus_num_vec;
    // contain clus numbers strding clus_num_width

    size_t clus_num_width;

    Zeta_Core_StagingVector stage_vec;
};

struct NodeVector {
    Zeta_Core_FAT32_Manager* manager;

    size_t size;  // The number of bytes in the node.

    Zeta_Core_DynamicVector seg_nums;  // The segment numbers of the node.
};

// -----------------------------------------------------------------------------

static size_t NodeVector_GetWidth(void* nv);

static size_t NodeVector_GetWidth(void* nv);

static size_t NodeVector_GetSize(void* nv);

static size_t NodeVector_GetCapacity(void* nv);

static size_t NodeVector_GetCapacity(void* nv);

static void NodeVector_GetLBCursor(void* nv, void* dst_cursor);

static void NodeVector_GetRBCursor(void* nv, void* dst_cursor);

static void* NodeVector_PeekL(void* nv, void* dst_cursor, void* dst_elem);

static void* NodeVector_PeekR(void* nv, void* dst_cursor, void* dst_elem);

static void* NodeVector_Access(void* nv, void* dst_cursor, void* ele,
                               size_t idx);

static void* NodeVector_Refer(void* nv, void* pos_cursor);

static void NodeVector_Read(void* nv, void* pos_cursor, size_t cnt, void* dst,
                            void* dst_cursor);

static void NodeVector_Write(void* nv, void* pos_cursor, size_t cnt,
                             void const* src, void* dst_cursor);

static void NodeVector_Check(void* nv);

static bool_t NodeVector_Cursor_AreEqual(void* nv, void const* cursor_a,
                                         void const* cursor_b);

static int NodeVector_Cursor_Compare(void* nv, void const* cursor_a,
                                     void const* cursor_b);

static size_t NodeVector_Cursor_GetDist(void* nv, void const* cursor_a,
                                        void const* cursor_b);

static size_t NodeVector_Cursor_GetIdx(void* nv, void const* cursor);

void NodeVector_Cursor_Check(void* nv, void const* cursor);

// -----------------------------------------------------------------------------

static size_t NodeVector_GetWidth(void* nv_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    return 1;
}

static size_t NodeVector_GetWidth(void* nv_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    return 1;
}

static size_t NodeVector_GetSize(void* nv_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    return nv->size;
}

static void NodeVector_GetLBCursor(void* nv_, void* dst_cursor_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t* dst_cursor = dst_cursor_;
    ZETA_Core_DebugAssert(dst_cursor != NULL);

    *dst_cursor = -1;
}

static void NodeVector_GetRBCursor(void* nv_, void* dst_cursor_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t* dst_cursor = dst_cursor_;
    ZETA_Core_DebugAssert(dst_cursor != NULL);

    *dst_cursor = nv->size;
}

static void* NodeVector_PeekL(void* nv_, void* dst_cursor_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t* dst_cursor = dst_cursor_;

    if (dst_cursor != NULL) { *dst_cursor = 0; }

    return NULL;
}

static void* NodeVector_PeekR(void* nv_, void* dst_cursor_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t* dst_cursor = dst_cursor_;

    if (dst_cursor != NULL) { *dst_cursor = nv->size - 1; }

    return NULL;
}

void* NodeVector_Access(void* nv_, void* dst_cursor_, void* dst_elem,
                        size_t idx) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t* dst_cursor = dst_cursor_;

    size_t size = nv->size;
    ZETA_Core_DebugAssert(idx + 1 < size + 2);

    if (dst_cursor != NULL) { *dst_cursor = idx; }

    if (dst_elem != NULL) {
        // TODO Read

        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        // size_t byte_beg = *Zeta_Core_MultiLevelTable_Access(&nv->mlt, )

        // Read_(nv->manager, nv->snode, dst_elem, )
    }

    return NULL;
}

static void NodeVector_Read(void* nv_, void* pos_cursor, size_t cnt, void* dst,
                            void* dst_cursor) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    return NULL;
}

static void NodeVector_Check(void* nv_) {
    NodeVector* nv = nv_;
    ZETA_Core_DebugAssert(nv != NULL);

    Zeta_Core_FAT32_Manager* manager = nv->manager;
    Zeta_Core_FAT32_Manager_Check(manager);

    size_t seg_cnt = Zeta_Core_DynamicVector_GetSize(&nv->seg_nums);

    ZETA_Core_DebugAssert(seg_cnt == ZETA_CeilInvDiv(nv->size, nv->manager))
}

bool_t NodeVector_Cursor_AreEqual(void* nv_, void const* cursor_a_,
                                  void const* cursor_b_) {
    NodeVector* nv = nv_;

    size_t* cursor_a = cursor_a_;
    size_t* cursor_b = cursor_b_;

    CheckNodeVectorCursor_(nv, cursor_a);
    CheckNodeVectorCursor_(nv, cursor_a);

    return *cursor_a == *cursor_b;
}

int NodeVector_Cursor_Compare(void* nv_, void const* cursor_a_,
                              void const* cursor_b_) {
    NodeVector* nv = nv_;

    size_t* cursor_a = cursor_a_;
    size_t* cursor_b = cursor_b_;

    CheckNodeVectorCursor_(nv, cursor_a);
    CheckNodeVectorCursor_(nv, cursor_a);

    if (*cursor_a < *cursor_b) { return -1; }
    if (*cursor_b < *cursor_a) { return 1; }
    return 0;
}

size_t NodeVector_Cursor_GetDist(void* nv_, void const* cursor_a_,
                                 void const* cursor_b_) {
    NodeVector* nv = nv_;

    size_t* cursor_a = cursor_a_;
    size_t* cursor_b = cursor_b_;

    CheckNodeVectorCursor_(nv, cursor_a);
    CheckNodeVectorCursor_(nv, cursor_a);

    return *cursor_b - *cursor_a;
}

size_t NodeVector_Cursor_GetIdx(void* nv_, void const* cursor_) {
    NodeVector* nv = nv_;
    size_t* cursor = cursor_;

    CheckNodeVectorCursor_(nv, cursor);

    return *cursor;
}

void NodeVector_Cursor_StepL(void* sv, void* cursor) {
    NodeVector_Cursor_AdvanceL(sv, cursor, 1);
}

void NodeVector_Cursor_StepR(void* sv, void* cursor) {
    NodeVector_Cursor_AdvanceR(sv, cursor, 1);
}

void NodeVector_Cursor_AdvanceL(void* nv_, void* cursor_, size_t step) {
    NodeVector* nv = nv_;
    size_t* cursor = cursor_;

    CheckNodeVectorCursor_(nv, cursor);

    ZETA_Core_DebugAssert(step <= *cursor + 1);

    *cursor -= step;
}

void NodeVector_Cursor_AdvanceR(void* nv_, void* cursor_, size_t step) {
    NodeVector* nv = nv_;
    size_t* cursor = cursor_;

    CheckNodeVectorCursor_(nv, cursor);

    ZETA_Core_DebugAssert(step <= nv->size - *cursor);

    *cursor += step;
}

void NodeVector_Cursor_Check(void* nv_, void const* cursor_) {
    NodeVector* nv = nv_;
    CheckNodeVector_(nv);

    size_t const* cursor = cursor_;
    ZETA_Core_DebugAssert(*cursor + 1 < nv->size + 2);
}

static void NodeVector_DeploySeqCntr(void* nv_, Zeta_Core_SeqCntr* seq_cntr) {
    NodeVector* nv = nv_;
    ZETA_Core_DebugAssert(nv != NULL);

    seq_cntr->GetWidth = NodeVector_GetWidth;

    seq_cntr->GetWidth = NodeVector_GetWidth;

    seq_cntr->GetSize = NodeVector_GetSize;

    seq_cntr->GetLBCursor = NodeVector_GetLBCursor;

    seq_cntr->GetRBCursor = NodeVector_GetRBCursor;

    seq_cntr->PeekL = NodeVector_PeekL;

    seq_cntr->Refer = NodeVector_Refer;

    seq_cntr->Read = NodeVector_Read;

    seq_cntr->Cursor_AreEqual = NodeVector_Cursor_AreEqual;

    seq_cntr->Cursor_Compare = NodeVector_Cursor_Compare;

    seq_cntr->Cursor_GetDist = NodeVector_Cursor_GetDist;

    seq_cntr->Cursor_GetIdx = NodeVector_Cursor_GetIdx;

    seq_cntr->Cursor_StepL = NodeVector_Cursor_StepL;

    seq_cntr->Cursor_StepR = NodeVector_Cursor_StepR;

    seq_cntr->Cursor_AdvanceL = NodeVector_Cursor_AdvanceL;

    seq_cntr->Cursor_AdvanceR = NodeVector_Cursor_AdvanceR;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct DirRawEntry {
    bool_t is_long_name;

    union {
        struct {
            byte_t name[11];

            u8_t attr;

            u64_t first_clus_num;

            s32_t crt_year;
            u8_t crt_month;
            u8_t crt_day;
            u8_t crt_hour;
            u8_t crt_min;
            u8_t crt_sec;
            u16_t crt_msec;

            s32_t acc_year;
            u8_t acc_month;
            u8_t acc_day;

            s32_t wrt_year;
            u8_t wrt_month;
            u8_t wrt_day;
            u8_t wrt_hour;
            u8_t wrt_min;
            u8_t wrt_sec;
        } content;

        struct {
            //
        } long_name;
    };
};

struct DirEntry {
    u8_t attr;

    u64_t first_clus_num;

    s32_t crt_year;
    u8_t crt_month;
    u8_t crt_day;
    u8_t crt_hour;
    u8_t crt_min;
    u8_t crt_sec;
    u16_t crt_msec;

    s32_t acc_year;
    u8_t acc_month;
    u8_t acc_day;

    s32_t wrt_year;
    u8_t wrt_month;
    u8_t wrt_day;
    u8_t wrt_hour;
    u8_t wrt_min;
    u8_t wrt_sec;

    void* name_root;
    void* name_lb;
    void* name_rb;
};

bool_t ReadHeader_(Zeta_Core_FAT32_Header* dst, byte_t const* data) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(data != NULL);

    for (int i = 0; i < 3; ++i) { dst->jmp_boot[i] = data[i]; }
    data += 3;

    for (int i = 0; i < 8; ++i) { dst->oem_name[i] = data[i]; }
    data += 8;

    dst->bytes_per_sec = READ(data, 2);

    switch (dst->bytes_per_sec) {
        case 512:
        case 1024:
        case 2048:
        case 4096: break;

        default: goto ERR_RET;
    }

    dst->secs_per_clus = READ(data, 1);

    switch (dst->secs_per_clus) {
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
        case 128: break;

        default: goto ERR_RET;
    }

    dst->reserved_sec_cnt = READ(data, 2);
    ASSERT_RET(0 < dst->reserved_sec_cnt);

    dst->fat_cnt = READ(data, 1);
    ASSERT_RET(0 < dst->fat_cnt);

    u32_t root_ent_cnt = READ(data, 2);
    ASSERT_RET(root_ent_cnt == 0);

    u32_t sec_16_cnt = READ(data, 2);
    ASSERT_RET(sec_16_cnt == 0);

    dst->media = READ(data, 1);

    switch (dst->media) {
        case 0xf0:
        case 0xf8:
        case 0xf9:
        case 0xfa:
        case 0xfb:
        case 0xfc:
        case 0xfd:
        case 0xfe:
        case 0xff: break;

        default: goto ERR_RET;
    }

    u32_t fat_16_size = READ(data, 2);
    ASSERT_RET(fat_16_size == 0);

    dst->sec_per_trk = READ(data, 2);

    dst->head_cnt = READ(data, 2);

    dst->hidden_sec_cnt = READ(data, 4);

    u32_t sec_32_cnt = READ(data, 4);
    ASSERT_RET(0 < sec_32_cnt);

    dst->sec_cnt = sec_32_cnt;

    u32_t fat_32_size = READ(data, 4);
    ASSERT_RET(0 < fat_32_size);

    dst->fat_size = fat_32_size;

    dst->ext_flags = READ(data, 2);

    dst->fs_ver = READ(data, 2);

    dst->root_clus = READ(data, 4);

    dst->fs_info = READ(data, 2);

    dst->bk_boot_sec = READ(data, 2);

    // padding
    data += 12;

    dst->drv_num = READ(data, 1);

    data += 1;

    u32_t boot_sig = READ(data, 1);

    ZETA_Core_Unused(boot_sig);

    dst->vol_id = READ(data, 4);

    for (int i = 0; i < 11; ++i) { dst->vol_lab[i] = data[i]; }
    data += 11;

    for (int i = 0; i < 8; ++i) { dst->fs_type[i] = data[i]; }
    data += 8;

    data += 420;

    ASSERT_RET(data[0] == 0x55 && data[1] == 0xaa);
    data += 2;

    return data;

ERR_RET:
    return NULL;
}

static u64_t GetFATEntryOffset_(Zeta_Core_FAT32_Header* header, size_t fat_idx,
                                u32_t clus_num) {
    ZETA_Core_DebugAssert(header != NULL);

    ZETA_Core_DebugAssert(fat_idx < header->fat_cnt);

    u32_t clus_cnt = header->bytes_per_sec * header->fat_size / 4;
    ZETA_Core_DebugAssert(2 <= clus_num);
    ZETA_Core_DebugAssert(clus_num < clus_cnt);

    u64_t ret = header->bytes_per_sec *
                    (header->reserved_sec_cnt + header->fat_size * fat_idx) +
                clus_num * 4;

    return ret;
}

static size_t GetFATEntry_(Zeta_Core_FAT32_Header* header, size_t fat_idx,
                           u32_t clus_num, Zeta_Core_Disk* disk) {
    ZETA_Core_DebugAssert(header != NULL);
    ZETA_Core_DebugAssert(disk != NULL);

    u64_t offset = GetFATEntryOffset_(header, fat_idx, clus_num);

    byte_t tmp[4];
    disk->Read(disk->context, idx, idx + 4, tmp);

    return Zeta_Core_ReadLittleEndian(tmp, 4);
}
