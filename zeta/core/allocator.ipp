#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>

namespace zeta::core {

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method_name, ...)              \
    return alctr.method_name(Tag{}, __VA_ARGS__); \
    static_assert(true)

template <allocator::IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) allocator::GetReferedInstPtr(Allocator&& alctr) {
    CallMethod(GetReferedInstPtr, alctr);
}

template <allocator::IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) allocator::GetAlign(Allocator&& alctr) {
    size_t align{ alctr.GetAlign(Tag{}) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < align);

    return align;
}

template <allocator::IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) allocator::Allocate(Allocator&& alctr, size_t size) {
    CallMethod(Allocate, size);
}

template <allocator::IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) allocator::Deallocate(Allocator&& alctr, void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")

template <allocator::IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) allocator::SafeAllocate(Allocator&& alctr,
                                                 size_t align, size_t size) {
    size_t self_align{ (GetAlign)(alctr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < align);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_align % align == 0);

    void* ptr{ (Allocate)(alctr, size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        __builtin_is_aligned(ptr, self_align));

    return ptr;
}

namespace allocator::detail {

template <allocator::IsAllocator Allocator>
constexpr allocator::VTable BuildVTable_() {
    return {
        .Allocate{ [](void* alctr, size_t size) {
            return (Allocate)(*static_cast<Allocator*>(alctr), size);
        } },

        .Deallocate{ [](void* alctr, void* ptr) {
            (Deallocate)(*static_cast<Allocator*>(alctr), ptr);
        } },
    };
}

template <IsAllocator Allocator>
inline constexpr VTable vtable_{ (BuildVTable_<Allocator>)() };

}  // namespace allocator::detail

template <allocator::IsAllocator Allocator>
constexpr allocator::VTable const& allocator::GetVTable() {
    return detail::vtable_<Allocator>;
}

constexpr void* allocator::EmptyAllocator::GetReferedInstPtr(Tag) {
    return nullptr;
}

constexpr size_t allocator::EmptyAllocator::GetAlign(Tag) { return 1; }

constexpr void* allocator::EmptyAllocator::Allocate(Tag, size_t) {
    return nullptr;
}

constexpr void allocator::EmptyAllocator::Deallocate(Tag, void*) {}

}  // namespace zeta::core
