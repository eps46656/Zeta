#pragma once

#include <zeta/core/define.h>

#define ZETA_Core_IsSigned(type) (((type) - 1) < 0)

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

#define ZETA_Core_uchar_min ((unsigned char)0)
#define ZETA_Core_uchar_max ((unsigned char)(-1))
#define ZETA_Core_schar_min (-ZETA_Core_schar_max)
#define ZETA_Core_schar_max ((signed char)(ZETA_Core_uchar_max / 2))

#define ZETA_Core_char_min \
    (ZETA_Core_IsSigned(char) ? ZETA_Core_schar_min : ZETA_Core_uchar_min)
#define ZETA_Core_char_max \
    (ZETA_Core_IsSigned(char) ? ZETA_Core_schar_max : ZETA_Core_uchar_max)

#define ZETA_Core_ushrt_min ((unsigned short)(0))
#define ZETA_Core_ushrt_max ((unsigned short)(-1))
#define ZETA_Core_sshrt_min (-ZETA_Core_sshrt_max)
#define ZETA_Core_sshrt_max ((signed short)(ZETA_Core_ushrt_max / 2))

#define ZETA_Core_uint_min ((unsigned int)(0))
#define ZETA_Core_uint_max ((unsigned int)(-1))
#define ZETA_Core_sint_min (-ZETA_Core_sint_max)
#define ZETA_Core_sint_max ((signed int)(ZETA_Core_uint_max / 2))

#define ZETA_Core_ulong_min ((unsigned long)(0))
#define ZETA_Core_ulong_max ((unsigned long)(-1))
#define ZETA_Core_slong_min (-ZETA_Core_slong_max)
#define ZETA_Core_slong_max ((signed long)(ZETA_Core_ulong_max / 2))

#define ZETA_Core_ullong_min ((unsigned long long)(0))
#define ZETA_Core_ullong_max ((unsigned long long)(-1))
#define ZETA_Core_sllong_min (-ZETA_Core_sllong_max)
#define ZETA_Core_sllong_max ((signed long long)(ZETA_Core_ullong_max / 2))

#define ZETA_Core_ubit8_min ((ubit8_t)(0))
#define ZETA_Core_ubit8_max ((ubit8_t)(-1))
#define ZETA_Core_sbit8_min (-ZETA_Core_sbit8_max)
#define ZETA_Core_sbit8_max ((sbit8_t)(ZETA_Core_ubit8_max / 2))

#define ZETA_Core_ubit16_min ((ubit16_t)(0))
#define ZETA_Core_ubit16_max ((ubit16_t)(-1))
#define ZETA_Core_sbit16_min (-ZETA_Core_sbit16_max)
#define ZETA_Core_sbit16_max ((sbit16_t)(ZETA_Core_ubit16_max / 2))

#define ZETA_Core_ubit32_min ((ubit32_t)(0))
#define ZETA_Core_ubit32_max ((ubit32_t)(-1))
#define ZETA_Core_sbit32_min (-ZETA_Core_sbit32_max)
#define ZETA_Core_sbit32_max ((sbit32_t)(ZETA_Core_ubit32_max / 2))

#define ZETA_Core_ubit64_min ((ubit64_t)(0))
#define ZETA_Core_ubit64_max ((ubit64_t)(-1))
#define ZETA_Core_sbit64_min (-ZETA_Core_sbit64_max)
#define ZETA_Core_sbit64_max ((sbit64_t)(ZETA_Core_ubit64_max / 2))

#define ZETA_Core_ubit128_min ((ubit128_t)(0))
#define ZETA_Core_ubit128_max ((ubit128_t)(-1))
#define ZETA_Core_sbit128_min (-ZETA_Core_sbit128_max)
#define ZETA_Core_sbit128_max ((sbit128_t)(ZETA_Core_ubit128_max / 2))

#define ZETA_Core_u8_min ((u8_t)(0))
#define ZETA_Core_u8_max ((u8_t)(-1))
#define ZETA_Core_s8_min (-ZETA_Core_s8_max)
#define ZETA_Core_s8_max ((s8_t)(ZETA_Core_u8_max / 2))

#define ZETA_Core_u16_min ((u16_t)(0))
#define ZETA_Core_u16_max ((u16_t)(-1))
#define ZETA_Core_s16_min (-ZETA_Core_s16_max)
#define ZETA_Core_s16_max ((s16_t)(ZETA_Core_u16_max / 2))

