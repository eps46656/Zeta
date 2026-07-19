#pragma once

namespace zeta::core::integral_math {

template <typename Num>
constexpr Num CeilDiv(Num x, Num y);

template <typename Num>
constexpr Num AlignDown(Num val, Num align);

template <typename Num>
constexpr Num AlignUp(Num val, Num align);

template <typename Num>
constexpr bool IsPowerOf2(Num num);

template <typename Base, typename Exp>
constexpr Base PowerOf2Minus1(Exp exp);

template <typename Base, typename Exp>
constexpr Base PowerOf2(Exp exp);

template <typename Base, typename Exp>
constexpr Base Power(Base base, Exp exp);

template <typename Num>
constexpr unsigned long long FloorLog2(Num num);

template <typename Num>
constexpr unsigned long long CeilLog2(Num num);

template <typename Num, typename Base>
constexpr unsigned long long FloorLog(Num num, Base base);

template <typename Num, typename Base>
constexpr unsigned long long CeilLog(Num num, Base base);

template <typename Num>
constexpr Num FloorSqrt(Num num);

template <typename Num>
constexpr Num CeilSqrt(Num num);

template <typename Num>
constexpr auto GCD(Num x, Num y);

template <typename Num>
constexpr auto LCM(Num x, Num y);

}  // namespace zeta::core::integral_math
