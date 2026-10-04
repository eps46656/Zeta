#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/sha256.ipp>

namespace zeta::core::type_identity {

namespace detail {

template <typename T>
constexpr auto GetTypeName_() {
    constexpr size_t l_size{ []() {
        constexpr char func_name[]{ __PRETTY_FUNCTION__ };

        size_t ret{ 0 };

        while (func_name[ret] != 'T' || func_name[ret + 1] != ' ' ||
               func_name[ret + 2] != '=' || func_name[ret + 3] != ' ') {
            ++ret;
        }

        return ret + 4;
    }() };

    constexpr size_t r_size{ 1 };

    constexpr char func_name[]{ __PRETTY_FUNCTION__ };
    size_t func_name_size{ sizeof(func_name) };

    constexpr size_t ret_size{ func_name_size - l_size - r_size };

    array::Array<char, ret_size> ret;

    for (size_t i{ 0 }; i < ret_size; ++i) { ret[i] = func_name[l_size + i]; }

    return ret;
}

}  // namespace detail

template <typename T>
constexpr auto type_name{ detail::GetTypeName_<T>() };

namespace detail {

template <typename T>
constexpr sha256::Digest GetTypeHashCode_() {
    constexpr char func_name[]{ __PRETTY_FUNCTION__ };
    constexpr size_t func_name_size{ sizeof(func_name) };

    size_t l_size{ 4 };

    while (func_name[l_size - 4] != 'T' || func_name[l_size - 3] != ' ' ||
           func_name[l_size - 2] != '=' || func_name[l_size - 1] != ' ') {
        ++l_size;
    }

    size_t r_size{ 1 };

    size_t data_size{ func_name_size - l_size - r_size };

    unsigned char data[func_name_size];

    for (size_t i{ 0 }; i < data_size; ++i) {
        data[i] = static_cast<unsigned char>(func_name[l_size + i]);
    }

    sha256::Hasher hasher;

    hasher.Rotate(data, data_size);

    sha256::Digest ret;

    hasher.GetDigest(ret);

    return ret;
}

}  // namespace detail

template <typename T>
constexpr auto type_hash_code{ detail::GetTypeHashCode_<T>() };

using TypeHashCode = meta::RemoveCVRef<decltype(type_hash_code<void>)>;

}  // namespace zeta::core::type_identity
