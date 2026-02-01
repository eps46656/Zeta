#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename Allocator>
void CheckContract(Allocator* alctr) {
    constexpr type_wrapper::TypeWrapper<Allocator*> allocator_ptr_type_wrapper;
    void* void_ptr{ nullptr };

    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")
#pragma push_macro("CheckConstMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(Allocator::method(__VA_ARGS__)), return_type>);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckConstMethod(method, return_type, ...) \
    if constexpr (is_const) { CheckMethod(method, return_type, __VA_ARGS__); }

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(Allocator::IsConst(allocator_ptr_type_wrapper)),
                bool>);

    constexpr bool is_const{ Allocator::IsConst(allocator_ptr_type_wrapper) };

    CheckConstMethod(  //
        GetAlign,      // method
                       //
        size_t,        // return size_t
                       //
        alctr          // allocator
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
#pragma pop_macro("CheckConstMethod")
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

// -----------------------------------------------------------------------------

#pragma push_macro("CallMethod_")
#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method, ...)                             \
    {                                                       \
        auto* ref{ static_cast<Ref<ConstTag>*>(ref_view) }; \
                                                            \
        auto method_ptr{ ref->vtable->method };             \
        ZETA_Core_DebugAssert(method_ptr != nullptr);       \
                                                            \
        return method_ptr(ref->alctr, __VA_ARGS__);         \
    }                                                       \
    ZETA_Core_StaticAssert(true);

template <typename ConstTag>
size_t RefView<ConstTag>::GetAlign(RefView const* ref_view) {
    auto* ref{ static_cast<Ref<ConstTag>*>(ref_view) };

    return ref->align;
}

template <typename ConstTag>
void* RefView<ConstTag>::Allocate(RefView* ref_view, size_t size) {
    CallMethod(Allocate, size);
}

template <typename ConstTag>
void RefView<ConstTag>::Deallocate(RefView* ref_view, void* ptr) {
    CallMethod(Deallocate, ptr);
}

template <typename Allocator>
auto MakeRef(Allocator* alctr) {
    CheckContract(alctr);

    return Ref<value_wrapper::StaticValueWrapper<Allocator::IsConst(
        static_cast<Allocator*>(nullptr))>>{
        .align = static_cast<unsigned short>(Allocator::GetAlign(alctr)),
        .vtable = &GetVTable<Allocator>(),

        .alctr = const_cast<void*>(static_cast<void const*>(alctr)),
    };
}

// -----------------------------------------------------------------------------

template <typename Allocator>
void* SafeAllocate(Allocator* alctr, size_t align, size_t size) {
    CheckContract(alctr);

    size_t self_align{ Allocator::GetAlign(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ Allocator::Allocate(alctr, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

inline Ref<value_wrapper::FalseType> weak_lifo_allocator{
    .align = 1,
    .vtable = nullptr,
    .alctr = nullptr,
};

}  // namespace zeta::core::allocator
