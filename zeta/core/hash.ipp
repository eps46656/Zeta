#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Hasher, typename Value>
constexpr unsigned long long hash::HasherTraits<Hasher, Value>::Hash(
    Hasher const& hasher, Value const& value, unsigned long long salt) {
    return hasher(value, salt);
}

template <typename Hasher, typename Value>
constexpr unsigned long long hash::Hash(Hasher const& hasher,
                                        Value const& value,
                                        unsigned long long salt) {
    return HasherTraits<Hasher, Value>::Hash(hasher, value, salt);
}

template <typename Hasher, typename Value>
void hash::CheckContract() {
#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                                   \
    ZETA_Core_Unused([&](Hasher const& hasher, Value const& value, \
                         unsigned long long salt) {                \
        ZETA_Core_Unused(hasher);                                  \
        ZETA_Core_Unused(value);                                   \
        ZETA_Core_Unused(salt);                                    \
        (method)(__VA_ARGS__);                                     \
    })

    CheckMethod(  //
        Hash,     // method
                  //
        hasher,   // hasher
        value,    // value
        salt      // salt
    );
}

template <typename Value>
unsigned long long hash::BasicHash(Value const& value,
                                   unsigned long long salt) {
    return (Hash)(BasicHasher<Value>{}, value, salt);
}

template <typename Value>
constexpr unsigned long long hash::UniversalBasicHasher::operator()(
    Value const& value, unsigned long long salt) const {
    return (BasicHash<Value>)(value, salt);
}

template <typename Value>
unsigned long long hash::TypeErasedBasicHash(void const* value,
                                             unsigned long long salt) {
    return (BasicHash<Value>)(*static_cast<Value const*>(value), salt);
}

template <typename Value>
size_t hash::CppStdBasicHash<Value>::operator()(Value const& value) const {
    return static_cast<size_t>((BasicHash<Value>)(value, 0));
}

template <typename Value>
struct hash::BasicHasher<
    Value, meta::EnableIf<
               (integral::IsIntegral<Value> || meta::IsPointer<Value>), void>> {
    unsigned long long operator()(Value const& value_,
                                  unsigned long long salt) const {
        unsigned long long value;

        if constexpr (integral::IsIntegral<Value>) {
            value = static_cast<unsigned long long>(value_);
        } else if constexpr (meta::IsPointer<Value>) {
            value = reinterpret_cast<unsigned long long>(value_);
        }

        value ^= salt;

#if ZETA_Core_ullong_width == 32
        value = (value ^ (value >> 16)) * 0x45d9f3bULL;
        value = (value ^ (value >> 16)) * 0x45d9f3bULL;
        value = value ^ (value >> 16);
#elif ZETA_Core_ullong_width == 64
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        value = value ^ (value >> 31);
#else
#error "Unsupported architecture."
#endif

        value ^= salt;

        return value;
    }
};

}  // namespace zeta::core
