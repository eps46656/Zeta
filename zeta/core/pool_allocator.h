#pragma once

#include <zeta/core/allocator.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_PoolAllocator);

struct Zeta_Core_PoolAllocator {
    void* n;
};

void Zeta_Core_PoolAllocator_Init(void* pa);

size_t Zeta_Core_PoolAllocator_GetAlign(void const* pa);

size_t Zeta_Core_PoolAllocator_Query(void const* pa, size_t size);

void* Zeta_Core_PoolAllocator_Allocate(void* pa, size_t size);

void Zeta_Core_PoolAllocator_Deallocate(void* pa, void* ptr);

extern Zeta_Core_Allocator_VTable const
    Zeta_Core_PoolAllocator_allocator_vtable;

ZETA_Core_ExternC_End;
