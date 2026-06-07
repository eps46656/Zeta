#pragma once

namespace zeta::core::integral_bit {

template <typename Integral, typename = void>
struct IntegralTraits;  // IWYU pragma: export

template <typename Integral>
constexpr unsigned long long BasicPopCount(Integral num);

template <typename Integral>
constexpr unsigned long long PopCount(Integral num);

template <typename Integral>
constexpr unsigned long long BasicCLZ(Integral num);

template <typename Integral>
constexpr unsigned long long CLZ(Integral num);

template <typename Integral>
constexpr unsigned long long BasicCTZ(Integral num);

template <typename Integral>
constexpr unsigned long long CTZ(Integral num);

template <typename Integral>
constexpr unsigned long long FindPrevBit(Integral num, unsigned long long pos);

template <typename Integral>
constexpr unsigned long long FindNextBit(Integral num, unsigned long long pos);

}  // namespace zeta::core::integral_bit
