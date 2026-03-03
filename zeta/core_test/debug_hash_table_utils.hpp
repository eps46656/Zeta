#pragma once

#include <cstdlib>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/assoc_cntr_ref.ipp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_hash_table.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core_test/assoc_cntr_utils.hpp>

namespace zeta::core_test::debug_hash_table_utils {

using AssocCntrRef = core::assoc_cntr_ref::Ref;

namespace DebugHashTableNS = core::debug_hash_table;
namespace DebugHashTableOps = DebugHashTableNS::ops;
using DebugHashTable = DebugHashTableNS::Cntr<core::assoc_cntr::FnHash,
                                              core::assoc_cntr::FnCompare>;

struct DebugHashTablePack {
    DebugHashTable debug_ht;
};

template <typename Elem>
AssocCntrRef Create();

void Sanitize(void const*);

void Destroy(void* dht);

// -----------------------------------------------------------------------------

template <typename Elem>
AssocCntrRef Create() {
    DebugHashTablePack* pack{ new DebugHashTablePack{} };

    pack->debug_ht.width = sizeof(Elem);

    pack->debug_ht.elem_key_hash_proxy.elem_hash =
        core::hash::ops::TypeErasedBasicHash<Elem>;

    pack->debug_ht.elem_key_eq_proxy.elem_compare =
        core::compare::ops::TypeErasedBasicCompare<Elem, Elem>;

    DebugHashTableOps::Init(&pack->debug_ht);

    AssocCntrRef assoc_cntr_ref{ zeta::core::assoc_cntr_ref::ops::MakeRef(
        &pack->debug_ht) };

    assoc_cntr_utils::AddSanitizeFunc(&pack->debug_ht, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(&pack->debug_ht, Destroy);

    return assoc_cntr_ref;
}

inline void Sanitize(void const*) {}

inline void Destroy(void* dht_) {
    DebugHashTable* dht{ static_cast<DebugHashTable*>(dht_) };

    DebugHashTableOps::Deinit(dht);
}

}  // namespace zeta::core_test::debug_hash_table_utils
