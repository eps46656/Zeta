#pragma once

#include <zeta/core/allocator.h>
#include <zeta/core/debug_hash_table.h>
#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>

#include <unordered_map>

struct Zeta_CoreTest_StdAllocator {
    Zeta_Core_MemRecorder* mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    Zeta_CoreTest_StdAllocator();
    ~Zeta_CoreTest_StdAllocator();
};

Zeta_CoreTest_StdAllocator::Zeta_CoreTest_StdAllocator() {
#if ZETA_Core_EnableDebug
    this->mem_recorder = Zeta_Core_MemRecorder_Create();
#else
    this->mem_recorder = NULL;
#endif
}

Zeta_CoreTest_StdAllocator::~Zeta_CoreTest_StdAllocator() {
#if ZETA_Core_EnableDebug
    Zeta_Core_MemRecorder_Destroy(this->mem_recorder);
#endif
}

size_t Zeta_CoreTest_StdAllocator_GetAlign(void const* sa_) {
    ZETA_Core_Unused(sa_);
    return alignof(max_align_t);
}

size_t Zeta_CoreTest_StdAllocator_Query(void const* sa_, size_t size) {
    ZETA_Core_Unused(sa_);
    return 0 < size ? size : 0;
}

void* Zeta_CoreTest_StdAllocator_Allocate(void* sa_, size_t size) {
    Zeta_CoreTest_StdAllocator* sa = (Zeta_CoreTest_StdAllocator*)sa_;
    ZETA_Core_DebugAssert(sa != NULL);

    if (size == 0) { return NULL; }

    void* ptr = std::malloc(size);

#if ZETA_Core_EnableDebug
    Zeta_Core_MemRecorder_Record(sa->mem_recorder, ptr, size);
    sa->usage += size;
#endif

    return ptr;
}

void Zeta_CoreTest_StdAllocator_Deallocate(void* sa_, void* ptr) {
    Zeta_CoreTest_StdAllocator* sa = (Zeta_CoreTest_StdAllocator*)sa_;
    ZETA_Core_DebugAssert(sa != NULL);

    if (ptr == NULL) { return; }

#if ZETA_Core_EnableDebug
    ZETA_Core_DebugAssert(
        Zeta_Core_MemRecorder_Unrecord(sa->mem_recorder, ptr));
#endif

    std::free(ptr);
}

Zeta_Core_Allocator_VTable const Zeta_CoreTest_StdAllocator_allocator_vtable = {
    .GetAlign = Zeta_CoreTest_StdAllocator_GetAlign,

    .Query = Zeta_CoreTest_StdAllocator_Query,

    .Allocate = Zeta_CoreTest_StdAllocator_Allocate,

    .Deallocate = Zeta_CoreTest_StdAllocator_Deallocate,
};
