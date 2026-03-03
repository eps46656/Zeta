#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

inline unsigned long long hash::ops::BasicMemHash(void const* data, size_t size,
                                                  unsigned long long salt) {
    return (BasicElemHash)(data, size, 0, 1, salt);
}

inline unsigned long long hash::ops::BasicElemHash(void const* data_,
                                                   size_t width, size_t stride,
                                                   size_t cnt,
                                                   unsigned long long salt) {
    constexpr unsigned long long fnv_offset_basis{ 14695981039346656037ULL };
    constexpr unsigned long long fnv_prime{ 1099511628211ULL };

    unsigned char const* data{ static_cast<unsigned char const*>(data_) };
    ZETA_Core_DebugAssert(data != nullptr);

    unsigned long long ret{ fnv_offset_basis ^ salt };

    for (size_t i{ 0 }; i < cnt; ++i, data += stride) {
        for (size_t j{ 0 }; j < width; ++j) {
            ret = (ret ^ data[j]) * fnv_prime;
        }

        ret += salt;
    }

    return ret;
}

namespace hash::ops::detail {

template <typename T>
struct HashImplHolder_ {
    static constexpr BasicHashImpl<T> impl;
};

}  // namespace hash::ops::detail

template <typename T>
unsigned long long hash::ops::BasicHash(T const& x, unsigned long long salt) {
    return detail::HashImplHolder_<T>::impl(x, salt);
}

template <typename T>
unsigned long long hash::ops::TypeErasedBasicHash(void const* x,
                                                  unsigned long long salt) {
    return BasicHash<T>(*static_cast<T const*>(x), salt);
}

template <typename T>
size_t hash::CppStdBasicHash<T>::operator()(T const& x) const {
    return static_cast<size_t>(ops::BasicHash<T>(x, 0));
}

template <typename T>
struct hash::ops::BasicHashImpl<
    T, meta::EnableIf<(integral::IsIntegral<T> || meta::IsPointer<T>), void>> {
    unsigned long long operator()(T const& x_, unsigned long long salt) const {
        unsigned long long x;

        if constexpr (integral::IsIntegral<T>) {
            x = static_cast<unsigned long long>(x_);
        } else if constexpr (meta::IsPointer<T>) {
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

}  // namespace zeta::core
