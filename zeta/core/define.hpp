#pragma once

#define ZETA_Core_Unused(x) (void)(x)

#define ZETA_Core_Identity(...) __VA_ARGS__

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

#define ZETA_Core_ToStr_(x) #x
#define ZETA_Core_ToStr(x) ZETA_Core_ToStr_(x)

#define ZETA_Core_Concat2_(x, y) x##y
#define ZETA_Core_Concat2(x, y) ZETA_Core_Concat2_(x, y)

#define ZETA_Core_Concat(x, ...) \
    ZETA_Core_ForEach(ZETA_Core_Concat2, x, __VA_ARGS__)

#define ZETA_Core_UniqueName(prefix) ZETA_Core_Concat2(prefix, __COUNTER__)

#define ZETA_Core_TmpName ZETA_Core_UniqueName(ZETA_tmp_)

#define ZETA_Core_StaticAssert(...) static_assert((__VA_ARGS__), "")

#define ZETA_Core_ImmPrint 1

// -----------------------------------------------------------------------------

#define ZETA_Core_PtrToAddr(x) (reinterpret_cast<uintptr_t>(x))

#define ZETA_Core_AddrToPtr(x) (reinterpret_cast<void*>(x))

#define ZETA_Core_MemberToStruct(struct_type, member_name, member_ptr) \
    (reinterpret_cast<struct_type*>(                                   \
        const_cast<char*>(reinterpret_cast<char const*>(member_ptr)) - \
        offsetof(struct_type, member_name)))

// -----------------------------------------------------------------------------

#define ZETA_Core_ConstexprAndIfElse(static_cond, dynamic_cond, if_block, \
                                     else_block)                          \
    if constexpr (!static_cast<bool>(static_cond)) {                      \
        else_block                                                        \
    } else if (dynamic_cond) {                                            \
        if_block                                                          \
    } else {                                                              \
        else_block                                                        \
    }

#define ZETA_Core_ConstexprAndIf(static_cond, dynamic_cond, if_block) \
    ZETA_Core_ConstexprAndIfElse(static_cond, dynamic_cond, if_block, {})

#define ZETA_Core_ConstexprOrIfElse(static_cond, dynamic_cond, if_block, \
                                    else_block)                          \
    if constexpr (static_cast<bool>(static_cond)) {                      \
        if_block                                                         \
    } else if (dynamic_cond) {                                           \
        if_block                                                         \
    } else {                                                             \
        else_block                                                       \
    }

#define ZETA_Core_ConstexprOrIf(static_cond, dynamic_cond, if_block) \
    ZETA_Core_ConstexprOrIfElse(static_cond, dynamic_cond, if_block, {})

// -----------------------------------------------------------------------------

#define ZETA_Core_BreakableIf_(tmp, cond) \
    for (bool tmp{ static_cast<bool>(cond) }; tmp; tmp = false)

#define ZETA_Core_BreakableIf(cond) \
    ZETA_Core_BreakableIf_(ZETA_Core_TmpName, (cond))

#define ZETA_Core_Breakable ZETA_Core_BreakableIf(true)

#define ZETA_Core_ConstexprThreeWay_(tmp_ret, constexpr_cond, result_a, \
                                     result_b)                          \
    ({                                                                  \
        zeta::core::Conditional<constexpr_cond, decltype(result_a),     \
                                decltype(result_b)>                     \
            tmp_ret;                                                    \
                                                                        \
        if constexpr (constexpr_cond) {                                 \
            tmp_ret = (result_a);                                       \
        } else {                                                        \
            tmp_ret = (result_b);                                       \
        }                                                               \
                                                                        \
        tmp_ret;                                                        \
    })

#define ZETA_Core_ConstexprThreeWay(constexpr_cond, result_a, result_b)       \
    ZETA_Core_ConstexprThreeWay_(ZETA_Core_TmpName, constexpr_cond, result_a, \
                                 result_b)