#define ZETA_Core_u32_min ((u32_t)(0))
#define ZETA_Core_u32_max ((u32_t)(-1))
#define ZETA_Core_s32_min (-ZETA_Core_s32_max)
#define ZETA_Core_s32_max ((s32_t)(ZETA_Core_u32_max / 2))

#define ZETA_Core_u64_min ((u64_t)(0))
#define ZETA_Core_u64_max ((u64_t)(-1))
#define ZETA_Core_s64_min (-ZETA_Core_s64_max)
#define ZETA_Core_s64_max ((s64_t)(ZETA_Core_u64_max / 2))

#define ZETA_Core_u128_min ((u128_t)(0))
#define ZETA_Core_u128_max ((u128_t)(-1))
#define ZETA_Core_s128_min (-ZETA_Core_s128_max)
#define ZETA_Core_s128_max ((s128_t)(ZETA_Core_u128_max / 2))

#define ZETA_Core_usize_min ((size_t)(0))
#define ZETA_Core_usize_max ((size_t)(-1))
#define ZETA_Core_ssize_min (-ZETA_Core_ssize_max)
#define ZETA_Core_ssize_max ((ssize_t)(ZETA_Core_usize_max / 2))

#define ZETA_Core_size_min ZETA_Core_usize_min
#define ZETA_Core_size_max ZETA_Core_usize_max

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
constexpr bool is_unsigned_integer{ is_integer<T> && 0 <= static_cast<T>(-1) };

template <typename T>
constexpr bool is_signed_integer{ is_integer<T> && static_cast<T>(-1) < 0 };

