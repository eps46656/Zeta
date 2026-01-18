#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>

namespace zeta::core::compare {

template <typename A, typename B>
int Compare(A const& a, B const& b) {
    static CompareCore<A, B> const compare_core;
    return compare_core(a, b);
}

template <typename A, typename B>
int TypeErasedCompare(void const* a, void const* b) {
    return Compare<A, B>(*static_cast<A const*>(a), *static_cast<B const*>(b));
}

// -----------------------------------------------------------------------------

template <typename A, typename B>
bool CppStdEqualTo<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) == 0;
}

template <typename A, typename B>
bool CppStdNotEqualTo<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) == 0;
}

template <typename A, typename B>
bool CppStdLess<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) < 0;
}

template <typename A, typename B>
bool CppStdLessEqual<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) <= 0;
}

template <typename A, typename B>
bool CppStdGreater<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) > 0;
}

template <typename A, typename B>
bool CppStdGreaterEqual<A, B>::operator()(A const& a, B const& b) const {
    return Compare(a, b) >= 0;
}

// -----------------------------------------------------------------------------

template <typename A, typename B>
struct CompareCore<A, B,
                   EnableIf<(IsIntegral<A> || IsPointer<A> || IsArray<A>) &&
                                (IsIntegral<B> || IsPointer<B> || IsArray<B>),
                            void>> {
    int operator()(A const& a, B const& b) const { return (b < a) - (a < b); }
};

}  // namespace zeta::core::compare
