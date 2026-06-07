#pragma once

#include <zeta/core/allocator_ref.hpp>
#include <zeta/core/debug_utils.ipp>

namespace zeta::core {

#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method, ...)                       \
    {                                                 \
        auto method_ptr{ ref.vtable->method };        \
        ZETA_Core_DebugAssert(method_ptr != nullptr); \
                                                      \
        return method_ptr(ref.alctr, __VA_ARGS__);    \
    }                                                 \
    ZETA_Core_StaticAssert(true);

inline size_t allocator_ref::GetAlign(Ref const& ref) { return ref.align; }

inline void* allocator_ref::Allocate(Ref& ref, size_t size) {
    CallMethod(Allocate, size);
}

inline void allocator_ref::Deallocate(Ref& ref, void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")

inline void allocator_ref::CheckRef(Ref& ref) {
    ZETA_Core_DebugAssert(0 < ref.align);
    ZETA_Core_DebugAssert(ref.vtable != nullptr);
    ZETA_Core_DebugAssert(ref.alctr != nullptr);

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method) \
    ZETA_Core_DebugAssert(ref.vtable->method != nullptr);

    CheckMethod(Allocate);
    CheckMethod(Deallocate);

#pragma pop_macro("CheckMethod")
}

template <typename Allocator>
allocator_ref::Ref allocator_ref::MakeRef  // NOLINT(misc-use-internal-linkage)
    (Allocator& alctr) {
    allocator::CheckContract(alctr);

    return {
        .align = allocator::GetAlign(alctr),
        .vtable = &allocator::GetVTable<Allocator>(),

        .alctr = const_cast<void*>(static_cast<void const*>(&alctr)),
    };
}

inline void*
allocator::AllocatorTraits<allocator_ref::Ref const>::GetReferedInstPtr(
    allocator_ref::Ref const& ref) {
    return ref.alctr;
}

inline size_t allocator::AllocatorTraits<allocator_ref::Ref const>::GetAlign(
    allocator_ref::Ref const& ref) {
    return allocator_ref::GetAlign(ref);
}

inline void* allocator::AllocatorTraits<allocator_ref::Ref>::Allocate(
    allocator_ref::Ref& ref, size_t size) {
    return allocator_ref::Allocate(ref, size);
}

inline void allocator::AllocatorTraits<allocator_ref::Ref>::Deallocate(
    allocator_ref::Ref& ref, void* ptr) {
    allocator_ref::Deallocate(ref, ptr);
}

}  // namespace zeta::core
