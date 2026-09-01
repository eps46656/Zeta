#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>

namespace zeta::core {

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method_name, ...)              \
    return alctr.method_name(Tag{}, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true)

template <typename Allocator>
constexpr decltype(auto) allocator::GetReferedInstPtr(Allocator& alctr) {
    CallMethod(GetReferedInstPtr, alctr);
}

template <typename Allocator>
constexpr decltype(auto) allocator::GetAlign(Allocator& alctr) {
    size_t align{ alctr.GetAlign(Tag{}) };

    ZETA_Core_DebugAssert(0 < align);

    return align;
}

template <typename Allocator>
constexpr decltype(auto) allocator::Allocate(Allocator& alctr, size_t size) {
    CallMethod(Allocate, size);
}

template <typename Allocator>
constexpr decltype(auto) allocator::Deallocate(Allocator& alctr, void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")

template <typename Allocator>
constexpr decltype(auto) allocator::SafeAllocate(Allocator& alctr, size_t align,
                                                 size_t size) {
    size_t self_align{ (GetAlign)(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ (Allocate)(alctr, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

template <typename Allocator>
constexpr allocator::VTable allocator::BuildVTableBasic() {
    return {
        .Allocate =
            [](void* alctr, size_t size) {
                return (Allocate)(*static_cast<Allocator*>(alctr), size);
            },

        .Deallocate =
            [](void* alctr, void* ptr) {
                (Deallocate)(*static_cast<Allocator*>(alctr), ptr);
            },
    };
}

template <typename Allocator>
constexpr allocator::VTable allocator::BuildVTableImpl<Allocator>::Call() {
    return (BuildVTableBasic<Allocator>)();
}

namespace allocator::detail {

template <typename Allocator>
struct VTableHolder_ {
    static constexpr VTable vtable{ BuildVTableImpl<Allocator>::Call() };
};

}  // namespace allocator::detail

template <typename Allocator>
constexpr allocator::VTable const& allocator::GetVTable() {
    return detail::VTableHolder_<Allocator>::vtable;
}

}  // namespace zeta::core
