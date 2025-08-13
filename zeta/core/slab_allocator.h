#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/llist_node.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_SlabAllocator);
ZETA_Core_DeclareStruct(Zeta_Core_SlabAllocator_SlabHead);

#define ZETA_SlabAllocator_max_units_per_slab ZETA_Core_uchar_max

struct Zeta_Core_SlabAllocator {
    size_t align;
    size_t width;
    size_t units_per_slab;

    size_t buffer_units_cnt;

    size_t vacant_units_cnt;
    size_t occupied_units_cnt;

    Zeta_Core_OrdLListNode* hot_slab_units_list;

    Zeta_Core_OrdLListNode* vacant_slab_list;
    Zeta_Core_OrdLListNode* occupied_slab_list;

    Zeta_Core_Allocator origin_allocator;
};

struct Zeta_Core_SlabAllocator_SlabHead {
    Zeta_Core_OrdLListNode n;
    unsigned char vacant_units_cnt;
};

void Zeta_Core_SlabAllocator_Init(void* sa);

void Zeta_Core_SlabAllocator_Deinit(void* sa);

size_t Zeta_Core_SlabAllocator_GetAlign(void* sa);

size_t Zeta_Core_SlabAllocator_Query(void* sa, size_t size);

void* Zeta_Core_SlabAllocator_Allocate(void* sa, size_t size);

void Zeta_Core_SlabAllocator_Deallocate(void* sa, void* ptr);

bool_t Zeta_Core_SlabAllocator_ReleaseBuffer(void* sa);

void Zeta_Core_SlabAllocator_Check(void* sa,
                                   Zeta_Core_MemRecorder* dst_used_records,
                                   Zeta_Core_MemRecorder* dst_released_records);

extern Zeta_Core_Allocator_VTable const
    Zeta_Core_DynamicVector_seq_cntr_vtableslab_allocator;

ZETA_Core_ExternC_End;
