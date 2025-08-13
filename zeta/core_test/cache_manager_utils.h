#pragma once

#include <zeta/core/cache_manager.h>
#include <zeta/core/debugger.h>

#include <unordered_map>

struct Zeta_CoreTest_CacheManagerUtils_Funcs {
    void (*Destroy)(Zeta_CacheManager cm) = NULL;

    void (*Sanitize)(Zeta_CacheManager cm) = NULL;
};

auto& Zeta_CoreTest_CacheManagerUtils_GetFuncs() {
    static std::unordered_map<Zeta_CacheManager_VTable const*,
                              Zeta_CoreTest_CacheManagerUtils_Funcs>
        instance;
    return instance;
}

// -----------------------------------------------------------------------------

void Zeta_CoreTest_CacheManagerUtils_AddDestroyFunc(
    Zeta_CacheManager_VTable const* cache_manager_vtable,
    void (*Destroy)(Zeta_CacheManager cm)) {
    auto& map{ Zeta_CoreTest_CacheManagerUtils_GetFuncs() };

    Zeta_CoreTest_CacheManagerUtils_Funcs& funcs{
        map.insert({ cache_manager_vtable, {} }).first->second
    };

    ZETA_Core_DebugAssert(funcs.Destroy == NULL || funcs.Destroy == Destroy);

    funcs.Destroy = Destroy;
}

void Zeta_CoreTest_CacheManagerUtils_Destroy(Zeta_CacheManager cache_manager) {
    auto& map{ Zeta_CoreTest_CacheManagerUtils_GetFuncs() };

    auto iter{ map.find(cache_manager.vtable) };
    ZETA_Core_DebugAssert(iter != map.end());

    auto Destroy{ iter->second.Destroy };

    ZETA_Core_DebugAssert(Destroy != NULL);

    Destroy(cache_manager);
}

// ---

void Zeta_CoreTest_CacheManagerUtils_AddSanitizeFunc(
    Zeta_CacheManager_VTable const* cache_manager_vtable,
    void (*Sanitize)(Zeta_CacheManager cm)) {
    auto& map{ Zeta_CoreTest_CacheManagerUtils_GetFuncs() };

    Zeta_CoreTest_CacheManagerUtils_Funcs& funcs{
        map.insert({ cache_manager_vtable, {} }).first->second
    };

    ZETA_Core_DebugAssert(funcs.Sanitize == NULL || funcs.Sanitize == Sanitize);

    funcs.Sanitize = Sanitize;
}

void Zeta_CoreTest_CacheManagerUtils_Sanitize(Zeta_CacheManager cm) {
    auto& map{ Zeta_CoreTest_CacheManagerUtils_GetFuncs() };

    auto iter{ map.find(cm.vtable) };
    ZETA_Core_DebugAssert(iter != map.end());

    auto Sanitize{ iter->second.Sanitize };

    ZETA_Core_DebugAssert(Sanitize != NULL);

    Sanitize(cm);
}
