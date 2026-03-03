#pragma once

#include <random>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core_test {

inline std::mt19937_64& GetRandomEngine() {
    static std::mt19937_64 en;
    return en;
}

inline void SetRandomSeed(unsigned random_seed) {
    GetRandomEngine().seed(random_seed);
}

template <typename RetInt, typename LBInt, typename RBInt>
RetInt GetRandomInt(LBInt lb, RBInt rb) {
    ZETA_Core_StaticAssert(core::integral::IsIntegral<RetInt>);
    ZETA_Core_StaticAssert(core::integral::IsIntegral<LBInt>);
    ZETA_Core_StaticAssert(core::integral::IsIntegral<RBInt>);

    ZETA_Core_DebugAssert(core::integral::MathCompare(
                              core::integral::RangeMinOf<RetInt>, lb) <= 0);
    ZETA_Core_DebugAssert(core::integral::MathCompare(
                              rb, core::integral::RangeMaxOf<RetInt>) <= 0);

    RetInt ret_lb{ static_cast<RetInt>(lb) };
    RetInt ret_rb{ static_cast<RetInt>(rb) };

    ZETA_Core_DebugAssert(ret_lb <= ret_rb);

    using GenInt =
        core::meta::Conditional<core::integral::IsSigned<RetInt>,
                                signed long long, unsigned long long>;

    std::uniform_int_distribution<GenInt> generator{
        static_cast<GenInt>(ret_lb), static_cast<GenInt>(ret_rb)
    };

    return static_cast<RetInt>(generator(GetRandomEngine()));
}

template <typename Value, typename = void>
struct RandomCore;

template <typename Value>
Value GetRandom() {
    static RandomCore<Value> random_core;
    return random_core();
}

template <typename Value, typename Iterator>
void GetRandoms(Iterator beg, Iterator end) {
    for (; beg != end; ++beg) { *beg = GetRandom<Value>(); }
}

template <typename T>
struct RandomCore<
    T, core::meta::EnableIf<
           (core::integral::IsIntegral<T> || core::meta::IsPointer<T>), void>> {
    T operator()() const {
        if constexpr (core::integral::IsIntegral<T>) {
            return GetRandomInt<T>(core::integral::RangeMinOf<T>,
                                   core::integral::RangeMaxOf<T>);
        }

        if constexpr (core::meta::IsPointer<T>) {
            return reinterpret_cast<T>(
                GetRandomInt<uintptr_t>(0x1'0000'0000, 0x1'ffff'ffff) *
                alignof(T));
        }
    }
};

template <typename First, typename Second>
struct RandomCore<core::utils::Pair<First, Second>> {
    core::utils::Pair<First, Second> operator()() const {
        return { GetRandom<First>(), GetRandom<Second>() };
    }
};

}  // namespace zeta::core_test
