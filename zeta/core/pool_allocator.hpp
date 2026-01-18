#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core {

struct PoolAllocator;

// -----------------------------------------------------------------------------

struct PoolAllocator {
    static void Init(void* pa);

    static size_t GetAlign(void const* pa);

    static void* Allocate(void* pa, size_t size);

    static void Deallocate(void* pa, void* ptr);

    // -------------------------------------------------------------------------

    void* n;
};

}  // namespace zeta::core
