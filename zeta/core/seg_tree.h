#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_SegTree);

struct Zeta_Core_SegTree {
    size_t n;
    size_t* data;
};

size_t Zeta_Core_SegTree_GetDataSize(size_t n);

void Zeta_Core_SegTree_Init(void* st);

size_t Zeta_Core_SegTree_Query(void* st, size_t lb, size_t rb);

size_t Zeta_Core_SegTree_Set(void* st, size_t idx, size_t val);

size_t Zeta_Core_SegTree_Del(void* st, size_t idx, size_t del);

size_t Zeta_Core_SegTree_Find(void* st, size_t offset, size_t* dst_res_offset);

void Zeta_Core_SegTree_Check(void* st);

void Zeta_Core_SegTree_Sanitize(void* st);

ZETA_Core_ExternC_End;
