#pragma once

#include <zeta/core/array.hpp>

namespace zeta::core {

template <typename Elem, size_t N>
constexpr Elem& array::Array<Elem, N>::operator[](size_t idx) & {
    return this->elems[idx];
}

template <typename Elem, size_t N>
constexpr Elem&& array::Array<Elem, N>::operator[](size_t idx) && {
    return static_cast<Elem&&>(this->elems[idx]);
}

template <typename Elem, size_t N>
constexpr Elem const& array::Array<Elem, N>::operator[](size_t idx) const& {
    return this->elems[idx];
}

}  // namespace zeta::core
