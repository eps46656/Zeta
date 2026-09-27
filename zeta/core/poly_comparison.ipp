#pragma once

#include <zeta/core/poly_comparison.hpp>

namespace zeta::core {

template <typename InnerComparator, typename A, typename B>
    requires comparison::CanCompare<InnerComparator, A, B>
constexpr poly_comparison::Comparator::Comparator(InnerComparator const& cmptr,
                                                  meta::TypeWrapper<A>,
                                                  meta::TypeWrapper<B>) {
    this->Set(cmptr, meta::TypeWrapper<A>{}, meta::TypeWrapper<B>{});
}

constexpr void poly_comparison::Comparator::Set(this Comparator& self,
                                                Comparator const& other) {
    self = other;
}

template <typename InnerComparator, typename A, typename B>
    requires comparison::CanCompare<InnerComparator, A, B>
constexpr void poly_comparison::Comparator::Set(
    this Comparator& self, InnerComparator const& target_cmptr,
    meta::TypeWrapper<A>, meta::TypeWrapper<B>) {
    self.target_cmptr = &target_cmptr;

    self.compare_func = [](void const* target_cmptr, void const* a,
                           void const* b) {
        return (comparison::Compare)(*static_cast<InnerComparator const*>(
                                         target_cmptr),
                                     comparison::OpTags::Order{},
                                     *static_cast<A const*>(a),
                                     *static_cast<B const*>(b));
    };
}

template <comparison::IsOpTag OpTag>
constexpr auto poly_comparison::Comparator::Compare(this Comparator const& self,
                                                    comparison::Tag, OpTag,
                                                    void const* a,
                                                    void const* b) {
    comparison::Ordering ordering{ self.compare_func(self.target_cmptr, a, b) };

    if constexpr (meta::IsSame<OpTag, comparison::OpTags::Order>) {
        return ordering;
    } else {
        return (meta::ToUnderlying(ordering) &
                meta::ToUnderlying(OpTag::value)) != 0;
    }
}

}  // namespace zeta::core
