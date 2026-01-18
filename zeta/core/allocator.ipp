#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename AllocatorOperator, typename Allocator>
void CheckContract(AllocatorOperator const& alctr_opr, Allocator* alctr) {
    constexpr type_wrapper::TypeWrapper<Allocator*> a_ptr_type_wrapper;
    void* void_ptr{ nullptr };

    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")
#pragma push_macro("CheckConstMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(alctr_opr.method(__VA_ARGS__)), return_type>);

// NOLINTEND(cppcoreguidelines-macro-usage)
#define CheckConstMethod(method, return_type, ...) \
    if constexpr (is_const) { CheckMethod(method, return_type, __VA_ARGS__); }

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AllocatorOperator::IsConst(a_ptr_type_wrapper)),
                bool>);

    constexpr bool is_const{ AllocatorOperator::IsConst(a_ptr_type_wrapper) };

    CheckConstMethod(  //
        GetAlign,      // method
                       //
        size_t,        // return size_t
                       //
        alctr          // allocator
    );

    size_t align{ alctr_opr.GetAlign(alctr) };

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

template <typename AllocatorOperator, typename Allocator>
constexpr VTable BasicVTableBuilder<AllocatorOperator, Allocator>::Build() {
    constexpr VTable table{
        .Allocate = Allocate,
        .Deallocate = Deallocate,
    };

    return table;
}

template <typename AllocatorOperator, typename Allocator>
void* BasicVTableBuilder<AllocatorOperator, Allocator>::Allocate(
    void const* alctr_opr, void* alctr, size_t size) {
    return static_cast<AllocatorOperator const*>(alctr_opr)->Allocate(
        static_cast<Allocator*>(alctr), size);
}

template <typename AllocatorOperator, typename Allocator>
void BasicVTableBuilder<AllocatorOperator, Allocator>::Deallocate(
    void const* alctr_opr, void* alctr, void* ptr) {
    return static_cast<AllocatorOperator const*>(alctr_opr)->Deallocate(
        static_cast<Allocator*>(alctr), ptr);
}

namespace detail {

template <typename AllocatorOperator, typename Allocator>
struct VTableHolder_ {
    static constexpr VTable vtable{
        BasicVTableBuilder<AllocatorOperator, Allocator>::Build()
    };
};

}  // namespace detail

template <typename AllocatorOperator, typename Allocator>
constexpr VTable const& GetVTable() {
    return detail::VTableHolder_<AllocatorOperator, Allocator>::vtable;
}

// -----------------------------------------------------------------------------

#pragma push_macro("CallMethod_")
#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(tmp_method_ptr, method, ...)                            \
    {                                                                       \
        auto tmp_method_ptr{ a_ref->vtable->method };                       \
        ZETA_Core_DebugAssert(tmp_method_ptr != nullptr);                   \
                                                                            \
        return tmp_method_ptr(a_ref->alctr_opr, a_ref->alctr, __VA_ARGS__); \
    }                                                                       \
    ZETA_Core_StaticAssert(true);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method, ...) \
    CallMethod_(ZETA_Core_TmpName, method, __VA_ARGS__)

template <typename IsConst, typename>
size_t RefOperator::GetAlign(Ref<IsConst> const* a_ref) {
    ZETA_Core_DebugAssert(CheckAllocator(a_ref));

    return a_ref->align;
}

inline void* RefOperator::Allocate(Ref<value_wrapper::FalseType>* a_ref,
                                   size_t size) {
    CallMethod(Allocate, size);
}

inline void RefOperator::Deallocate(Ref<value_wrapper::FalseType>* a_ref,
                                    void* ptr) {
    CallMethod(Deallocate, ptr);
}

template <typename AllocatorOperator, typename A>
auto MakeRef(AllocatorOperator const& alctr_opr, A&& alctr_) {
    auto* a{ GetInstPtr(Forward<A>(alctr_)) };

    using Allocator = RemovePointer<decltype(a)>;

    CheckContract(alctr_opr, a);

    return Ref<value_wrapper::StaticValueWrapper<AllocatorOperator::IsConst(
        type_wrapper::TypeWrapper<Allocator*>{})>>{
        .align = static_cast<unsigned short>(AllocatorOperator::GetAlign(a)),
        .vtable = &GetVTable<AllocatorOperator, Allocator>(),

        .alctr_opr = &alctr_opr,
        .a = const_cast<void*>(static_cast<void const*>(a)),
    };
}

// -----------------------------------------------------------------------------

template <typename AllcoatorOperaotr, typename Allocator>
void* SafeAllocate(AllcoatorOperaotr const& alctr_opr, Allocator* alctr,
                   size_t align, size_t size) {
    CheckContract(alctr_opr, alctr);

    size_t self_align{ alctr_opr.GetAlign(alctr) };

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(self_align % align == 0);

    void* ptr{ alctr_opr.Allocate(alctr, size) };

    ZETA_Core_DebugAssert(ptr != nullptr);
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, self_align));

    return ptr;
}

inline Ref<value_wrapper::FalseType> weak_lifo_allocator{
    .align = 1,
    .vtable = nullptr,
    .alctr_opr = nullptr,
    .alctr = nullptr,
};

}  // namespace zeta::core::allocator
