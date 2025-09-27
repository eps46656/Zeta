#pragma once

#include <random>
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

template <typename RetInt, typename RangeInt>
RetInt GetRandomInt(RangeInt lb, RangeInt rb) {
    static std::uniform_int_distribution<unsigned long long> ll_generator;

    ZETA_Core_StaticAssert(core::IsIntegral<RetInt>);
    ZETA_Core_StaticAssert(core::IsIntegral<RangeInt>);

    return static_cast<RetInt>(
        lb + static_cast<RangeInt>(ll_generator(GetRandomEngine()) %
                                   (static_cast<unsigned long long>(rb) -
                                    static_cast<unsigned long long>(lb) + 1)));
}

template <typename Value>
struct GetRandomCore {
    Value operator()() const {
        std::uniform_int_distribution<Value> generator;
        return generator(GetRandomEngine());
    }
};

template <typename Value>
struct GetRandomCore<Value*> {
    GetRandomCore<uintptr_t> random_core;

    Value* operator()() const {
        return reinterpret_cast<Value*>(this->random_core());
    }
};

template <typename Value>
Value GetRandom() {
    return GetRandomCore<Value>{}();
}

template <typename Value, typename Iterator>
void GetRandoms(Iterator beg, Iterator end) {
    for (; beg != end; ++beg) { *beg = GetRandom<Value>(); }
}

}  // namespace zeta::core_test
