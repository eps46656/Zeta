#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_GenericHashTable);
ZETA_Core_DeclareStruct(Zeta_Core_GenericHashTable_Node);

struct Zeta_Core_GenericHashTable {
    unsigned long long cur_salt;
    unsigned long long nxt_salt;

    void* cur_table_root;
    void* nxt_table_root;

    size_t cur_table_size;
    size_t nxt_table_size;

    size_t cur_capacity;
    size_t nxt_capacity;

    size_t size;

    Zeta_Core_Hash NodeHash;
    void const* node_hash_context;

    Zeta_Core_Compare NodeCompare;
    void const* node_cmp_context;

    Zeta_Core_Allocator table_node_allocator;
};

struct Zeta_Core_GenericHashTable_Node {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_OrdRBTreeNode n;

    ZETA_Core_DebugStructPadding;
};

void Zeta_Core_GenericHashTable_Init(void* ght);

void Zeta_Core_GenericHashTable_Deinit(void* ght);

size_t Zeta_Core_GenericHashTable_GetSize(void const* ght);

bool_t Zeta_Core_GenericHashTable_Contain(void const* ght, void const* node);

void* Zeta_Core_GenericHashTable_Find(void const* ght, void const* key,
                                      Zeta_Core_Hash KeyHash,
                                      void const* key_hash_context,
                                      Zeta_Core_Compare KeyNodeCompare,
                                      void const* key_node_cmp_context);

void Zeta_Core_GenericHashTable_Insert(void* ght, void* node);

void Zeta_Core_GenericHashTable_Extract(void* ght, void* node);

void* Zeta_Core_GenericHashTable_ExtractAny(void* ght);

void Zeta_Core_GenericHashTable_ExtractAll(void* ght);

bool_t Zeta_Core_GenericHashTable_RunPending(void* lrucm, size_t quata);

unsigned long long Zeta_Core_GenericHashTable_GetEffFactor(void const* ght);

void Zeta_Core_GenericHashTable_Check(void const* ght);

void Zeta_Core_GenericHashTable_Sanitize(void const* ght,
                                         Zeta_Core_MemRecorder* dst_table,
                                         Zeta_Core_MemRecorder* dst_node);

void Zeta_Core_GenericHashTable_Node_Init(void* ghtn);

ZETA_Core_ExternC_End;
