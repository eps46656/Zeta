#pragma once

#include <zeta/core/comparison_deref.hpp>

namespace zeta::core {

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
template <typename... TargetComparatorInitArgs>
constexpr comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>::
    Comparator(TargetComparatorInitArgs&&... target_cmptr_init_args) {
    this->Init(
        meta::Forward<TargetComparatorInitArgs>(target_cmptr_init_args)...);
}

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
constexpr void
comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>::Init(
    this Comparator& cmptr, Comparator const& other_cmptr) {
    cmptr = other_cmptr;
}

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
template <typename... TargetComparatorInitArgs>
constexpr void
comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>::Init(
    this Comparator& cmptr,
    TargetComparatorInitArgs&&... target_cmptr_init_args) {
    lifecycle::Init(cmptr.target_cmptr, meta::Forward<TargetComparatorInitArgs>(
                                            target_cmptr_init_args)...);
}

template <typename TargetComparatorLike, meta::IsValueWrapperT<bool> DerefA,
          meta::IsValueWrapperT<bool> DerefB>
template <comparison::IsOpType OpType, typename ValueA, typename ValueB>
constexpr auto
comparison_deref::Comparator<TargetComparatorLike, DerefA, DerefB>::Compare(
    this Comparator const& cmptr, OpType, ValueA&& a, ValueB&& b) {
    if constexpr (DerefA::value && DerefB::value) {
        return comparison::Compare(cmptr.target_cmptr, OpType{}, *a, *b);
    } else if constexpr (DerefA::value && !DerefB::value) {
        return comparison::Compare(cmptr.target_cmptr, OpType{}, *a,
                                   meta::Forward<ValueB>(b));
    } else if constexpr (!DerefA::value && DerefB::value) {
        return comparison::Compare(cmptr.target_cmptr, OpType{},
                                   meta::Forward<ValueA>(a), *b);
    } else {
        return comparison::Compare(cmptr.target_cmptr, OpType{},
                                   meta::Forward<ValueA>(a),
                                   meta::Forward<ValueB>(b));
    }
}

}  // namespace zeta::core
