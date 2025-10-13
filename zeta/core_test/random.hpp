#pragma once

#include <random>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

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
    ZETA_Core_StaticAssert(core::IsIntegral<RetInt>);
    ZETA_Core_StaticAssert(core::IsIntegral<LBInt>);
    ZETA_Core_StaticAssert(core::IsIntegral<RBInt>);

    ZETA_Core_DebugAssert(core::MathCompare(core::RangeMinOf<RetInt>, lb) <= 0);
    ZETA_Core_DebugAssert(core::MathCompare(rb, core::RangeMaxOf<RetInt>) <= 0);

    std::uniform_int_distribution<core::Conditional<
        core::IsSignedIntegral<RetInt>, signed long long, unsigned long long>>
        generator{ static_cast<RetInt>(lb), static_cast<RetInt>(rb) };

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

// -----------------------------------------------------------------------------

template <typename T>
struct RandomCore<
    T, core::EnableIf<(core::IsIntegral<T> || core::IsPointer<T>), void>> {
    T operator()() const {
        if constexpr (core::IsIntegral<T>) {
            return GetRandomInt<T>(core::RangeMinOf<T>, core::RangeMaxOf<T>);
        }

        if constexpr (core::IsPointer<T>) {
            return reinterpret_cast<T>(
                GetRandomInt<uintptr_t>(0x1'0000'0000, 0x1'ffff'ffff) *
                alignof(T));
        }
    }
};

template <typename First, typename Second>
struct RandomCore<core::Pair<First, Second>> {
    core::Pair<First, Second> operator()() const {
        return { GetRandom<First>(), GetRandom<Second>() };
    }
};

}  // namespace zeta::core_test
