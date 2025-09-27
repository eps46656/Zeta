#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/utils.hpp>

#define ZETA_Core_uchar_width (__CHAR_BIT__)
#define ZETA_Core_ushrt_width (__SHRT_WIDTH__)
#define ZETA_Core_uint_width (__INT_WIDTH__)
#define ZETA_Core_ulong_width (__LONG_WIDTH__)
#define ZETA_Core_ullong_width (__LLONG_WIDTH__)
#define ZETA_Core_ubit8_width (8)
#define ZETA_Core_ubit16_width (16)
#define ZETA_Core_ubit32_width (32)
#define ZETA_Core_ubit64_width (64)
#define ZETA_Core_ubit128_width (128)
#define ZETA_Core_size_width (__SIZE_WIDTH__)

#define ZETA_Core_uchar_min (static_cast<unsigned char>(0))
#define ZETA_Core_uchar_max (static_cast<unsigned char>(-1))
#define ZETA_Core_schar_min (-ZETA_Core_schar_max)
#define ZETA_Core_schar_max (static_cast<signed char>(ZETA_Core_uchar_max / 2))

#define ZETA_Core_SignedChar (static_cast<char>(-1) < 0)

#define ZETA_Core_char_min                                         \
    (ZETA_Core_SignedChar ? static_cast<char>(ZETA_Core_schar_min) \
                          : static_cast<char>(ZETA_Core_uchar_min))
#define ZETA_Core_char_max                                         \
    (ZETA_Core_SignedChar ? static_cast<char>(ZETA_Core_schar_max) \
                          : static_cast<char>(ZETA_Core_uchar_max))

#define ZETA_Core_ushrt_min (static_cast<unsigned short>(0))
#define ZETA_Core_ushrt_max (static_cast<unsigned short>(-1))
#define ZETA_Core_sshrt_min (-ZETA_Core_sshrt_max)
#define ZETA_Core_sshrt_max (static_cast<signed short>(ZETA_Core_ushrt_max / 2))

#define ZETA_Core_uint_min (static_cast<unsigned int>(0))
#define ZETA_Core_uint_max (static_cast<unsigned int>(-1))
#define ZETA_Core_sint_min (-ZETA_Core_sint_max)
#define ZETA_Core_sint_max (static_cast<signed int>(ZETA_Core_uint_max / 2))

#define ZETA_Core_ulong_min (static_cast<unsigned long>(0))
#define ZETA_Core_ulong_max (static_cast<unsigned long>(-1))
#define ZETA_Core_slong_min (-ZETA_Core_slong_max)
#define ZETA_Core_slong_max (static_cast<signed long>(ZETA_Core_ulong_max / 2))

#define ZETA_Core_ullong_min (static_cast<unsigned long long>(0))
#define ZETA_Core_ullong_max (static_cast<unsigned long long>(-1))
#define ZETA_Core_sllong_min (-ZETA_Core_sllong_max)
#define ZETA_Core_sllong_max \
    (static_cast<signed long long>(ZETA_Core_ullong_max / 2))

#define ZETA_Core_ubit8_min (static_cast<ubit8_t>(0))
#define ZETA_Core_ubit8_max (static_cast<ubit8_t>(-1))
#define ZETA_Core_sbit8_min (-ZETA_Core_sbit8_max)
#define ZETA_Core_sbit8_max (static_cast<sbit8_t>(ZETA_Core_ubit8_max / 2))

#define ZETA_Core_ubit16_min (static_cast<ubit16_t>(0))
#define ZETA_Core_ubit16_max (static_cast<ubit16_t>(-1))
#define ZETA_Core_sbit16_min (-ZETA_Core_sbit16_max)
#define ZETA_Core_sbit16_max (static_cast<sbit16_t>(ZETA_Core_ubit16_max / 2))

#define ZETA_Core_ubit32_min (static_cast<ubit32_t>(0))
#define ZETA_Core_ubit32_max (static_cast<ubit32_t>(-1))
#define ZETA_Core_sbit32_min (-ZETA_Core_sbit32_max)
#define ZETA_Core_sbit32_max (static_cast<sbit32_t>(ZETA_Core_ubit32_max / 2))

#define ZETA_Core_ubit64_min (static_cast<ubit64_t>(0))
#define ZETA_Core_ubit64_max (static_cast<ubit64_t>(-1))
#define ZETA_Core_sbit64_min (-ZETA_Core_sbit64_max)
#define ZETA_Core_sbit64_max (static_cast<sbit64_t>(ZETA_Core_ubit64_max / 2))

#define ZETA_Core_ubit128_min (static_cast<ubit128_t>(0))
#define ZETA_Core_ubit128_max (static_cast<ubit128_t>(-1))
#define ZETA_Core_sbit128_min (-ZETA_Core_sbit128_max)
#define ZETA_Core_sbit128_max \
    (static_cast<sbit128_t>(ZETA_Core_ubit128_max / 2))

