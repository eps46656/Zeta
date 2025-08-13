#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DynamicVector);
ZETA_Core_DeclareStruct(Zeta_Core_DynamicVector_Cursor);

struct Zeta_Core_DynamicVector {
    size_t width;

    size_t seg_capacity;

    size_t offset;

    size_t size;

    size_t size_a;
    size_t size_b;
    size_t size_c;

    void* cur_data;
    size_t cur_offset;
    size_t cur_capacity;

    void* nxt_data;
    size_t nxt_offset;
    size_t nxt_capacity;

    Zeta_Core_Allocator data_allocator;

    Zeta_Core_Allocator seg_allocator;
};

struct Zeta_Core_DynamicVector_Cursor {
    void const* dv;
    size_t idx;
    void* ref;
};

void Zeta_Core_DynamicVector_Init(void* dv);

void Zeta_Core_DynamicVector_Deinit(void* dv);

size_t Zeta_Core_DynamicVector_GetWidth(void const* dv);

size_t Zeta_Core_DynamicVector_GetSize(void const* dv);

size_t Zeta_Core_DynamicVector_GetCapacity(void const* dv);

void Zeta_Core_DynamicVector_GetLBCursor(void const* dv, void* dst_cursor);

void Zeta_Core_DynamicVector_GetRBCursor(void const* dv, void* dst_cursor);

void* Zeta_Core_DynamicVector_PeekL(void* dv, void* dst_cursor, void* dst_elem);

void* Zeta_Core_DynamicVector_PeekR(void* dv, void* dst_cursor, void* dst_elem);

void* Zeta_Core_DynamicVector_Refer(void* dv, void const* pos_cursor);

void* Zeta_Core_DynamicVector_Access(void* dv, size_t idx, void* dst_cursor,
                                     void* dst_elem);

void Zeta_Core_DynamicVector_Read(void const* dv, void const* pos_cursor,
                                  size_t cnt, void* dst, void* dst_cursor);

void Zeta_Core_DynamicVector_Write(void* dv, void* pos_cursor, size_t cnt,
                                   void const* src, void* dst_cursor);

void* Zeta_Core_DynamicVector_PushL(void* dv, size_t cnt, void* dst_cursor);

void* Zeta_Core_DynamicVector_PushR(void* dv, size_t cnt, void* dst_cursor);

void Zeta_Core_DynamicVector_PopL(void* dv, size_t cnt);

void Zeta_Core_DynamicVector_PopR(void* dv, size_t cnt);

void Zeta_Core_DynamicVector_EraseAll(void* dv);

void Zeta_Core_DynamicVector_Check(void const* dv);

void Zeta_Core_DynamicVector_Sanitize(void* dv, Zeta_Core_MemRecorder* dst_data,
                                      Zeta_Core_MemRecorder* dst_seg);

bool_t Zeta_Core_DynamicVector_Cursor_AreEqual(void const* dv,
                                               void const* cursor_a,
                                               void const* cursor_b);

int Zeta_Core_DynamicVector_Cursor_Compare(void const* dv, void const* cursor_a,
                                           void const* cursor_b);

size_t Zeta_Core_DynamicVector_Cursor_GetDist(void const* dv,
                                              void const* cursor_a,
                                              void const* cursor_b);

size_t Zeta_Core_DynamicVector_Cursor_GetIdx(void const* dv,
                                             void const* cursor);

void Zeta_Core_DynamicVector_Cursor_StepL(void const* dv, void* cursor);

void Zeta_Core_DynamicVector_Cursor_StepR(void const* dv, void* cursor);

void Zeta_Core_DynamicVector_Cursor_AdvanceL(void const* dv, void* cursor,
                                             size_t step);

void Zeta_Core_DynamicVector_Cursor_AdvanceR(void const* dv, void* cursor,
                                             size_t step);

void Zeta_Core_DynamicVector_Cursor_Check(void const* dv, void const* cursor);

extern Zeta_Core_SeqCntr_VTable const Zeta_Core_DynamicVector_seq_cntr_vtable;

ZETA_Core_ExternC_End;
