#pragma once

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------

#define ZETA_Core_ForEach_GetFirst(x, ...) x
#define ZETA_Core_ForEach_GetRes(x, ...) __VA_OPT__(, __VA_ARGS__)

#define ZETA_Core_ForEach_0(func, x, ...)                            \
    func, ZETA_Core_ForEach_GetFirst(__VA_OPT__(                     \
              func(x, ZETA_Core_ForEach_GetFirst(__VA_ARGS__)), ) x) \
              __VA_OPT__(ZETA_Core_ForEach_GetRes(__VA_ARGS__))
#define ZETA_Core_ForEach_1(func, ...)         \
    ZETA_Core_ForEach_A_1(ZETA_Core_ForEach_0, \
                          ZETA_Core_ForEach_0(func, __VA_ARGS__))
#define ZETA_Core_ForEach_2(func, ...)         \
    ZETA_Core_ForEach_A_2(ZETA_Core_ForEach_1, \
                          ZETA_Core_ForEach_1(func, __VA_ARGS__))
#define ZETA_Core_ForEach_3(func, ...)         \
    ZETA_Core_ForEach_A_3(ZETA_Core_ForEach_2, \
                          ZETA_Core_ForEach_2(func, __VA_ARGS__))
#define ZETA_Core_ForEach_4(func, ...)         \
    ZETA_Core_ForEach_A_4(ZETA_Core_ForEach_3, \
                          ZETA_Core_ForEach_3(func, __VA_ARGS__))
#define ZETA_Core_ForEach_5(func, ...)         \
    ZETA_Core_ForEach_A_5(ZETA_Core_ForEach_4, \
                          ZETA_Core_ForEach_4(func, __VA_ARGS__))
#define ZETA_Core_ForEach_6(func, ...)         \
    ZETA_Core_ForEach_A_6(ZETA_Core_ForEach_5, \
                          ZETA_Core_ForEach_5(func, __VA_ARGS__))
#define ZETA_Core_ForEach_7(func, ...)         \
    ZETA_Core_ForEach_A_7(ZETA_Core_ForEach_6, \
                          ZETA_Core_ForEach_6(func, __VA_ARGS__))
#define ZETA_Core_ForEach_8(func, ...)         \
    ZETA_Core_ForEach_A_8(ZETA_Core_ForEach_7, \
                          ZETA_Core_ForEach_7(func, __VA_ARGS__))

#define ZETA_Core_ForEach_A_1(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_2(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_3(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_4(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_5(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_6(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_7(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A_8(func, ...) func(__VA_ARGS__)
#define ZETA_Core_ForEach_A(func, ...) func(__VA_ARGS__)

#define ZETA_Core_DetectError()

#define ZETA_Core_GetResult(func, x, ...) x ZETA_Core_DetectError(__VA_ARGS__)

#define ZETA_Core_ForEach(func, ...)         \
    ZETA_Core_ForEach_A(ZETA_Core_GetResult, \
                        ZETA_Core_ForEach_8(func, __VA_ARGS__))

// -----------------------------------------------------------------------------

#if defined(__cplusplus)
#define bool_t bool
#else
#define bool_t _Bool
#endif

#define TRUE ((bool_t)1)
#define FALSE ((bool_t)0)

#define ZETA_Core_StaticAssert(cond) _Static_assert(cond, "")

#if !defined(ZETA_Core_EnableDebug)
#define ZETA_Core_EnableDebug 0
#endif

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

typedef unsigned char byte_t;

typedef unsigned _BitInt(8) ubit8_t;
typedef signed _BitInt(8) sbit8_t;

typedef unsigned _BitInt(16) ubit16_t;
typedef signed _BitInt(16) sbit16_t;

typedef unsigned _BitInt(32) ubit32_t;
typedef signed _BitInt(32) sbit32_t;

typedef unsigned _BitInt(64) ubit64_t;
typedef signed _BitInt(64) sbit64_t;

typedef unsigned _BitInt(128) ubit128_t;
typedef signed _BitInt(128) sbit128_t;

#if defined(__UINT8_TYPE__)
typedef __UINT8_TYPE__ u8_t;
#else
typedef ubit8_t u8_t;
#endif

#if defined(__UINT16_TYPE__)
typedef __UINT16_TYPE__ u16_t;
#else
typedef ubit16_t u16_t;
#endif

#if defined(__UINT32_TYPE__)
typedef __UINT32_TYPE__ u32_t;
#else
typedef ubit32_t u32_t;
#endif

#if defined(__UINT64_TYPE__)
typedef __UINT64_TYPE__ u64_t;
#else
typedef ubit64_t u64_t;
#endif

