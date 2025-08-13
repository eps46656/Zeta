#pragma once

#include <zeta/core/define.h>

#include <random>

std::mt19937_64& Zeta_CoreTest_GetRandomEngine() {
    static std::mt19937_64 en;
    return en;
}

void Zeta_CoreTest_SetRandomSeed(unsigned random_seed) {
    Zeta_CoreTest_GetRandomEngine().seed(random_seed);
}

template <typename Int>
constexpr bool_t Zeta_CoreTest_IsInteger() {
    return std::is_same<Int, char>() ||                //
           std::is_same<Int, unsigned char>() ||       //
           std::is_same<Int, signed char>() ||         //
           std::is_same<Int, unsigned short>() ||      //
           std::is_same<Int, signed short>() ||        //
           std::is_same<Int, unsigned>() ||            //
           std::is_same<Int, signed>() ||              //
           std::is_same<Int, unsigned long>() ||       //
           std::is_same<Int, signed long>() ||         //
           std::is_same<Int, unsigned long long>() ||  //
           std::is_same<Int, signed long long>();      //
}

template <typename RetInt, typename RangeInt>
RetInt Zeta_CoreTest_GetRandomInt(RangeInt lb, RangeInt rb) {
    static std::uniform_int_distribution<unsigned long long> ll_generator;

    ZETA_Core_StaticAssert(Zeta_CoreTest_IsInteger<RetInt>());
    ZETA_Core_StaticAssert(Zeta_CoreTest_IsInteger<RangeInt>());

    return static_cast<RetInt>(
        lb +
        static_cast<RangeInt>(ll_generator(Zeta_CoreTest_GetRandomEngine()) %
                              (static_cast<unsigned long long>(rb) -
                               static_cast<unsigned long long>(lb) + 1)));
}

template <typename Value>
struct Zeta_CoreTest_GetRandomCore {
    Value operator()() const {
        std::uniform_int_distribution<Value> generator;
        return generator(Zeta_CoreTest_GetRandomEngine());
    }
};

template <typename Value>
struct Zeta_CoreTest_GetRandomCore<Value*> {
    Zeta_CoreTest_GetRandomCore<uintptr_t> random_core;

    Value* operator()() const {
        return reinterpret_cast<Value*>(this->random_core());
    }
};

template <typename Value>
Value Zeta_CoreTest_GetRandom() {
    return Zeta_CoreTest_GetRandomCore<Value>{}();
}

template <typename Value, typename Iterator>
void Zeta_CoreTest_GetRandoms(Iterator beg, Iterator end) {
    for (; beg != end; ++beg) { *beg = Zeta_CoreTest_GetRandom<Value>(); }
}
