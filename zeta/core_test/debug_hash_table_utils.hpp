#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/assoc_cntr_ref.ipp>
#include <zeta/core/compare.ipp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_hash_table.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core_test/assoc_cntr_utils.hpp>

namespace zeta::core_test::debug_hash_table_utils {

using AssocCntrRef = core::assoc_cntr_ref::Ref;

namespace DebugHashTableNS = core::debug_hash_table;
using DebugHashTable = DebugHashTableNS::Cntr<core::assoc_cntr::FnHash,
                                              core::assoc_cntr::FnCompare>;

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

    pack->debug_ht.elem_key_hasher_proxy.elem_hasher =
        core::hash::TypeErasedBasicHash<Elem>;

    pack->debug_ht.elem_key_eq_proxy.elem_cmptr =
        core::comparison::TypeErasedBasicCompare<Elem, Elem>;

    DebugHashTableNS::Init(pack->debug_ht);

    AssocCntrRef assoc_cntr_ref{ zeta::core::assoc_cntr_ref::MakeRef(
        pack->debug_ht) };

    assoc_cntr_utils::AddSanitizeFunc(&pack->debug_ht, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(&pack->debug_ht, Destroy);

    return assoc_cntr_ref;
}

inline void Sanitize(void const*) {}

inline void Destroy(void* dht_) {
    DebugHashTable* dht{ static_cast<DebugHashTable*>(dht_) };

    DebugHashTableNS::Deinit(*dht);
}

}  // namespace zeta::core_test::debug_hash_table_utils
