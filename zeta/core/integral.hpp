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

#define ZETA_Core_bitint_max_width static_cast<size_t>(__BITINT_MAXWIDTH__)

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

template <typename Integral>
struct IsUnsignedCore {
    static constexpr bool value{ false };
};

template <typename Integral>
struct IsSignedCore {
    static constexpr bool value{ false };
};

template <typename Integral>
constexpr bool IsUnsigned{ []() {
    constexpr bool is_unsigned{ IsUnsignedCore<Integral>::value };
    constexpr bool is_signed{ IsSignedCore<Integral>::value };

    ZETA_Core_StaticAssert(!is_unsigned || !is_signed);

    return is_unsigned;
}() };

template <typename Integral>
constexpr bool IsSigned{ []() {
    constexpr bool is_unsigned{ IsUnsignedCore<Integral>::value };
    constexpr bool is_signed{ IsSignedCore<Integral>::value };

    ZETA_Core_StaticAssert(!is_unsigned || !is_signed);

    return is_signed;
}() };

template <>
struct IsUnsignedCore<char> {
    static constexpr bool value{ 0 <= static_cast<char>(-1) };
};

template <>
struct IsSignedCore<char> {
    static constexpr bool value{ static_cast<char>(-1) < 0 };
};

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(utype, stype)                      \
    template <>                              \
    struct IsUnsignedCore<utype> {           \
        static constexpr bool value{ true }; \
    };                                       \
                                             \
    template <>                              \
    struct IsSignedCore<stype> {             \
        static constexpr bool value{ true }; \
    };                                       \
                                             \
    ZETA_Core_StaticAssert(true)

F(unsigned char, signed char);
F(unsigned short, signed short);
F(unsigned int, signed int);
F(unsigned long, signed long);
F(unsigned long long, signed long long);

#pragma pop_macro("F")

template <size_t N>
struct IsUnsignedCore<unsigned _BitInt(N)> {
    static constexpr bool value{ true };
};

template <size_t N>
struct IsSignedCore<signed _BitInt(N)> {
    static constexpr bool value{ true };
};

template <typename Integral>
constexpr bool IsIntegral{ []() {
    constexpr bool is_unsigned{ IsUnsignedCore<Integral>::value };
    constexpr bool is_signed{ IsSignedCore<Integral>::value };

    ZETA_Core_StaticAssert(!is_unsigned || !is_signed);

    return is_unsigned || is_signed;
}() };

template <typename Integral>
struct WidthOfImpl;

template <typename Integral>
constexpr size_t WidthOf{ WidthOfImpl<Integral>::value };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(type, val)                          \
    template <>                               \
    struct WidthOfImpl<type> {                \
        static constexpr size_t value{ val }; \
    };                                        \
                                              \
    ZETA_Core_StaticAssert(true)

F(char, IsSigned<char> ? ZETA_Core_schar_width : ZETA_Core_uchar_width);
F(unsigned char, ZETA_Core_uchar_width);
F(signed char, ZETA_Core_schar_width);

F(unsigned short, ZETA_Core_ushrt_width);
F(signed short, ZETA_Core_sshrt_width);

F(unsigned int, ZETA_Core_uint_width);
F(signed int, ZETA_Core_sint_width);

F(unsigned long, ZETA_Core_ulong_width);
F(signed long, ZETA_Core_slong_width);

F(unsigned long long, ZETA_Core_ullong_width);
F(signed long long, ZETA_Core_sllong_width);

#pragma pop_macro("F")

template <size_t N>
struct WidthOfImpl<unsigned _BitInt(N)> {
    static constexpr size_t value{ static_cast<int>(N) };
};

template <size_t N>
struct WidthOfImpl<signed _BitInt(N)> {
    static constexpr size_t value{ static_cast<int>(N) };
};

namespace detail {

template <typename Integral>
constexpr Integral Pow2Minus1(size_t exp) {  // Returns 2^exp - 1.
    if (exp == 0) { return 0; }

    Integral ret{ 1 };
    Integral base{ 2 };

    for (--exp;;) {
        if (exp % 2 != 0) { ret *= base; }
        exp /= 2;
        if (exp == 0) { break; }
        base *= base;
    }

    Integral one{ 1 };
    Integral two{ 2 };

    return (ret - one) * two + one;
}

}  // namespace detail

