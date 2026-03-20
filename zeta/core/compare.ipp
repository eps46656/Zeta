#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

inline int compare::LexMemCompare(void const* a, void const* b, size_t a_size,
                                  size_t b_size) {
    if (a == b) { return 0; }

    if (0 < a_size) { ZETA_Core_DebugAssert(a != nullptr); }
    if (0 < b_size) { ZETA_Core_DebugAssert(b != nullptr); }

    if (a_size < b_size) {
        int cmp{ __builtin_memcmp(a, b, a_size) };
        return cmp == 0 ? -1 : cmp;
    }

    if (b_size < a_size) {
        int cmp{ __builtin_memcmp(a, b, b_size) };
        return cmp == 0 ? 1 : cmp;
    }

    return __builtin_memcmp(a, b, a_size);
}

inline int compare::LexElemCompare(void const* a_, void const* b_,
                                   size_t a_width, size_t b_width,
                                   size_t a_stride, size_t b_stride,
                                   size_t a_cnt, size_t b_cnt) {
    unsigned char const* a{ static_cast<unsigned char const*>(a_) };
    unsigned char const* b{ static_cast<unsigned char const*>(b_) };

    if (0 < a_cnt) {
        ZETA_Core_DebugAssert(a != nullptr);
        ZETA_Core_DebugAssert(a_stride == 0 || a_width <= a_stride);
    }

    if (0 < b_cnt) {
        ZETA_Core_DebugAssert(b != nullptr);
        ZETA_Core_DebugAssert(b_stride == 0 || b_width <= b_stride);
    }

    ZETA_Core_DebugAssert(a != nullptr);
    ZETA_Core_DebugAssert(b != nullptr);

    if (a_cnt == 0 && b_cnt == 0) { return 0; }
    if (a_cnt == 0) { return -1; }
    if (b_cnt == 0) { return 1; }

    if (a_cnt == 1) { a_stride = 0; }
    if (b_cnt == 1) { b_stride = 0; }

    size_t cnt{ a_stride == 0 && b_stride == 0 ? 1 : utils::Min(a_cnt, b_cnt) };

    int cmp{ 0 };

    for (size_t i{ 0 }; i < cnt; ++i, a += a_stride, b += b_stride) {
        int cur_cmp{ LexMemCompare(a, b, a_width, b_width) };

        if (cur_cmp != 0) {
            cmp = cur_cmp;
            break;
        }
    }

    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    return cmp == 0 ? (b_cnt < a_cnt) - (a_cnt < b_cnt) : cmp;
}

namespace compare::detail {

template <typename A, typename B>
struct BasicCompareImplHolder_ {
    static constexpr BasicCompareImpl<A, B> impl;
};

}  // namespace compare::detail

template <typename A, typename B>
int compare::BasicCompare(A const& a, B const& b) {
    return detail::BasicCompareImplHolder_<A, B>::impl(a, b);
}

template <typename A, typename B>
int compare::TypeErasedBasicCompare(void const* a, void const* b) {
    return BasicCompare(*static_cast<A const*>(a), *static_cast<B const*>(b));
}

template <typename A, typename B>
bool compare::CppStdBasicEqualTo<A, B>::operator()(A const& a,
                                                   B const& b) const {
    return BasicCompare(a, b) == 0;
}

template <typename A, typename B>
bool compare::CppStdBasicNotEqualTo<A, B>::operator()(A const& a,
                                                      B const& b) const {
    return BasicCompare(a, b) == 0;
}

template <typename A, typename B>
bool compare::CppStdBasicLess<A, B>::operator()(A const& a, B const& b) const {
    return BasicCompare(a, b) < 0;
}

template <typename A, typename B>
bool compare::CppStdBasicLessEqual<A, B>::operator()(A const& a,
                                                     B const& b) const {
    return BasicCompare(a, b) <= 0;
}

template <typename A, typename B>
bool compare::CppStdBasicGreater<A, B>::operator()(A const& a,
                                                   B const& b) const {
    return BasicCompare(a, b) > 0;
}

template <typename A, typename B>
bool compare::CppStdBasicGreaterEqual<A, B>::operator()(A const& a,
                                                        B const& b) const {
    return BasicCompare(a, b) >= 0;
}

template <typename A, typename B>
struct compare::BasicCompareImpl<
    A, B,
    meta::EnableIf<
        (integral::IsIntegral<A> || meta::IsPointer<A> || meta::IsArray<A>) &&
            (integral::IsIntegral<B> || meta::IsPointer<B> || meta::IsArray<B>),
        void>> {
    int operator()(A const& a, B const& b) const { return (b < a) - (a < b); }
};

}  // namespace zeta::core
