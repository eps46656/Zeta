#pragma once

#include <zeta/core/debugger.h>

ZETA_Core_ExternC_Beg;

#define ZETA_ColorPtr_GetPtr(color_ptr, align) \
    __builtin_align_down(ZETA_Core_ToVoidPtr((*(color_ptr))), (align))

#define ZETA_ColorPtr_GetColor_(tmp, color_ptr, align) \
    ({                                                 \
        void* tmp = *(color_ptr);                      \
        tmp - __builtin_align_down(tmp, (align));      \
    })

#define ZETA_ColorPtr_GetColor(color_ptr, align) \
    ZETA_ColorPtr_GetColor_(ZETA_Core_TmpName, (color_ptr), (align))

#define ZETA_ColorPtr_Set_(tmp_color_ptr, tmp_color, color_ptr, align, ptr, \
                           color)                                           \
    {                                                                       \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                      \
        ZETA_Core_AutoVar(tmp_color, (color));                              \
        ZETA_Core_DebugAssert(0 <= tmp_color &&                             \
                              (unsigned long long)tmp_color < (align));     \
        *tmp_color_ptr = ZETA_Core_ToVoidPtr((ptr)) + tmp_color;            \
    }                                                                       \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_ColorPtr_Set(color_ptr, align, ptr, color)                   \
    ZETA_ColorPtr_Set_(ZETA_Core_TmpName, ZETA_Core_TmpName, (color_ptr), \
                       (align), (ptr), (color))

#define ZETA_ColorPtr_SetPtr_(tmp_color_ptr, tmp_align, tmp_ptr, color_ptr,    \
                              align, ptr)                                      \
    {                                                                          \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                         \
        ZETA_Core_AutoVar(tmp_align, (align));                                 \
        ZETA_Core_AutoVar(tmp_ptr, (ptr));                                     \
        ZETA_ColorPtr_Set(tmp_color_ptr, tmp_align, tmp_ptr,                   \
                          (ZETA_ColorPtr_GetColor(tmp_color_ptr, tmp_align))); \
    }                                                                          \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_ColorPtr_SetPtr(color_ptr, align, ptr)             \
    ZETA_ColorPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                          ZETA_Core_TmpName, (color_ptr), (align), (ptr))

#define ZETA_ColorPtr_SetColor_(tmp_color_ptr, tmp_align, tmp_color,        \
                                color_ptr, align, color)                    \
    {                                                                       \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                      \
        ZETA_Core_AutoVar(tmp_align, (align));                              \
        ZETA_Core_AutoVar(tmp_color, (color));                              \
        ZETA_ColorPtr_Set(tmp_color_ptr, tmp_align,                         \
                          (ZETA_ColorPtr_GetPtr(tmp_color_ptr, tmp_align)), \
                          tmp_color);                                       \
    }                                                                       \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_ColorPtr_SetColor(color_ptr, align, color)           \
    ZETA_ColorPtr_SetColor_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                            ZETA_Core_TmpName, (color_ptr), (align), (color))

// -----------------------------------------------------------------------------

#define ZETA_Core_RelPtr_GetPtr(rel_ptr, base) \
    (ZETA_Core_ToVoidPtr((base)) + *(rel_ptr))

#define ZETA_Core_RelPtr_SetPtr_(tmp_rel_ptr, tmp_val, rel_ptr, base, ptr) \
    {                                                                      \
        ZETA_Core_AutoVar(tmp_rel_ptr, (rel_ptr));                         \
        ZETA_Core_AutoVar(tmp_val, (ZETA_Core_ToVoidPtr((ptr)) -           \
                                    ZETA_Core_ToVoidPtr((base))));         \
        *tmp_rel_ptr = tmp_val;                                            \
        ZETA_Core_DebugAssert(*tmp_rel_ptr == tmp_val);                    \
    }                                                                      \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_RelPtr_SetPtr(rel_ptr, base, ptr)                           \
    ZETA_Core_RelPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, (rel_ptr), \
                             (base), (ptr))

