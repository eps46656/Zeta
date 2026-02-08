#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename AllocatorLike>
void CheckContract(AllocatorLike&& alctr_) {
    auto* alctr{ GetInstPtr(Forward<AllocatorLike&&>(alctr_)) };
    using Allocator = RemovePointer<decltype(alctr)>;

    void* void_ptr{ nullptr };

    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(Allocator::method(__VA_ARGS__)), return_type>);

    CheckMethod(   //
        GetAlign,  // method
                   //
        size_t,    // return size_t
                   //
        alctr      // allocator
    );

    size_t align{ Allocator::GetAlign(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(align <= ZETA_Core_ushrt_max);

    CheckMethod(   //
        Allocate,  // method
                   //
        void*,     // return void*
                   //
        alctr,     // allocator
        size_val   // size
    );

    CheckMethod(     //
        Deallocate,  // method
                     //
        void,        // return void
                     //
        alctr,       // allocator
        void_ptr     // ptr
    );

#pragma pop_macro("CheckMethod")
}

template <typename Allocator>
constexpr VTable BasicVTableBuilder<Allocator>::Build() {
    constexpr VTable table{
        .Allocate = Allocate,
        .Deallocate = Deallocate,
    };

    return table;
}

template <typename Allocator>
void* BasicVTableBuilder<Allocator>::Allocate(void* alctr, size_t size) {
    return Allocator::Allocate(static_cast<Allocator*>(alctr), size);
}

template <typename Allocator>
void BasicVTableBuilder<Allocator>::Deallocate(void* alctr, void* ptr) {
    return Allocator::Deallocate(static_cast<Allocator*>(alctr), ptr);
}

namespace detail {

template <typename Allocator>
struct VTableHolder_ {
    static constexpr VTable vtable{ BasicVTableBuilder<Allocator>::Build() };
};

}  // namespace detail

template <typename Allocator>
constexpr VTable const& GetVTable() {
    return detail::VTableHolder_<Allocator>::vtable;
}

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

inline size_t Ref::GetAlign(Ref const* ref) { return ref->align; }

inline void* Ref::Allocate(Ref* ref, size_t size) {
    CallMethod(Allocate, size);
}

inline void Ref::Deallocate(Ref* ref, void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")

namespace ops {

#pragma push_macro("CheckMethod")

#define CheckMethod(method, ...)                                 \
    auto* alctr{ GetInstPtr(Forward<AllocatorLike&&>(alctr_)) }; \
    using Allocator = RemovePointer<decltype(alctr)>;            \
                                                                 \
    CheckContract(alctr);                                        \
                                                                 \
    return Allocator::method(alctr, __VA_ARGS__);

template <typename AllocatorLike>
size_t GetAlign(AllocatorLike&& alctr_) {
    CheckMethod(GetAlign);
}

template <typename AllocatorLike>
void* Allocate(AllocatorLike&& alctr_, size_t size) {
    CheckMethod(Allocate, size);
}

template <typename AllocatorLike>
void Deallocate(AllocatorLike&& alctr_, void* ptr) {
    CheckMethod(Deallocate, ptr);
}

#pragma pop_macro("CheckMethod")

template <typename AllocatorLike>
void* SafeAllocate(AllocatorLike&& alctr_, size_t align, size_t size) {
    auto* alctr{ GetInstPtr(Forward<AllocatorLike&&>(alctr_)) };
    using Allocator = RemovePointer<decltype(alctr)>;

    CheckContract(alctr);

    size_t self_align{ Allocator::GetAlign(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ Allocator::Allocate(alctr, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

template <typename AllocatorLike>
Ref MakeRef(AllocatorLike&& alctr_) {
    auto* alctr{ GetInstPtr(Forward<AllocatorLike&&>(alctr_)) };
    using Allocator = RemovePointer<decltype(alctr)>;

    CheckContract(alctr);

    return {
        .align = static_cast<unsigned short>(Allocator::GetAlign(alctr)),
        .vtable = &GetVTable<Allocator>(),

        .alctr = const_cast<void*>(static_cast<void const*>(alctr)),
    };
}

}  // namespace ops

inline Ref weak_lifo_allocator{
    .align = 1,
    .vtable = nullptr,
    .alctr = nullptr,
};

}  // namespace zeta::core::allocator