#define ZETA_Core_u8_min (static_cast<u8_t>(0))
#define ZETA_Core_u8_max (static_cast<u8_t>(-1))
#define ZETA_Core_s8_min (-ZETA_Core_s8_max)
#define ZETA_Core_s8_max (static_cast<s8_t>(ZETA_Core_u8_max / 2))

#define ZETA_Core_u16_min (static_cast<u16_t>(0))
#define ZETA_Core_u16_max (static_cast<u16_t>(-1))
#define ZETA_Core_s16_min (-ZETA_Core_s16_max)
#define ZETA_Core_s16_max (static_cast<s16_t>(ZETA_Core_u16_max / 2))

#define ZETA_Core_u32_min (static_cast<u32_t>(0))
#define ZETA_Core_u32_max (static_cast<u32_t>(-1))
#define ZETA_Core_s32_min (-ZETA_Core_s32_max)
#define ZETA_Core_s32_max (static_cast<s32_t>(ZETA_Core_u32_max / 2))

#define ZETA_Core_u64_min (static_cast<u64_t>(0))
#define ZETA_Core_u64_max (static_cast<u64_t>(-1))
#define ZETA_Core_s64_min (-ZETA_Core_s64_max)
#define ZETA_Core_s64_max (static_cast<s64_t>(ZETA_Core_u64_max / 2))

#define ZETA_Core_u128_min (static_cast<u128_t>(0))
#define ZETA_Core_u128_max (static_cast<u128_t>(-1))
#define ZETA_Core_s128_min (-ZETA_Core_s128_max)
#define ZETA_Core_s128_max (static_cast<s128_t>(ZETA_Core_u128_max / 2))

#define ZETA_Core_usize_min (static_cast<size_t>(0))
#define ZETA_Core_usize_max (static_cast<size_t>(-1))
#define ZETA_Core_ssize_min (-ZETA_Core_ssize_max)
#define ZETA_Core_ssize_max (static_cast<ssize_t>(ZETA_Core_usize_max / 2))

#define ZETA_Core_size_min ZETA_Core_usize_min
#define ZETA_Core_size_max ZETA_Core_usize_max

#define ZETA_Core_max_capacity (ZETA_Core_size_max / 4)

