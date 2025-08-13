#pragma once

#include <zeta/core/utils.h>
#include <zeta/core_test/cmp_utils.h>
#include <zeta/core_test/hash_utils.h>
#include <zeta/core_test/random.h>

#include <iostream>
#include <unordered_map>

struct Zeta_CoreTest_PODValue {
    static constexpr size_t width{ 7 };
    unsigned char pre_padding[8];
    unsigned char data[width];
    unsigned char post_padding[8];
};

std::ostream& operator<<(std::ostream& os, Zeta_CoreTest_PODValue const& val) {
    char cs[]{ "0123456789ABCDEF" };

    for (size_t i{ Zeta_CoreTest_PODValue::width }; 0 < i--;) {
        os << cs[val.data[i] / 16] << cs[val.data[i] % 16];
    }

    return os;
}

template <>
struct HashCore<Zeta_CoreTest_PODValue> {
    unsigned long long operator()(void const* context,
                                  Zeta_CoreTest_PODValue const& x,
                                  unsigned long long salt) {
        ZETA_Core_Unused(context);

        return Zeta_Core_MemHash(x.data, Zeta_CoreTest_PODValue::width, salt);
    }
};

namespace std {

template <>
struct hash<Zeta_CoreTest_PODValue> {
    unsigned long long operator()(Zeta_CoreTest_PODValue const& x) const {
        return Hash(NULL, x, Zeta_CoreTest_PODValue::width);
    }
};

}  // namespace std

template <>
struct CompareCore<Zeta_CoreTest_PODValue, Zeta_CoreTest_PODValue> {
    int operator()(void const* context, Zeta_CoreTest_PODValue const& x,
                   Zeta_CoreTest_PODValue const& y) const {
        ZETA_Core_Unused(context);

        for (size_t i{ 0 }; i < Zeta_CoreTest_PODValue::width; ++i) {
            switch (ZETA_Core_ThreeWayCompare(x.data[i], y.data[i])) {
                case -1: return -1;
                case 1: return 1;
            }
        }

        return 0;
    }
};

bool_t operator==(Zeta_CoreTest_PODValue const& x,
                  Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) == 0;
}

bool_t operator!=(Zeta_CoreTest_PODValue const& x,
                  Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) != 0;
}

bool_t operator<(Zeta_CoreTest_PODValue const& x,
                 Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) < 0;
}

bool_t operator<=(Zeta_CoreTest_PODValue const& x,
                  Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) <= 0;
}

bool_t operator>(Zeta_CoreTest_PODValue const& x,
                 Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) > 0;
}

bool_t operator>=(Zeta_CoreTest_PODValue const& x,
                  Zeta_CoreTest_PODValue const& y) {
    return Compare(NULL, x, y) >= 0;
}

template <>
struct Zeta_CoreTest_GetRandomCore<Zeta_CoreTest_PODValue> {
    Zeta_CoreTest_PODValue operator()() const {
        Zeta_CoreTest_PODValue ret;

        for (size_t i{ 0 }; i < Zeta_CoreTest_PODValue::width; ++i) {
            ret.data[i] = Zeta_CoreTest_GetRandomInt<unsigned char>(0, 255);
        }

        return ret;
    }
};
