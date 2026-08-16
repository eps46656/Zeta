#pragma once

#include <vcruntime_new.h>

#include <cstdlib>
#include <map>
#include <vector>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_ref.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/dynamic_hash_table.ipp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/hash.ipp>
#include <zeta/core/hash_ref.ipp>
#include <zeta/core/lcg_random_engine.hpp>
#include <zeta/core/lcg_random_engine.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core_test/assoc_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::dynamic_hash_table_utils {

using AssocCntrRef = core::assoc_cntr_ref::Cntr;

namespace DynamicHashTableNS = core::dynamic_hash_table;

using DynamicHashTable = DynamicHashTableNS::Cntr<
    core::hash_ref::Hasher, core::comparison_ref::Comparator,
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

    pack->dht.Init(
        sizeof(Elem),
        // elem_width

        {
            .move_quata_per_op = 8,
            .center_load_ratio = 4,
            .drift_ratio = zeta::core::generic_hash_table::UFP::FromFraction(
                3U, 2U),  // 1.5
        },
        // rehashing_config

        core::lifecycle::MakeInitArgsTuple(   //
            core::hash::BasicHasher<Elem>{},  //
            core::meta::TypeWrapper<Elem>{}   //
            ),
        // elem_hash_init_arg

        core::lifecycle::MakeInitArgsTuple(                   //
            core::comparison::BasicComparator<Elem, Elem>{},  //
            core::meta::TypeWrapper<Elem>{},                  //
            core::meta::TypeWrapper<Elem>{}                   //
            ),
        // elem_cmptr_init_argd

        core::lifecycle::SkipInitTag{},  // salt_random_engine_init_arg

        core::lifecycle::SkipInitTag{},  // node_alctr_init_arg
        core::lifecycle::SkipInitTag{}   // table_node_alctr_init_arg
    );

    if (false) {
        using Cntr = zeta::core::meta::RemoveRef<decltype(pack->dht)>;

        zeta::core::assoc_cntr::CntrTraits<Cntr>::Find(
            pack->dht, nullptr, zeta::core::hash::ArchetHasher{},
            zeta::core::comparison::ArchetComparator{}, true, nullptr, nullptr,
            nullptr);

        zeta::core::hash::ArchetHasher key_hasher;
        zeta::core::comparison::ArchetComparator key_elem_cmptr;
        zeta::core::assoc_cntr::ElemPtrView* elem_ptr_view_ptr;

        void* void_ptr{ nullptr };
        void const* const_void_ptr{ nullptr };

        bool bool_val{ false };

        ZETA_Core_StaticAssert(requires {
            zeta::core::assoc_cntr::CntrTraits<Cntr>::Find(
                pack->dht, const_void_ptr, key_hasher, key_elem_cmptr, bool_val,
                elem_ptr_view_ptr, void_ptr, void_ptr);
        });

        ZETA_Core_StaticAssert(
            requires(Cntr& cntr, bool bool_val, void* void_ptr,
                     void const* const_void_ptr,
                     DynamicHashTableNS::Cursor* cursor, size_t size_val,
                     zeta::core::elem_stream::acceptor::ArchetAcceptor reader) {
                zeta::core::assoc_cntr::CntrTraits<Cntr>::Erase(
                    cntr, cursor, size_val, reader);
            });
    }

    AssocCntrRef assoc_cntr_ref{ pack->dht };

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

    pack->dht.Sanitize(&table_recorder, &node_recorder);

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

    dht->Deinit();

    std::free(pack);
}

}  // namespace zeta::core_test::dynamic_hash_table_utils
