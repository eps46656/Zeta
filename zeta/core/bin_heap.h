#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

void Zeta_Core_BinHeap_Construct(void* data, size_t width, size_t stride,
                                 size_t size, void* cmp_context,
                                 int (*Compare)(void* cmp_context,
                                                void const* x, void const* y));

void* Zeta_Core_BinHeap_Push(void* data, size_t width, size_t stride,
                             size_t size, void const* src_elem,
                             void* cmp_context,
                             int (*Compare)(void* cmp_context, void const* x,
                                            void const* y));

void Zeta_Core_BinHeap_Pop(void* data, size_t width, size_t stride, size_t size,
                           void* dst_elem, void* cmp_context,
                           int (*Compare)(void* cmp_context, void const* x,
                                          void const* y));

void* Zeta_Core_BinHeap_Adjust(void* data, size_t width, size_t stride,
                               size_t idx, size_t size, void* cmp_context,
                               int (*Compare)(void* cmp_context, void const* x,
                                              void const* y));

ZETA_Core_ExternC_End;
