#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_CascadeAllocator);

struct Zeta_CascadeAllocator {
    size_t align;

    void* mem;
    size_t size;

    void* first_node;
};

void Zeta_CascadeAllocator_Init(void* ca);

size_t Zeta_CascadeAllocator_GetAlign(void const* ca);

size_t Zeta_CascadeAllocator_Query(void const* ca, size_t size);

void* Zeta_CascadeAllocator_Allocate(void* ca, size_t size);

void Zeta_CascadeAllocator_Deallocate(void* ca, void* ptr);

void Zeta_CascadeAllocator_Check(void const* ca);

void Zeta_CascadeAllocator_Sanitize(void const* ca,
                                    Zeta_Core_MemRecorder* dst_recorder);

extern Zeta_Core_Allocator_VTable const
    Zeta_Core_CascadeAllocator_allocator_vtable;

ZETA_Core_ExternC_End;
