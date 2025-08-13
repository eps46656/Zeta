#pragma once

#include <zeta/core_test/define.h>
#include <zeta/core_test/random.h>

#include <utility>

template <typename Key, typename Value>
struct Zeta_CoreTest_KeyValuePair {
    Key key;
    Value second;
};

template <typename Key, typename Value>
unsigned long long Hash(
    void* context, Zeta_CoreTest_KeyValuePair<Key, Value> const& key_val_pair,
    unsigned long long salt) {
    ZETA_Core_Unused(context);

    return Hash(key_val_pair.key) * 32 + salt;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
unsigned long long Compare(void* context,
                           Zeta_CoreTest_KeyValuePair<KeyA, ValueB> const& a,
                           Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    ZETA_Core_Unused(context);

    return Compare(a.key, b.key);
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator==(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                  Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) == 0;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator!=(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                  Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) != 0;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator<(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                 Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) < 0;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator>(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                 Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) > 0;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator<=(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                  Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) <= 0;
}

template <typename KeyA, typename ValueA, typename KeyB, typename ValueB>
bool_t operator>=(Zeta_CoreTest_KeyValuePair<KeyA, ValueA> const& a,
                  Zeta_CoreTest_KeyValuePair<KeyB, ValueB> const& b) {
    return Compare(NULL, a, b) >= 0;
}

namespace std {

template <typename Key, typename Value>
struct hash<Zeta_CoreTest_KeyValuePair<Key, Value>> {
    unsigned long long operator()(
        Zeta_CoreTest_KeyValuePair<Key, Value> const& key_value_pair) const {
        return Hash(NULL, key_value_pair, 0);
    }
};

}  // namespace std

template <typename Key, typename Value>
struct Zeta_CoreTest_GetRandomCore<Zeta_CoreTest_KeyValuePair<Key, Value>> {
    KeyValuePair<Key, Value> operator()() const {
        return { Zeta_CoreTest_GetRandom<Key>(),
                 Zeta_CoreTest_GetRandom<Value>() };
    }
};
