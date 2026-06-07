#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/hash_utils.hpp>

namespace zeta::core {

inline unsigned long long hash_utils::BasicMemHash(void const* data,
                                                   size_t size,
                                                   unsigned long long salt) {
    return (BasicElemHash)(data, size, 0, 1, salt);
}

inline unsigned long long hash_utils::BasicElemHash(void const* data_,
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

}  // namespace zeta::core
