#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.ipp>
#include <zeta/core/integral_math.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Num>
constexpr Num integral_math::CeilDiv(Num x, Num y) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < y);

    return x / y + static_cast<Num>(x % y != static_cast<Num>(0) ? 1 : 0);
}

template <typename Num>
constexpr Num integral_math::AlignDown(Num val, Num align) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < align);

    return val - val % align;
}

template <typename Num>
constexpr Num integral_math::AlignUp(Num val, Num align) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < align);

    Num res{ val % align };

    return res == static_cast<Num>(0) ? val : val + (align - res);
}

template <typename Num>
constexpr bool integral_math::IsPowerOf2(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    return static_cast<Num>(0) < num && integral_bit::PopCount(num) == 1;
}

template <typename Base, typename Exp>
constexpr Base integral_math::PowerOf2(Exp exp) {
    ZETA_Core_DebugAssert(static_cast<Exp>(0) <= exp);
    return static_cast<Base>(1) << exp;
}

template <typename Base, typename Exp>
constexpr Base integral_math::Power(Base base, Exp exp) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Exp>);

    constexpr Base base_0{ static_cast<Base>(0) };
    constexpr Base base_1{ static_cast<Base>(1) };

    if (exp == static_cast<Exp>(0)) {
        ZETA_Core_DebugAssert(base != base_0);
        return base_1;
    }

    if constexpr (integral::IsIntegral<Base>) {
        if ((IsPowerOf2)(base)) {
            return (PowerOf2<Base>)(static_cast<Exp>(FloorLog2(base)) * exp);
        }
    }

    ZETA_Core_DebugAssert(static_cast<Exp>(0) <= exp);

    Base ret{ base_1 };

    for (;;) {
        if (exp % static_cast<Exp>(2) != static_cast<Exp>(0)) { ret *= base; }
        exp /= static_cast<Exp>(2);
        if (exp == static_cast<Exp>(0)) { break; }
        base *= base;
    }

    return ret;
}

template <typename Num>
constexpr unsigned long long integral_math::FloorLog2(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < num);

    return integral_bit::CLZ(static_cast<Num>(1)) - 1ULL -
           integral_bit::CLZ(num);
}

template <typename Num>
constexpr unsigned long long integral_math::CeilLog2(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < num);

    return num == static_cast<Num>(1)
               ? 0ULL
               : integral_bit::CLZ(static_cast<Num>(1)) + 1ULL -
                     integral_bit::CLZ(num - static_cast<Num>(1));
}

namespace integral_math::detail {

template <bool IsCeil, typename Num, typename Base>
constexpr unsigned long long Log_(Num num_, Base base_) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);
    ZETA_Core_StaticAssert(integral::IsIntegral<Base>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < num_);
    ZETA_Core_DebugAssert(static_cast<Base>(1) < base_);

    unsigned long long approx_ans{ (FloorLog2)(num_) / (CeilLog2)(base_) };

    if ((IsPowerOf2)(base_)) {
        if constexpr (IsCeil) {
            return approx_ans + (integral_bit::PopCount(num_) != 1);
        } else {
            return approx_ans;
        }
    }

    if (approx_ans <= 4) {
        unsigned long long ret{ 0 };

        if constexpr (IsCeil) {
            for (; static_cast<Num>(1) < num_; num_ /= base_) { ++ret; }
        } else {
            for (; base_ <= num_; num_ /= base_) { ++ret; }
        }

        return ret;
    }

    using Op = meta::Conditional<
        (integral::RangeMaxOf<Num>) < (integral::RangeMaxOf<Base>), Base, Num>;

    Op num{ static_cast<Op>(num_) };
    Op base{ static_cast<Op>(base_) };

    unsigned long long max_k{ (FloorLog2)(approx_ans) };

    /*

    0   1
    1   2
    2   4

    k   base^(2^k)

    max_k   base^(2^max_k) <= x

    2^max_k <= approx_ans

    max_k <= FloorLog2(approx_ans)

    */

    Op bases[ZETA_Core_ullong_width + 1];

    bases[0] = base;

    for (unsigned long long k{ 1 }; k <= max_k; ++k) {
        bases[k] = bases[k - 1] * bases[k - 1];
    }

    unsigned long long ret{ 0 };

    while (bases[max_k] <= num) {
        num /= bases[max_k];
        ++ret;
    }

    Op p{ 1 };

    for (unsigned long long k{ max_k }; 0 < k;) {
        --k;

        ret *= 2;

        Op try_p{ p * bases[k] };

        if (try_p <= num) {
            p = try_p;
            ++ret;
        }
    }

    if constexpr (IsCeil) {
        if (p < num) { ++ret; }
    }

    return ret;
}

}  // namespace integral_math::detail

template <typename Num, typename Base>
constexpr unsigned long long integral_math::FloorLog(Num num, Base base) {
    return detail::Log_<false, Num, Base>(num, base);
}

template <typename Num, typename Base>
constexpr unsigned long long integral_math::CeilLog(Num num, Base base) {
    return detail::Log_<true, Num, Base>(num, base);
}

template <typename Num>
constexpr Num integral_math::FloorSqrt(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) <= num);

    if (num <= static_cast<Num>(3)) {
        return static_cast<Num>(num == static_cast<Num>(0) ? 0 : 1);
    }

    Num x{ static_cast<Num>(1) << (((CeilLog2)(num) + 1) / 2) };

    for (;;) {
        Num y{ (x + num / x) / static_cast<Num>(2) };
        if (x <= y) { return x; }
        x = y;
    }

    __buildin_unreachable();
}

template <typename Num>
constexpr Num integral_math::CeilSqrt(Num num) {
    Num k{ (FloorSqrt)(num) };
    return k * k == num ? k : k + static_cast<Num>(1);
}

template <typename Num>
constexpr auto integral_math::GCD(Num x, Num y) {
    if (x == static_cast<Num>(0)) {
        return y == static_cast<Num>(0) ? static_cast<Num>(1) : y;
    }

    for (;;) {
        if (y == static_cast<Num>(0)) {
            return x < static_cast<Num>(0) ? -x : x;
        }

        x %= y;

        if (x == static_cast<Num>(0)) {
            return y < static_cast<Num>(0) ? -y : y;
        }

        y %= x;
    }

    __builtin_unreachable();
}

template <typename Num>
constexpr auto integral_math::LCM(Num x, Num y) {
    Num gcd{ GCD(x, y) };
    return x / gcd * y;
}

}  // namespace zeta::core
