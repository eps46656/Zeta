#pragma once

#include <zeta/core/debug_utils/sanity.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::circular_array {

template <typename Elem>
struct Cntr;

struct Cursor {
    void const* cntr;
    size_t idx;
    void* elem;

    constexpr Cursor() = default;

    constexpr Cursor(Cursor const& src_cursor) = default;

    constexpr Cursor(seq_cntr::CursorLimit const& src_cursor);

    constexpr operator seq_cntr::CursorLimit(this Cursor const& self);

    constexpr Cursor& operator=(Cursor const& src_cursor) = default;

    constexpr Cursor& operator=(seq_cntr::CursorLimit const& src_cursor);

    constexpr bool operator==(Cursor const&) const = default;
    constexpr bool operator!=(Cursor const&) const = default;
};

template <typename Elem>
constexpr Elem* ReferElem(Elem* data, size_t elem_stride, size_t elem_capacity,
                          size_t slot_offset, size_t idx);

constexpr size_t GetLongestContSucr(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

constexpr size_t GetLongestContPred(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

template <lifecycle::IsDataTransferOpLike TransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void SegAssign(TransferOpLike transfer_op_like,
                         Cntr<DstElem>& dst_cntr, Cntr<SrcElem>& src_cntr,
                         size_t dst_beg, size_t src_beg, size_t cnt);

template <lifecycle::IsDataTransferOpLike TransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void SegAssign(TransferOpLike transfer_op_like,
                         Cntr<DstElem>& dst_cntr, Cntr<SrcElem> const& src_cntr,
                         size_t dst_beg, size_t src_beg, size_t cnt);

template <typename Elem_>
struct Cntr {
    using Elem = Elem_;

    Elem* data;
    size_t elem_stride;
    size_t elem_cnt;
    size_t slot_cnt;
    size_t rot;

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    constexpr size_t GetElemStride(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetIdxOffset(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    template <typename DstElem>
    constexpr void PeekL(this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         DstElem* dst_elem);

    template <typename DstElem>
    constexpr void PeekR(this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         DstElem* dst_elem);

    template <typename DstElem>
    constexpr void Refer(this auto& self, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         DstElem* dst_elem);

    template <typename DstElem>
    constexpr void Derefer(this auto& self, seq_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           lifecycle::DataLifeState dst_elem_life_state,
                           DstElem* dst_elem);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this Cntr const& self, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this Cntr& self, seq_cntr::Tag,
                         Cursor const* pos_cursor, size_t cnt,
                         Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this Cntr& self, seq_cntr::Tag,
                             Cursor const* pos_cursor, size_t cnt,
                             Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxRead(this Cntr const& self, size_t idx, size_t cnt,
                           Acceptor&& acceptor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void IdxWrite(this Cntr& self, size_t idx, size_t cnt,
                            Provider&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxReadWrite(this Cntr& self, size_t idx, size_t cnt,
                                Acceptor&& acceptor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Provider&& provider, Cursor* dst_beg_cursor,
                         Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Provider&& provider, Cursor* dst_beg_cursor,
                         Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this Cntr& self, seq_cntr::Tag, Cursor* pos_cursor,
                          size_t cnt, Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void IdxInsert(this Cntr& self, size_t idx, size_t cnt,
                             Provider&& provider);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this Cntr& self, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxErase(this Cntr& self, size_t idx, size_t cnt,
                            Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this Cntr& self, seq_cntr::Tag,
                            Acceptor&& acceptor);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    static constexpr void SanityCheck(
        void const* self, debug_utils::sanity::SanityCheckScope scope);
};

}  // namespace zeta::core::circular_array
