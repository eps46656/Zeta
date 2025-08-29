#pragma once

#include <iomanip>
#include <iostream>

#define ZETA_Core_Unused(x) (void)(x)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define ZETA_Core_ToStr_(x) #x
#define ZETA_Core_ToStr(x) ZETA_Core_ToStr_(x)

#define ZETA_Core_Concat2_(x, y) x##y
#define ZETA_Core_Concat2(x, y) ZETA_Core_Concat2_(x, y)

#define ZETA_Core_Concat(x, ...) \
    ZETA_Core_ForEach(ZETA_Core_Concat2, x, __VA_ARGS__)

#define ZETA_Core_UniqueName(prefix) ZETA_Core_Concat2(prefix, __COUNTER__)

#define ZETA_Core_TmpName ZETA_Core_UniqueName(ZETA_tmp_)

#if defined(__cplusplus)
#define ZETA_Core_StaticAssert(cond) static_assert(cond, "")
#else
#define ZETA_Core_StaticAssert(cond) _Static_assert(cond, "")
#endif

#if defined(__cplusplus)

#define ZETA_Core_AutoVar(var, expression) \
    auto var{ (expression) };              \
    ZETA_Core_StaticAssert(true)

#else

#define ZETA_Core_AutoVar(var, expression) \
    __auto_type var = (expression);        \
    ZETA_Core_StaticAssert(true)

#endif

#define ZETA_Core_ImmPrint 1

#define ZETA_Core_PrintPos(filename, line, func)                            \
    std::cout << std::setw(48) << std::right << filename << ":"             \
              << std::setw(4) << std::left << line << "\t" << std::setw(24) \
              << std::left << func;                                         \
                                                                            \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_PrintCurPos                         \
    ZETA_Core_PrintPos(__FILE__, __LINE__, __func__); \
    std::cout << '\n';                                \
    if (ZETA_Core_ImmPrint) { std::cout.flush(); }    \
    ZETA_Core_StaticAssert(true)

#if __has_feature(address_sanitizer)

#define ZETA_Core_PrintStackTrace __sanitizer_print_stack_trace()

#else

#define ZETA_Core_PrintStackTrace ZETA_Core_StaticAssert(true)

#endif

#define ZETA_Core_Assert(cond)                         \
    if (cond) {                                        \
    } else {                                           \
        ZETA_Core_PrintVar("Assert !!!");              \
        ZETA_Core_PrintStackTrace;                     \
        if (ZETA_Core_ImmPrint) { std::cout.flush(); } \
        exit(1);                                       \
    }

#define ZETA_Core_PrintVar(var)                                              \
    {                                                                        \
        ZETA_Core_PrintPos(__FILE__, __LINE__, __func__);                    \
                                                                             \
        std::cout << std::setw(24) << ZETA_Core_ToStr(var) << " = " << (var) \
                  << '\n';                                                   \
                                                                             \
        if (ZETA_Core_ImmPrint) { std::cout.flush(); }                       \
    }                                                                        \
    ZETA_Core_StaticAssert(true)

#if ZETA_Core_EnableDebug

#define ZETA_Core_DebugAssert(cond) ZETA_Core_Assert(cond)

#else

#define ZETA_Core_DebugAssert(cond) ZETA_Core_Unused(cond)

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define ZETA_Core_max_capacity (ZETA_Core_size_max / 4)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#if defined(__cplusplus)

#define ZETA_Core_ToVoidPtr(ptr) \
    (const_cast<void*>(static_cast<void const*>(ptr)))

#else

#define ZETA_Core_ToVoidPtr(ptr) ((void*)(void const*)(ptr))

#endif

#define ZETA_Core_ToCharPtr(ptr) \
    (static_cast<char*>(ZETA_Core_ToVoidPtr((ptr))))

#define ZETA_Core_PtrToAddr(x) ((uintptr_t)(void const*)(x))
#define ZETA_Core_AddrToPtr(x) ((void*)(uintptr_t)(x))

#define ZETA_Core_MemberToStruct(struct_type, member_name, member_ptr)         \
    ((struct_type*)(void*)((unsigned char*)ZETA_Core_ToVoidPtr((member_ptr)) - \
                           offsetof(struct_type, member_name)))
