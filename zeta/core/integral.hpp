#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

#define ZETA_Core_uchar_width (__CHAR_BIT__)
#define ZETA_Core_ushrt_width (__SHRT_WIDTH__)
#define ZETA_Core_uint_width (__INT_WIDTH__)
#define ZETA_Core_ulong_width (__LONG_WIDTH__)
#define ZETA_Core_ullong_width (__LLONG_WIDTH__)

#define ZETA_Core_schar_width (__CHAR_BIT__)
#define ZETA_Core_sshrt_width (__SHRT_WIDTH__)
#define ZETA_Core_sint_width (__INT_WIDTH__)
#define ZETA_Core_slong_width (__LONG_WIDTH__)
#define ZETA_Core_sllong_width (__LLONG_WIDTH__)

#define ZETA_Core_size_width (__SIZE_WIDTH__)

#define ZETA_Core_bitint_max_width \
    static_cast<unsigned long long>(__BITINT_MAXWIDTH__)

#define ZETA_Core_max_capacity (::zeta::core::integral::RangeMaxOf<size_t> / 4)

namespace zeta::core::integral {

using byte_t = unsigned char;

using ubit8_t = unsigned _BitInt(8);
using sbit8_t = signed _BitInt(8);

using ubit16_t = unsigned _BitInt(16);
using sbit16_t = signed _BitInt(16);

using ubit32_t = unsigned _BitInt(32);
using sbit32_t = signed _BitInt(32);

using ubit64_t = unsigned _BitInt(64);
using sbit64_t = signed _BitInt(64);

using ubit128_t = unsigned _BitInt(128);
using sbit128_t = signed _BitInt(128);

using sdllong_t = signed _BitInt(ZETA_Core_ullong_width * 2);
using udllong_t = unsigned _BitInt(ZETA_Core_ullong_width * 2);

namespace detail {

struct IncompleteTraitsBase_ {};

}  // namespace detail

template <typename Integral, typename = void>
struct IntegralTraits : public detail::IncompleteTraitsBase_ {};

template <typename Integral>
constexpr bool IsIntegral{
    !meta::IsBaseOf<detail::IncompleteTraitsBase_, IntegralTraits<Integral>>
};

template <typename Integral>
constexpr bool IsSignedIntegral{ []() {
    if constexpr (integral::IsIntegral<Integral>) {
        constexpr bool is_signed{ IntegralTraits<Integral>::is_signed };
        return is_signed;
    } else {
        return false;
    }
}() };

template <typename Integral>
constexpr bool IsUnsignedIntegral{ []() {
    if constexpr (integral::IsIntegral<Integral>) {
        constexpr bool is_signed{ IntegralTraits<Integral>::is_signed };
        return !is_signed;
    } else {
        return false;
    }
}() };

template <typename Integral>
constexpr unsigned long long WidthOf{ []() {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    constexpr unsigned long long width{ IntegralTraits<Integral>::width };

    return width;
}() };

template <typename Integral>
using TryMakeUnsignedOf = decltype([]() {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    using UnsignedIntegral = IntegralTraits<Integral>::UnsignedType;

    ZETA_Core_StaticAssert(
        meta::IsAnyOf<UnsignedIntegral, meta::NeverMatchTag> ||
        integral::IsIntegral<UnsignedIntegral>);

    return meta::TypeWrapper<UnsignedIntegral>{};
}())::Type;

template <typename Integral>
using TryMakeSignedOf = decltype([]() {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    using SignedIntegral = IntegralTraits<Integral>::SignedType;

    ZETA_Core_StaticAssert(meta::IsAnyOf<SignedIntegral, meta::NeverMatchTag> ||
                           integral::IsIntegral<SignedIntegral>);

    return meta::TypeWrapper<SignedIntegral>{};
}())::Type;

template <typename Integral>
using MakeUnsignedOf = decltype([]() {
    using UnsignedIntegral = TryMakeUnsignedOf<Integral>;

    ZETA_Core_StaticAssert(integral::IsIntegral<UnsignedIntegral>);

    return meta::TypeWrapper<UnsignedIntegral>{};
}())::Type;

template <typename Integral>
using MakeSignedOf = decltype([]() {
    using SignedIntegral = TryMakeSignedOf<Integral>;

    ZETA_Core_StaticAssert(integral::IsIntegral<SignedIntegral>);

    return meta::TypeWrapper<SignedIntegral>{};
}())::Type;

template <>
struct IntegralTraits<char> {
    static constexpr bool is_signed{ static_cast<char>(-1) <
                                     static_cast<char>(0) };

