#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>

#pragma push_macro("Call_")
#pragma push_macro("ConstCall_")

#define Call_(func, ...)                                                  \
    auto allocator_ref{ static_cast<AllocatorRefTpl*>(allocator_ref_) };  \
                                                                          \
    ZETA_Core_DebugAssert(CheckAllocator(allocator_ref));                 \
                                                                          \
    return allocator_ref->vtable->func(allocator_ref->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

#define ConstCall_(func, ...)                                                  \
    auto allocator_ref{ static_cast<AllocatorRefTpl const*>(allocator_ref_) }; \
                                                                               \
    ZETA_Core_DebugAssert(CheckAllocator(allocator_ref));                      \
                                                                               \
    return allocator_ref->vtable->func(allocator_ref->inst, __VA_ARGS__);      \
    ZETA_Core_StaticAssert(true);

namespace zeta::core::allocator {

template <typename AllocatorImpl>
bool CheckAllocator(AllocatorImpl const* const_allocator_inst) {
    AllocatorImpl* allocator_inst{ const_cast<AllocatorImpl*>(
        const_allocator_inst) };

    ZETA_Core_StaticAssert(IsAnyOf<RemoveCVRef<decltype(AllocatorImpl::GetAlign(
                                       const_allocator_inst))>,
                                   size_t>);

    size_t align{ AllocatorImpl::GetAlign(const_allocator_inst) };

    if (!(0 < align)) { return false; }

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AllocatorImpl::Allocate(allocator_inst, size_t{ 0 })),
                void*>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AllocatorImpl::Deallocate(allocator_inst,
                                                   static_cast<void*>(nullptr)),
                         0),
                int>);
}

// -----------------------------------------------------------------------------

template <typename AllocatorImpl>
AllocatorVTable const& AllocatorVTable::Make() {
    static AllocatorVTable vtable{
        .Allocate = AllocatorImpl::Allocate,

        .Deallocate = AllocatorImpl::Deallocate,
    };

    return vtable;
}

// -----------------------------------------------------------------------------

template <typename IsConst>
size_t AllocatorRefTpl<IsConst>::GetAlign(void const* allocator_ref_) {
    auto allocator_ref{ static_cast<AllocatorRef const*>(allocator_ref_) };

    ZETA_Core_DebugAssert(CheckAllocator(allocator_ref));

    return allocator_ref->align;
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AllocatorRefTpl<IsConst>::Allocate(
    void* allocator_ref_, size_t size) {
    Call_(Allocate, size);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void, _> AllocatorRefTpl<IsConst>::Deallocate(
    void* allocator_ref_, void* ptr) {
    Call_(Deallocate, ptr);
}

template <typename IsConst>
bool AllocatorRefTpl<IsConst>::CheckAllocator(void const* allocator_ref_) {
    auto allocator_ref{ static_cast<AllocatorRefTpl<IsConst> const*>(
        allocator_ref_) };
    if (!(allocator_ref != nullptr)) { return false; }

    if (!(allocator_ref->inst != nullptr)) { return false; }
    if (!(allocator_ref->vtable != nullptr)) { return false; }

    return true;
}

// -----------------------------------------------------------------------------

template <typename AllocatorImpl>
void* SafeAllocate(AllocatorImpl* allocator_impl, size_t align, size_t size) {
    size_t self_align{ AllocatorImpl::GetAlign(allocator_impl) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ AllocatorImpl::Allocate(allocator_impl, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

inline AllocatorRef weak_lifo_allocator{
    .inst = nullptr,    //
    .align = 1,         //
    .vtable = nullptr,  //
};

}  // namespace zeta::core::allocator

#pragma pop_macro("Call_")
#pragma pop_macro("ConstCall_")
