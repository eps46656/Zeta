#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/multi_level_circular_array.ipp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::multi_level_circular_array_utils {

using PolySeqCntr = core::poly_seq_cntr::Cntr;

namespace MultiLevelCircularArrayNS = core::multi_level_circular_array;

using MultiLevelCircularArray = MultiLevelCircularArrayNS::Cntr<
    core::meta::ValueWrapper<core::multi_level_ptr_table::BranchNum, 4>,
    core::poly_allocator::Allocator, core::poly_allocator::Allocator>;

struct Pack {
    std_allocator::Allocator node_alctr;
    std_allocator::Allocator seg_alctr;

    MultiLevelCircularArray mlca;

    constexpr Pack(size_t elem_size, size_t elem_stride, size_t seg_slot_cnt)
        : mlca{ elem_size, elem_stride, seg_slot_cnt, this->node_alctr,
                this->seg_alctr } {}
};

template <typename Elem>
PolySeqCntr Create(size_t elem_stride, size_t seg_slot_cnt);

void Destroy(void* mlca);

void Sanitize(void const* mlca);

template <typename Elem>
PolySeqCntr Create(size_t elem_size, size_t elem_stride, size_t seg_slot_cnt) {
    Pack* pack{ new Pack{ elem_size, elem_stride, seg_slot_cnt } };

    auto* mlca{ &pack->mlca };

    seq_cntr_utils::AddSanitizeFunc(mlca, Sanitize);

    seq_cntr_utils::AddDestroyFunc(mlca, Destroy);

    return *mlca;
}

inline void Destroy(void* mlca_) {
    MultiLevelCircularArray* mlca{ static_cast<MultiLevelCircularArray*>(
        mlca_) };

    if (mlca == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, mlca, mlca) };

    pack->mlca.Deinit();

    delete pack;
}

inline void Sanitize(void const* mlca_) {
    MultiLevelCircularArray const* mlca{
        static_cast<MultiLevelCircularArray const*>(mlca_)
    };

    if (mlca == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, mlca, mlca) };

    core::mem_recorder::MemRecorder node;
    core::mem_recorder::MemRecorder seg;

    mlca->Sanitize(&node, &seg);

    core::mem_recorder::MemRecorder::MatchRecords(pack->node_alctr.mem_recorder,
                                                  node);
    core::mem_recorder::MemRecorder::MatchRecords(pack->seg_alctr.mem_recorder,
                                                  seg);
}

}  // namespace zeta::core_test::multi_level_circular_array_utils
