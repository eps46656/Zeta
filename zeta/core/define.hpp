#pragma once

#define ZETA_Core_Unused(x) static_cast<void>(x)

#define ZETA_Core_Error()

#define ZETA_Core_Identity(...) __VA_ARGS__

#define ZETA_Core_Comma ,

#define ZETA_Core_LParen (
#define ZETA_Core_RParen )

#define ZETA_Core_LBrace {
#define ZETA_Core_RBrace }

#define ZETA_Core_Get0(x, ...) x
#define ZETA_Core_Get1(x, ...) ZETA_Core_Get0(__VA_ARGS__)
#define ZETA_Core_Get2(x, ...) ZETA_Core_Get1(__VA_ARGS__)
#define ZETA_Core_Get3(x, ...) ZETA_Core_Get2(__VA_ARGS__)
#define ZETA_Core_Get4(x, ...) ZETA_Core_Get3(__VA_ARGS__)
#define ZETA_Core_Get5(x, ...) ZETA_Core_Get4(__VA_ARGS__)

#define ZETA_Core_GetRes(x, ...) __VA_ARGS__

#define ZETA_Core_HasArgs(...) ZETA_Core_Get0(__VA_OPT__(1, ) 0)

#define ZETA_Core_ForEach_Identity(...) __VA_ARGS__
#define ZETA_Core_ForEach_Get0(x, ...) x
#define ZETA_Core_ForEach_GetRes(x, ...) __VA_ARGS__

#define ZETA_Core_ForEach_Get012_(x, y, z, ...) x, y, z
#define ZETA_Core_ForEach_Get012(...) ZETA_Core_ForEach_Get012_(__VA_ARGS__)

#define ZETA_Core_ForEach_0_(func, seq, ...)               \
    func,                                                  \
        (ZETA_Core_ForEach_Identity seq __VA_OPT__(        \
            , func(ZETA_Core_ForEach_Get0(__VA_ARGS__)))), \
        __VA_OPT__(ZETA_Core_ForEach_GetRes(__VA_ARGS__))
#define ZETA_Core_ForEach_0(...) ZETA_Core_ForEach_0_(__VA_ARGS__)

