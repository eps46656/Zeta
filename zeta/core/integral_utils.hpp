#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/tuple.ipp>

namespace zeta::core::integral_utils {

template <typename Integral>
constexpr Integral FromString(char const* str);

namespace detail {

auto SelectIntegralCands_(auto... cands) {
    constexpr size_t best_i{ [](auto... cands) {
        constexpr size_t cand_cnt{ sizeof...(cands) };

        struct Score {
            bool is_void;
            unsigned long long width;
        };

        constexpr Score scores[]{ [](auto cand) {
            using Cand = decltype(cand)::Type;

            constexpr bool is_void{ meta::IsSame<Cand, void> };

            if constexpr (is_void) {
                return Score{ is_void, 0ULL };
            } else {
                return Score{ is_void, integral::WidthOf<Cand> };
            }
        }(cands)... };

        size_t best_i{ 0 };
        int best_is_void{ scores[0].is_void };
        unsigned long long best_width{ scores[0].width };

        for (size_t i{ 1 }; i < cand_cnt; ++i) {
            int cur_is_void{ scores[i].is_void };
            unsigned long long cur_width{ scores[i].width };

            if (cur_is_void < best_is_void ||
                (cur_is_void == best_is_void && cur_width < best_width)) {
                best_i = i;
                best_is_void = cur_is_void;
                best_width = cur_width;
            }
        }

        return best_i;
    }(cands...) };

    tuple::Tuple<decltype(cands)...> t{ cands... };

    return t.template Access<best_i>();
}

template <unsigned long long N>
auto UnsignedFastIntegral_() {
    constexpr auto cand0{ []() {
#if defined(__UINT_FAST8_TYPE__)
        if constexpr (N <= 8) {
            return meta::TypeWrapper<__UINT_FAST8_TYPE__>{};
        } else
#endif

#if defined(__UINT_FAST16_TYPE__)
            if constexpr (N <= 16) {
            return meta::TypeWrapper<__UINT_FAST16_TYPE__>{};
        } else
#endif

#if defined(__UINT_FAST32_TYPE__)
            if constexpr (N <= 32) {
            return meta::TypeWrapper<__UINT_FAST32_TYPE__>{};
        } else
#endif

#if defined(__UINT_FAST64_TYPE__)
            if constexpr (N <= 64) {
            return meta::TypeWrapper<__UINT_FAST64_TYPE__>{};
        } else
#endif

#if defined(__UINT_FAST128_TYPE__)
            if constexpr (N <= 128) {
            return meta::TypeWrapper<__UINT_FAST128_TYPE__>{};
        } else
#endif

#if defined(__UINT_FAST256_TYPE__)
            if constexpr (N <= 256) {
            return meta::TypeWrapper<__UINT_FAST256_TYPE__>{};
        } else
#endif

        {
            return meta::TypeWrapper<void>{};
        }
    }() };

    constexpr auto cand1{ []() {
        if constexpr (N <= integral::WidthOf<unsigned char>) {
            return meta::TypeWrapper<unsigned char>{};
        } else if constexpr (N <= integral::WidthOf<unsigned short>) {
            return meta::TypeWrapper<unsigned short>{};
        } else if constexpr (N <= integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (N <= integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (N <= integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<void>{};
        }
    }() };

    constexpr auto cand2{ []() {
        constexpr unsigned long long natural_width{
            integral::WidthOf<unsigned long long>
        };

        constexpr unsigned long long max_k{
            comparison_utils::BasicMin(integral::RangeMaxOf<unsigned long long>,
                                       ZETA_Core_bitint_max_width) /
            natural_width
        };

        constexpr unsigned long long k{ N / natural_width +
                                        (N % natural_width != 0) };

        ZETA_Core_StaticAssert(k <= max_k);

        return meta::TypeWrapper<unsigned _BitInt(k * natural_width)>{};
    }() };

    return (SelectIntegralCands_)(cand0, cand1, cand2);
}

template <unsigned long long N>
auto SignedFastIntegral_() {
    constexpr auto cand0{ []() {
#if defined(__INT_FAST8_TYPE__)
        if constexpr (N <= 8) {
            return meta::TypeWrapper<__INT_FAST8_TYPE__>{};
        } else
#endif

#if defined(__INT_FAST16_TYPE__)
            if constexpr (N <= 16) {
            return meta::TypeWrapper<__INT_FAST16_TYPE__>{};
        } else
#endif

#if defined(__INT_FAST32_TYPE__)
            if constexpr (N <= 32) {
            return meta::TypeWrapper<__INT_FAST32_TYPE__>{};
        } else
#endif

#if defined(__INT_FAST64_TYPE__)
            if constexpr (N <= 64) {
            return meta::TypeWrapper<__INT_FAST64_TYPE__>{};
        } else
#endif

#if defined(__INT_FAST128_TYPE__)
            if constexpr (N <= 128) {
            return meta::TypeWrapper<__INT_FAST128_TYPE__>{};
        } else
#endif

#if defined(__INT_FAST256_TYPE__)
            if constexpr (N <= 256) {
            return meta::TypeWrapper<__INT_FAST256_TYPE__>{};
        } else
#endif

        {
            return meta::TypeWrapper<void>{};
        }
    }() };

    constexpr auto cand1{ []() {
        if constexpr (N <= integral::WidthOf<signed char>) {
            return meta::TypeWrapper<signed char>{};
        } else if constexpr (N <= integral::WidthOf<signed short>) {
            return meta::TypeWrapper<signed short>{};
        } else if constexpr (N <= integral::WidthOf<signed>) {
            return meta::TypeWrapper<signed>{};
        } else if constexpr (N <= integral::WidthOf<signed long>) {
            return meta::TypeWrapper<signed long>{};
        } else if constexpr (N <= integral::WidthOf<signed long long>) {
            return meta::TypeWrapper<signed long long>{};
        } else {
            return meta::TypeWrapper<void>{};
        }
    }() };

    constexpr auto cand2{ []() {
        constexpr unsigned long long natural_width{
            integral::WidthOf<unsigned long long>
        };

        constexpr unsigned long long max_k{
            comparison_utils::BasicMin(integral::RangeMaxOf<unsigned long long>,
                                       ZETA_Core_bitint_max_width) /
            natural_width
        };

        constexpr unsigned long long k{ N / natural_width +
                                        (N % natural_width != 0) };

        ZETA_Core_StaticAssert(k <= max_k);

        return meta::TypeWrapper<signed _BitInt(k * natural_width)>{};
    }() };

    return (SelectIntegralCands_)(cand0, cand1, cand2);
}

}  // namespace detail

template <size_t N>
using UnsignedFastIntegral = decltype(detail::UnsignedFastIntegral_<N>())::Type;

template <size_t N>
using SignedFastIntegral = decltype(detail::SignedFastIntegral_<N>())::Type;

}  // namespace zeta::core::integral_utils
