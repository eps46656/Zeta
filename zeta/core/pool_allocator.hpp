#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core {

struct PoolAllocator;

// -----------------------------------------------------------------------------

struct PoolAllocator {
    static allocator::AllocatorVTable const allocator_vtable;

    // -------------------------------------------------------------------------

    static void Init(void* pa);

    static size_t GetAlign(void const* pa);

    static void* Allocate(void* pa, size_t size);

    static void Deallocate(void* pa, void* ptr);

    static allocator::AllocatorRef GetAllocatorRef(void* pa);

    static allocator::ConstAllocatorRef GetAllocatorRef(void const* pa);

    // -------------------------------------------------------------------------

    void* n;
};

}  // namespace zeta::core
