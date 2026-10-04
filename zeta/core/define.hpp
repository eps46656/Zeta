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

#define ZETA_Core_ImmPrint 1

#define ZETA_Core_PtrToAddr(x) (reinterpret_cast<uintptr_t>(x))

#define ZETA_Core_AddrToPtr(x) (reinterpret_cast<void*>(x))

#define ZETA_Core_MemberToStruct(struct_type, member_name, member_ptr) \
    ((reinterpret_cast<struct_type*>(                                  \
        const_cast<char*>(reinterpret_cast<char const*>(member_ptr)) - \
        __builtin_offsetof(ZETA_Core_Identity(struct_type), member_name))))

#define ZETA_Core_NotAutoDestruct_(tmp_mem, type, name) \
    alignas(type) unsigned char tmp_mem[sizeof(type)];  \
    type* name{ reinterpret_cast<type*>(tmp_mem) };     \
    new (tmp_mem) type

#define ZETA_Core_NotAutoDestruct(type, name)                               \
    ZETA_Core_NotAutoDestruct_(ZETA_Core_TmpName, ZETA_Core_Identity(type), \
                               name)

#define ZETA_Core_ClangdPreambleBarrier static_assert(true)

namespace zeta::core {

using nullptr_t = decltype(nullptr);

using size_t = __SIZE_TYPE__;

using uintptr_t = __UINTPTR_TYPE__;
using sintptr_t = __INTPTR_TYPE__;
using ptrdiff_t = __PTRDIFF_TYPE__;

static constexpr size_t max_align{ alignof(void*) };

struct None {};

constexpr None* none_ptr{ nullptr };

}  // namespace zeta::core