    static constexpr unsigned long long width{ is_signed
                                                   ? ZETA_Core_schar_width
                                                   : ZETA_Core_uchar_width };

    using UnsignedType = meta::Conditional<is_signed, unsigned char, char>;

    using SignedType = meta::Conditional<is_signed, char, signed char>;
};

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(utype, stype, width_)                              \
    template <>                                              \
    struct IntegralTraits<utype> {                           \
        static constexpr bool is_signed{ false };            \
        static constexpr unsigned long long width{ width_ }; \
                                                             \
        using UnsignedType = utype;                          \
        using SignedType = stype;                            \
    };                                                       \
                                                             \
    template <>                                              \
    struct IntegralTraits<stype> {                           \
        static constexpr bool is_signed{ true };             \
        static constexpr unsigned long long width{ width_ }; \
                                                             \
        using UnsignedType = utype;                          \
        using SignedType = stype;                            \
    };                                                       \
                                                             \
    ZETA_Core_StaticAssert(true)

F(unsigned char, signed char, ZETA_Core_uchar_width);
F(unsigned short, signed short, ZETA_Core_ushrt_width);
F(unsigned int, signed int, ZETA_Core_uint_width);
F(unsigned long, signed long, ZETA_Core_ulong_width);
F(unsigned long long, signed long long, ZETA_Core_ullong_width);

#pragma pop_macro("F")

template <unsigned long long N>
struct IntegralTraits<unsigned _BitInt(N)> {
    static constexpr bool is_signed{ false };
    static constexpr unsigned long long width{ N };

    using UnsignedType = unsigned _BitInt(N);
    using SignedType = signed _BitInt(N);
};

template <unsigned long long N>
struct IntegralTraits<signed _BitInt(N)> {
    static constexpr bool is_signed{ true };
    static constexpr unsigned long long width{ N };

    using UnsignedType = unsigned _BitInt(N);
    using SignedType = signed _BitInt(N);
};

namespace detail {

template <typename Integral>
constexpr Integral Pow2Minus1_(unsigned long long exp) {  // Returns 2^exp - 1.
    if (exp == 0) { return 0; }

    Integral x{ 1 };

    x <<= exp - 1;  // x = 2^(exp - 1)
    --x;            // x = 2^(exp - 1) - 1
    x <<= 1;        // x = 2^exp - 2
    ++x;            // x = 2^exp - 1

    return x;
}

}  // namespace detail

template <typename Integral>
constexpr Integral RangeMinOf{ []() {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    return IsSignedIntegral<Integral>
               ? -detail::Pow2Minus1_<Integral>(WidthOf<Integral> - 1)
               : static_cast<Integral>(0);
}() };

template <typename Integral>
constexpr Integral RangeMaxOf{ []() {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    return IsSignedIntegral<Integral>
               ? detail::Pow2Minus1_<Integral>(WidthOf<Integral> - 1)
               : detail::Pow2Minus1_<Integral>(WidthOf<Integral>);
}() };

template <typename IntegralX, typename IntegralY>
int MathCompare(IntegralX x, IntegralY y) {
    ZETA_Core_StaticAssert(integral::IsIntegral<IntegralX>);
    ZETA_Core_StaticAssert(integral::IsIntegral<IntegralY>);

    constexpr bool x_is_signed{ IsSignedIntegral<IntegralX> };
    constexpr bool y_is_signed{ IsSignedIntegral<IntegralY> };

    if constexpr (x_is_signed == y_is_signed) { return (y < x) - (x < y); }

    if constexpr (x_is_signed && !y_is_signed) {
        if (x < 0) { return -1; }
        auto unsigned_x{ static_cast<MakeUnsignedOf<IntegralX>>(x) };
        return (y < unsigned_x) - (unsigned_x < y);
    }

    if constexpr (!x_is_signed && y_is_signed) {
        if (y < 0) { return 1; }
        auto unsigned_y{ static_cast<MakeUnsignedOf<IntegralY>>(y) };
        return (unsigned_y < x) - (x < unsigned_y);
    }
}

}  // namespace zeta::core::integral