#if defined(__UINT128_TYPE__)
typedef __UINT128_TYPE__ u128_t;
#else
typedef ubit128_t u128_t;
#endif

#if defined(__INT8_TYPE__)
typedef __INT8_TYPE__ s8_t;
#else
typedef sbit8_t s8_t;
#endif

#if defined(__INT16_TYPE__)
typedef __INT16_TYPE__ s16_t;
#else
typedef sbit16_t s16_t;
#endif

#if defined(__INT32_TYPE__)
typedef __INT32_TYPE__ s32_t;
#else
typedef sbit32_t s32_t;
#endif

#if defined(__INT64_TYPE__)
typedef __INT64_TYPE__ s64_t;
#else
typedef sbit64_t s64_t;
#endif

#if defined(__INT128_TYPE__)
typedef __INT128_TYPE__ s128_t;
#else
typedef sbit128_t s128_t;
#endif

#if ZETA_Core_ullong_width == 32

typedef s64_t sdllong_t;
typedef u64_t udllong_t;

#elif ZETA_Core_ullong_width == 64

typedef s128_t sdllong_t;
typedef u128_t udllong_t;

#else

#error "Unsupported architecture."

#endif

typedef unsigned unichar_t;

#define ZETA_Core_Unused(x)     \
    if (FALSE) { ((void)(x)); } \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_ToStr_(x) #x
#define ZETA_Core_ToStr(x) ZETA_Core_ToStr_(x)

#define ZETA_Core_Concat2_(x, y) x##y
#define ZETA_Core_Concat2(x, y) ZETA_Core_Concat2_(x, y)

#define ZETA_Core_Concat(x, ...) \
    ZETA_Core_ForEach(ZETA_Core_Concat2, x, __VA_ARGS__)

#define ZETA_Core_UniqueName(prefix) ZETA_Core_Concat2(prefix, __COUNTER__)

#define ZETA_Core_TmpName ZETA_Core_UniqueName(ZETA_tmp_)

#define ZETA_Core_DeclareStruct(struct_name) \
    typedef struct struct_name struct_name;  \
    struct struct_name;                      \
    ZETA_Core_StaticAssert(TRUE)

#if defined(__cplusplus)
#define ZETA_Core_AutoVar(var, expression) \
    auto var{ (expression) };              \
    ZETA_Core_StaticAssert(TRUE)
#else
#define ZETA_Core_AutoVar(var, expression) \
    __auto_type var = (expression);        \
    ZETA_Core_StaticAssert(TRUE)
#endif

#if defined(__cplusplus)
#define ZETA_Core_ExternC_Beg \
    extern "C" {              \
    ZETA_Core_StaticAssert(TRUE)
#define ZETA_Core_ExternC_End \
    }                         \
    ZETA_Core_StaticAssert(TRUE)
#else
#define ZETA_Core_ExternC_Beg ZETA_Core_StaticAssert(TRUE)
#define ZETA_Core_ExternC_End ZETA_Core_StaticAssert(TRUE)
#endif

#if defined(__cplusplus)
#define ZETA_Core_ToVoidPtr(ptr) (const_cast<void*>((void const*)(ptr)))
#else
#define ZETA_Core_ToVoidPtr(ptr) ((void*)(void const*)(ptr))
#endif

#define ZETA_Core_IsSigned(type) ((type)(-1) < 0)

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

#define ZETA_Core_WidthOf(type)                     \
    _Generic((type)0,                               \
        unsigned char: ZETA_Core_uchar_width,       \
        unsigned short: ZETA_Core_ushrt_width,      \
        unsigned: ZETA_Core_uint_width,             \
        unsigned long: ZETA_Core_ulong_width,       \
        unsigned long long: ZETA_Core_ullong_width, \
        ubit8_t: ZETA_Core_ubit8_width,             \
        ubit16_t: ZETA_Core_ubit16_width,           \
        ubit32_t: ZETA_Core_ubit32_width,           \
        ubit64_t: ZETA_Core_ubit64_width,           \
        ubit128_t: ZETA_Core_ubit128_width)

