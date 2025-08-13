#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/multi_level_ptr_table.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_Node);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_Cursor);

ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_InsertResult);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_EraseResult);

/**
 * The maximum number of level.
 */
#define ZETA_Core_MultiLevelTable_max_level \
    ZETA_Core_MultiLevelPtrTable_max_level

#define ZETA_Core_MultiLevelTable_min_branch_num \
    ZETA_Core_MultiLevelPtrTable_min_branch_num
#define ZETA_Core_MultiLevelTable_max_branch_num \
    ZETA_Core_MultiLevelPtrTable_max_branch_num

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct Zeta_Core_MultiLevelTable {
    int level;
    unsigned short const* branch_nums;

    size_t width;
    size_t stride;
    size_t seg_size;

    Zeta_Core_MultiLevelPtrTable_Node* root;

    Zeta_Core_Allocator mlpt_node_allocator;
    Zeta_Core_Allocator data_node_allocator;
};

struct Zeta_Core_MultiLevelTable_Node {
    unsigned long long active_map;

    unsigned char data[];
    __attribute__((__aligned__(alignof(max_align_t))));
};

struct Zeta_Core_MultiLevelTable_Cursor {
    unsigned short seg_idx;
    Zeta_Core_MultiLevelPtrTable_Cursor mlpt_cursor;
};

struct Zeta_Core_MultiLevelTable_InsertResult {
    void** addr;
    bool_t inserted;
};

struct Zeta_Core_MultiLevelTable_EraseResult {
    void* ptr;
    bool_t erased;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * @brief Initialize the mlt.
 *
 * @param mlt The target mlt.
 */
void Zeta_Core_MultiLevelTable_Init(void* mlt);

/**
 * @brief Deinitialize the mlt.
 *
 * @param mlt The target mlt.
 */
void Zeta_Core_MultiLevelTable_Deinit(void* mlt);

/**
 * @brief Get the number of entries in mlt.
 *
 * @param mlt The target mlt.
 */
size_t Zeta_Core_MultiLevelTable_GetSize(void* mlt);

/**
 * @brief Get the total capacity of mlt. Assume the value does not overflow
 * max range of size_t.
 *
 * @param mlt The target mlt.
 */
size_t Zeta_Core_MultiLevelTable_GetCapacity(void* mlt);

/**
 * @brief Get the reference of target entry by indexes.
 *
 * @param mlt The target mlt.
 * @param idx The index of target entry in each level.
 *
 * @return The reference of target entry. If the it is not inserted, return
 * NULL.
 */
void** Zeta_Core_MultiLevelTable_Access(void* mlt, size_t* idxes);

void** Zeta_Core_MultiLevelTable_FindFirst(void* mlt, size_t* dst_idxes);

void** Zeta_Core_MultiLevelTable_FindLast(void* mlt, size_t* dst_idxes);

/**
 * @brief Find the first entry before idx.
 *
 * @param mlt The target mlt.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
void** Zeta_Core_MultiLevelTable_FindPrev(void* mlt, size_t* idxes,
                                          bool_t included);

/**
 * @brief Find the first entry after idx.
 *
 * @param mlt The target mlt.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
void** Zeta_Core_MultiLevelTable_FindNext(void* mlt, size_t* idxes,
                                          bool_t included);

/**
 * @brief Insert a new entry at idxes then return its reference. If
 * it has already existed, remaining its origin value.
 *
 * @param mlt The target mlt.
 * @param idxes The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
void** Zeta_Core_MultiLevelTable_Insert(void* mlt, size_t* idxes);

/**
 * @brief Erase the target entry by indexes. If it has not existen.
 *
 * @param mlt The target mlt.
 * @param idx The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
void* Zeta_Core_MultiLevelTable_Erase(void* mlt, size_t* idxes);

/**
 * @brief Erase all existed entries.
 *
 * @param mlt The target mlt.
 */
void Zeta_Core_MultiLevelTable_EraseAll(void* mlt);

void Zeta_Core_MultiLevelTable_Check(void* mlt);

void Zeta_Core_MultiLevelTable_CheckIdxes(void* mlt, size_t const* idxes);

void Zeta_Core_MultiLevelTable_Sanitize(void* mlt,
                                        Zeta_Core_MemRecorder* dst_node);

ZETA_Core_ExternC_End;
