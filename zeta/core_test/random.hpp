#pragma once

#include <stdint.h>

#include <random>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core_test {

constexpr std::mt19937_64& GetRandomEngine() {
    static std::mt19937_64 en;
    return en;
}

constexpr void SetRandomSeed(unsigned random_seed) {
    GetRandomEngine().seed(random_seed);
}

template <typename RetInt, typename LBInt, typename RBInt>
constexpr RetInt GenUniformRandomInt(LBInt lb, RBInt rb) {
    static_assert(core::integral::IsIntegral<RetInt>);
    static_assert(core::integral::IsIntegral<LBInt>);
    static_assert(core::integral::IsIntegral<RBInt>);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::integral_math::Compare(core::comparison::OpTags::LessEqual{},
                                     core::integral::RangeMinOf<RetInt>, lb));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::integral_math::Compare(core::comparison::OpTags::LessEqual{}, rb,
                                     core::integral::RangeMaxOf<RetInt>));

    RetInt ret_lb{ static_cast<RetInt>(lb) };
    RetInt ret_rb{ static_cast<RetInt>(rb) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(ret_lb <= ret_rb);

    using GenInt =
        core::meta::Conditional<core::integral::IsSignedIntegral<RetInt>,
                                signed long long, unsigned long long>;

    std::uniform_int_distribution<GenInt> generator{
        static_cast<GenInt>(ret_lb), static_cast<GenInt>(ret_rb)
    };

    return static_cast<RetInt>(generator(GetRandomEngine()));
}

constexpr void GenRandomMem(void* data_, size_t size) {
    unsigned char* data{ static_cast<unsigned char*>(data_) };

    for (size_t i{ 0 }; i < size; ++i) {
        data[i] = GenUniformRandomInt<unsigned char, unsigned char>(0, 255);
    }
}

constexpr void GenRandomLinSeq(void* data_, size_t elem_size,
                               size_t elem_stride, size_t cnt) {
    unsigned char* data{ static_cast<unsigned char*>(data_) };

    for (size_t i{ 0 }; i < cnt; ++i, data += elem_stride) {
        (GenRandomMem)(data, elem_size);
    }
}

constexpr double GenRandomDouble(double lb, double rb) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(lb <= rb);

    std::uniform_real_distribution<double> generator{ lb, rb };

    return generator(GetRandomEngine());
}

constexpr std::deque<size_t> GenRandomPartition(size_t total, size_t part_cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < part_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(part_cnt <= total);

    if (part_cnt == 1) { return std::deque<size_t>{ total }; }

    std::deque<size_t> cnts;
    cnts.resize(part_cnt - 1);

    for (size_t i{ 0 }; i < part_cnt - 1; ++i) {
        cnts[i] = zeta::core_test::GenUniformRandomInt<size_t, size_t>(
            0, total - part_cnt);
    }

    std::sort(cnts.begin(), cnts.end());

    std::deque<size_t> partition;
    partition.resize(part_cnt);

    partition[0] = cnts[0] + 1;

    for (size_t i{ 1 }; i < part_cnt - 1; ++i) {
        partition[i] = cnts[i] - cnts[i - 1] + 1;
    }

    partition[part_cnt - 1] = total - part_cnt - cnts[part_cnt - 2] + 1;

    return partition;
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
            return GenUniformRandomInt<T>(core::integral::RangeMinOf<T>,
                                          core::integral::RangeMaxOf<T>);
        }

        if constexpr (core::meta::IsPointer<T>) {
            return reinterpret_cast<T>(
                GenUniformRandomInt<uintptr_t>(0x1'0000'0000, 0x1'ffff'ffff) *
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
