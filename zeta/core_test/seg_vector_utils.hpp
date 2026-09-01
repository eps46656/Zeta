#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seg_vector.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::seg_vector_utils {

using PolySeqCntr = core::poly_seq_cntr::Cntr;

namespace SegVectorNS = core::seg_vector;

using SegVector = SegVectorNS::Cntr<core::poly_allocator::Allocator,
                                    core::poly_allocator::Allocator>;

struct Pack {
    std_allocator::Allocator seg_alctr;
    std_allocator::Allocator data_alctr;

    SegVector sv;

    constexpr Pack(size_t elem_size, size_t elem_stride, size_t seg_capacity)
        : seg_alctr{},
          data_alctr{},
          sv{
              elem_size,         //
              elem_stride,       //
              seg_capacity,      //
              this->seg_alctr,   //
              this->data_alctr,  //
          } {}
};

PolySeqCntr Create(size_t stride, size_t seg_capacity);

constexpr void Destroy(void* sv);

constexpr void Sanitize(void const* sv);

constexpr PolySeqCntr Create(size_t elem_size, size_t elem_stride,
                             size_t seg_capacity) {
    Pack* pack{ new Pack{ elem_size, elem_stride, seg_capacity } };

    auto* sv{ &pack->sv };

    PolySeqCntr poly_seq_cntr{ *sv };

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return poly_seq_cntr;
}

constexpr void Destroy(void* sv_) {
    SegVector* sv{ static_cast<SegVector*>(sv_) };

    if (sv == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    delete pack;
}

constexpr void Sanitize(void const* sv_) {
    SegVector const* sv{ static_cast<SegVector const*>(sv_) };

    if (sv == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    core::mem_recorder::MemRecorder seg;
    core::mem_recorder::MemRecorder data;

    sv->Sanitize(&seg, &data);

    core::mem_recorder::MatchRecords(pack->seg_alctr.mem_recorder, seg);
    core::mem_recorder::MatchRecords(pack->data_alctr.mem_recorder, data);
}

}  // namespace zeta::core_test::seg_vector_utils
