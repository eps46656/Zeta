#pragma once

#include <zeta/core/allocator.hpp>

namespace zeta::core {

struct PoolAllocator;

// -----------------------------------------------------------------------------

struct PoolAllocator {
    void* n;

    static void Init(void* pa);

    static size_t GetAlign(void const* pa);

    static void* Allocate(void* pa, size_t size);

    static void Deallocate(void* pa, void* ptr);

    static Allocator ToAllocator(void* pa);
};

}  // namespace zeta::core
