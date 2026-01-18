#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/dynamic_hash_table.ipp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core_test/assoc_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::dynamic_hash_table_utils {

using AssocCntrRef = core::assoc_cntr::AssocCntrRef;
using DynamicHashTable = core::DynamicHashTable<
    core::assoc_cntr::FnHash, core::assoc_cntr::FnCompare,
    core::allocator::AllocatorRef, core::allocator::AllocatorRef>;

struct DynamicHashTablePack {
    StdAllocator ght_table_node_allocator;
    StdAllocator node_allocator;

    DynamicHashTable dht;
};

template <typename Elem>
AssocCntrRef Create();

void Destroy(AssocCntrRef assoc_cntr);

void Sanitize(AssocCntrRef assoc_cntr);

// -----------------------------------------------------------------------------

template <typename Elem>
AssocCntrRef Create() {
    DynamicHashTablePack* pack{ new DynamicHashTablePack{} };

    pack->dht.width = sizeof(Elem);

    pack->dht.ght.node_hash.elem_hash = core::hash::TypeErasedHash<Elem>;

    pack->dht.ght.node_compare.elem_compare =
        core::compare::TypeErasedCompare<Elem, Elem>;

    pack->dht.ght.table_node_allocator =
        zeta::core::allocator::MakeAllocatorRef(
            &pack->ght_table_node_allocator);

    pack->dht.node_allocator =
        zeta::core::allocator::MakeAllocatorRef(&pack->node_allocator);

    DynamicHashTable::Init(&pack->dht);

    AssocCntrRef assoc_cntr_ref{ zeta::core::assoc_cntr::MakeAssocCntrRef(
        &pack->dht) };

    assoc_cntr_utils::AddSanitizeFunc(assoc_cntr_ref.vtable, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(assoc_cntr_ref.vtable, Destroy);

    return assoc_cntr_ref;
}

inline void Destroy(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }

    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(
        DynamicHashTablePack, dht, assoc_cntr.inst) };

    DynamicHashTable::Deinit(&pack->dht);

    std::free(pack);
}

inline void Sanitize(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }

#if ZETA_Core_EnableDebug
    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(
        DynamicHashTablePack, dht, assoc_cntr.inst) };

    core::MemRecorder* table_recorder{ core::MemRecorder::Create() };
    core::MemRecorder* node_recorder{ core::MemRecorder::Create() };

    DynamicHashTable::Sanitize(&pack->dht, table_recorder, node_recorder);

    core::MemRecorder::MatchRecords(pack->ght_table_node_allocator.mem_recorder,
                                    table_recorder);
    core::MemRecorder::MatchRecords(pack->node_allocator.mem_recorder,
                                    node_recorder);

    core::MemRecorder::Destroy(table_recorder);
    core::MemRecorder::Destroy(node_recorder);
#endif
}

}  // namespace zeta::core_test::dynamic_hash_table_utils
