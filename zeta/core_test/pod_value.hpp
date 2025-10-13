#pragma once

#include <iostream>
#include <zeta/core/compare.ipp>
#include <zeta/core/hash.hpp>
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
struct hash::HashCore<core_test::PODValue> {
    unsigned long long operator()(core_test::PODValue const& x,
                                  unsigned long long salt) const {
        return core::MemHash(x.data, core_test::PODValue::width, salt);
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

namespace zeta::core::compare {

template <>
struct CompareCore<core_test::PODValue, core_test::PODValue> {
    int operator()(core_test::PODValue const& x,
                   core_test::PODValue const& y) const {
        return core::MemCompare(x.data, y.data, core_test::PODValue::width);
    }
};

}  // namespace zeta::core::compare

namespace zeta::core_test {

inline bool operator==(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) == 0;
}

inline bool operator!=(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) != 0;
}

inline bool operator<(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) < 0;
}

inline bool operator<=(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) <= 0;
}

inline bool operator>(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) > 0;
}

inline bool operator>=(PODValue const& x, PODValue const& y) {
    return core::compare::Compare(x, y) >= 0;
}

}  // namespace zeta::core_test
