#pragma once

#include <cstddef>
#include <cstdlib>
#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core_test::std_allocator {

struct Allocator {
    core::mem_recorder::MemRecorder mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    static size_t GetAlign(Allocator const& alctr) {
        Check(alctr);
        return alignof(max_align_t);
    }

    static void* Allocate(Allocator& alctr, size_t size) {
        Check(alctr);

        if (size == 0) { return nullptr; }

        void* ptr{ std::malloc(size) };

#if ZETA_Core_EnableDebug
        core::mem_recorder::Record(alctr.mem_recorder, ptr, size);
        alctr.usage += size;
#endif

        return ptr;
    }

    static void Deallocate(Allocator& alctr, void* ptr) {
        Check(alctr);

        if (ptr == nullptr) { return; }

        bool b{ core::mem_recorder::Unrecord(alctr.mem_recorder, ptr) };

        ZETA_Core_DebugAssert(b);

        std::free(ptr);
    }

    static void Check(Allocator const&) {}
};

}  // namespace zeta::core_test::std_allocator

namespace zeta::core {

template <>
struct allocator::AllocatorTraits<core_test::std_allocator::Allocator> {
    static void* GetReferedInstPtr(
        core_test::std_allocator::Allocator const& alctr) {
        return const_cast<void*>(static_cast<void const*>(&alctr));
    }

    static size_t GetAlign(core_test::std_allocator::Allocator const& alctr) {
        return zeta::core_test::std_allocator::Allocator::GetAlign(alctr);
    }

    static void* Allocate(core_test::std_allocator::Allocator& alctr,
                          size_t size) {
        return zeta::core_test::std_allocator::Allocator::Allocate(alctr, size);
    }

    static void Deallocate(core_test::std_allocator::Allocator& alctr,
                           void* ptr) {
        zeta::core_test::std_allocator::Allocator::Deallocate(alctr, ptr);
    }
};

}  // namespace zeta::core
