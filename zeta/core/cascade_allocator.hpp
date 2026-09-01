#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/llist_node_tpl.hpp>
#include <zeta/core/mem_check_utils.hpp>

namespace zeta::core {

struct CascadeAllocator;

// -----------------------------------------------------------------------------

struct CascadeAllocator {
    using LListNode = LListNodeTpl<void*, false, false>;

    size_t align;

    void* mem;
    size_t size;

    LListNode* last_node;

    static void Init(void* ca);

    static size_t GetAlign(void const* ca);

    static size_t Query(void const* ca, size_t size);

    static void* Allocate(void* ca, size_t size);

    static void Deallocate(void* ca, void* ptr);

    static bool CheckAllocator(void const* ca);

    static allocator::PolyAllocator GetAlloccatorRef(void* ca);

    static allocator::ConstAllocatorRef GetAlloccatorRef(void const* ca);

    static void Sanitize(void const* ca, MemRecorder* dst_recorder);
};

}  // namespace zeta::core
