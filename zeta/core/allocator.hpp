#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core {

struct Allocator {
    struct VTable {
        void* (*Allocate)(void* allocator, size_t size);

        void (*Deallocate)(void* allocator, void* ptr);
    };

    void* inst;
    void const* inst_const;

    size_t align;

    VTable const* vtable;

    // -------------------------------------------------------------------------

    static size_t GetAlign(void const* allocator);

    static void* Allocate(void* allocator, size_t size);

    static void Deallocate(void* allocator, void* ptr);

    template <typename AllocatorImpl>
    static void* SafeAllocate(AllocatorImpl* allocator_impl, size_t align,
                              size_t size);

    // -------------------------------------------------------------------------

    template <typename AllocatorImpl>
    static VTable const& MakeVTable();

    // -------------------------------------------------------------------------

    static void CheckAllocator(void* allocator);

    static void CheckAllocator(void const* allocator);
};

}  // namespace zeta::core
