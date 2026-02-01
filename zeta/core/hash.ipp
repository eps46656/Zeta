#pragma once

#include <zeta/core/hash.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::hash {

template <typename T>
unsigned long long Hash(T const& x, unsigned long long salt) {
    static HashCore<T> const hash_core;
    return hash_core(x, salt);
}

template <typename T>
unsigned long long TypeErasedHash(void const* x, unsigned long long salt) {
    return Hash<T>(*static_cast<T const*>(x), salt);
}

template <typename T>
size_t CppStdHash<T>::operator()(T const& x) const {
    return static_cast<size_t>(Hash<T>(x, 0));
}

template <typename T>
struct HashCore<T, EnableIf<(IsIntegral<T> || IsPointer<T>), void>> {
    unsigned long long operator()(T const& x_, unsigned long long salt) const {
        unsigned long long x;

        if constexpr (IsIntegral<T>) {
            x = static_cast<unsigned long long>(x_);
        } else if constexpr (IsPointer<T>) {
            x = reinterpret_cast<unsigned long long>(x_);
        }

        x ^= salt;

#if ZETA_Core_ullong_width == 32
        x = (x ^ (x >> 16)) * 0x45d9f3bULL;
        x = (x ^ (x >> 16)) * 0x45d9f3bULL;
        x = x ^ (x >> 16);
#elif ZETA_Core_ullong_width == 64
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        x = x ^ (x >> 31);
#else
#error "Unsupported architecture."
#endif

        x ^= salt;

        return x;
    }
};

}  // namespace zeta::core::hash
