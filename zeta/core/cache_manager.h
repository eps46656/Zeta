#pragma once

#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_CacheManager_VTable);
ZETA_Core_DeclareStruct(Zeta_CacheManager);

struct Zeta_CacheManager_VTable {
    void (*Deinit)(void* context);

    void* (*Open)(void* context, size_t max_caches_num);

    void (*Close)(void* context, void* sd);

    Zeta_Core_SeqCntr (*GetOrigin)(void const* context);

    size_t (*GetCacheSize)(void const* context);

    void (*SetMaxCacheCnt)(void* context, void* sd, size_t max_cache_cnt);

    void (*Read)(void* context, void* sd, size_t idx, size_t cnt, void* dst,
                 size_t dst_stride);

    void (*Write)(void* context, void* sd, size_t idx, size_t cnt,
                  void const* src, size_t src_stride);

    size_t (*Flush)(void* context, size_t quata);
};

struct Zeta_CacheManager {
    Zeta_CacheManager_VTable const* vtable;
    void* context;
};

#define ZETA_CacheManager_AllocaCursor_(tmp_cache_manager, cache_manager)  \
    ({                                                                     \
        ZETA_Core_AutoVar(tmp_cache_manager, (cache_manager));             \
                                                                           \
        ZETA_Core_DebugAssert(tmp_cache_manager.vtable != NULL);           \
                                                                           \
        __builtin_alloca_with_align(tmp_cache_manager.vtable->cursor_size, \
                                    __CHAR_BIT__ * alignof(max_align_t));  \
    })

#define ZETA_CacheManager_AllocaCursor(cache_manager) \
    ZETA_CacheManager_AllocaCursor_(ZETA_Core_TmpName, (cache_manager))

#define ZETA_CacheManager_Call__(tmp_cache_manager, cache_manager,           \
                                 member_func, ...)                           \
    ({                                                                       \
        ZETA_Core_AutoVar(tmp_cache_manager, (cache_manager));               \
                                                                             \
        ZETA_Core_FuncPtrTable_Call(tmp_cache_manager.vtable, member_func,   \
                                    tmp_cache_manager.context, __VA_ARGS__); \
    })

#define ZETA_CacheManager_Call_(cache_manager, member_func, ...)              \
    ZETA_CacheManager_Call__(ZETA_Core_TmpName, (cache_manager), member_func, \
                             __VA_ARGS__)

#define ZETA_CacheManager_Open(cache_manager, max_cache_cnt) \
    ZETA_CacheManager_Call_((cache_manager), Open, (max_cache_cnt))

#define ZETA_CacheManager_Close(cache_manager, sd) \
    ZETA_CacheManager_Call_((cache_manager), Close, (sd))

#define ZETA_CacheManager_GetOrigin(cache_manager) \
    ZETA_CacheManager_Call_((cache_manager), GetOrigin)

#define ZETA_CacheManager_GetCacheSize(cache_manager) \
    ZETA_CacheManager_Call_((cache_manager), GetCacheSize)

#define ZETA_CacheManager_SetMaxCacheCnt(cache_manager, sd, max_cache_cnt) \
    ZETA_CacheManager_Call_((cache_manager), SetMaxCacheCnt, (sd),         \
                            (max_cache_cnt))

#define ZETA_CacheManager_Read(cache_manager, sd, idx, cnt, dst, dst_stride)  \
    ZETA_CacheManager_Call_((cache_manager), Read, (sd), (idx), (cnt), (dst), \
                            (dst_stride))

#define ZETA_CacheManager_Write(cache_manager, sd, idx, cnt, src, src_stride)  \
    ZETA_CacheManager_Call_((cache_manager), Write, (sd), (idx), (cnt), (src), \
                            (src_stride))

#define ZETA_CacheManager_Flush(cache_manager, quata) \
    ZETA_CacheManager_Call_((cache_manager), PeekL, (quata))

ZETA_Core_ExternC_End;
