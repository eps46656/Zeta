#pragma once

namespace zeta::core::array {

template <typename Elem_, size_t N_>
struct Array {
    using Elem = Elem_;
    static constexpr size_t N{ N_ };

    Elem elems[N];

    constexpr Elem& operator[](size_t idx) &;
    constexpr Elem&& operator[](size_t idx) &&;
    constexpr Elem const& operator[](size_t idx) const&;
};

}  // namespace zeta::core::array
