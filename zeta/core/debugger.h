#pragma once

#include <sanitizer/common_interface_defs.h>
#include <stdio.h>
#include <zeta/core/logger.h>

ZETA_Core_ExternC_Beg;

extern Zeta_Core_Pipe Zeta_Core_debug_pipe;

extern bool_t Zeta_Core_assert_stage;

#define ZETA_Core_Debug_ImmPrint TRUE

#define ZETA_Core_Pause                                              \
    {                                                                \
        ZETA_Core_PrintPos(__FILE__, __LINE__, __PRETTY_FUNCTION__); \
                                                                     \
        printf("\tpause...");                                        \
                                                                     \
        if (TRUE) {                                                  \
            fflush(stdout);                                          \
            char tmp;                                                \
            scanf_s("%c", &tmp);                                     \
        } else {                                                     \
            printf("\n");                                            \
            if (ZETA_Core_ImmPrint) { fflush(stdout); }              \
        }                                                            \
    }                                                                \
    ZETA_Core_StaticAssert(TRUE)

#if __has_feature(address_sanitizer)

#define ZETA_Core_PrintStackTrace __sanitizer_print_stack_trace()

#else

#define ZETA_Core_PrintStackTrace ZETA_Core_StaticAssert(TRUE)

#endif

#define ZETA_Core_Assert(cond)                                \
    {                                                         \
        Zeta_Core_Debugger_InitPipe();                        \
                                                              \
        if (cond) {                                           \
        } else {                                              \
            if (!Zeta_Core_assert_stage) {                    \
                Zeta_Core_assert_stage = TRUE;                \
                                                              \
                ZETA_Core_PrintCurPos;                        \
                printf("\033[31massert\t%s\033[0m\n", #cond); \
                fflush(stdout);                               \
                                                              \
                Zeta_Core_Debugger_FlushPipe();               \
                                                              \
                ZETA_Core_PrintCurPos;                        \
                printf("\033[31massert\t%s\033[0m\n", #cond); \
                printf("\a");                                 \
            }                                                 \
                                                              \
            ZETA_Core_PrintStackTrace;                        \
                                                              \
            if (ZETA_Core_ImmPrint) { fflush(stdout); }       \
                                                              \
            exit(1);                                          \
        }                                                     \
    }                                                         \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SanitizeAssert(cond) ZETA_Core_Assert(cond)

#if ZETA_Core_EnableDebug
#define ZETA_Core_DebugAssert(cond) ZETA_Core_Assert((cond))
#else
#define ZETA_Core_DebugAssert(cond) ZETA_Core_Unused((cond))
#endif

#define ZETA_Core_NotNull_(tmp_x, x)          \
    ({                                        \
        ZETA_Core_AutoVar(tmp_x, x);          \
        ZETA_Core_DebugAssert(tmp_x != NULL); \
        tmp_x;                                \
    })

#define ZETA_Core_NotNull(x) ZETA_Core_NotNull_(ZETA_Core_TmpName, x)

void Zeta_Core_Debugger_InitPipe();

void Zeta_Core_Debugger_ClearPipe();

void Zeta_Core_Debugger_FlushPipe();

ZETA_Core_ExternC_End;
