#pragma once

#include <zeta/core/comparison_ref.hpp>

namespace zeta::core {

template <typename InnerComparator, typename A, typename B>
    requires comparison::CanCompare<InnerComparator, A, B>
constexpr comparison_ref::Comparator::Comparator(InnerComparator const& cmptr,
                                                 meta::TypeWrapper<A>,
                                                 meta::TypeWrapper<B>) {
    this->Set(cmptr, meta::TypeWrapper<A>{}, meta::TypeWrapper<B>{});
}

constexpr void comparison_ref::Comparator::Init(this Comparator& cmptr,
                                                Comparator const& other_cmptr) {
    cmptr = other_cmptr;
}

template <typename InnerComparator, typename A, typename B>
    requires comparison::CanCompare<InnerComparator, A, B>
constexpr void comparison_ref::Comparator::Init(
    this Comparator& cmptr, InnerComparator const& target_cmptr,
    meta::TypeWrapper<A>, meta::TypeWrapper<B>) {
    cmptr.Set(target_cmptr, meta::TypeWrapper<A>{}, meta::TypeWrapper<B>{});
}

constexpr void comparison_ref::Comparator::Set(this Comparator& cmptr,
                                               Comparator const& other_cmptr) {
    cmptr = other_cmptr;
}

template <typename InnerComparator, typename A, typename B>
    requires comparison::CanCompare<InnerComparator, A, B>
constexpr void comparison_ref::Comparator::Set(
    this Comparator& cmptr, InnerComparator const& target_cmptr,
    meta::TypeWrapper<A>, meta::TypeWrapper<B>) {
    cmptr.target_cmptr = &target_cmptr;

    cmptr.compare_func = [](void const* target_cmptr, void const* a,
                            void const* b) {
        return (comparison::Compare)(
            *static_cast<InnerComparator const*>(target_cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Order>{},
            *static_cast<A const*>(a), *static_cast<B const*>(b));
    };
}

template <comparison::IsOpType OpType>
constexpr auto comparison_ref::Comparator::Compare(this Comparator const& cmptr,
                                                   OpType, void const* a,
                                                   void const* b) {
    comparison::Ordering ordering{ cmptr.compare_func(cmptr.target_cmptr, a,
                                                      b) };

    if constexpr (meta::IsSame<OpType, meta::AutoValueWrapper<
                                           comparison::OpEnum::Order>>) {
        return ordering;
    } else {
        return (meta::ToUnderlying(ordering) &
                meta::ToUnderlying(OpType::value)) != 0;
    }
}

}  // namespace zeta::core
