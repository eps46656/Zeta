#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_Allocator_VTable);
ZETA_Core_DeclareStruct(Zeta_Core_Allocator);

struct Zeta_Core_Allocator_VTable {
    size_t (*GetAlign)(void const* context);

    size_t (*Query)(void const* context, size_t size);

    void* (*Allocate)(void* context, size_t size);

    void (*Deallocate)(void* context, void* ptr);
};

struct Zeta_Core_Allocator {
    Zeta_Core_Allocator_VTable const* vtable;
    void* context;
};

#define ZETA_Core_Allocator_Call__(tmp_allocator, allocator, func_name, ...) \
    ({                                                                       \
        ZETA_Core_AutoVar(tmp_allocator, (allocator));                       \
        ZETA_Core_FuncPtrTable_Call(tmp_allocator.vtable, func_name,         \
                                    tmp_allocator.context, __VA_ARGS__);     \
    })

#define ZETA_Core_Allocator_Call_(allocator, func_name, ...)              \
    ZETA_Core_Allocator_Call__(ZETA_Core_TmpName, (allocator), func_name, \
                               __VA_ARGS__)

#define ZETA_Core_Allocator_Check_(tmp_allocator, tmp_align, allocator, align) \
    {                                                                          \
        ZETA_Core_AutoVar(tmp_allocator, (allocator));                         \
        ZETA_Core_AutoVar(tmp_align, (align));                                 \
                                                                               \
        ZETA_Core_DebugAssert(tmp_allocator.vtable != NULL);                   \
        ZETA_Core_DebugAssert(tmp_allocator.vtable->GetAlign != NULL);         \
        ZETA_Core_DebugAssert(tmp_allocator.vtable->Query != NULL);            \
        ZETA_Core_DebugAssert(tmp_allocator.vtable->Allocate != NULL);         \
        ZETA_Core_DebugAssert(tmp_allocator.vtable->Deallocate != NULL);       \
                                                                               \
        ZETA_Core_DebugAssert(0 < tmp_align);                                  \
        ZETA_Core_DebugAssert(                                                 \
            ZETA_Core_Allocator_GetAlign(tmp_allocator) % tmp_align == 0);     \
    }                                                                          \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_Allocator_Check(allocator, align)                  \
    ZETA_Core_Allocator_Check_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                               (allocator), (align))

#define ZETA_Core_Allocator_GetAlign(allocator) \
    ZETA_Core_Allocator_Call_((allocator), GetAlign)

#define ZETA_Core_Allocator_Query(allocator, size) \
    ZETA_Core_Allocator_Call_((allocator), Query, (size))

#define ZETA_Core_Allocator_SafeAllocate_(tmp_ret, tmp_align, tmp_size,      \
                                          allocator, align, size)            \
    ({                                                                       \
        ZETA_Core_AutoVar(tmp_align, (align));                               \
        ZETA_Core_AutoVar(tmp_size, (size));                                 \
                                                                             \
        ZETA_Core_DebugAssert(0 < tmp_align);                                \
                                                                             \
        void* tmp_ret;                                                       \
                                                                             \
        if (tmp_size == 0) {                                                 \
            tmp_ret = NULL;                                                  \
        } else {                                                             \
            tmp_ret =                                                        \
                ZETA_Core_Allocator_Call_((allocator), Allocate, tmp_size);  \
            ZETA_Core_DebugAssert(tmp_ret != NULL);                          \
            ZETA_Core_DebugAssert(__builtin_is_aligned(tmp_ret, tmp_align)); \
        }                                                                    \
                                                                             \
        tmp_ret;                                                             \
    })

#define ZETA_Core_Allocator_SafeAllocate(allocator, align, size)               \
    ZETA_Core_Allocator_SafeAllocate_(ZETA_Core_TmpName, ZETA_Core_TmpName,    \
                                      ZETA_Core_TmpName, (allocator), (align), \
                                      (size))

#define ZETA_Core_Allocator_Deallocate(allocator, ptr)         \
    ZETA_Core_Allocator_Call_((allocator), Deallocate, (ptr)); \
    ZETA_Core_StaticAssert(TRUE)

ZETA_Core_ExternC_End;
