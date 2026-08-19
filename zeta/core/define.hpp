#pragma once

#define ZETA_Core_ProjDir

#define ZETA_Core_Unused(x) static_cast<void>(x)

#define ZETA_Core_Error()

#define ZETA_Core_Identity(...) __VA_ARGS__

#define ZETA_Core_Comma ,

#define ZETA_Core_ToStr_(x) #x
#define ZETA_Core_ToStr(x) ZETA_Core_ToStr_(x)

#define ZETA_Core_Concat2_(x, y) x##y
#define ZETA_Core_Concat2(x, y) ZETA_Core_Concat2_(x, y)

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
