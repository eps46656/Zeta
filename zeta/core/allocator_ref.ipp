#pragma once

#include <zeta/core/allocator_ref.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method, ...)                       \
    {                                                 \
        auto method_ptr{ ref->vtable->method };       \
        ZETA_Core_DebugAssert(method_ptr != nullptr); \
                                                      \
        return method_ptr(ref->alctr, __VA_ARGS__);   \
    }                                                 \
    ZETA_Core_StaticAssert(true);

inline size_t allocator_ref::ops::GetAlign(Ref const* ref) {
    return ref->align;
}

inline void* allocator_ref::ops::Allocate(Ref* ref, size_t size) {
    CallMethod(Allocate, size);
}

inline void allocator_ref::ops::Deallocate(Ref* ref, void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")

inline void allocator_ref::ops::CheckRef(Ref* ref) {
    ZETA_Core_DebugAssert(ref != nullptr);
    ZETA_Core_DebugAssert(0 < ref->align);
    ZETA_Core_DebugAssert(ref->vtable != nullptr);
    ZETA_Core_DebugAssert(ref->alctr != nullptr);

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method) \
    ZETA_Core_DebugAssert(ref->vtable->method != nullptr);

    CheckMethod(Allocate);
    CheckMethod(Deallocate);

#pragma pop_macro("CheckMethod")
}

template <typename AllocatorLike>
allocator_ref::Ref
    allocator_ref::ops::MakeRef  // NOLINT(misc-use-internal-linkage)
    (AllocatorLike&& alctr_) {
    auto* alctr{ utils::GetInstPtr(meta::Forward<AllocatorLike>(alctr_)) };
    using Allocator = meta::RemovePointer<decltype(alctr)>;

    allocator::ops::CheckContract(alctr);

    return {
        .align = allocator::ops::GetAlign(alctr),
        .vtable = &allocator::ops::GetVTable<Allocator>(),

        .alctr = const_cast<void*>(static_cast<void const*>(alctr)),
    };
}

inline void* allocator::Traits<allocator_ref::Ref const, void>::GetReferedInst(
    allocator_ref::Ref const* ref) {
    return ref->alctr;
}

inline size_t allocator::Traits<allocator_ref::Ref const, void>::GetAlign(
    allocator_ref::Ref const* ref) {
    return allocator_ref::ops::GetAlign(ref);
}

inline void* allocator::Traits<allocator_ref::Ref, void>::Allocate(
    allocator_ref::Ref* ref, size_t size) {
    return allocator_ref::ops::Allocate(ref, size);
}

inline void allocator::Traits<allocator_ref::Ref, void>::Deallocate(
    allocator_ref::Ref* ref, void* ptr) {
    allocator_ref::ops::Deallocate(ref, ptr);
}

}  // namespace zeta::core
