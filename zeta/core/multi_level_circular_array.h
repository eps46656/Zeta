#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/llist_node.h>
#include <zeta/core/multi_level_ptr_table.h>
#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelCircularArray);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelCircularArray_Seg);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelCircularArray_Cursor);

#define ZETA_Core_MultiLevelCircularArray_max_seg_capacity (8)

#define ZETA_Core_MultiLevelCircularArray_branch_num \
    ZETA_Core_MultiLevelTable_max_branch_num

struct Zeta_Core_MultiLevelCircularArray {
    size_t width;

    size_t stride;

    size_t seg_capacity;

    size_t offset;

    size_t size;

    int level;

    size_t rotations[ZETA_Core_MultiLevelTable_max_level];

    void* root;

    Zeta_Core_MultiLevelCircularArray_Seg* seg_head;

    Zeta_Core_Allocator node_allocator;
    Zeta_Core_Allocator seg_allocator;

    void (*ElemDeinit)(void const* elem_deinit_context, void* elem,
                       size_t stride, size_t size);
    void const* elem_deinit_context;
};

struct Zeta_Core_MultiLevelCircularArray_Seg {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_OrdLListNode ln;

    ZETA_Core_DebugStructPadding;

    char data[] __attribute__((aligned(alignof(max_align_t))));
};

struct Zeta_Core_MultiLevelCircularArray_Cursor {
    Zeta_Core_MultiLevelCircularArray const* mlca;

    size_t idx;

    Zeta_Core_MultiLevelCircularArray_Seg* seg;
    size_t seg_slot_idx;

    void* elem;
};

void Zeta_Core_MultiLevelCircularArray_Init(void* mlca);

/**
 * @copydoc Zeta_Core_SeqCntr::Deinit
 */
void Zeta_Core_MultiLevelCircularArray_Deinit(void* mlca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetWidth
 */
size_t Zeta_Core_MultiLevelCircularArray_GetWidth(void const* mlca);

size_t Zeta_Core_MultiLevelCircularArray_GetStride(void const* mlca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetSize
 */
size_t Zeta_Core_MultiLevelCircularArray_GetSize(void const* mlca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetCapacity
 */
size_t Zeta_Core_MultiLevelCircularArray_GetCapacity(void const* mlca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetLBCursor
 */
void Zeta_Core_MultiLevelCircularArray_GetLBCursor(void const* mlca,
                                                   void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::GetRBCursor
 */
void Zeta_Core_MultiLevelCircularArray_GetRBCursor(void const* mlca,
                                                   void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::PeekL
 */
void* Zeta_Core_MultiLevelCircularArray_PeekL(void* mlca, void* dst_cursor,
                                              void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::PeekR
 */
void* Zeta_Core_MultiLevelCircularArray_PeekR(void* mlca, void* dst_cursor,
                                              void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::Access
 */
void* Zeta_Core_MultiLevelCircularArray_Access(void* mlca, size_t idx,
                                               void* dst_cursor,
                                               void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::Refer
 */
void* Zeta_Core_MultiLevelCircularArray_Refer(void* mlca,
                                              void const* pos_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Read
 *
 */
void Zeta_Core_MultiLevelCircularArray_Read(void const* mlca,
                                            void const* pos_cursor, size_t cnt,
                                            void* dst, size_t dst_stride,
                                            void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Write
 */
void Zeta_Core_MultiLevelCircularArray_Write(void* mlca, void* pos_cursor,
                                             size_t cnt, void const* src,
                                             size_t src_stride,
                                             void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Write
 */
void Zeta_Core_MultiLevelCircularArray_RangeAssign(void* mlca, void* dst_cursor,
                                                   void* src_cursor,
                                                   size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::PushL
 */
void* Zeta_Core_MultiLevelCircularArray_PushL(void* mlca, size_t cnt,
                                              void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::PushR
 */
void* Zeta_Core_MultiLevelCircularArray_PushR(void* mlca, size_t cnt,
                                              void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Insert
 */
void* Zeta_Core_MultiLevelCircularArray_Insert(void* mlca, void* pos_cursor,
                                               size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::PopL
 */
void Zeta_Core_MultiLevelCircularArray_PopL(void* mlca, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::PopR
 */
void Zeta_Core_MultiLevelCircularArray_PopR(void* mlca, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::Erase
 */
void Zeta_Core_MultiLevelCircularArray_Erase(void* mlca, void* pos_cursor,
                                             size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::EraseAll
 */
void Zeta_Core_MultiLevelCircularArray_EraseAll(void* mlca);

void Zeta_Core_MultiLevelCircularArray_Check(void const* mlca);

void Zeta_Core_MultiLevelCircularArray_Sanitize(
    void const* mlca, Zeta_Core_MemRecorder* dst_node_allocator,
    Zeta_Core_MemRecorder* dst_seg_allocator);

/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AreEqual
 */
bool_t Zeta_Core_MultiLevelCircularArray_Cursor_AreEqual(void const* mlca,
                                                         void const* cursor_a,
                                                         void const* cursor_b);

/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_Compare
 */
int Zeta_Core_MultiLevelCircularArray_Cursor_Compare(void const* mlca,
                                                     void const* cursor_a,
                                                     void const* cursor_b);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_GetDist
 */
size_t Zeta_Core_MultiLevelCircularArray_Cursor_GetDist(void const* mlca,
                                                        void const* cursor_a,
                                                        void const* cursor_b);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_GetIdx
 */
size_t Zeta_Core_MultiLevelCircularArray_Cursor_GetIdx(void const* mlca,
                                                       void const* cursor);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_StepL
 */
void Zeta_Core_MultiLevelCircularArray_Cursor_StepL(void const* mlca,
                                                    void* cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_StepR
 */
void Zeta_Core_MultiLevelCircularArray_Cursor_StepR(void const* mlca,
                                                    void* cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AdvanceL
 */
void Zeta_Core_MultiLevelCircularArray_Cursor_AdvanceL(void const* mlca,
                                                       void* cursor,
                                                       size_t step);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AdvanceR
 */
void Zeta_Core_MultiLevelCircularArray_Cursor_AdvanceR(void const* mlca,
                                                       void* cursor,
                                                       size_t step);

void Zeta_Core_MultiLevelCircularArray_Cursor_Check(void const* mlca,
                                                    void const* cursor);

extern Zeta_Core_SeqCntr_VTable const
    Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable;

ZETA_Core_ExternC_End;
