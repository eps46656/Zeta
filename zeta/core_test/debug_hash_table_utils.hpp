#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/assoc_cntr_ref.ipp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_hash_table.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/hash.ipp>
#include <zeta/core/hash_ref.ipp>
#include <zeta/core_test/assoc_cntr_utils.hpp>

namespace zeta::core_test::debug_hash_table_utils {

using AssocCntrRef = core::assoc_cntr_ref::Cntr;

namespace DebugHashTableNS = core::debug_hash_table;
using DebugHashTable = DebugHashTableNS::Cntr<core::hash_ref::Hasher,
                                              core::comparison_ref::Comparator>;

struct DebugHashTablePack {
    DebugHashTable debug_ht;
};

template <typename Elem>
AssocCntrRef Create();

void Sanitize(void const*);

void Destroy(void* dht);

template <typename Elem>
AssocCntrRef Create() {
    DebugHashTablePack* pack{ new DebugHashTablePack{} };

    pack->debug_ht.elem_size = sizeof(Elem);

    pack->debug_ht.hasher_proxy.elem_hasher.Set(
        core::hash::BasicHasher<Elem>{}, core::meta::TypeWrapper<Elem>{});

    pack->debug_ht.eq_proxy.elem_cmptr.Set(
        core::comparison::BasicComparator<Elem, Elem>{},
        core::meta::TypeWrapper<Elem>{}, core::meta::TypeWrapper<Elem>{});

    pack->debug_ht.Init();

    ZETA_Core_StaticAssert(
        zeta::core::assoc_cntr::IsAssocCntr<decltype(pack->debug_ht)>);

    // AssocCntrRef assoc_cntr_ref{ pack->debug_ht };
    AssocCntrRef assoc_cntr_ref;

    assoc_cntr_utils::AddSanitizeFunc(&pack->debug_ht, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(&pack->debug_ht, Destroy);

    return assoc_cntr_ref;
}

inline void Sanitize(void const*) {}

inline void Destroy(void* dht_) {
    DebugHashTable* dht{ static_cast<DebugHashTable*>(dht_) };

    dht->Deinit();
}

}  // namespace zeta::core_test::debug_hash_table_utils
