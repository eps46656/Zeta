#pragma once

#include <stdint.h>

#include <random>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>

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

    ZETA_Core_DebugAssert(
        core::integral_math::Compare(core::comparison::OpTag::LessEqual{},
                                     core::integral::RangeMinOf<RetInt>, lb));
    ZETA_Core_DebugAssert(
        core::integral_math::Compare(core::comparison::OpTag::LessEqual{}, rb,
                                     core::integral::RangeMaxOf<RetInt>));

    RetInt ret_lb{ static_cast<RetInt>(lb) };
    RetInt ret_rb{ static_cast<RetInt>(rb) };

    ZETA_Core_DebugAssert(ret_lb <= ret_rb);

    using GenInt =
        core::meta::Conditional<core::integral::IsSignedIntegral<RetInt>,
                                signed long long, unsigned long long>;

    std::uniform_int_distribution<GenInt> generator{
        static_cast<GenInt>(ret_lb), static_cast<GenInt>(ret_rb)
    };

    return static_cast<RetInt>(generator(GetRandomEngine()));
}

inline void GetRandomMem(void* data_, size_t size) {
    unsigned char* data{ static_cast<unsigned char*>(data_) };

    for (size_t i{ 0 }; i < size; ++i) {
        data[i] = GetRandomInt<unsigned char, unsigned char>(0, 255);
    }
}

inline void GetRandomMemSeq(void* data_, size_t elem_size, size_t elem_stride,
                            size_t cnt) {
    unsigned char* data{ static_cast<unsigned char*>(data_) };

    for (size_t i{ 0 }; i < cnt; ++i) {
        GetRandomMem(data + elem_stride * i, elem_size);
    }
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
struct RandomCore<core::pair::Pair<First, Second>> {
    core::pair::Pair<First, Second> operator()() const {
        return { GetRandom<First>(), GetRandom<Second>() };
    }
};

}  // namespace zeta::core_test
