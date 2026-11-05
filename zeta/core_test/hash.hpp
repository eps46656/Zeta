#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/integral.hpp>

namespace zeta::core_test::hash {

template <typename T>
struct HashCore;

template <typename T>
    requires(core::integral::IsIntegralType<T> || core::meta::IsPointerType<T>)
struct HashCore<T> {
    static constexpr unsigned long long F(T const& x, unsigned long long salt) {
        return std::hash<T>{}(x) ^ salt;
    }
};

}  // namespace zeta::core_test::hash
