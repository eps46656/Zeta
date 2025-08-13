#pragma once

#include <zeta/core/cache_manager.h>
#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_DummyCacheManager);

struct Zeta_Core_DummyCacheManager {
    Zeta_Core_SeqCntr* origin;
};

void Zeta_Core_DummyCacheManager_Init(void* dcm);

Zeta_Core_SeqCntr* Zeta_Core_DummyCacheManager_GetOrigin(void const* dcm);

size_t Zeta_Core_DummyCacheManager_GetCacheSize(void const* dcm);

void* Zeta_Core_DummyCacheManager_Open(void* dcm, size_t max_cache_cnt);

void Zeta_Core_DummyCacheManager_Close(void* dcm, void* sd);

void Zeta_Core_DummyCacheManager_SetMaxCacheCnt(void* dcm, void* sd,
                                                size_t max_cache_cnt);

void Zeta_Core_DummyCacheManager_Read(void* dcm, void* sd, size_t idx,
                                      size_t cnt, void* dst, size_t dst_stride);

void Zeta_Core_DummyCacheManager_WriteBlock(void* dcm, void* sd, size_t idx,
                                            size_t cnt, void* src,
                                            size_t src_stride);

size_t Zeta_Core_DummyCacheManager_Flush(void* dcm, size_t quata);

ZETA_Core_ExternC_End;
