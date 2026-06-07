#pragma once

#include <vcruntime_new.h>

#include <cstdlib>
#include <map>
#include <vector>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/compare.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/dynamic_hash_table.ipp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/lcg_random_engine.hpp>
#include <zeta/core/lcg_random_engine.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core_test/assoc_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::dynamic_hash_table_utils {

using AssocCntrRef = core::assoc_cntr_ref::Ref;

namespace DynamicHashTableNS = core::dynamic_hash_table;

using DynamicHashTable = DynamicHashTableNS::Cntr<
    core::assoc_cntr::FnHash, core::assoc_cntr::FnCompare,
    core::lcg_random_engine::rand48, std_allocator::Allocator,
    std_allocator::Allocator>;

struct DynamicHashTablePack {
    DynamicHashTable dht;
};

template <typename Elem>
AssocCntrRef Create();

void Sanitize(void const* dht);

void Destroy(void* dht);

template <typename Elem>
AssocCntrRef Create() {
    DynamicHashTablePack* pack{ new DynamicHashTablePack{} };

    new (&pack->dht.ght.table_node_alctr) std_allocator::Allocator;

    new (&pack->dht.node_alctr) std_allocator::Allocator;

    DynamicHashTableNS::Init(
        pack->dht,  // dht

        sizeof(Elem),
        // elem_width

        core::generic_hash_table::RehashingConfig{
            .move_quata_per_op = 8,
            .center_load_ratio = 4,
            .drift_ratio = zeta::core::generic_hash_table::UFP::FromFraction(
                3U, 2U),  // 1.5
        },
        // rehashing_config

        core::hash::TypeErasedBasicHash<Elem>,  // elem_hash_init_arg

        core::compare::TypeErasedBasicCompare<Elem, Elem>,
        // elem_compare_init_arg

        core::lifecycle::SkipInitTag{},  // salt_random_engine_init_arg

        core::lifecycle::SkipInitTag{},  // table_node_alctr_init_arg
        core::lifecycle::SkipInitTag{}   // node_alctr_init_arg
    );

    AssocCntrRef assoc_cntr_ref{ zeta::core::assoc_cntr_ref::MakeRef(
        pack->dht) };

    assoc_cntr_utils::AddSanitizeFunc(&pack->dht, Sanitize);

    assoc_cntr_utils::AddDestroyFunc(&pack->dht, Destroy);

    return assoc_cntr_ref;
}

inline void Sanitize(void const* dht_) {
    auto const* dht{ static_cast<DynamicHashTable const*>(dht_) };

#if ZETA_Core_EnableDebug
    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(DynamicHashTablePack,
                                                         dht, dht) };

    core::mem_recorder::MemRecorder table_recorder;
    core::mem_recorder::MemRecorder node_recorder;

    DynamicHashTableNS::Sanitize(pack->dht, &table_recorder, &node_recorder);

    core::mem_recorder::MatchRecords(
        pack->dht.ght.table_node_alctr.mem_recorder, table_recorder);
    core::mem_recorder::MatchRecords(pack->dht.node_alctr.mem_recorder,
                                     node_recorder);

#endif
}

inline void Destroy(void* dht_) {
    auto* dht{ static_cast<DynamicHashTable*>(dht_) };

    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(DynamicHashTablePack,
                                                         dht, dht) };

    DynamicHashTableNS::Deinit(*dht);

    std::free(pack);
}

}  // namespace zeta::core_test::dynamic_hash_table_utils
