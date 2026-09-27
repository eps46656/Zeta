#pragma once

#include <zeta/core/deref_comparison.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core {

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
template <typename... TargetComparatorInitArgs>
constexpr deref_comparison::Comparator<TargetComparatorLike, DerefA, DerefB>::
    Comparator(TargetComparatorInitArgs&&... target_cmptr_init_args)
    : target_cmptr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          TargetComparatorLike, TargetComparatorInitArgs,
          target_cmptr_init_args) } {}

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
constexpr void
deref_comparison::Comparator<TargetComparatorLike, DerefA, DerefB>::Set(
    this Comparator& self, Comparator const& other) {
    self = other;
}

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
template <comparison::IsOpTag OpTag, typename ValueA, typename ValueB>
constexpr auto
deref_comparison::Comparator<TargetComparatorLike, DerefA, DerefB>::Compare(
    this Comparator const& self, OpTag, ValueA&& a, ValueB&& b) {
    if constexpr (DerefA::value && DerefB::value) {
        return comparison::Compare(self.target_cmptr_like, OpTag{}, *a, *b);
    } else if constexpr (DerefA::value && !DerefB::value) {
        return comparison::Compare(self.target_cmptr_like, OpTag{}, *a,
                                   meta::Forward<ValueB>(b));
    } else if constexpr (!DerefA::value && DerefB::value) {
        return comparison::Compare(self.target_cmptr_like, OpTag{},
                                   meta::Forward<ValueA>(a), *b);
    } else {
        return comparison::Compare(self.target_cmptr_like, OpTag{},
                                   meta::Forward<ValueA>(a),
                                   meta::Forward<ValueB>(b));
    }
}

}  // namespace zeta::core
