#pragma once

#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_CircularArray);
ZETA_Core_DeclareStruct(Zeta_Core_CircularArray_Cursor);

struct Zeta_Core_CircularArray {
    void* data;
    size_t width;
    size_t stride;
    size_t offset;
    size_t size;
    size_t capacity;
};

struct Zeta_Core_CircularArray_Cursor {
    Zeta_Core_CircularArray const* ca;
    size_t idx;
    void* ref;
};

void Zeta_Core_CircularArray_Init(void* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::Deinit
 */
void Zeta_Core_CircularArray_Deinit(void* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetWidth
 */
size_t Zeta_Core_CircularArray_GetWidth(void const* ca);

size_t Zeta_Core_CircularArray_GetStride(void const* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetOffset
 */
size_t Zeta_Core_CircularArray_GetOffset(void const* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetSize
 */
size_t Zeta_Core_CircularArray_GetSize(void const* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetCapacity
 */
size_t Zeta_Core_CircularArray_GetCapacity(void const* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::GetLBCursor
 */
void Zeta_Core_CircularArray_GetLBCursor(void const* ca, void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::GetRBCursor
 */
void Zeta_Core_CircularArray_GetRBCursor(void const* ca, void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::PeekL
 */
void* Zeta_Core_CircularArray_PeekL(void* ca, void* dst_cursor, void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::PeekR
 */
void* Zeta_Core_CircularArray_PeekR(void* ca, void* dst_cursor, void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::Access
 */
void* Zeta_Core_CircularArray_Access(void* ca, size_t idx, void* dst_cursor,
                                     void* dst_elem);

/**
 * @copydoc Zeta_Core_SeqCntr::Refer
 */
void* Zeta_Core_CircularArray_Refer(void* ca, void const* pos_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Read
 */
void Zeta_Core_CircularArray_Read(void const* ca, void const* pos_cursor,
                                  size_t cnt, void* dst, size_t dst_stride,
                                  void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Write
 */
void Zeta_Core_CircularArray_Write(void* ca, void* pos_cursor, size_t cnt,
                                   void const* src, size_t src_stride,
                                   void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::PushL
 */
void* Zeta_Core_CircularArray_PushL(void* ca, size_t cnt, void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::PushR
 */
void* Zeta_Core_CircularArray_PushR(void* ca, size_t cnt, void* dst_cursor);

/**
 * @copydoc Zeta_Core_SeqCntr::Insert
 */
void* Zeta_Core_CircularArray_Insert(void* ca, void* pos_cursor, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::PopL
 */
void Zeta_Core_CircularArray_PopL(void* ca, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::PopR
 */
void Zeta_Core_CircularArray_PopR(void* ca, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::Erase
 */
void Zeta_Core_CircularArray_Erase(void* ca, void* pos_cursor, size_t cnt);

/**
 * @copydoc Zeta_Core_SeqCntr::EraseAll
 */
void Zeta_Core_CircularArray_EraseAll(void* ca);

size_t Zeta_Core_CircularArray_GetLongestContPred(void const* ca, size_t idx);

size_t Zeta_Core_CircularArray_GetLongestContSucr(void const* ca, size_t idx);

void Zeta_Core_CircularArray_Assign(void* dst_ca, void* src_ca, size_t dst_beg,
                                    size_t src_beg, size_t cnt);

void Zeta_Core_CircularArray_AssignFromSeqCntr(void* ca, void* ca_cursor,
                                               Zeta_Core_SeqCntr seq_cntr,
                                               void* seq_cntr_cursor,
                                               size_t cnt);

void Zeta_Core_CircularArray_Check(void const* ca);

/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AreEqual
 */
bool_t Zeta_Core_CircularArray_Cursor_AreEqual(void const* ca,
                                               void const* cursor_a,
                                               void const* cursor_b);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_Compare
 */
int Zeta_Core_CircularArray_Cursor_Compare(void const* ca, void const* cursor_a,
                                           void const* cursor_b);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_GetDist
 */
size_t Zeta_Core_CircularArray_Cursor_GetDist(void const* ca,
                                              void const* cursor_a,
                                              void const* cursor_b);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_GetIdx
 */
size_t Zeta_Core_CircularArray_Cursor_GetIdx(void const* ca,
                                             void const* cursor);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_StepL
 */
void Zeta_Core_CircularArray_Cursor_StepL(void const* ca, void* cursor);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_StepR
 */
void Zeta_Core_CircularArray_Cursor_StepR(void const* ca, void* cursor);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AdvanceL
 */
void Zeta_Core_CircularArray_Cursor_AdvanceL(void const* ca, void* cursor,
                                             size_t step);
/**
 * @copydoc Zeta_Core_SeqCntr::Cursor_AdvanceR
 */
void Zeta_Core_CircularArray_Cursor_AdvanceR(void const* ca, void* cursor,
                                             size_t step);

void Zeta_Core_CircularArray_Cursor_Check(void const* ca, void const* cursor);

extern Zeta_Core_SeqCntr_VTable const Zeta_Core_CircularArray_seq_cntr_vtable;

ZETA_Core_ExternC_End;
