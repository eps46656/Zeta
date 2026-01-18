#pragma once

#include <cstdlib>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_hash_table.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core_test/assoc_cntr_utils.hpp>

namespace zeta::core_test::debug_hash_table_utils {

using AssocCntrRef = core::assoc_cntr::AssocCntrRef;
using DebugHashTable =
    core::DebugHashTable<core::assoc_cntr::FnHash, core::assoc_cntr::FnCompare>;

struct DebugHashTablePack {
    DebugHashTable debug_ht;
};

template <typename Elem>
AssocCntrRef Create();

void Destroy(AssocCntrRef assoc_cntr);

void Sanitize(AssocCntrRef assoc_cntr);

// -----------------------------------------------------------------------------

template <typename Elem>
AssocCntrRef Create() {
    DebugHashTablePack* pack{ new DebugHashTablePack{} };

    pack->debug_ht.width = sizeof(Elem);

    pack->debug_ht.elem_key_hash.elem_hash = core::hash::TypeErasedHash<Elem>;

    pack->debug_ht.elem_key_eq.elem_compare =
        core::compare::TypeErasedCompare<Elem, Elem>;

    DebugHashTable::Init(&pack->debug_ht);

    AssocCntrRef assoc_cntr_ref{ zeta::core::assoc_cntr::MakeAssocCntrRef(
        &pack->debug_ht) };

    assoc_cntr_utils::AddSanitizeFunc(assoc_cntr_ref.vtable, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(assoc_cntr_ref.vtable, Destroy);

    return assoc_cntr_ref;
}

void Destroy(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }

    DebugHashTablePack* pack{ ZETA_Core_MemberToStruct(
        DebugHashTablePack, debug_ht, assoc_cntr.inst) };

    DebugHashTable::Deinit(&pack->debug_ht);

    std::free(pack);
}

inline void Sanitize(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }
}

}  // namespace zeta::core_test::debug_hash_table_utils
