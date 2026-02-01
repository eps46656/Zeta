#pragma once

#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/type_wrapper.hpp>

namespace zeta::core_test::std_allocator {

struct Allocator {
    core::MemRecorder* mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    Allocator() {
#if ZETA_Core_EnableDebug
        this->mem_recorder = core::MemRecorder::Create();
#else
        this->mem_recorder = nullptr;
#endif
    }

    ~Allocator() {
#if ZETA_Core_EnableDebug
        core::MemRecorder::Destroy(this->mem_recorder);
#endif
    }

    static constexpr bool IsConst(core::type_wrapper::TypeWrapper<Allocator*>) {
        return false;
    }

    static constexpr bool IsConst(
        core::type_wrapper::TypeWrapper<Allocator const*>) {
        return true;
    }

    static size_t GetAlign(Allocator const* std_allocator) {
        Check(std_allocator);
        return alignof(max_align_t);
    }

    static void* Allocate(Allocator* std_allocator, size_t size) {
        Check(std_allocator);

        if (size == 0) { return nullptr; }

        void* ptr{ std::malloc(size) };

#if ZETA_Core_EnableDebug
        core::MemRecorder::Record(std_allocator->mem_recorder, ptr, size);
        std_allocator->usage += size;
#endif

        return ptr;
    }

    static void Deallocate(Allocator* std_allocator, void* ptr) {
        Check(std_allocator);

        if (ptr == nullptr) { return; }

        bool b{ core::MemRecorder::Unrecord(std_allocator->mem_recorder, ptr) };

        ZETA_Core_DebugAssert(b);

        std::free(ptr);
    }

    static void Check(Allocator const* std_allocator) {
        ZETA_Core_DebugAssert(std_allocator != nullptr);
    }
};

// -----------------------------------------------------------------------------

struct AllocatorView {
    static constexpr bool IsConst(core::type_wrapper::TypeWrapper<Allocator*>) {
        return false;
    }

    static constexpr bool IsConst(
        core::type_wrapper::TypeWrapper<Allocator const*>) {
        return true;
    }

    static size_t GetAlign(Allocator* /* a */) { return alignof(max_align_t); }

    static void* Allocate(Allocator* a, size_t size) {
        return Allocator::Allocate(a, size);
    }

    static void Deallocate(Allocator* a, void* ptr) {
        Allocator::Deallocate(a, ptr);
    }
};

}  // namespace zeta::core_test::std_allocator
