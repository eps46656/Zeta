#pragma once

#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/type_wrapper.hpp>

namespace zeta::core_test::std_allocator {

struct Allocator {
    core::mem_recorder::MemRecorder mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    static size_t GetAlign(Allocator const* std_allocator) {
        Check(std_allocator);
        return alignof(max_align_t);
    }

    static void* Allocate(Allocator* std_allocator, size_t size) {
        Check(std_allocator);

        if (size == 0) { return nullptr; }

        void* ptr{ std::malloc(size) };

#if ZETA_Core_EnableDebug
        core::mem_recorder::Record(&std_allocator->mem_recorder, ptr, size);
        std_allocator->usage += size;
#endif

        return ptr;
    }

    static void Deallocate(Allocator* std_allocator, void* ptr) {
        Check(std_allocator);

        if (ptr == nullptr) { return; }

        bool b{ core::mem_recorder::Unrecord(&std_allocator->mem_recorder,
                                             ptr) };

        ZETA_Core_DebugAssert(b);

        std::free(ptr);
    }

    static void Check(Allocator const* std_allocator) {
        ZETA_Core_DebugAssert(std_allocator != nullptr);
    }
};

}  // namespace zeta::core_test::std_allocator

namespace zeta::core {

template <typename Allocator>
struct allocator::AllocatorTraits<
    Allocator, meta::EnableIf<meta::IsAnyOf<
                   Allocator, zeta::core_test::std_allocator::Allocator,
                   zeta::core_test::std_allocator::Allocator const>>> {
    static void* GetReferedInst(Allocator const* std_allocator) {
        return const_cast<void*>(static_cast<void const*>(std_allocator));
    }

    static size_t GetAlign(Allocator const* std_allocator) {
        return zeta::core_test::std_allocator::Allocator::GetAlign(
            std_allocator);
    }

    static void* Allocate(Allocator* std_allocator, size_t size) {
        return zeta::core_test::std_allocator::Allocator::Allocate(
            std_allocator, size);
    }

    static void Deallocate(Allocator* std_allocator, void* ptr) {
        zeta::core_test::std_allocator::Allocator::Deallocate(std_allocator,
                                                              ptr);
    }
};

}  // namespace zeta::core
