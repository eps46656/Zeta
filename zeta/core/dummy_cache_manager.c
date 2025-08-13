#include <zeta/core/debugger.h>
#include <zeta/core/dummy_cache_manager.h>

void Zeta_Core_DummyCacheManager_Init(void* dcm_) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_DebugAssert(dcm->origin != NULL);
}

Zeta_Core_SeqCntr* Zeta_Core_DummyCacheManager_GetOrigin(void const* dcm_) {
    Zeta_Core_DummyCacheManager const* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    return dcm->origin;
}

size_t Zeta_Core_DummyCacheManager_GetCacheSize(void const* dcm_) {
    Zeta_Core_DummyCacheManager const* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    return 1;
}

void* Zeta_Core_DummyCacheManager_Open(void* dcm_, size_t max_cache_cnt) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(dcm);
    ZETA_Core_Unused(max_cache_cnt);

    return NULL;
}

void Zeta_Core_DummyCacheManager_Close(void* dcm_, void* sd) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(dcm);
    ZETA_Core_Unused(sd);
}

void Zeta_Core_DummyCacheManager_SetMaxCacheCnt(void* dcm_, void* sd,
                                                size_t max_cache_cnt) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(dcm);
    ZETA_Core_Unused(sd);
    ZETA_Core_Unused(max_cache_cnt);
}

void Zeta_Core_DummyCacheManager_Read(void* dcm_, void* sd, size_t idx,
                                      size_t cnt, void* dst,
                                      size_t dst_stride) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(sd);

    void* cursor = ZETA_Core_SeqCntr_AllocaCursor(dcm->origin);

    ZETA_Core_SeqCntr_Access(dcm->origin, idx, cursor, NULL);

    ZETA_Core_SeqCntr_Read(dcm->origin, cursor, cnt, dst, dst_stride, NULL);
}

void Zeta_Core_DummyCacheManager_Write(void* dcm_, void* sd, size_t idx,
                                       size_t cnt, void const* src,
                                       size_t src_stride) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(sd);

    void* cursor = ZETA_Core_SeqCntr_AllocaCursor(dcm->origin);

    ZETA_Core_SeqCntr_Access(dcm->origin, idx, cursor, NULL);

    ZETA_Core_SeqCntr_Write(dcm->origin, cursor, cnt, src, src_stride, NULL);
}

size_t Zeta_Core_DummyCacheManager_Flush(void* dcm_, size_t quata) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    ZETA_Core_Unused(dcm);
    ZETA_Core_Unused(quata);

    return 0;
}

void Zeta_Core_DummyCacheManager_DeployCacheManager(void* dcm_,
                                                    Zeta_CacheManager* cm) {
    Zeta_Core_DummyCacheManager* dcm = dcm_;
    ZETA_Core_DebugAssert(dcm != NULL);

    Zeta_CacheManager_Init(cm);

    cm->context = dcm;

    cm->GetOrigin = Zeta_Core_DummyCacheManager_GetOrigin;

    cm->GetCacheSize = Zeta_Core_DummyCacheManager_GetCacheSize;

    cm->Open = Zeta_Core_DummyCacheManager_Open;
    cm->Close = Zeta_Core_DummyCacheManager_Close;

    cm->SetMaxCacheCnt = Zeta_Core_DummyCacheManager_SetMaxCacheCnt;

    cm->Read = Zeta_Core_DummyCacheManager_Read;
    cm->Write = Zeta_Core_DummyCacheManager_Write;

    cm->Flush = Zeta_Core_DummyCacheManager_Flush;
}
