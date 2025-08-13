
#include <zeta/core/allocator.h>
#include <zeta/core/debugger.h>

#include <unordered_map>
#include <vector>

struct BufferdPoolAllocator {
    size_t size;

    size_t max_buffered_ptrs_num;
    std::vector<void*> buffered_ptrs;

    std::unordered_map<unsigned long long, unsigned long long> records;
    size_t usage;
};

size_t BufferdPoolAllocator_GetAlign(void const* pa_) {
    ZETA_Core_Unused(pa_);
    return alignof(max_align_t);
}

size_t BufferdPoolAllocator_Query(void const* pa_, size_t size) {
    BufferdPoolAllocator const* pa = (BufferdPoolAllocator const*)pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    if (size == 0) { return 0; }
    if (size <= pa->size) { return pa->size; }
    return 0;
}

void* BufferdPoolAllocator_Allocate(void* pa_, size_t size) {
    BufferdPoolAllocator* pa = (BufferdPoolAllocator*)pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    if (size == 0 || pa->size < size) { return NULL; }

    void* ptr;

    if (!pa->buffered_ptrs.empty()) {
        ptr = pa->buffered_ptrs.back();
        pa->buffered_ptrs.pop_back();
    } else {
        ptr = malloc(pa->size);
    }

#if ZETA_Core_EnableDebug
    bool_t b{
        pa->records.insert({ ZETA_Core_PtrToAddr(ptr), pa->size }).second
    };
    ZETA_Core_DebugAssert(b);
    pa->usage += pa->size;
#endif

    return ptr;
}

void BufferdPoolAllocator_Deallocate(void* pa_, void* ptr) {
    BufferdPoolAllocator* pa = (BufferdPoolAllocator*)pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    if (ptr == NULL) { return; }

#if ZETA_Core_EnableDebug
    auto iter{ pa->records.find(ZETA_Core_PtrToAddr(ptr)) };
    ZETA_Core_DebugAssert(iter != pa->records.end());
    pa->usage -= iter->second;
    pa->records.erase(iter);
#endif

    if (pa->buffered_ptrs.size() < pa->max_buffered_ptrs_num) {
        pa->buffered_ptrs.push_back(ptr);
    } else {
        free(ptr);
    }
}

size_t BufferdPoolAllocator_GetUsage(void const* pa_) {
    BufferdPoolAllocator* pa = (BufferdPoolAllocator*)pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    return pa->usage;
}

extern Zeta_Core_Allocator_VTable const buffered_pool_allocator_vtable = {
    .GetAlign = BufferdPoolAllocator_GetAlign,

    .Query = BufferdPoolAllocator_Query,

    .Allocate = BufferdPoolAllocator_Allocate,

    .Deallocate = BufferdPoolAllocator_Deallocate,
};