namespace zeta::core {

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

#if defined(__UINT8_TYPE__)
using u8_t = __UINT8_TYPE__;
#else
using u8_t = ubit8_t;
#endif

#if defined(__UINT16_TYPE__)
using u16_t = __UINT16_TYPE__;
#else
using u16_t = ubit16_t;
#endif

#if defined(__UINT32_TYPE__)
using u32_t = __UINT32_TYPE__;
#else
using u32_t = ubit32_t;
#endif

#if defined(__UINT64_TYPE__)
using u64_t = __UINT64_TYPE__;
#else
using u64_t = ubit64_t;
#endif

#if defined(__UINT128_TYPE__)
using u128_t = __UINT128_TYPE__;
#else
using u128_t = ubit128_t;
#endif

#if defined(__INT8_TYPE__)
using s8_t = __INT8_TYPE__;
#else
using s8_t = sbit8_t;
#endif

#if defined(__INT16_TYPE__)
using s16_t = __INT16_TYPE__;
#else
using s16_t = sbit16_t;
#endif

#if defined(__INT32_TYPE__)
using s32_t = __INT32_TYPE__;
#else
using s32_t = sbit32_t;
#endif

#if defined(__INT64_TYPE__)
using s64_t = __INT64_TYPE__;
#else
using s64_t = sbit64_t;
#endif

#if defined(__INT128_TYPE__)
using s128_t = __INT128_TYPE__;
#else
using s128_t = sbit128_t;
#endif

using sdllong_t = signed _BitInt(ZETA_Core_ullong_width * 2);
using udllong_t = unsigned _BitInt(ZETA_Core_ullong_width * 2);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename T>
constexpr bool IsUnsignedIntegral{
    (!ZETA_Core_SignedChar && IsSame<T, char>) ||  //
    IsSame<T, unsigned char> ||                    //
    IsSame<T, unsigned short> ||                   //
    IsSame<T, unsigned int> ||                     //
    IsSame<T, unsigned long> ||                    //
    IsSame<T, unsigned long long> ||               //
    IsSame<T, ubit8_t> ||                          //
    IsSame<T, ubit16_t> ||                         //
    IsSame<T, ubit32_t> ||                         //
    IsSame<T, ubit64_t> ||                         //
    IsSame<T, ubit128_t> ||                        //
    IsSame<T, u8_t> ||                             //
    IsSame<T, u16_t> ||                            //
    IsSame<T, u32_t> ||                            //
    IsSame<T, u64_t> ||                            //
    IsSame<T, u128_t> ||                           //
    IsSame<T, size_t> ||                           //
    IsSame<T, udllong_t>                           //
};

template <typename T>
constexpr bool IsSignedIntegral{
    (ZETA_Core_SignedChar && IsSame<T, char>) ||  //
    IsSame<T, signed char> ||                     //
    IsSame<T, signed short> ||                    //
    IsSame<T, signed int> ||                      //
    IsSame<T, signed long> ||                     //
    IsSame<T, signed long long> ||                //
    IsSame<T, sbit8_t> ||                         //
    IsSame<T, sbit16_t> ||                        //
    IsSame<T, sbit32_t> ||                        //
    IsSame<T, sbit64_t> ||                        //
    IsSame<T, sbit128_t> ||                       //
    IsSame<T, s8_t> ||                            //
    IsSame<T, s16_t> ||                           //
    IsSame<T, s32_t> ||                           //
    IsSame<T, s64_t> ||                           //
    IsSame<T, s128_t> ||                          //
    IsSame<T, sdllong_t>                          //
};

template <typename T>
constexpr bool IsIntegral{ IsSame<T, bool> || IsUnsignedIntegral<T> ||
                           IsSignedIntegral<T> };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace detail {

template <typename Integral>
struct WidthOf_;

#pragma push_macro("F_")

#define F_(type, val)                      \
    template <>                            \
    struct WidthOf_<type> {                \
        static constexpr int value{ val }; \
    };                                     \
                                           \
    ZETA_Core_StaticAssert(true)

F_(unsigned char, ZETA_Core_uchar_width);
F_(unsigned short, ZETA_Core_ushrt_width);
F_(unsigned int, ZETA_Core_uint_width);
F_(unsigned long, ZETA_Core_ulong_width);
F_(unsigned long long, ZETA_Core_ullong_width);
F_(ubit8_t, ZETA_Core_ubit8_width);
F_(ubit16_t, ZETA_Core_ubit16_width);
F_(ubit32_t, ZETA_Core_ubit32_width);
F_(ubit64_t, ZETA_Core_ubit64_width);
F_(ubit128_t, ZETA_Core_ubit128_width);

#pragma pop_macro("F_")

}  // namespace detail

template <typename Integral>
constexpr int WidthOf{ detail::WidthOf_<Integral>::value };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace detail {

template <typename Integral>
struct RangeMinOf_;

template <typename Integral>
struct RangeMaxOf_;

#pragma push_macro("F_")

#define F_(type, min_val, max_val)              \
    template <>                                 \
    struct RangeMinOf_<type> {                  \
        static constexpr type value{ min_val }; \
    };                                          \
                                                \
    template <>                                 \
    struct RangeMaxOf_<type> {                  \
        static constexpr type value{ max_val }; \
    };                                          \
                                                \
    ZETA_Core_StaticAssert(true)

F_(char, ZETA_Core_char_min, ZETA_Core_char_max);
F_(unsigned char, ZETA_Core_uchar_min, ZETA_Core_uchar_max);
F_(signed char, ZETA_Core_schar_min, ZETA_Core_schar_max);

F_(unsigned short, ZETA_Core_ushrt_min, ZETA_Core_ushrt_max);
F_(signed short, ZETA_Core_sshrt_min, ZETA_Core_sshrt_max);

F_(unsigned, ZETA_Core_uint_min, ZETA_Core_uint_max);
F_(signed, ZETA_Core_sint_min, ZETA_Core_sint_max);

F_(unsigned long, ZETA_Core_ulong_min, ZETA_Core_ulong_max);
F_(signed long, ZETA_Core_slong_min, ZETA_Core_slong_max);

F_(unsigned long long, ZETA_Core_ullong_min, ZETA_Core_ullong_max);
F_(signed long long, ZETA_Core_sllong_min, ZETA_Core_sllong_max);

F_(ubit8_t, ZETA_Core_ubit8_min, ZETA_Core_ubit8_max);
F_(sbit8_t, ZETA_Core_sbit8_min, ZETA_Core_sbit8_max);

F_(ubit16_t, ZETA_Core_ubit16_min, ZETA_Core_ubit16_max);
F_(sbit16_t, ZETA_Core_sbit16_min, ZETA_Core_sbit16_max);

F_(ubit32_t, ZETA_Core_ubit32_min, ZETA_Core_ubit32_max);
F_(sbit32_t, ZETA_Core_sbit32_min, ZETA_Core_sbit32_max);

F_(ubit64_t, ZETA_Core_ubit64_min, ZETA_Core_ubit64_max);
F_(sbit64_t, ZETA_Core_sbit64_min, ZETA_Core_sbit64_max);

F_(ubit128_t, ZETA_Core_ubit128_min, ZETA_Core_ubit128_max);
F_(sbit128_t, ZETA_Core_sbit128_min, ZETA_Core_sbit128_max);

#pragma pop_macro("F_")

}  // namespace detail

template <typename Integral>
constexpr Integral RangeMinOf{ detail::RangeMinOf_<Integral>::value };

template <typename Integral>
constexpr Integral RangeMaxOf{ detail::RangeMaxOf_<Integral>::value };

}  // namespace zeta::core
