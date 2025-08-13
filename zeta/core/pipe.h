#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_Pipe_VTable);
ZETA_Core_DeclareStruct(Zeta_Core_Pipe);

struct Zeta_Core_Pipe_VTable {
    unsigned (*GetType)(void* context);

    unsigned (*GetFlags)(void* context);

    size_t (*GetWidth)(void* context);

    size_t (*GetSize)(void* context);

    void (*Read)(void* context, size_t cnt, void* dst, size_t dst_stride,
                 bool_t pop);

    size_t (*Write)(void* context, size_t cnt, void const* src,
                    size_t src_stride);

    void (*Clear)(void* context);

    void (*Flush)(void* context);
};

struct Zeta_Core_Pipe {
    Zeta_Core_Pipe_VTable const* vtable;
    void* context;
};

#define ZETA_Core_Pipe_Call__(tmp_pipe, pipe, member_func, ...)     \
    ({                                                              \
        ZETA_Core_AutoVar(tmp_pipe, (pipe));                        \
        ZETA_Core_FuncPtrTable_Call(tmp_pipe.vtable, member_func,   \
                                    tmp_pipe.context, __VA_ARGS__); \
    })

#define ZETA_Core_Pipe_Call_(pipe, member_func, ...) \
    ZETA_Core_Pipe_Call__(ZETA_Core_TmpName, (pipe), member_func, __VA_ARGS__)

#define ZETA_Core_Pipe_GetType(pipe) ZETA_Core_Pipe_Call_((pipe), GetType)

#define ZETA_Core_Pipe_GetFlags(pipe) ZETA_Core_Pipe_Call_((pipe), GetFlags)

#define ZETA_Core_Pipe_GetWidth(pipe) ZETA_Core_Pipe_Call_((pipe), GetWidth)

#define ZETA_Core_Pipe_GetSize(pipe) ZETA_Core_Pipe_Call_((pipe), GetSize)

#define ZETA_Core_Pipe_Read(pipe, cnt, dst, dst_stride, pop) \
    ZETA_Core_Pipe_Call_((pipe), Read, (cnt), (dst), (dst_stride), (pop))

#define ZETA_Core_Pipe_Write(pipe, cnt, src, src_stride) \
    ZETA_Core_Pipe_Call_((pipe), Write, (cnt), (src), (src_stride))

#define ZETA_Core_Pipe_Clear(pipe) ZETA_Core_Pipe_Call_((pipe), Clear)

#define ZETA_Core_Pipe_Flush(pipe) ZETA_Core_Pipe_Call_((pipe), Flush)

ZETA_Core_ExternC_End;