template <typename Integral>
constexpr Integral BasicRangeMinOf{ []() {
    if constexpr (IsUnsigned<Integral>) { return static_cast<Integral>(0); }

    if constexpr (IsSigned<Integral>) {
        return -detail::Pow2Minus1<Integral>(WidthOf<Integral> - 1);
    }
}() };

template <typename Integral>
constexpr Integral BasicRangeMaxOf{ []() {
    if constexpr (IsUnsigned<Integral>) {
        return detail::Pow2Minus1<Integral>(WidthOf<Integral>);
    }

    if constexpr (IsSigned<Integral>) {
        return detail::Pow2Minus1<Integral>(WidthOf<Integral> - 1);
    }
}() };

template <typename Integral>
struct RangeMinOfImpl {
    static constexpr Integral value{ BasicRangeMinOf<Integral> };
};

template <typename Integral>
struct RangeMaxOfImpl {
    static constexpr Integral value{ BasicRangeMaxOf<Integral> };
};

template <typename Integral>
constexpr Integral RangeMinOf{ RangeMinOfImpl<Integral>::value };

template <typename Integral>
constexpr Integral RangeMaxOf{ RangeMaxOfImpl<Integral>::value };

#pragma push_macro("F")

template <>
struct RangeMinOfImpl<char> {
    static constexpr char value{
        IsSigned<char> ? static_cast<char>(RangeMinOfImpl<signed char>::value)
                       : static_cast<char>(RangeMinOfImpl<unsigned char>::value)
    };
};

template <typename Integral>
struct UnsignedOfImpl;

template <typename Integral>
struct SignedOfImpl;

template <typename Integral>
using UnsignedOf = typename UnsignedOfImpl<Integral>::type;

template <typename Integral>
using SignedOf = typename SignedOfImpl<Integral>::type;

template <>
struct UnsignedOfImpl<char> {
    using type = meta::Conditional<IsSigned<char>, unsigned char, char>;
};

template <>
struct SignedOfImpl<char> {
    using type = meta::Conditional<IsSigned<char>, char, signed char>;
};

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(unsigned_integral, signed_integral)  \
    template <>                                \
    struct UnsignedOfImpl<unsigned_integral> { \
        using type = unsigned_integral;        \
    };                                         \
                                               \
    template <>                                \
    struct UnsignedOfImpl<signed_integral> {   \
        using type = unsigned_integral;        \
    };                                         \
                                               \
    template <>                                \
    struct SignedOfImpl<unsigned_integral> {   \
        using type = signed_integral;          \
    };                                         \
                                               \
    template <>                                \
    struct SignedOfImpl<signed_integral> {     \
        using type = signed_integral;          \
    };                                         \
                                               \
    ZETA_Core_StaticAssert(true)

F(unsigned char, signed char);
F(unsigned short, signed short);
F(unsigned int, signed int);
F(unsigned long, signed long);
F(unsigned long long, signed long long);

#pragma pop_macro("F")

template <size_t N>
struct UnsignedOfImpl<unsigned _BitInt(N)> {
    using type = unsigned _BitInt(N);
};

template <size_t N>
struct UnsignedOfImpl<signed _BitInt(N)> {
    using type = unsigned _BitInt(N);
};

template <size_t N>
struct SignedOfImpl<unsigned _BitInt(N)> {
    using type = signed _BitInt(N);
};

template <size_t N>
struct SignedOfImpl<signed _BitInt(N)> {
    using type = signed _BitInt(N);
};

template <typename IntegralX, typename IntegralY>
int MathCompare(IntegralX x, IntegralY y) {
    constexpr bool x_is_signed{ IsSigned<IntegralX> };
    constexpr bool y_is_signed{ IsSigned<IntegralY> };

    if constexpr (x_is_signed == y_is_signed) { return (y < x) - (x < y); }

    if constexpr (x_is_signed && !y_is_signed) {
        if (x < 0) { return -1; }
        auto unsigned_x{ static_cast<UnsignedOf<IntegralX>>(x) };
        return (y < unsigned_x) - (unsigned_x < y);
    }

    if constexpr (!x_is_signed && y_is_signed) {
        if (y < 0) { return 1; }
        auto unsigned_y{ static_cast<UnsignedOf<IntegralY>>(y) };
        return (unsigned_y < x) - (x < unsigned_y);
    }
}

}  // namespace zeta::core::integral
