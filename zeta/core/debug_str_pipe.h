#pragma once

#include <zeta/core/pipe.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DebugStrPipe);

void* Zeta_Core_DebugStrPipe_Create();

void Zeta_Core_DebugStrPipe_Destroy(void* debug_str_pipe);

unsigned Zeta_Core_DebugStrPipe_GetType(void* debug_str_pipe);

unsigned Zeta_Core_DebugStrPipe_GetFlags(void* debug_str_pipe);

size_t Zeta_Core_DebugStrPipe_GetWidth(void* debug_str_pipe);

size_t Zeta_Core_DebugStrPipe_GetSize(void* debug_str_pipe);

void Zeta_Core_DebugStrPipe_Read(void* debug_str_pipe, size_t cnt, void* dst,
                                 size_t stride, bool_t pop);

size_t Zeta_Core_DebugStrPipe_Write(void* debug_str_pipe, size_t cnt,
                                    void const* src, size_t stride);

void Zeta_Core_DebugStrPipe_Clear(void* debug_str_pipe);

void Zeta_Core_DebugStrPipe_Flush(void* debug_str_pipe);

extern Zeta_Core_Pipe_VTable const Zeta_Core_DebugStrPipe_pipe_vtable;

ZETA_Core_ExternC_End;