// -----------------------------------------------------------------------------

#define ZETA_Core_RelColorPtr_GetPtr(rel_color_ptr, align, base)           \
    __builtin_align_down((ZETA_Core_ToVoidPtr((base)) + *(rel_color_ptr)), \
                         (align))

#define ZETA_Core_RelColorPtr_GetColor_(tmp, rel_color_ptr, align, base) \
    ({                                                                   \
        void* tmp = ZETA_Core_ToVoidPtr((base)) + *(rel_color_ptr);      \
        tmp - __builtin_align_down(tmp, (align));                        \
    })

#define ZETA_Core_RelColorPtr_GetColor(rel_color_ptr, align, base)      \
    ZETA_Core_RelColorPtr_GetColor_(ZETA_Core_TmpName, (rel_color_ptr), \
                                    (align), (base))

#define ZETA_Core_RelColorPtr_Set_(tmp_rel_color_ptr, tmp_color, tmp_val,    \
                                   rel_color_ptr, align, base, ptr, color)   \
    {                                                                        \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));               \
        ZETA_Core_AutoVar(tmp_color, (color));                               \
        ZETA_Core_DebugAssert(0 <= tmp_color &&                              \
                              (unsigned long long)tmp_color < (align));      \
        ZETA_Core_AutoVar(tmp_val, (ZETA_Core_ToVoidPtr((ptr)) + tmp_color - \
                                    ZETA_Core_ToVoidPtr((base))));           \
        *tmp_rel_color_ptr = tmp_val;                                        \
        ZETA_Core_DebugAssert(*tmp_rel_color_ptr == tmp_val);                \
    }                                                                        \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_RelColorPtr_Set(rel_color_ptr, align, base, ptr, color)   \
    ZETA_Core_RelColorPtr_Set_(ZETA_Core_TmpName, ZETA_Core_TmpName,        \
                               ZETA_Core_TmpName, (rel_color_ptr), (align), \
                               (base), (ptr), (color))

#define ZETA_Core_RelColorPtr_SetPtr_(tmp_rel_color_ptr, tmp_align, tmp_base, \
                                      tmp_ptr, rel_color_ptr, align, base,    \
                                      ptr)                                    \
    {                                                                         \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));                \
        ZETA_Core_AutoVar(tmp_align, (align));                                \
        ZETA_Core_AutoVar(tmp_base, (base));                                  \
        ZETA_Core_AutoVar(tmp_ptr, (ptr));                                    \
        ZETA_Core_RelColorPtr_Set(                                            \
            tmp_rel_color_ptr, tmp_align, tmp_base, tmp_ptr,                  \
            (ZETA_Core_RelColorPtr_GetColor(tmp_rel_color_ptr, tmp_align,     \
                                            tmp_base)));                      \
    }                                                                         \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_RelColorPtr_SetPtr(rel_color_ptr, align, base, ptr)   \
    ZETA_Core_RelColorPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                  ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                  (rel_color_ptr), (align), (base), (ptr))

#define ZETA_Core_RelColorPtr_SetColor_(tmp_rel_color_ptr, tmp_align,       \
                                        tmp_base, tmp_color, rel_color_ptr, \
                                        align, base, color)                 \
    {                                                                       \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));              \
        ZETA_Core_AutoVar(tmp_align, (align));                              \
        ZETA_Core_AutoVar(tmp_base, (base));                                \
        ZETA_Core_AutoVar(tmp_color, (color));                              \
        ZETA_Core_RelColorPtr_Set(                                          \
            tmp_rel_color_ptr, tmp_align, tmp_base,                         \
            (ZETA_Core_RelColorPtr_GetPtr(tmp_rel_color_ptr, tmp_align,     \
                                          tmp_base)),                       \
            tmp_color);                                                     \
    }                                                                       \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_RelColorPtr_SetColor(rel_color_ptr, align, base, color) \
    ZETA_Core_RelColorPtr_SetColor_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                    ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                    (rel_color_ptr), (align), (base), (color))

ZETA_Core_ExternC_End;
