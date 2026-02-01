#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename Allocator>
void CheckContract(Allocator* alctr);

struct VTable {
    template <typename Allocator>
    static constexpr VTable const& Build();

    void* (*Allocate)(void* alctr, size_t size);

    void (*Deallocate)(void* alctr, void* ptr);
};

template <typename Allocator>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static void* Allocate(void* alctr, size_t size);

    static void Deallocate(void* alctr, void* ptr);
};

template <typename Allocator>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename Allocator>
constexpr VTable const& GetVTable();

template <typename ConstTag>
struct Ref {
    ZETA_Core_StaticAssert(
        IsAnyOf<ConstTag, value_wrapper::FalseType, value_wrapper::TrueType>);

    unsigned short align;

    VTable const* vtable;

    void* alctr;
};

#pragma push_macro("TplDecl")

#define TplDecl                                                               \
    template <typename ConstTag,                                              \
              typename = EnableIf<IsAnyOf<ConstTag, value_wrapper::FalseType, \
                                          value_wrapper::TrueType>>>

template <typename ConstTag>
struct RefView {
    ZETA_Core_StaticAssert(
        IsAnyOf<ConstTag, value_wrapper::FalseType, value_wrapper::TrueType>);

    static constexpr bool IsConst(RefView*);

    static constexpr bool IsConst(RefView const*);

    static size_t GetAlign(RefView const* ref_view);

    static void* Allocate(RefView* ref_view, size_t size);

    static void Deallocate(RefView* ref_view, void* ptr);
};

template <typename Allocator>
auto MakeRef(Allocator* alctr);

// -----------------------------------------------------------------------------

template <typename Allocator>
void* SafeAllocate(Allocator* alctr, size_t align, size_t size);

// -----------------------------------------------------------------------------

extern Ref<value_wrapper::FalseType> weak_lifo_allocator;

}  // namespace zeta::core::allocator
