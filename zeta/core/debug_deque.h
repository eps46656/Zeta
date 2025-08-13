#pragma once

#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DebugDeque);

struct Zeta_Core_DebugDeque {
    void* deque;
    size_t width;
};

void Zeta_Core_DebugDeque_Init(void* dd);

void Zeta_Core_DebugDeque_Deinit(void* dd);

size_t Zeta_Core_DebugDeque_GetWidth(void const* dd);

size_t Zeta_Core_DebugDeque_GetSize(void const* dd);

size_t Zeta_Core_DebugDeque_GetCapacity(void const* dd);

void Zeta_Core_DebugDeque_GetLBCursor(void const* context, void* dst_cursor);

void Zeta_Core_DebugDeque_GetRBCursor(void const* context, void* dst_cursor);

void* Zeta_Core_DebugDeque_PeekL(void* dd, void* dst_cursor, void* dst_elem);

void* Zeta_Core_DebugDeque_PeekR(void* dd, void* dst_cursor, void* dst_elem);

void* Zeta_Core_DebugDeque_Access(void* dd, size_t idx, void* dst_cursor,
                                  void* dst_elem);

void* Zeta_Core_DebugDeque_Refer(void* dd, void const* pos_cursor);

void Zeta_Core_DebugDeque_Read(void const* dd, void const* pos_cursor,
                               size_t cnt, void* dst, size_t dst_stride,
                               void* dst_cursor);

void Zeta_Core_DebugDeque_Write(void* dd, void* pos_cursor, size_t cnt,
                                void const* src, size_t src_stride,
                                void* dst_cursor);

void* Zeta_Core_DebugDeque_PushL(void* dd, size_t cnt, void* dst_cursor);

void* Zeta_Core_DebugDeque_PushR(void* dd, size_t cnt, void* dst_cursor);

void* Zeta_Core_DebugDeque_Insert(void* dd, void* pos_cursor, size_t cnt);

void Zeta_Core_DebugDeque_PopL(void* dd, size_t cnt);

void Zeta_Core_DebugDeque_PopR(void* dd, size_t cnt);

void Zeta_Core_DebugDeque_Erase(void* dd, void* pos_cursor, size_t cnt);

void Zeta_Core_DebugDeque_EraseAll(void* dd);

void Zeta_Core_DebugDeque_Check(void const* dd);

bool_t Zeta_Core_DebugDeque_Cursor_AreEqual(void const* dd,
                                            void const* cursor_a,
                                            void const* cursor_b);

int Zeta_Core_DebugDeque_Cursor_Compare(void const* dd, void const* cursor_a,
                                        void const* cursor_b);

size_t Zeta_Core_DebugDeque_Cursor_GetDist(void const* dd, void const* cursor_a,
                                           void const* cursor_b);

size_t Zeta_Core_DebugDeque_Cursor_GetIdx(void const* dd, void const* cursor);

void Zeta_Core_DebugDeque_Cursor_StepL(void const* dd, void* cursor);

void Zeta_Core_DebugDeque_Cursor_StepR(void const* dd, void* cursor);

void Zeta_Core_DebugDeque_Cursor_AdvanceL(void const* dd, void* cursor,
                                          size_t step);

void Zeta_Core_DebugDeque_Cursor_AdvanceR(void const* dd, void* cursor,
                                          size_t step);

void Zeta_Core_DebugDeque_Cursor_Check(void const* dd, void const* cursor);

extern Zeta_Core_SeqCntr_VTable const Zeta_Core_DebugDeque_seq_cntr_vtable;

ZETA_Core_ExternC_End;
