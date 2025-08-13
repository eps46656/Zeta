#pragma once

#include <zeta/core/debugger.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_FlowBlock);

#if defined(__x86_64)

struct Zeta_Core_FlowBlock {
    unsigned char regs[64];
} __attribute__((aligned(8)));

#endif

typedef __attribute__((sysv_abi,
                       noreturn)) void (*Zeta_Core_FlowFunc)(void* context,
                                                             void* code);

__attribute__((sysv_abi, optnone)) void Zeta_Core_Flow_SetFunc_(
    Zeta_Core_FlowBlock* flow_blk, void* flow_context,
    Zeta_Core_FlowFunc flow_func, void* stack_top);

#define ZETA_Core_Flow_SetFunc_(tmp_flow_blk, tmp_flow_func, tmp_stack_top,   \
                                flow_blk, flow_context, flow_func, stack_top) \
    {                                                                         \
        Zeta_Core_FlowBlock* tmp_flow_blk = (flow_blk);                       \
        Zeta_Core_FlowFunc tmp_flow_func = (flow_func);                       \
        void* tmp_stack_top = (stack_top);                                    \
                                                                              \
        ZETA_Core_DebugAssert(tmp_flow_blk != NULL);                          \
        ZETA_Core_DebugAssert(tmp_flow_func != NULL);                         \
        ZETA_Core_DebugAssert(tmp_stack_top != NULL);                         \
                                                                              \
        Zeta_Core_Flow_SetFunc_(tmp_flow_blk, (flow_context), tmp_flow_func,  \
                                __builtin_align_down(tmp_stack_top, 64));     \
    }                                                                         \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_Flow_SetFunc(flow_blk, flow_context, flow_func, stack_top) \
    ZETA_Core_Flow_SetFunc_(ZETA_Core_TmpName, ZETA_Core_TmpName,            \
                            ZETA_Core_TmpName, (flow_blk), (flow_context),   \
                            (flow_func), (stack_top))

__attribute__((sysv_abi, noreturn, optnone)) void Zeta_Core_Flow_Jump_(
    Zeta_Core_FlowBlock* dst_flow_blk, void* code);

#define ZETA_Core_Flow_Jump_(tmp_dst_flow_blk, dst_flow_blk, code) \
    ({                                                             \
        Zeta_Core_FlowBlock* tmp_dst_flow_blk = (dst_flow_blk);    \
        ZETA_Core_DebugAssert(tmp_dst_flow_blk != NULL);           \
                                                                   \
        Zeta_Core_Flow_Jump_(tmp_dst_flow_blk, (code));            \
    })

#define ZETA_Core_Flow_Jump(dst_flow_blk, code) \
    ZETA_Core_Flow_Jump_(ZETA_Core_TmpName, (dst_flow_blk), (code))

__attribute__((sysv_abi, optnone)) void* Zeta_Core_Flow_Switch_(
    Zeta_Core_FlowBlock* src_flow_blk, Zeta_Core_FlowBlock* dst_flow_blk,
    void* code);

#define ZETA_Core_Flow_Switch_(tmp_src_flow_blk, tmp_dst_flow_blk,          \
                               src_flow_blk, dst_flow_blk, code)            \
    ({                                                                      \
        Zeta_Core_FlowBlock* tmp_src_flow_blk = (src_flow_blk);             \
        Zeta_Core_FlowBlock* tmp_dst_flow_blk = (dst_flow_blk);             \
                                                                            \
        ZETA_Core_DebugAssert(tmp_src_flow_blk != NULL);                    \
        ZETA_Core_DebugAssert(tmp_dst_flow_blk != NULL);                    \
                                                                            \
        Zeta_Core_Flow_Switch_(tmp_src_flow_blk, tmp_dst_flow_blk, (code)); \
    })

#define ZETA_Core_Flow_Switch(src_flow_blk, dst_flow_blk, code)  \
    ZETA_Core_Flow_Switch_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                           (src_flow_blk), (dst_flow_blk), (code))

ZETA_Core_ExternC_End;
