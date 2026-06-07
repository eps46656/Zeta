#pragma once

namespace zeta::core::array {

template <typename Elem, size_t N>
struct Array {
    Elem elems[N];

    constexpr Elem& operator[](size_t idx) &;
    constexpr Elem&& operator[](size_t idx) &&;
    constexpr Elem const& operator[](size_t idx) const&;
};

}  // namespace zeta::core::array
