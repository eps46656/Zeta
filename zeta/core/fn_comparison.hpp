

#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::fn_comparison {

struct FnComparator {
    comparison::Ordering (*compare)(void const* a, void const* b);

    constexpr FnComparator() = default;

    constexpr FnComparator(FnComparator const&) = default;

    constexpr FnComparator(FnComparator&&) = default;

    constexpr comparison::Ordering Compare(
        this FnComparator const& fn_comparator, void const* a, void const* b);
};

}  // namespace zeta::core::fn_comparison