#define ZETA_Core_ForEach_1_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_0(                                      \
                       ZETA_Core_ForEach_0(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_1(...) ZETA_Core_ForEach_1_(__VA_ARGS__)

#define ZETA_Core_ForEach_2_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_1(                                      \
                       ZETA_Core_ForEach_1(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_2(...) ZETA_Core_ForEach_2_(__VA_ARGS__)

#define ZETA_Core_ForEach_3_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_2(                                      \
                       ZETA_Core_ForEach_2(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_3(...) ZETA_Core_ForEach_3_(__VA_ARGS__)

#define ZETA_Core_ForEach_4_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_3(                                      \
                       ZETA_Core_ForEach_3(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_4(...) ZETA_Core_ForEach_4_(__VA_ARGS__)

#define ZETA_Core_ForEach_5_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_4(                                      \
                       ZETA_Core_ForEach_4(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_5(...) ZETA_Core_ForEach_5_(__VA_ARGS__)

#define ZETA_Core_ForEach_6_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_5(                                      \
                       ZETA_Core_ForEach_5(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_6(...) ZETA_Core_ForEach_6_(__VA_ARGS__)

#define ZETA_Core_ForEach_7_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_6(                                      \
                       ZETA_Core_ForEach_6(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_7(...) ZETA_Core_ForEach_7_(__VA_ARGS__)

#define ZETA_Core_ForEach_8_(func, seq, ...)                                 \
    ZETA_Core_ForEach_Get012(                                                \
        __VA_OPT__(ZETA_Core_ForEach_7(                                      \
                       ZETA_Core_ForEach_7(func, seq, __VA_ARGS__)), ) func, \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_8(...) ZETA_Core_ForEach_8_(__VA_ARGS__)

#define ZETA_Core_ForEach_Fetch_(func, seq, _) ZETA_Core_ForEach_GetRes seq

#define ZETA_Core_ForEach_Fetch(...) ZETA_Core_ForEach_Fetch_(__VA_ARGS__)

#define ZETA_Core_ForEach(func, ...) \
    ZETA_Core_ForEach_Fetch(ZETA_Core_ForEach_8(func, (), __VA_ARGS__))

#define ZETA_Core_ForEach_NoCamma_Identity(...) __VA_ARGS__
#define ZETA_Core_ForEach_NoCamma_Get0(x, ...) x
#define ZETA_Core_ForEach_NoCamma_GetRes(x, ...) __VA_ARGS__

#define ZETA_Core_ForEach_NoCamma_Get012_(x, y, z, ...) x, y, z
#define ZETA_Core_ForEach_NoCamma_Get012(...) \
    ZETA_Core_ForEach_NoCamma_Get012_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_0_(func, seq, ...)             \
    func,                                                        \
        (ZETA_Core_ForEach_NoCamma_Identity seq __VA_OPT__(      \
            func(ZETA_Core_ForEach_NoCamma_Get0(__VA_ARGS__)))), \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_GetRes(__VA_ARGS__))
#define ZETA_Core_ForEach_NoCamma_0(...) \
    ZETA_Core_ForEach_NoCamma_0_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_1_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_0(ZETA_Core_ForEach_NoCamma_0( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_1(...) \
    ZETA_Core_ForEach_NoCamma_1_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_2_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_1(ZETA_Core_ForEach_NoCamma_1( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_2(...) \
    ZETA_Core_ForEach_NoCamma_2_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_3_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_2(ZETA_Core_ForEach_NoCamma_2( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_3(...) \
    ZETA_Core_ForEach_NoCamma_3_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_4_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_3(ZETA_Core_ForEach_NoCamma_3( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_4(...) \
    ZETA_Core_ForEach_NoCamma_4_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_5_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_4(ZETA_Core_ForEach_NoCamma_4( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_5(...) \
    ZETA_Core_ForEach_NoCamma_5_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_6_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_5(ZETA_Core_ForEach_NoCamma_5( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_6(...) \
    ZETA_Core_ForEach_NoCamma_6_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_7_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_6(ZETA_Core_ForEach_NoCamma_6( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_7(...) \
    ZETA_Core_ForEach_NoCamma_7_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_8_(func, seq, ...)                        \
    ZETA_Core_ForEach_NoCamma_Get012(                                       \
        __VA_OPT__(ZETA_Core_ForEach_NoCamma_7(ZETA_Core_ForEach_NoCamma_7( \
                       func, seq, __VA_ARGS__)), ) func,                    \
        seq, __VA_ARGS__)
#define ZETA_Core_ForEach_NoCamma_8(...) \
    ZETA_Core_ForEach_NoCamma_8_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma_Fetch_(func, seq, _) \
    ZETA_Core_ForEach_NoCamma_Identity seq

#define ZETA_Core_ForEach_NoCamma_Fetch(...) \
    ZETA_Core_ForEach_NoCamma_Fetch_(__VA_ARGS__)

#define ZETA_Core_ForEach_NoCamma(func, ...) \
    ZETA_Core_ForEach_NoCamma_Fetch(         \
        ZETA_Core_ForEach_NoCamma_8(func, (), __VA_ARGS__))

#define ZETA_Core_Zip2_Identity(...) __VA_ARGS__
#define ZETA_Core_Zip2_Get0(x, ...) x
#define ZETA_Core_Zip2_GetRes(x, ...) __VA_ARGS__

#define ZETA_Core_Zip2_HasArgs(...) ZETA_Core_Zip2_Get0(__VA_OPT__(1, ) 0)

#define ZETA_Core_Zip2_AddTuple_00(func, ret_tuple_seq, seq_a, seq_b) \
    func, ret_tuple_seq, seq_a, seq_b

#define ZETA_Core_Zip2_AddTuple_01()

#define ZETA_Core_Zip2_AddTuple_10()

#define ZETA_Core_Zip2_AddTuple_11(func, ret_tuple_seq, seq_a, seq_b) \
    func,                                                             \
        (ZETA_Core_Identity ret_tuple_seq,                            \
         func(ZETA_Core_Zip2_Get0 seq_a, ZETA_Core_Zip2_Get0 seq_b)), \
        (ZETA_Core_Zip2_GetRes seq_a), (ZETA_Core_Zip2_GetRes seq_b)

#define ZETA_Core_Zip2_AddTuple__(func, ret_tuple_seq, seq_a, seq_b,           \
                                  has_seq_a, has_seq_b)                        \
    ZETA_Core_Zip2_AddTuple_##has_seq_a##has_seq_b(func, ret_tuple_seq, seq_a, \
                                                   seq_b)

#define ZETA_Core_Zip2_AddTuple_(func, ret_tuple_seq, seq_a, seq_b, has_seq_a, \
                                 has_seq_b)                                    \
    ZETA_Core_Zip2_AddTuple__(func, ret_tuple_seq, seq_a, seq_b, has_seq_a,    \
                              has_seq_b)

#define ZETA_Core_Zip2_AddTuple(func, ret_tuple_seq, seq_a, seq_b) \
    ZETA_Core_Zip2_AddTuple_(func, ret_tuple_seq, seq_a, seq_b,    \
                             ZETA_Core_Zip2_HasArgs seq_a,         \
                             ZETA_Core_Zip2_HasArgs seq_b)

#define ZETA_Core_Zip2_0_(func, ret_tuple_seq, seq_a, seq_b) \
    ZETA_Core_Zip2_AddTuple(func, ret_tuple_seq, seq_a, seq_b)
#define ZETA_Core_Zip2_0(...) ZETA_Core_Zip2_0_(__VA_ARGS__)

#define ZETA_Core_Zip2_1_(...) ZETA_Core_Zip2_0(ZETA_Core_Zip2_0(__VA_ARGS__))
#define ZETA_Core_Zip2_1(...) ZETA_Core_Zip2_1_(__VA_ARGS__)

#define ZETA_Core_Zip2_2_(...) ZETA_Core_Zip2_1(ZETA_Core_Zip2_1(__VA_ARGS__))
#define ZETA_Core_Zip2_2(...) ZETA_Core_Zip2_2_(__VA_ARGS__)

#define ZETA_Core_Zip2_3_(...) ZETA_Core_Zip2_2(ZETA_Core_Zip2_2(__VA_ARGS__))
#define ZETA_Core_Zip2_3(...) ZETA_Core_Zip2_3_(__VA_ARGS__)

#define ZETA_Core_Zip2_4_(...) ZETA_Core_Zip2_3(ZETA_Core_Zip2_3(__VA_ARGS__))
#define ZETA_Core_Zip2_4(...) ZETA_Core_Zip2_4_(__VA_ARGS__)

#define ZETA_Core_Zip2_5_(...) ZETA_Core_Zip2_4(ZETA_Core_Zip2_4(__VA_ARGS__))
#define ZETA_Core_Zip2_5(...) ZETA_Core_Zip2_5_(__VA_ARGS__)

#define ZETA_Core_Zip2_6_(...) ZETA_Core_Zip2_5(ZETA_Core_Zip2_5(__VA_ARGS__))
#define ZETA_Core_Zip2_6(...) ZETA_Core_Zip2_6_(__VA_ARGS__)

#define ZETA_Core_Zip2_7_(...) ZETA_Core_Zip2_6(ZETA_Core_Zip2_6(__VA_ARGS__))
#define ZETA_Core_Zip2_7(...) ZETA_Core_Zip2_7_(__VA_ARGS__)

#define ZETA_Core_Zip2_8_(...) ZETA_Core_Zip2_7(ZETA_Core_Zip2_7(__VA_ARGS__))
#define ZETA_Core_Zip2_8(...) ZETA_Core_Zip2_8_(__VA_ARGS__)

#define ZETA_Core_Zip2_DetectError_(x, y)
#define ZETA_Core_Zip2_DetectError(...) ZETA_Core_Zip2_DetectError_(__VA_ARGS__)

#define ZETA_Core_Zip2_Fetch_(func, ret_tuple_seq, seq_a, seq_b)    \
    ZETA_Core_Zip2_GetRes ret_tuple_seq ZETA_Core_Zip2_DetectError( \
        ZETA_Core_Zip2_Identity seq_a, ZETA_Core_Zip2_Identity seq_b)
#define ZETA_Core_Zip2_Fetch(...) ZETA_Core_Zip2_Fetch_(__VA_ARGS__)

#define ZETA_Core_Zip2(func, seq_a, seq_b) \
    ZETA_Core_Zip2_Fetch(ZETA_Core_Zip2_8(func, (), seq_a, seq_b))

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

#define ZETA_Core_PtrToAddr(x) (reinterpret_cast<uintptr_t>(x))

#define ZETA_Core_AddrToPtr(x) (reinterpret_cast<void*>(x))

#define ZETA_Core_MemberToStruct(struct_type, member_name, member_ptr) \
    ((reinterpret_cast<struct_type*>(                                  \
        const_cast<char*>(reinterpret_cast<char const*>(member_ptr)) - \
        __builtin_offsetof(ZETA_Core_Identity(struct_type), member_name))))

namespace zeta::core {

using size_t = __SIZE_TYPE__;

using uintptr_t = __UINTPTR_TYPE__;
using sintptr_t = __INTPTR_TYPE__;
using ptrdiff_t = __PTRDIFF_TYPE__;

static constexpr size_t max_align{ alignof(void*) };

}  // namespace zeta::core
