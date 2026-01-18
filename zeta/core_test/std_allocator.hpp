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

    Allocator();
    ~Allocator();

    static size_t GetAlign(void const* std_allocator);

    static void* Allocate(void* std_allocator, size_t size);

    static void Deallocate(void* std_allocator, void* ptr);

    static void Check(void const* std_allocator);
};

// -----------------------------------------------------------------------------

struct AllocatorOperator {
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

// -----------------------------------------------------------------------------

inline Allocator::Allocator() {
#if ZETA_Core_EnableDebug
    this->mem_recorder = core::MemRecorder::Create();
#else
    this->mem_recorder = nullptr;
#endif
}

inline Allocator::~Allocator() {
#if ZETA_Core_EnableDebug
    core::MemRecorder::Destroy(this->mem_recorder);
#endif
}

inline size_t Allocator::GetAlign(void const* std_allocator_) {
    auto std_allocator{ static_cast<Allocator const*>(std_allocator_) };
    Check(std_allocator);

    return alignof(max_align_t);
}

inline void* Allocator::Allocate(void* std_allocator_, size_t size) {
    auto std_allocator{ static_cast<Allocator*>(std_allocator_) };
    Check(std_allocator);

    if (size == 0) { return nullptr; }

    void* ptr{ std::malloc(size) };

#if ZETA_Core_EnableDebug
    core::MemRecorder::Record(std_allocator->mem_recorder, ptr, size);
    std_allocator->usage += size;
#endif

    return ptr;
}

inline void Allocator::Deallocate(void* std_allocator_, void* ptr) {
    auto std_allocator{ static_cast<Allocator*>(std_allocator_) };
    Check(std_allocator);

    if (ptr == nullptr) { return; }

    bool b{ core::MemRecorder::Unrecord(std_allocator->mem_recorder, ptr) };

    ZETA_Core_DebugAssert(b);

    std::free(ptr);
}

inline void Allocator::Check(void const* std_allocator_) {
    auto std_allocator{ static_cast<Allocator const*>(std_allocator_) };
    ZETA_Core_DebugAssert(std_allocator != nullptr);
}

}  // namespace zeta::core_test::std_allocator
