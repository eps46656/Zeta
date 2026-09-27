

#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::deref_comparison {

template <typename TargetComparatorLike_, meta::IsValueWrapperT<bool> DerefA_,
          meta::IsValueWrapperT<bool> DerefB_>
struct Comparator {
    using TargetComparatorLike = TargetComparatorLike_;
    using DerefA = DerefA_;
    using DerefB = DerefB_;

    TargetComparatorLike target_cmptr_like;

    constexpr Comparator() = default;

    constexpr Comparator(Comparator const&) = default;

    template <typename... TargetComparatorLikeInitArgs>
    constexpr Comparator(
        TargetComparatorLikeInitArgs&&... target_cmptr_like_init_args);

    constexpr Comparator& operator=(Comparator const&) = default;

    constexpr void Set(this Comparator& self, Comparator const& other);

    template <comparison::IsOpTag OpTag, typename ValueA, typename ValueB>
    constexpr auto Compare(this Comparator const& self, OpTag, ValueA&& a,
                           ValueB&& b);
};

}  // namespace zeta::core::deref_comparison
