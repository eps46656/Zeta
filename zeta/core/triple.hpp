#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/hash.hpp>

namespace zeta::core::triple {

template <typename First, typename Second, typename Third>
struct Triple {
    First first;
    Second second;
    Third third;
};

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator==(Triple<XFirst, XSecond, XThird> const& x,
                          Triple<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator!=(Triple<XFirst, XSecond, XThird> const& x,
                          Triple<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator<(Triple<XFirst, XSecond, XThird> const& x,
                         Triple<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator<=(Triple<XFirst, XSecond, XThird> const& x,
                          Triple<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator>(Triple<XFirst, XSecond, XThird> const& x,
                         Triple<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
constexpr bool operator>=(Triple<XFirst, XSecond, XThird> const& x,
                          Triple<YFirst, YSecond, YThird> const& y);

}  // namespace zeta::core::triple

namespace zeta::core {

template <typename First, typename Second, typename Third>
struct hash::BasicHashImpl<triple::Triple<First, Second, Third>> {
    constexpr unsigned long long operator()(
        triple::Triple<First, Second, Third> const& x,
        unsigned long long salt) const;
};

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
struct compare::BasicCompareImpl<triple::Triple<XFirst, XSecond, XThird>,
                                 triple::Triple<YFirst, YSecond, YThird>> {
    constexpr int operator()(
        triple::Triple<XFirst, XSecond, XThird> const& x,
        triple::Triple<YFirst, YSecond, YThird> const& y) const;
};

}  // namespace zeta::core
