#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ret, method_name, ...)                             \
    using Allocator = meta::RemovePointer<decltype(utils::GetInstPtr( \
        meta::Forward<AllocatorLike>(alctr)))>;                       \
                                                                      \
    if constexpr (ret) {                                              \
        return Traits<Allocator>::method_name(__VA_ARGS__);           \
    } else {                                                          \
        Traits<Allocator>::method_name(__VA_ARGS__);                  \
    };                                                                \
                                                                      \
    ZETA_Core_StaticAssert(true)

template <typename AllocatorLike>
void* allocator::ops::GetReferedInst(AllocatorLike&& alctr) {
    CallMethod(true, GetReferedInst, meta::Forward<AllocatorLike>(alctr));
}

template <typename AllocatorLike>
size_t allocator::ops::GetAlign(AllocatorLike&& alctr_) {
    auto* alctr{ utils::GetInstPtr(meta::Forward<AllocatorLike>(alctr_)) };

    using Allocator = meta::RemovePointer<decltype(alctr)>;

    size_t align{ Traits<Allocator>::GetAlign(alctr) };

    ZETA_Core_DebugAssert(0 < align);

    return align;
}

template <typename AllocatorLike>
void* allocator::ops::Allocate(AllocatorLike&& alctr, size_t size) {
    CallMethod(true, Allocate,
               utils::GetInstPtr(meta::Forward<AllocatorLike>(alctr)), size);
}

template <typename AllocatorLike>
void allocator::ops::Deallocate(AllocatorLike&& alctr, void* ptr) {
    CallMethod(false, Deallocate,
               utils::GetInstPtr(meta::Forward<AllocatorLike>(alctr)), ptr);
}

#pragma pop_macro("CallMethod")

template <typename AllocatorLike>
void* allocator::ops::SafeAllocate(AllocatorLike&& alctr_, size_t align,
                                   size_t size) {
    auto* alctr{ utils::GetInstPtr(meta::Forward<AllocatorLike&&>(alctr_)) };

    (CheckContract)(alctr);

    size_t self_align{ (GetAlign)(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ (Allocate)(alctr, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

template <typename AllocatorLike>
void allocator::ops::CheckContract(AllocatorLike&& alctr_) {
    auto* alctr{ utils::GetInstPtr(meta::Forward<AllocatorLike&&>(alctr_)) };

    void* void_ptr{ nullptr };

    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                            \
    {                                                       \
        ZETA_Core_Unused([=]() { (method)(__VA_ARGS__); }); \
    }                                                       \
                                                            \
    ZETA_Core_StaticAssert(true);

    CheckMethod(         //
        GetReferedInst,  // method
                         //
        alctr            // allocator
    );

    CheckMethod(   //
        GetAlign,  // method
                   //
        alctr      // allocator
    );

    CheckMethod(   //
        Allocate,  // method
                   //
        alctr,     // allocator
        size_val   // size
    );

    CheckMethod(     //
        Deallocate,  // method
                     //
        alctr,       // allocator
        void_ptr     // ptr
    );

#pragma push_macro("CheckMethod")
}

template <typename Allocator>
constexpr allocator::VTable allocator::ops::BuildVTableBasic() {
    return {
        .Allocate =
            [](void* alctr, size_t size) {
                return (Allocate)(static_cast<Allocator*>(alctr), size);
            },

        .Deallocate =
            [](void* alctr, void* ptr) {
                (Deallocate)(static_cast<Allocator*>(alctr), ptr);
            },
    };
}

template <typename Allocator, typename En>
constexpr allocator::VTable
allocator::ops::BuildVTableImpl<Allocator, En>::Call() {
    return (BuildVTableBasic<Allocator>)();
}

namespace allocator::ops::detail {

template <typename Allocator>
struct VTableHolder_ {
    static constexpr VTable vtable{ BuildVTableImpl<Allocator>::Call() };
};

}  // namespace allocator::ops::detail

template <typename Allocator>
constexpr allocator::VTable const& allocator::ops::GetVTable() {
    return detail::VTableHolder_<Allocator>::vtable;
}

}  // namespace zeta::core
