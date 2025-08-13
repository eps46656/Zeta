#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/assoc_cntr.h>
#include <zeta/core/bin_tree_node.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

#define ZETA_Core_LinSpaceAllocator_AllocationStrategy_BestFit (0)
#define ZETA_Core_LinSpaceAllocator_AllocationStrategy_TwiceBestFit (1)

ZETA_Core_DeclareStruct(Zeta_Core_LinSpaceAllocator_Node);
ZETA_Core_DeclareStruct(Zeta_Core_LinSpaceAllocator);

struct Zeta_Core_LinSpaceAllocator_Node {
    ZETA_Core_DebugStructPadding;

    Zeta_Core_ShortRelCnt3RBTreeNode gn;

    ZETA_Core_DebugStructPadding;

    Zeta_Core_ShortRelRBTreeNode sn;

    ZETA_Core_DebugStructPadding;
};

struct Zeta_Core_LinSpaceAllocator {
    size_t beg;
    size_t end;

    int strategy;

    Zeta_Core_LinSpaceAllocator_Node* gt_root;
    Zeta_Core_LinSpaceAllocator_Node* st_root;

    Zeta_Core_Allocator node_allocator;
};

/**
 * @brief Initialize the allocator managed space [beg, end).
 *
 * @param lsa The target allocator.
 */
void Zeta_Core_LinSpaceAllocator_Init(void* lsa);

/**
 * @brief Allocate a continuous space at least \p cnt units.
 *
 * @param lsa The target allocator.
 * @param cnt
 *
 * @return The begin of allocated space.
 */
size_t Zeta_Core_LinSpaceAllocator_Allocate(void* lsa, size_t cnt);

void Zeta_Core_LinSpaceAllocator_Deallocate(void* lsa, size_t idx);

size_t Zeta_Core_LinSpaceAllocator_GetVacantSizeL(void* lsa);

size_t Zeta_Core_LinSpaceAllocator_GetVacantSizeR(void* lsa);

void Zeta_Core_LinSpaceAllocator_ExtendL(void* lsa, size_t cnt);

void Zeta_Core_LinSpaceAllocator_ExtendR(void* lsa, size_t cnt);

bool_t Zeta_Core_LinSpaceAllocator_ShrinkL(void* lsa, size_t cnt);

bool_t Zeta_Core_LinSpaceAllocator_ShrinkR(void* lsa, size_t cnt);

void Zeta_Core_LinSpaceAllocator_Check(void* lsa);

void Zeta_Core_LinSpaceAllocator_Sanitize(void* lsa,
                                          Zeta_Core_MemRecorder* dst_ns);

ZETA_Core_ExternC_End;
