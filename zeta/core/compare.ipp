#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>

namespace zeta::core::compare {

template <typename X, typename Y>
int Compare(X const& x, Y const& y) {
    static CompareCore<X, Y> const compare_core;
    return compare_core(x, y);
}

template <typename X, typename Y>
int TypeErasedCompare(void const* x, void const* y) {
    return Compare<X, Y>(*static_cast<X const*>(x), *static_cast<Y const*>(y));
}

// -----------------------------------------------------------------------------

template <typename X, typename Y>
bool CppStdEqualTo<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) == 0;
}

template <typename X, typename Y>
bool CppStdNotEqualTo<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) == 0;
}

template <typename X, typename Y>
bool CppStdLess<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) < 0;
}

template <typename X, typename Y>
bool CppStdLessEqual<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) <= 0;
}

template <typename X, typename Y>
bool CppStdGreater<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) > 0;
}

template <typename X, typename Y>
bool CppStdGreaterEqual<X, Y>::operator()(X const& x, Y const& y) const {
    return Compare(x, y) >= 0;
}

// -----------------------------------------------------------------------------

template <typename X, typename Y>
struct CompareCore<X, Y,
                   EnableIf<(IsIntegral<X> || IsPointer<X> || IsArray<X>) &&
                                (IsIntegral<Y> || IsPointer<Y> || IsArray<Y>),
                            void>> {
    int operator()(X const& x, Y const& y) const { return (y < x) - (x < y); }
};

}  // namespace zeta::core::compare
