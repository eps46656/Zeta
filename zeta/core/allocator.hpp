#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename AllocatorOperator, typename Allocator>
void CheckContract(AllocatorOperator const& alctr_opr, Allocator* alctr);

struct VTable {
    template <typename AllocatorOprator, typename Allocator>
    static constexpr VTable const& Build();

    void* (*Allocate)(void const* alctr_opr, void* alctr, size_t size);

    void (*Deallocate)(void const* alctr_opr, void* alctr, void* ptr);
};

template <typename AllocatorOprator, typename Allocator>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static void* Allocate(void const* alctr_opr, void* alctr, size_t size);

    static void Deallocate(void const* alctr_opr, void* alctr, void* ptr);
};

template <typename AllocatorOperator, typename Allocator>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename AllocatorOperator, typename Allocator>
constexpr VTable const& GetVTable();

template <typename IsConst>
struct Ref {
    ZETA_Core_StaticAssert(
        IsAnyOf<IsConst, value_wrapper::FalseType, value_wrapper::TrueType>);

    unsigned short align;

    VTable const* vtable;

    void const* alctr_opr;
    void* alctr;
};

#pragma push_macro("TplDecl")

#define TplDecl                                                              \
    template <typename IsConst,                                              \
              typename = EnableIf<IsAnyOf<IsConst, value_wrapper::FalseType, \
                                          value_wrapper::TrueType>>>

struct RefOperator {
    template <typename IsConst_,
              typename = EnableIf<IsAnyOf<IsConst_, value_wrapper::FalseType,
                                          value_wrapper::TrueType>>>
    static constexpr bool IsConst(type_wrapper::TypeWrapper<Ref<IsConst_>*>);

    template <typename IsConst_,
              typename = EnableIf<IsAnyOf<IsConst_, value_wrapper::FalseType,
                                          value_wrapper::TrueType>>>
    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<Ref<IsConst_> const*>);

    template <typename IsConst,
              typename = EnableIf<IsAnyOf<IsConst, value_wrapper::FalseType,
                                          value_wrapper::TrueType>>>
    static size_t GetAlign(Ref<IsConst> const* ref);

    static void* Allocate(Ref<value_wrapper::FalseType>* ref, size_t size);

    static void Deallocate(Ref<value_wrapper::FalseType>* ref, void* ptr);
};

template <typename AllocatorOperator, typename A>
auto MakeRef(AllocatorOperator const& alctr_opr, A&& a_);

// -----------------------------------------------------------------------------

template <typename AllocatorOperator, typename Allocator>
void* SafeAllocate(AllocatorOperator const& alctr_opr, Allocator* alctr,
                   size_t align, size_t size);

// -----------------------------------------------------------------------------

extern Ref<value_wrapper::FalseType> weak_lifo_allocator;

}  // namespace zeta::core::allocator
