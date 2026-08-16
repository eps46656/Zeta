

#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core::comparison_ref {

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

    constexpr void Init(this Comparator& cmptr, Comparator const& other_cmptr);

    template <typename InnerComparator, typename A, typename B>
        requires comparison::CanCompare<InnerComparator, A, B>
    constexpr void Init(this Comparator& cmptr,
                        InnerComparator const& target_cmptr,
                        meta::TypeWrapper<A>, meta::TypeWrapper<B>);

    constexpr Comparator& operator=(Comparator const&) = default;

    constexpr void Set(this Comparator& cmptr, Comparator const& other_cmptr);

    template <typename InnerComparator, typename A, typename B>
        requires comparison::CanCompare<InnerComparator, A, B>
    constexpr void Set(this Comparator& cmptr,
                       InnerComparator const& target_cmptr,
                       meta::TypeWrapper<A>, meta::TypeWrapper<B>);

    template <comparison::IsOpType OpType>
    constexpr auto Compare(this Comparator const& cmptr, OpType, void const* a,
                           void const* b);
};

}  // namespace zeta::core::comparison_ref

namespace zeta::core {

template <>
struct lifecycle::Traits<comparison_ref::Comparator>
    : public lifecycle::MemberFuncTraitsAdapter<comparison_ref::Comparator> {};

template <>
struct comparison::ComparatorTraits<comparison_ref::Comparator>
    : public comparison::MemberFuncComparatorTraitsAdapter<
          comparison_ref::Comparator> {};

}  // namespace zeta::core
