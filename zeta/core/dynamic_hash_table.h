#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/assoc_cntr.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/generic_hash_table.h>
#include <zeta/core/llist_node.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

ZETA_Core_DeclareStruct(Zeta_Core_DynamicHashTable);
ZETA_Core_DeclareStruct(Zeta_Core_DynamicHashTable_Node);
ZETA_Core_DeclareStruct(Zeta_Core_DynamicHashTable_Cursor);

struct Zeta_Core_DynamicHashTable {
    size_t width;

    Zeta_Core_OrdLListNode* lln;

    Zeta_Core_Hash ElemHash;
    void* elem_hash_context;

    Zeta_Core_Compare ElemCompare;
    void* elem_cmp_context;

    Zeta_Core_Allocator node_allocator;

    Zeta_Core_GenericHashTable ght;
};

struct Zeta_Core_DynamicHashTable_Node {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_OrdLListNode lln;

    ZETA_Core_DebugStructPadding;

    Zeta_Core_GenericHashTable_Node htn;

    ZETA_Core_DebugStructPadding;

    unsigned char data[] __attribute__((aligned(alignof(max_align_t))));
};

struct Zeta_Core_DynamicHashTable_Cursor {
    Zeta_Core_DynamicHashTable const* dht;
    Zeta_Core_OrdLListNode* lln;
};

void Zeta_Core_DynamicHashTable_Init(void* dht);

void Zeta_Core_DynamicHashTable_Deinit(void* dht);

size_t Zeta_Core_DynamicHashTable_GetWidth(void const* dht);

size_t Zeta_Core_DynamicHashTable_GetSize(void const* dht);

size_t Zeta_Core_DynamicHashTable_GetCapacity(void const* dht);

void Zeta_Core_DynamicHashTable_GetLBCursor(void const* dht, void* dst_cursor);

void Zeta_Core_DynamicHashTable_GetRBCursor(void const* dht, void* dst_cursor);

void* Zeta_Core_DynamicHashTable_PeekL(void* dht, void* dst_cursor);

void* Zeta_Core_DynamicHashTable_PeekR(void* dht, void* dst_cursor);

void* Zeta_Core_DynamicHashTable_Refer(void* dht, void const* pos_cursor);

void* Zeta_Core_DynamicHashTable_Find(void* dht, void const* key,
                                      Zeta_Core_Hash KeyHash,
                                      void const* key_hash_context,
                                      Zeta_Core_Compare KeyElemCompare,
                                      void const* key_elem_cmp_context,
                                      void* dst_cursor);

void* Zeta_Core_DynamicHashTable_Insert(void* dht, void const* elem,
                                        void* dst_cursor);

void Zeta_Core_DynamicHashTable_Erase(void* dht, void* pos_cursor);

void Zeta_Core_DynamicHashTable_EraseAll(void* dht);

unsigned long long Zeta_Core_DynamicHashTable_GetEffFactor(void* dht);

void Zeta_Core_DynamicHashTable_Check(void const* dht);

void Zeta_Core_DynamicHashTable_Sanitize(void* dht,
                                         Zeta_Core_MemRecorder* dst_table,
                                         Zeta_Core_MemRecorder* dst_node);

bool_t Zeta_Core_DynamicHashTable_Cursor_AreEqual(void const* dht,
                                                  void const* cursor_a,
                                                  void const* cursor_b);

void Zeta_Core_DynamicHashTable_Cursor_StepL(void const* dht, void* cursor);

void Zeta_Core_DynamicHashTable_Cursor_StepR(void const* dht, void* cursor);

void Zeta_Core_DynamicHashTable_Cursor_Check(void const* dht,
                                             void const* cursor);

extern Zeta_Core_AssocCntr_VTable const
    Zeta_Core_DynamicHashTable_assoc_cntr_vtable;

ZETA_Core_ExternC_End;
