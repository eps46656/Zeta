

#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::poly_comparison {

struct Comparator {
    void const* target_cmptr;

    comparison::Ordering (*compare_func)(void const* innser_cmptr,
                                         void const* a, void const* b);

    constexpr Comparator() = default;

    constexpr Comparator(Comparator const&) = default;

    template <typename InnerComparator, typename A, typename B>
        requires comparison::CanCompare<InnerComparator, A, B>
    constexpr Comparator(InnerComparator const& target_cmptr,
                         meta::TypeWrapper<A>, meta::TypeWrapper<B>);

    constexpr Comparator& operator=(Comparator const&) = default;

    constexpr void Set(this Comparator& self, Comparator const& other);

    template <typename InnerComparator, typename A, typename B>
        requires comparison::CanCompare<InnerComparator, A, B>
    constexpr void Set(this Comparator& self,
                       InnerComparator const& target_cmptr,
                       meta::TypeWrapper<A>, meta::TypeWrapper<B>);

    template <comparison::IsOpTag OpTag>
    constexpr auto Compare(this Comparator const& self, comparison::Tag, OpTag,
                           void const* a, void const* b);
};

}  // namespace zeta::core::poly_comparison