#define ZETA_Core_RangeMinOf(type)                \
    _Generic((type)0,                             \
        char: ZETA_Core_char_min,                 \
        unsigned char: ZETA_Core_uchar_min,       \
        signed char: ZETA_Core_schar_min,         \
                                                  \
        unsigned short: ZETA_Core_ushrt_min,      \
        signed short: ZETA_Core_sshrt_min,        \
                                                  \
        unsigned: ZETA_Core_uint_min,             \
        signed: ZETA_Core_sint_min,               \
                                                  \
        unsigned long: ZETA_Core_ulong_min,       \
        signed long: ZETA_Core_slong_min,         \
                                                  \
        unsigned long long: ZETA_Core_ullong_min, \
        signed long long: ZETA_Core_sllong_min,   \
                                                  \
        ubit8_t: ZETA_Core_ubit8_min,             \
        sbit8_t: ZETA_Core_sbit8_min,             \
                                                  \
        ubit16_t: ZETA_Core_ubit16_min,           \
        sbit16_t: ZETA_Core_sbit16_min,           \
                                                  \
        ubit32_t: ZETA_Core_ubit32_min,           \
        sbit32_t: ZETA_Core_sbit32_min,           \
                                                  \
        ubit64_t: ZETA_Core_ubit64_min,           \
        sbit64_t: ZETA_Core_sbit64_min,           \
                                                  \
        ubit128_t: ZETA_Core_ubit128_min,         \
        sbit128_t: ZETA_Core_sbit128_min)

#define ZETA_Core_RangeMaxOf(type)                \
    _Generic((type)0,                             \
        char: ZETA_Core_char_max,                 \
        unsigned char: ZETA_Core_uchar_max,       \
        signed char: ZETA_Core_schar_max,         \
                                                  \
        unsigned short: ZETA_Core_ushrt_max,      \
        signed short: ZETA_Core_sshrt_max,        \
                                                  \
        unsigned: ZETA_Core_uint_max,             \
        signed: ZETA_Core_sint_max,               \
                                                  \
        unsigned long: ZETA_Core_ulong_max,       \
        signed long: ZETA_Core_slong_max,         \
                                                  \
        unsigned long long: ZETA_Core_ullong_max, \
        signed long long: ZETA_Core_sllong_max,   \
                                                  \
        ubit8_t: ZETA_Core_ubit8_max,             \
        sbit8_t: ZETA_Core_sbit8_max,             \
                                                  \
        ubit16_t: ZETA_Core_ubit16_max,           \
        sbit16_t: ZETA_Core_sbit16_max,           \
                                                  \
        ubit32_t: ZETA_Core_ubit32_max,           \
        sbit32_t: ZETA_Core_sbit32_max,           \
                                                  \
        ubit64_t: ZETA_Core_ubit64_max,           \
        sbit64_t: ZETA_Core_sbit64_max,           \
                                                  \
        ubit128_t: ZETA_Core_ubit128_max,         \
        sbit128_t: ZETA_Core_sbit128_max)

#define ZETA_Core_InRangeOf_(tmp_val, val, type)           \
    ({                                                     \
        ZETA_Core_AutoVar(tmp_val, val);                   \
        ZETA_Core_RangeMinOf(type) <= tmp_val&& tmp_val <= \
            ZETA_Core_RangeMaxOf(type);                    \
    })

#define ZETA_Core_InRangeOf(val, type) \
    ZETA_Core_InRangeOf_(ZETA_Core_TmpName, (val), type)

#define ZETA_Core_MaxModOf(type) (ZETA_Core_RangeMaxOf(type) / 4)

#define ZETA_Core_max_capacity (ZETA_Core_size_max / 4)

ZETA_Core_StaticAssert(ZETA_Core_RangeMinOf(byte_t) <= 0);

ZETA_Core_StaticAssert(255 <= ZETA_Core_RangeMaxOf(byte_t));

#define ZETA_Core_ImmPrint (TRUE)

#define ZETA_Core_PrintPos(filename, line, func)                         \
    printf("%48s:%-4llu\t%-24s", (filename), (unsigned long long)(line), \
           (func));                                                      \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_PrintCurPos                         \
    ZETA_Core_PrintPos(__FILE__, __LINE__, __func__); \
    printf("\n");                                     \
    if (ZETA_Core_ImmPrint) { fflush(stdout); }       \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_PrintVar(var)                           \
    {                                                     \
        ZETA_Core_PrintPos(__FILE__, __LINE__, __func__); \
                                                          \
        printf("\t%24s = ", ZETA_Core_ToStr(var));        \
                                                          \
        printf(_Generic((var),                            \
                   bool_t: "%c\n",                        \
                                                          \
                   char: "%c\n",                          \
                   unsigned char: "%X\n",                 \
                   signed char: "%c\n",                   \
                                                          \
                   short: "%i\n",                         \
                   unsigned short: "%u\n",                \
                                                          \
                   int: "%i\n",                           \
                   unsigned: "%u\n",                      \
                                                          \
                   long: "%li\n",                         \
                   unsigned long: "%lu\n",                \
                                                          \
                   long long: "%lli\n",                   \
                   unsigned long long: "%llu\n",          \
                                                          \
                   float: "%g\n",                         \
                                                          \
                   double: "%g\n",                        \
                   long double: "%g\n",                   \
                                                          \
                   void*: "%p\n",                         \
                   void const*: "%p\n",                   \
                                                          \
                   char*: "%s\n",                         \
                   char const*: "%s\n"),                  \
               (var));                                    \
                                                          \
        if (ZETA_Core_ImmPrint) { fflush(stdout); }       \
    }                                                     \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_PtrToAddr(x) ((uintptr_t)(void const*)(x))
