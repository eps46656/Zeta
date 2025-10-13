#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/pool_allocator.hpp>

namespace zeta::core {

allocator::AllocatorVTable const PoolAllocator::allocator_vtable{
    .Allocate = &PoolAllocator::Allocate,
    .Deallocate = &PoolAllocator::Deallocate,
};

// -----------------------------------------------------------------------------

void PoolAllocator::Init(void* pa_) {
    auto* pa{ static_cast<PoolAllocator*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    pa->n = nullptr;
}

size_t PoolAllocator::GetAlign(void const* pa_) {
    auto* pa{ static_cast<PoolAllocator const*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    return alignof(void*);
}

void* PoolAllocator::Allocate(void* pa_, size_t size) {
    auto* pa{ static_cast<PoolAllocator*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    if (pa->n == nullptr || size == 0) { return nullptr; }

    auto* n{ static_cast<void**>(pa->n) };
    pa->n = *n;
    *n = nullptr;

    return static_cast<void*>(n);
}

void PoolAllocator::Deallocate(void* pa_, void* ptr) {
    auto* pa{ static_cast<PoolAllocator*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    if (ptr == nullptr) { return; }

    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, alignof(void*)));

    auto** n{ static_cast<void**>(ptr) };
    *n = pa->n;
    pa->n = static_cast<void*>(n);
}

allocator::AllocatorRef PoolAllocator::GetAllocatorRef(void* pa_) {
    auto* pa{ static_cast<PoolAllocator*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    return {
        .inst = pa,
        .align = alignof(void*),
        .vtable = &allocator_vtable,
    };
}

allocator::ConstAllocatorRef PoolAllocator::GetAllocatorRef(void const* pa_) {
    auto* pa{ static_cast<PoolAllocator const*>(pa_) };
    ZETA_Core_DebugAssert(pa != nullptr);

    return {
        .inst = pa,
        .align = alignof(void*),
        .vtable = &allocator_vtable,
    };
}

}  // namespace zeta::core
