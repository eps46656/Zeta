#pragma once

#include <iostream>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/hash_utils.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test {

struct PODValue {
    static constexpr size_t width{ 7 };
    unsigned char pre_padding[8];
    unsigned char data[width];
    unsigned char post_padding[8];
};

inline std::ostream& operator<<(std::ostream& os, PODValue const& val) {
    char cs[]{ "0123456789ABCDEF" };

    for (size_t i{ PODValue::width }; 0 < i--;) {
        os << cs[val.data[i] / 16] << cs[val.data[i] % 16];
    }

    return os;
}

}  // namespace zeta::core_test

namespace zeta::core {

template <>
struct hash::BasicHasher<core_test::PODValue> {
    unsigned long long operator()(core_test::PODValue const& x,
                                  unsigned long long salt) const {
        return hash_utils::BasicMemHash(x.data, core_test::PODValue::width,
                                        salt);
    }
};

}  // namespace zeta::core

namespace zeta::core_test {

template <>
struct RandomCore<PODValue> {
    PODValue operator()() const {
        PODValue ret;

        for (size_t i{ 0 }; i < PODValue::width; ++i) {
            ret.data[i] = GetRandomInt<unsigned char>(0, 255);
        }

        return ret;
    }
};

}  // namespace zeta::core_test

namespace zeta::core {

template <>
struct comparison::ComparatorTraits<
    comparison::BasicComparator<core_test::PODValue, core_test::PODValue>> {
    template <comparison::IsOpType OpType>
    static auto Compare(
        comparison::BasicComparator<core_test::PODValue, core_test::PODValue>,
        OpType op, core_test::PODValue const& a, core_test::PODValue const& b) {
        return comparison::BasicCompare(
            op,
            comparison_utils::MemLexCompare(a.data, b.data,
                                            core_test::PODValue::width,
                                            core_test::PODValue::width),
            0);
    }
};

}  // namespace zeta::core

namespace zeta::core_test {

inline bool operator==(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::Equal>{}, x, y);
}

inline bool operator!=(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::NotEqual>{}, x, y);
}

inline bool operator<(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::Less>{}, x, y);
}

inline bool operator<=(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::LessEqual>{}, x, y);
}

inline bool operator>(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::Greater>{}, x, y);
}

inline bool operator>=(PODValue const& x, PODValue const& y) {
    return core::comparison::BasicCompare(
        core::meta::AutoValueWrapper<core::comparison::Op::GreaterEqual>{}, x,
        y);
}

}  // namespace zeta::core_test
