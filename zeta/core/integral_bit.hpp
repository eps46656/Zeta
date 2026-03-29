#pragma once

namespace zeta::core::integral_bit {

template <typename Integral, typename = void>
struct Traits;

template <typename Num>
constexpr long long BasicPopCount(Num num);

template <typename Num>
constexpr long long PopCount(Num num);

template <typename Num>
constexpr long long BasicCLZ(Num num);

template <typename Num>
constexpr long long CLZ(Num num);

template <typename Num>
constexpr long long BasicCTZ(Num num);

template <typename Num>
constexpr long long CTZ(Num num);

template <typename Num>
constexpr long long FindPrevBit(Num num, long long pos);

template <typename Num>
constexpr long long FindNextBit(Num num, long long pos);

}  // namespace zeta::core::integral_bit
