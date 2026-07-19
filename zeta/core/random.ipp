#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/random.hpp>

namespace zeta ::core {

template <typename Engine>
constexpr bool random::IsConst() {
    return EngineTraits<Engine>::IsConst();
}

template <typename Engine>
constexpr size_t random::GetBitCnt() {
    return EngineTraits<Engine>::GetBitCnt();
}

template <typename Engine>
auto random::Fetch(Engine& engine) {
    constexpr size_t bit_cnt{ (GetBitCnt<Engine>)() };

    auto ret{ EngineTraits<Engine>::Fetch(engine) };

    using Integral = decltype(ret);

    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    ZETA_Core_StaticAssert(bit_cnt <= integral::WidthOf<Integral>);

    ZETA_Core_DebugAssert(static_cast<Integral>(0) <= ret);

    if constexpr (bit_cnt < integral::WidthOf<Integral>) {
        ZETA_Core_DebugAssert(ret < (static_cast<Integral>(1) << bit_cnt));
    }

    return ret;
}

template <typename Engine>
void random::CheckContract(Engine& engine) {
    constexpr bool is_const{ (IsConst<Engine>)() };

    ZETA_Core_Unused([&] { (GetBitCnt<Engine>)(); });

    if constexpr (!is_const) {
        ZETA_Core_Unused([&] { (Fetch)(engine); });
    }
}

template <typename UnsignedIntegral, typename Engine>
UnsignedIntegral random::GetRandomInt(Engine& engine) {
    (CheckContract<Engine>)(engine);

    ZETA_Core_StaticAssert(!(IsConst<Engine>)());

    ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<UnsignedIntegral>);

    UnsignedIntegral ret{ static_cast<UnsignedIntegral>((Fetch)(engine)) };

    constexpr size_t bit_cnt{ (GetBitCnt<Engine>)() };

    if constexpr (bit_cnt < integral::WidthOf<UnsignedIntegral>) {
        for (size_t k{ integral_math::CeilDiv(
                 integral::WidthOf<UnsignedIntegral>, bit_cnt) };
             1 < k; --k) {
            ret = (ret << bit_cnt) +
                  static_cast<UnsignedIntegral>((Fetch)(engine));
        }
    }

    return ret;
}

template <typename Engine, typename Integral>
Integral random::GetRandomInt(Engine& engine, Integral min, Integral max) {
    (CheckContract<Engine>)(engine);

    ZETA_Core_StaticAssert(!(IsConst<Engine>)());

    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    ZETA_Core_DebugAssert(min <= max);

    constexpr size_t bit_cnt{ (GetBitCnt<Engine>)() };

    constexpr size_t aug_bit_cnt{ 16 };

    using OpIntegral =
        unsigned _BitInt(integral::WidthOf<Integral> + aug_bit_cnt);

    using TmpIntegral =
        meta::Conditional<integral::IsSignedIntegral<Integral>,
                          signed _BitInt(integral::WidthOf<Integral> + 1),
                          Integral>;

    OpIntegral range{ static_cast<OpIntegral>(
        static_cast<TmpIntegral>(max) - static_cast<TmpIntegral>(min) + 1) };

    OpIntegral m{ static_cast<OpIntegral>((Fetch)(engine)) };

    if (bit_cnt < integral::WidthOf<OpIntegral>) {
        for (OpIntegral k{ (range << aug_bit_cnt) >> bit_cnt }; 0 < k;
             k >>= bit_cnt) {
            m = (m << bit_cnt) + static_cast<OpIntegral>((Fetch)(engine));
        }
    }

    return static_cast<Integral>(min + static_cast<Integral>(m % range));
}

}  // namespace zeta::core