template <typename T>
constexpr bool is_integer{
    is_same<T, char> ||                                                //
    is_same<T, unsigned char> || is_same<T, signed char> ||            //
    is_same<T, unsigned short> || is_same<T, signed short> ||          //
    is_same<T, unsigned int> || is_same<T, signed int> ||              //
    is_same<T, unsigned long> || is_same<T, signed long> ||            //
    is_same<T, unsigned long long> || is_same<T, signed long long> ||  //
    is_same<T, ubit8_t> || is_same<T, sbit8_t> ||                      //
    is_same<T, ubit16_t> || is_same<T, sbit16_t> ||                    //
    is_same<T, ubit32_t> || is_same<T, sbit32_t> ||                    //
    is_same<T, ubit64_t> || is_same<T, sbit64_t> ||                    //
    is_same<T, ubit128_t> || is_same<T, sbit128_t> ||                  //
    is_same<T, u8_t> || is_same<T, s8_t> ||                            //
    is_same<T, u16_t> || is_same<T, s16_t> ||                          //
    is_same<T, u32_t> || is_same<T, s32_t> ||                          //
    is_same<T, u64_t> || is_same<T, s64_t> ||                          //
    is_same<T, u128_t> || is_same<T, s128_t> ||                        //
    is_same<T, size_t> || is_same<T, ssize_t> ||                       //
    is_same<T, sdllong_t> || is_same<T, udllong_t>                     //
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Integer>
constexpr int width_of_();

#define ZETA_Core_width_of_(type, val) \
    template <>                        \
    constexpr int width_of_<type>() {  \
        return val;                    \
    }                                  \
    ZETA_Core_StaticAssert(true)

ZETA_Core_width_of_(unsigned char, ZETA_Core_uchar_width);
ZETA_Core_width_of_(unsigned short, ZETA_Core_ushrt_width);
ZETA_Core_width_of_(unsigned int, ZETA_Core_uint_width);
ZETA_Core_width_of_(unsigned long, ZETA_Core_ulong_width);
ZETA_Core_width_of_(unsigned long long, ZETA_Core_ullong_width);
ZETA_Core_width_of_(ubit8_t, ZETA_Core_ubit8_width);
ZETA_Core_width_of_(ubit16_t, ZETA_Core_ubit16_width);
ZETA_Core_width_of_(ubit32_t, ZETA_Core_ubit32_width);
ZETA_Core_width_of_(ubit64_t, ZETA_Core_ubit64_width);
ZETA_Core_width_of_(ubit128_t, ZETA_Core_ubit128_width);

template <typename Integer>
constexpr int width_of{ width_of_<Integer>() };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Integer>
constexpr Integer range_min_of_();

#define ZETA_Core_range_min_of_(type, val) \
    template <>                            \
    constexpr type range_min_of_<type>() { \
        return val;                        \
    }                                      \
    ZETA_Core_StaticAssert(true)

ZETA_Core_range_min_of_(char, ZETA_Core_char_min);
ZETA_Core_range_min_of_(unsigned char, ZETA_Core_uchar_min);
ZETA_Core_range_min_of_(signed char, ZETA_Core_schar_min);

ZETA_Core_range_min_of_(unsigned short, ZETA_Core_ushrt_min);
ZETA_Core_range_min_of_(signed short, ZETA_Core_sshrt_min);

ZETA_Core_range_min_of_(unsigned, ZETA_Core_uint_min);
ZETA_Core_range_min_of_(signed, ZETA_Core_sint_min);

ZETA_Core_range_min_of_(unsigned long, ZETA_Core_ulong_min);
ZETA_Core_range_min_of_(signed long, ZETA_Core_slong_min);

ZETA_Core_range_min_of_(unsigned long long, ZETA_Core_ullong_min);
ZETA_Core_range_min_of_(signed long long, ZETA_Core_sllong_min);

ZETA_Core_range_min_of_(ubit8_t, ZETA_Core_ubit8_min);
ZETA_Core_range_min_of_(sbit8_t, ZETA_Core_sbit8_min);

ZETA_Core_range_min_of_(ubit16_t, ZETA_Core_ubit16_min);
ZETA_Core_range_min_of_(sbit16_t, ZETA_Core_sbit16_min);

ZETA_Core_range_min_of_(ubit32_t, ZETA_Core_ubit32_min);
ZETA_Core_range_min_of_(sbit32_t, ZETA_Core_sbit32_min);

ZETA_Core_range_min_of_(ubit64_t, ZETA_Core_ubit64_min);
ZETA_Core_range_min_of_(sbit64_t, ZETA_Core_sbit64_min);

ZETA_Core_range_min_of_(ubit128_t, ZETA_Core_ubit128_min);
ZETA_Core_range_min_of_(sbit128_t, ZETA_Core_sbit128_min);

template <typename Integer>
constexpr Integer range_min_of{ range_min_of_<Integer>() };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Integer>
constexpr Integer range_max_of_();

#define ZETA_Core_range_max_of_(type, val) \
    template <>                            \
    constexpr type range_max_of_<type>() { \
        return val;                        \
    }                                      \
    ZETA_Core_StaticAssert(true)

ZETA_Core_range_max_of_(char, ZETA_Core_char_max);
ZETA_Core_range_max_of_(unsigned char, ZETA_Core_uchar_max);
ZETA_Core_range_max_of_(signed char, ZETA_Core_schar_max);

ZETA_Core_range_max_of_(unsigned short, ZETA_Core_ushrt_max);
ZETA_Core_range_max_of_(signed short, ZETA_Core_sshrt_max);

ZETA_Core_range_max_of_(unsigned, ZETA_Core_uint_max);
ZETA_Core_range_max_of_(signed, ZETA_Core_sint_max);

ZETA_Core_range_max_of_(unsigned long, ZETA_Core_ulong_max);
ZETA_Core_range_max_of_(signed long, ZETA_Core_slong_max);

ZETA_Core_range_max_of_(unsigned long long, ZETA_Core_ullong_max);
ZETA_Core_range_max_of_(signed long long, ZETA_Core_sllong_max);

ZETA_Core_range_max_of_(ubit8_t, ZETA_Core_ubit8_max);
ZETA_Core_range_max_of_(sbit8_t, ZETA_Core_sbit8_max);

ZETA_Core_range_max_of_(ubit16_t, ZETA_Core_ubit16_max);
ZETA_Core_range_max_of_(sbit16_t, ZETA_Core_sbit16_max);

ZETA_Core_range_max_of_(ubit32_t, ZETA_Core_ubit32_max);
ZETA_Core_range_max_of_(sbit32_t, ZETA_Core_sbit32_max);

ZETA_Core_range_max_of_(ubit64_t, ZETA_Core_ubit64_max);
ZETA_Core_range_max_of_(sbit64_t, ZETA_Core_sbit64_max);

ZETA_Core_range_max_of_(ubit128_t, ZETA_Core_ubit128_max);
ZETA_Core_range_max_of_(sbit128_t, ZETA_Core_sbit128_max);

template <typename Integer>
constexpr Integer range_max_of{ range_max_of_<Integer>() };

}  // namespace zeta::core
