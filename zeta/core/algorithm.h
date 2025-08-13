#pragma once

#include <zeta/core/utils.h>

ZETA_Core_ExternC_Beg;

void* Zeta_Core_NicePartition(void* data, size_t width, size_t stride,
                              size_t size, void* cmp_context,
                              Zeta_Core_Compare Compare);

void Zeta_Core_KthElement(void* data, size_t width, size_t stride, size_t mid,
                          size_t size, void* cmp_context,
                          Zeta_Core_Compare Compare);

void Zeta_Core_Sort(void* data, size_t width, size_t stride, size_t size,
                    void* cmp_context, Zeta_Core_Compare Compare);

void Zeta_Core_MergeSort(void* data, size_t width, size_t stride, size_t size,
                         void* cmp_context, Zeta_Core_Compare Compare);

void Zeta_TransMergeSort(void* data, size_t width, size_t stride, size_t size,
                         void* cmp_context, Zeta_Core_Compare Compare);

ZETA_Core_ExternC_End;
