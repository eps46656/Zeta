#pragma once

#include <unordered_map>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/mem_check_utils.hpp>

namespace zeta::core_test {

struct StdAllocator {
    core::MemRecorder* mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    StdAllocator();
    ~StdAllocator();

    static size_t GetAlign(void const* std_allocator);

    static void* Allocate(void* std_allocator, size_t size);

    static void Deallocate(void* std_allocator, void* ptr);

    static core::Allocator ToAllocator(void* std_allocatar);

    static void Check(void const* std_allocator);
};

// -----------------------------------------------------------------------------

StdAllocator::StdAllocator() {
#if ZETA_Core_EnableDebug
    this->mem_recorder = core::MemRecorder::Create();
#else
    this->mem_recorder = nullptr;
#endif
}

StdAllocator::~StdAllocator() {
#if ZETA_Core_EnableDebug
    core::MemRecorder::Destroy(this->mem_recorder);
#endif
}

size_t StdAllocator::GetAlign(void const* std_allocator_) {
    auto std_allocator{ static_cast<StdAllocator const*>(std_allocator_) };
    Check(std_allocator);

    return alignof(max_align_t);
}

void* StdAllocator::Allocate(void* std_allocator_, size_t size) {
    auto std_allocator{ static_cast<StdAllocator*>(std_allocator_) };
    Check(std_allocator);

    if (size == 0) { return nullptr; }

    void* ptr{ std::malloc(size) };

#if ZETA_Core_EnableDebug
    core::MemRecorder::Record(std_allocator->mem_recorder, ptr, size);
    std_allocator->usage += size;
#endif

    return ptr;
}

void StdAllocator::Deallocate(void* std_allocator_, void* ptr) {
    auto std_allocator{ static_cast<StdAllocator*>(std_allocator_) };
    Check(std_allocator);

    if (ptr == nullptr) { return; }

#if ZETA_Core_EnableDebug
    ZETA_Core_DebugAssert(
        core::MemRecorder::Unrecord(std_allocator->mem_recorder, ptr));
#endif

    std::free(ptr);
}

core::Allocator StdAllocator::ToAllocator(void* std_allocator_) {
    auto std_allocator{ static_cast<StdAllocator*>(std_allocator_) };
    Check(std_allocator);

    return {
        .inst = std_allocator,
        .inst_const = std_allocator,

        .align = alignof(max_align_t),

        .vtable = &core::Allocator::MakeVTable<StdAllocator>(),
    };
}

void StdAllocator::Check(void const* std_allocator_) {
    auto std_allocator{ static_cast<StdAllocator const*>(std_allocator_) };
    ZETA_Core_DebugAssert(std_allocator != nullptr);
}

}  // namespace zeta::core_test
