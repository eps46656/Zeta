#include <zeta/core/debugger.h>
#include <zeta/core/pool_allocator.h>

void Zeta_Core_PoolAllocator_Init(void* pa_) {
    Zeta_Core_PoolAllocator* pa = pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    pa->n = NULL;
}

size_t Zeta_Core_PoolAllocator_GetAlign(void const* pa_) {
    Zeta_Core_PoolAllocator const* pa = pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    return 1;
}

size_t Zeta_Core_PoolAllocator_Query(void const* pa_, size_t size) {
    Zeta_Core_PoolAllocator const* pa = pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    return size;
}

void* Zeta_Core_PoolAllocator_Allocate(void* pa_, size_t size) {
    Zeta_Core_PoolAllocator* pa = pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    if (pa->n == NULL || size == 0) { return NULL; }

    void** n = pa->n;
    pa->n = *n;
    *n = NULL;

    return n;
}

void Zeta_Core_PoolAllocator_Deallocate(void* pa_, void* ptr) {
    Zeta_Core_PoolAllocator* pa = pa_;
    ZETA_Core_DebugAssert(pa != NULL);

    if (ptr == NULL) { return; }

    void** n = ptr;
    *n = pa->n;
    pa->n = n;
}

Zeta_Core_Allocator_VTable const Zeta_Core_PoolAllocator_allocator_vtable = {
    .GetAlign = Zeta_Core_PoolAllocator_GetAlign,

    .Query = Zeta_Core_PoolAllocator_Query,

    .Allocate = Zeta_Core_PoolAllocator_Allocate,

    .Deallocate = Zeta_Core_PoolAllocator_Deallocate,
};