#define ZETA_Core_AddrToPtr(x) ((void*)(uintptr_t)(x))

#define ZETA_Core_MemberToStruct(struct_type, member_name, member_ptr)         \
    ((struct_type*)(void*)((unsigned char*)ZETA_Core_ToVoidPtr((member_ptr)) - \
                           offsetof(struct_type, member_name)))

#define ZETA_Core_CallMemberFunc__(tmp_obj, member_func, ...) \
    ZETA_Core_DebugAssert(tmp_obj != NULL);                   \
    ZETA_Core_DebugAssert(tmp_obj->member_func != NULL);      \
    tmp_obj->member_func(__VA_ARGS__);

#define ZETA_Core_CallMemberFunc_(tmp_obj, obj, member_func, ...)          \
    ({                                                                     \
        ZETA_Core_AutoVar(tmp_obj, (obj));                                 \
        ZETA_Core_CallMemberFunc__(tmp_obj, member_func, tmp_obj->context, \
                                   __VA_ARGS__)                            \
    })

#define ZETA_Core_CallMemberFunc(obj, member_func, ...) \
    ZETA_Core_CallMemberFunc_(ZETA_Core_TmpName, obj, member_func, __VA_ARGS__)

#define ZETA_Core_CallConstMemberFunc_(                                  \
    tmp_obj, tmp_context, tmp_const_context, obj, member_func, ...)      \
    ({                                                                   \
        ZETA_Core_AutoVar(tmp_obj, (obj));                               \
        void* tmp_context = tmp_obj->context;                            \
        void const* tmp_const_context = tmp_obj->const_context;          \
        ZETA_Core_CallMemberFunc__(                                      \
            tmp_obj, member_func,                                        \
            tmp_const_context == NULL ? tmp_context : tmp_const_context, \
            __VA_ARGS__)                                                 \
    })

#define ZETA_Core_CallConstMemberFunc(obj, member_func, ...)             \
    ZETA_Core_CallConstMemberFunc_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                   ZETA_Core_TmpName, obj, member_func,  \
                                   __VA_ARGS__)

#define ZETA_Core_FuncPtrTable_Call_(tmp_func_ptr_table, func_ptr_table, \
                                     func_name, ...)                     \
    ({                                                                   \
        ZETA_Core_AutoVar(tmp_func_ptr_table, (func_ptr_table));         \
        ZETA_Core_DebugAssert(tmp_func_ptr_table != NULL);               \
                                                                         \
        ZETA_Core_DebugAssert(tmp_func_ptr_table->func_name != NULL);    \
        tmp_func_ptr_table->func_name(__VA_ARGS__);                      \
    })

#define ZETA_Core_FuncPtrTable_Call(func_ptr_table, func_name, ...)   \
    ZETA_Core_FuncPtrTable_Call_(ZETA_Core_TmpName, (func_ptr_table), \
                                 func_name, __VA_ARGS__)

#define ZETA_Core_LittleEndian (0)
#define ZETA_Core_BigEndian (1)

#define ZETA_Core_Endian_L ZETA_Core_LittleEndian
#define ZETA_Core_Endian_B ZETA_Core_BigEndian

#if ZETA_Core_EnableDebug

#define ZETA_Core_DebugStructPadding \
    unsigned char ZETA_Core_TmpName[alignof(max_align_t) + 1]

#else

#define ZETA_Core_DebugStructPadding

#endif

typedef int (*Zeta_Core_Compare)(void const* context, void const* x,
                                 void const* y);

typedef unsigned long long (*Zeta_Core_Hash)(void const* context, void const* x,
                                             unsigned long long salt);

#define ZETA_SectStartOf(sect) (ZETA_Core_Concat(__start_, sect))

#define ZETA_SectStopOf(sect) (ZETA_Core_Concat(__stop_, sect))

#if defined(__i386__) || defined(__x86_64__)

#define ZETA_Core_CPU_RELAX() __asm__("pause")

#elif defined(__arm__) || defined(__aarch64__)

#define ZETA_Core_CPU_RELAX() __asm__("yield")

#else

#define ZETA_Core_CPU_RELAX()

#endif
