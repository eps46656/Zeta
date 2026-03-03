#include <vector>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core/value_wrapper.ipp>
#include <zeta/core_test/random.hpp>

using FP = zeta::core::fixed_point::FixedPoint<
    zeta::core::value_wrapper::TrueType,
    zeta::core::value_wrapper::StaticValueWrapper<size_t, 24>,
    zeta::core::value_wrapper::StaticValueWrapper<size_t, 32>>;

template <typename SignedTag, typename IntegralWidth, typename FractionWidth>
double ToDouble(zeta::core::fixed_point::FixedPoint<SignedTag, IntegralWidth,
                                                    FractionWidth> const& x) {
    return static_cast<double>(x.value) /
           std::pow(2.0, static_cast<unsigned long long>(FractionWidth::value));
}

template <typename Num, typename Denom>
zeta::core::utils::Pair<FP, double> Gen(Num num, Denom denom) {
    return { FromFraction<FP::FractionWidth>(num, denom),
             static_cast<double>(num) / static_cast<double>(denom) };
}

inline zeta::core::utils::Pair<FP, double> Gen() {
    using Value = FP::Value;

    Value value{ zeta::core_test::GetRandomInt<Value>(
        zeta::core::integral::RangeMinOf<Value>,
        zeta::core::integral::RangeMaxOf<Value>) };

    FP fp{ FP::FromValue(value) };

    double d{ static_cast<double>(value) /
              std::pow(2.0, static_cast<unsigned long long>(
                                FP::FractionWidth::value)) };

    return { fp, d };
}

inline void main1() {
    unsigned seed{ static_cast<unsigned>(time(nullptr)) };

    ZETA_Core_PrintVar(seed);

    for (size_t i{ 0 }; i < 1024; ++i) {
        size_t num{ zeta::core_test::GetRandomInt<size_t>(0,
                                                          (1ULL << 24) - 1) };
        size_t denom{ zeta::core_test::GetRandomInt<size_t>(1, 1024) };

        /*
        unsigned long long num{ ZETA_Core_ullong_max };
        unsigned long long denom{ 1 };
        ok
        */

        /*
        unsigned long long num{ ZETA_Core_ullong_max };
        unsigned long long denom{ ZETA_Core_ullong_max };
        ok
        */

        /*
        unsigned long long num{ 0 };
        unsigned long long denom{ ZETA_Core_ullong_max };
        ok
        */

        /*
        long long num{ ZETA_Core_sllong_min };
        unsigned long long denom{ 1 };
        ok
        */

        /*
        long long num{ ZETA_Core_sllong_min };
        unsigned long long denom{ ZETA_Core_ullong_max };
        ok
        */

        /*
        long long num{ ZETA_Core_sllong_max };
        unsigned long long denom{ ZETA_Core_ullong_max };
        ok
        */

        /*
        long long num{ ZETA_Core_sllong_max };
        unsigned long long denom{ 1 };
        ok
        */

        auto x{ zeta::core::fixed_point::ops::FromFraction<
            zeta::core::value_wrapper::StaticValueWrapper<size_t, 32>>(num,
                                                                       denom) };

        double ans{ ToDouble(x) };

        double expected{ static_cast<double>(num) /
                         static_cast<double>(denom) };

        double err{ ans - expected };

        double rel_err{ std::abs(err / expected) };

        ZETA_Core_PrintVar(ans);
        ZETA_Core_PrintVar(expected);
        ZETA_Core_PrintVar(rel_err);

        ZETA_Core_DebugAssert(expected < 1e-6 || rel_err < 1e-6);
    }
}

inline void main2() {
    unsigned seed{ static_cast<unsigned>(time(nullptr)) };

    ZETA_Core_PrintVar(seed);

    for (size_t i{ 0 }; i < 1024; ++i) {
        auto [x_fp, x_d]{ Gen() };
        auto [y_fp, y_d]{ Gen() };

        auto z_fp{ x_fp * y_fp };
        auto z_d{ x_d * y_d };

        ZETA_Core_PrintVar(decltype(z_fp)::IntegralWidth::value);
        ZETA_Core_PrintVar(decltype(z_fp)::FractionWidth::value);

        auto floor_x_fp{ zeta::core::fixed_point::ops::Floor(x_fp) };

        ZETA_Core_PrintVar(floor_x_fp);

        double err{ z_d - ToDouble(z_fp) };

        double rel_err{ std::abs(err / x_d) };

        ZETA_Core_PrintVar(rel_err);

        ZETA_Core_DebugAssert(err < 1e-6 || rel_err < 1e-6);
    }
}

int main() {
    main2();
    std::cout << "ok\n";
    return 0;
}
