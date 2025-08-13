#pragma once

#include <zeta/core/assoc_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DebugHashTable);
ZETA_Core_DeclareStruct(Zeta_Core_DebugHashTable_Pair);

struct Zeta_Core_DebugHashTable {
    size_t width;

    void* hash_table;

    void* elem_hash_context;
    Zeta_Core_Hash ElemHash;

    void* elem_cmp_context;
    Zeta_Core_Compare ElemCompare;
};

void Zeta_Core_DebugHashTable_Init(void* debug_ht);

void Zeta_Core_DebugHashTable_Deinit(void* debug_ht);

size_t Zeta_Core_DebugHashTable_GetWidth(void const* debug_ht);

size_t Zeta_Core_DebugHashTable_GetSize(void const* debug_ht);

size_t Zeta_Core_DebugHashTable_GetCapacity(void const* debug_ht);

// void Zeta_Core_DebugHashTable_GetLBCursor(void* debug_ht, void* dst_cursor);

void Zeta_Core_DebugHashTable_GetRBCursor(void const* debug_ht,
                                          void* dst_cursor);

void* Zeta_Core_DebugHashTable_PeekL(void* debug_ht, void* dst_cursor);

// void Zeta_Core_DebugHashTable_PeekR(void* debug_ht, void* dst_cursor);

void* Zeta_Core_DebugHashTable_Refer(void* debug_ht, void const* pos_cursor);

void* Zeta_Core_DebugHashTable_Find(void* debug_ht, void const* key,
                                    Zeta_Core_Hash KeyHash,
                                    void const* key_hash_context,
                                    Zeta_Core_Compare KeyElemCompare,
                                    void const* key_elem_cmp_context,
                                    void* dst_cursor);

void* Zeta_Core_DebugHashTable_Insert(void* debug_ht, void const* key,
                                      void* dst_cursor);

void Zeta_Core_DebugHashTable_Erase(void* debug_ht, void* pos_cursor);

void Zeta_Core_DebugHashTable_EraseAll(void* debug_ht);

bool_t Zeta_Core_DebugHashTable_Cursor_AreEqual(void const* debug_ht,
                                                void const* cursor_a,
                                                void const* cursor_b);

/*
int Zeta_Core_DebugHashTable_Cursor_Compare(void* debug_ht, void const*
cursor_a, void const* cursor_b);
*/

/*
size_t Zeta_Core_DebugHashTable_Cursor_GetDist(void* debug_ht, void const*
cursor_a, void const* cursor_b);
*/

// size_t Zeta_Core_DebugHashTable_Cursor_GetIdx(void* debug_ht, void const*
// cursor);

void Zeta_Core_DebugHashTable_Cursor_StepL(void const* debug_ht, void* cursor);

void Zeta_Core_DebugHashTable_Cursor_StepR(void const* debug_ht, void* cursor);

/* void Zeta_Core_DebugHashTable_Cursor_AdvanceL(void* debug_ht, void* cursor,
                                       size_t step); */

/* void Zeta_Core_DebugHashTable_Cursor_AdvanceR(void* debug_ht, void* cursor,
                                       size_t step); */

extern Zeta_Core_AssocCntr_VTable const
    Zeta_Core_DebugHashTable_assoc_cntr_vtable;

ZETA_Core_ExternC_End;
