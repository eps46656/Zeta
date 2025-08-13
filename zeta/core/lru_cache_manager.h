#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/cache_manager.h>
#include <zeta/core/generic_hash_table.h>
#include <zeta/core/llist_node.h>
#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

#define ZETA_Core_LRUCachaManager_max_cn_cnts (ZETA_Core_uint_max)

ZETA_Core_DeclareStruct(Zeta_Core_LRUCacheManager);
ZETA_Core_DeclareStruct(Zeta_Core_LRUCacheManager_BNode);
ZETA_Core_DeclareStruct(Zeta_Core_LRUCacheManager_SNode);
ZETA_Core_DeclareStruct(Zeta_Core_LRUCacheManager_CNode);
ZETA_Core_DeclareStruct(Zeta_Core_LRUCacheManager_XNode);

#define Zeta_Core_LRUCacheManager_SNode_color (0)
#define Zeta_Core_LRUCacheManager_CNode_color (1)
#define Zeta_Core_LRUCacheManager_XNode_color (2)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct Zeta_Core_LRUCacheManager_BNode {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_GenericHashTable_Node ghtn;

    ZETA_Core_DebugStructPadding;

    Zeta_Core_OrdRBLListNode ln;
    // p: color
};

struct Zeta_Core_LRUCacheManager_SNode {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_LRUCacheManager_BNode bn;

    ZETA_Core_DebugStructPadding;

    Zeta_Core_OrdLListNode sln;

    ZETA_Core_DebugStructPadding;

    unsigned cn_cnt;
    unsigned max_cn_cnt;
};

struct Zeta_Core_LRUCacheManager_CNode {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_LRUCacheManager_BNode bn;

    ZETA_Core_DebugStructPadding;

    size_t cache_idx;
    void* frame;

    unsigned ref_cnt;
    bool_t dirty;
};

struct Zeta_Core_LRUCacheManager_XNode {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_LRUCacheManager_BNode bn;

    ZETA_Core_DebugStructPadding;

    Zeta_Core_LRUCacheManager_SNode* sn;
    Zeta_Core_LRUCacheManager_CNode* cn;
};

struct Zeta_Core_LRUCacheManager {
    size_t cn_cnt;
    size_t max_cn_cnt;

    Zeta_Core_SeqCntr origin;

    size_t cache_size;

    Zeta_Core_Allocator sn_allocator;
    Zeta_Core_Allocator cn_allocator;
    Zeta_Core_Allocator xn_allocator;
    Zeta_Core_Allocator frame_allocator;

    Zeta_Core_GenericHashTable ght;

    Zeta_Core_OrdLListNode* fit_sl;
    Zeta_Core_OrdLListNode* over_sl;

    Zeta_Core_OrdRBLListNode* hot_clear_cl;
    Zeta_Core_OrdRBLListNode* hot_dirty_cl;
    Zeta_Core_OrdRBLListNode* cold_clear_cl;
    Zeta_Core_OrdRBLListNode* cold_dirty_cl;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void Zeta_Core_LRUCacheManager_Init(void* lrucm);

void Zeta_Core_LRUCacheManager_Deinit(void* lrucm);

Zeta_Core_SeqCntr Zeta_Core_LRUCacheManager_GetOrigin(void const* lrucm);

size_t Zeta_Core_LRUCacheManager_GetCacheSize(void const* lrucm);

void* Zeta_Core_LRUCacheManager_Open(void* lrucm, size_t max_cache_cnt);

void Zeta_Core_LRUCacheManager_Close(void* lrucm, void* sd);

void Zeta_Core_LRUCacheManager_SetMaxCacheCnt(void* lrucm, void* sd,
                                              size_t max_cache_cnt);

void Zeta_Core_LRUCacheManager_Read(void* lrucm, void* sd, size_t idx,
                                    size_t cnt, void* dst, size_t dst_stride);

void Zeta_Core_LRUCacheManager_Write(void* lrucm, void* sd, size_t idx,
                                     size_t cnt, void const* src,
                                     size_t src_stride);

size_t Zeta_Core_LRUCacheManager_Flush(void* lrucm, size_t quata);

bool_t Zeta_Core_LRUCacheManager_RunPending(void* lrucm, size_t calc_quata,
                                            size_t write_quata);

void Zeta_Core_LRUCacheManager_Check(void const* lrucm);

void Zeta_Core_LRUCacheManager_CheckSessionDescriptor(void const* lrucm,
                                                      void* sd);

extern Zeta_CacheManager_VTable const
    Zeta_Core_LRUCacheManager_cache_manager_vtable;

ZETA_Core_ExternC_End;
