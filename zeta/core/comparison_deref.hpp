

#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core::comparison_deref {

template <typename TargetComparatorLike_, meta::IsValueWrapperT<bool> DerefA_,
          meta::IsValueWrapperT<bool> DerefB_>
struct Comparator {
    using TargetComparatorLike = TargetComparatorLike_;
    using DerefA = DerefA_;
    using DerefB = DerefB_;

    TargetComparatorLike target_cmptr;

    constexpr Comparator() = default;

    constexpr Comparator(Comparator const&) = default;

    template <typename... TargetComparatorInitArgs>
    constexpr Comparator(TargetComparatorInitArgs&&... target_cmptr_init_args);

    constexpr void Init(this Comparator& cmptr, Comparator const& other_cmptr);

    template <typename... TargetComparatorInitArgs>
    constexpr void Init(this Comparator& cmptr,
                        TargetComparatorInitArgs&&... target_cmptr_init_args);

    constexpr Comparator& operator=(Comparator const&) = default;

    template <comparison::IsOpType OpType, typename ValueA, typename ValueB>
    constexpr auto Compare(this Comparator const& cmptr, OpType, ValueA&& a,
                           ValueB&& b);
};

}  // namespace zeta::core::comparison_deref

namespace zeta::core {

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
struct lifecycle::Traits<
    comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>>
    : public lifecycle::MemberFuncTraitsAdapter<
          comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>> {
};

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
struct comparison::ComparatorTraits<
    comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>>
    : public comparison::MemberFuncComparatorTraitsAdapter<
          comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>> {
};

}  // namespace zeta::core
