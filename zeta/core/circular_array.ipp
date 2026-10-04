#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

constexpr circular_array::Cursor::Cursor(
    seq_cntr::CursorLimit const& src_cursor) {
    *this = src_cursor;
}

constexpr circular_array::Cursor::operator seq_cntr::CursorLimit(
    this Cursor const& self) {
    seq_cntr::CursorLimit ret;

    utils::MemCopy(&ret, &self, sizeof(Cursor));

    return ret;
}

constexpr circular_array::Cursor& circular_array::Cursor::operator=(
    seq_cntr::CursorLimit const& src_cursor) {
    utils::MemCopy(this, &src_cursor, sizeof(Cursor));
    return *this;
}

template <typename Elem>
constexpr Elem* circular_array::ReferElem(Elem* data, size_t elem_stride,
                                          size_t slot_cnt, size_t rot,
                                          size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(slot_cnt <= ZETA_Core_max_capacity);

    size_t k{ rot + idx };

    return utils::PtrInc(data, elem_stride * (k < slot_cnt ? k : k - slot_cnt));
}

constexpr size_t circular_array::GetLongestContPred(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx <= elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ idx <= k ? idx : idx - k };

    return ret;
}

constexpr size_t circular_array::GetLongestContSucr(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx <= elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ (elem_cnt <= k || k <= idx) ? elem_cnt - idx : k - idx };

    return ret;
}

namespace circular_array::detail {

template <typename Elem>
constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self) {
    Elem* data{ self.data };
    constexpr size_t elem_size{ sizeof(Elem) };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_size);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_size <= elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(slot_cnt <= ZETA_Core_max_capacity);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr || slot_cnt == 0);
}

template <typename Elem>
constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(&self == cursor->cntr);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(cursor->idx, 1, elem_cnt));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_cnt <= cursor->idx ||
        cursor->elem ==
            (ReferElem)(data, elem_stride, slot_cnt, rot, cursor->idx));
}

template <typename TransferOpLike, typename DstElem, typename SrcElem>
constexpr void ForwardTransfer_(TransferOpLike transfer_op_like,
                                DstElem* dst_data, SrcElem* src_data,
                                size_t dst_elem_stride, size_t src_elem_stride,
                                size_t dst_rot, size_t src_rot,
                                size_t dst_elem_cnt, size_t src_elem_cnt,
                                size_t dst_slot_cnt, size_t src_slot_cnt,
                                size_t dst_beg, size_t src_beg, size_t cnt) {
    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt,
            (GetLongestContSucr)(dst_elem_cnt, dst_slot_cnt, dst_rot, dst_beg),
            (GetLongestContSucr)(src_elem_cnt, src_slot_cnt, src_rot,
                                 src_beg)) };

        {
            DstElem* dst_iter{ (ReferElem)(dst_data, dst_elem_stride,
                                           dst_slot_cnt, dst_rot, dst_beg) };

            SrcElem* src_iter{ (ReferElem)(src_data, src_elem_stride,
                                           src_slot_cnt, src_rot, src_beg) };

            for (size_t i{ 0 }; i < cur_cnt;
                 ++i, dst_iter = utils::PtrInc(dst_iter, dst_elem_stride),
                      src_iter = utils::PtrInc(src_iter, src_elem_stride)) {
                lifecycle::DataTransfer(transfer_op_like,
                                        static_cast<DstElem*>(dst_iter),
                                        *static_cast<SrcElem*>(src_iter));
            }
        }

        dst_beg += cur_cnt;
        src_beg += cur_cnt;

        cnt -= cur_cnt;
    }
}

template <typename TransferOpLike, typename DstElem, typename SrcElem>
constexpr void BackwardTransfer_(TransferOpLike transfer_op_like,
                                 DstElem* dst_data, SrcElem* src_data,
                                 size_t dst_elem_stride, size_t src_elem_stride,
                                 size_t dst_rot, size_t src_rot,
                                 size_t dst_elem_cnt, size_t src_elem_cnt,
                                 size_t dst_slot_cnt, size_t src_slot_cnt,
                                 size_t dst_beg, size_t src_beg, size_t cnt) {
    for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt,
            (GetLongestContPred)(dst_elem_cnt, dst_slot_cnt, dst_rot, dst_end),
            (GetLongestContPred)(src_elem_cnt, src_slot_cnt, src_rot,
                                 src_end)) };

        {
            DstElem* dst_iter{ (ReferElem)(dst_data, dst_elem_stride,
                                           dst_slot_cnt, dst_rot,
                                           dst_end - 1) };

            SrcElem* src_iter{ (ReferElem)(src_data, src_elem_stride,
                                           src_slot_cnt, src_rot,
                                           src_end - 1) };

            for (size_t i{ 0 }; i < cur_cnt;
                 ++i, dst_iter = utils::PtrDec(dst_iter, dst_elem_stride),
                      src_iter = utils::PtrDec(src_iter, src_elem_stride)) {
                lifecycle::DataTransfer(transfer_op_like,
                                        static_cast<DstElem*>(dst_iter),
                                        *static_cast<SrcElem*>(src_iter));
            }
        }

        dst_end -= cur_cnt;
        src_end -= cur_cnt;

        cnt -= cur_cnt;
    }
}

template <lifecycle::IsDataTransferOpLike TransferOpLike, typename DstCntr,
          typename SrcCntr>
constexpr void SegTransfer_(TransferOpLike transfer_op_like, DstCntr& dst_cntr,
                            SrcCntr& src_cntr, size_t dst_beg, size_t src_beg,
                            size_t cnt) {
    using DstElem =
        meta::MakeConstIf<typename DstCntr::Elem, meta::IsConst<DstCntr>>;
    using SrcElem =
        meta::MakeConstIf<typename SrcCntr::Elem, meta::IsConst<SrcCntr>>;

    detail::CheckCntr_(dst_cntr);
    detail::CheckCntr_(src_cntr);

    DstElem* dst_data{ dst_cntr.data };
    size_t dst_elem_stride{ dst_cntr.elem_stride };
    size_t dst_elem_cnt{ dst_cntr.elem_cnt };
    size_t dst_slot_cnt{ dst_cntr.slot_cnt };
    size_t dst_rot{ dst_cntr.rot };

    SrcElem* src_data{ src_cntr.data };
    size_t src_elem_stride{ src_cntr.elem_stride };
    size_t src_elem_cnt{ src_cntr.elem_cnt };
    size_t src_slot_cnt{ src_cntr.slot_cnt };
    size_t src_rot{ src_cntr.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(src_beg, cnt, src_elem_cnt));

    if (cnt == 0) { return; }

    if (&dst_cntr != &src_cntr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !ZETA_Core_AreOverlapped(dst_data, dst_data + dst_slot_cnt,
                                     src_data, src_data + src_slot_cnt));

        goto VEC_FW_COPY;
    }

    if (dst_beg < src_beg) { goto VEC_FW_COPY; }

    if (src_beg < dst_beg) { goto VEC_BW_MOVE; }

    return;

VEC_FW_COPY:
    {
        detail::ForwardTransfer_(
            transfer_op_like, dst_data, src_data, dst_elem_stride,
            src_elem_stride, dst_rot, src_rot, dst_elem_cnt, src_elem_cnt,
            dst_slot_cnt, src_slot_cnt, dst_beg, src_beg, cnt);

        return;
    }

VEC_BW_MOVE:
    {
        detail::BackwardTransfer_(
            transfer_op_like, dst_data, src_data, dst_elem_stride,
            src_elem_stride, dst_rot, src_rot, dst_elem_cnt, src_elem_cnt,
            dst_slot_cnt, src_slot_cnt, dst_beg, src_beg, cnt);

        return;
    }
}

template <typename SrcElem,
          seq_endpoint::acceptor::IsAcceptor<SrcElem> Acceptor>
constexpr void AcceptorTransfer_(
    lifecycle::DataTransferSemantics src_transfer_semantics, SrcElem* src_data,
    size_t src_elem_stride, size_t src_rot, size_t src_elem_cnt,
    size_t src_slot_cnt, size_t src_beg, size_t cnt, Acceptor&& acceptor) {
    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(src_elem_cnt, src_slot_cnt, src_rot,
                                      src_beg)) };

        seq_endpoint::acceptor::Transfer(
            acceptor, src_transfer_semantics,
            (ReferElem)(src_data, src_elem_stride, src_slot_cnt, src_rot,
                        src_beg),
            static_cast<ptrdiff_t>(src_elem_stride), cur_cnt);

        src_beg += cur_cnt;

        cnt -= cur_cnt;
    }
}

template <typename DstElem,
          seq_endpoint::provider::IsProvider<DstElem> Provider>
constexpr void ProviderTransfer_(lifecycle::DataLifeState dst_life_state,
                                 DstElem* dst_data, size_t dst_elem_stride,
                                 size_t dst_rot, size_t dst_elem_cnt,
                                 size_t dst_slot_cnt, size_t dst_beg,
                                 size_t cnt, Provider&& provider) {
    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(dst_elem_cnt, dst_slot_cnt, dst_rot,
                                      dst_beg)) };

        seq_endpoint::provider::Transfer(
            provider, dst_life_state,
            (ReferElem)(dst_data, dst_elem_stride, dst_slot_cnt, dst_rot,
                        dst_beg),
            static_cast<ptrdiff_t>(dst_elem_stride), cur_cnt);

        dst_beg += cur_cnt;

        cnt -= cur_cnt;
    }
}

}  // namespace circular_array::detail

template <lifecycle::IsDataTransferOpLike TransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void circular_array::SegAssign(TransferOpLike transfer_op_like,
                                         Cntr<DstElem>& dst_cntr,
                                         Cntr<SrcElem>& src_cntr,
                                         size_t dst_beg, size_t src_beg,
                                         size_t cnt) {
    detail::SegTransfer_(transfer_op_like, dst_cntr, src_cntr, dst_beg, src_beg,
                         cnt);
}

template <lifecycle::IsDataTransferOpLike TransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void circular_array::SegAssign(TransferOpLike transfer_op_like,
                                         Cntr<DstElem>& dst_cntr,
                                         Cntr<SrcElem> const& src_cntr,
                                         size_t dst_beg, size_t src_beg,
                                         size_t cnt) {
    detail::SegTransfer_(transfer_op_like, dst_cntr, src_cntr, dst_beg, src_beg,
                         cnt);
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer = true,
        .Derefer = true,

        .Read = true,
        .Write = true,
        .ReadWrite = true,

        .PushL = true,
        .PushR = true,
        .Insert = true,

        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           seq_cntr::capability::const_capability_flag;
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <typename Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <typename Elem>
constexpr void* circular_array::Cntr<Elem>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return const_cast<void*>(static_cast<void const*>(&self));
}

template <typename Elem>
constexpr meta::TypeWrapper<Elem> circular_array::Cntr<Elem>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

template <typename Elem>
constexpr meta::TypeWrapper<circular_array::Cursor>
circular_array::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                          meta::TypeWrapper<Cntr>) {
    return {};
}

template <typename Elem>
constexpr meta::TypeWrapper<circular_array::Cursor>
circular_array::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                          meta::TypeWrapper<Cntr const>) {
    return {};
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetElemStride(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_stride;
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetIdxOffset(this Cntr const& self,
                                                          seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.rot;
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetElemCnt(this Cntr const& self,
                                                        seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_cnt;
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetMaxElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.slot_cnt;
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::GetLBCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->elem = nullptr;
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::GetRBCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->idx = self.elem_cnt;
    dst_cursor->elem = nullptr;
}

template <typename Elem>
template <typename DstElem>
constexpr void circular_array::Cntr<Elem>::PeekL(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    Elem* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, 0)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <typename Elem>
template <typename DstElem>
constexpr void circular_array::Cntr<Elem>::PeekR(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    Elem* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot,
                                           elem_cnt - 1)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = elem_cnt - 1;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <typename Elem>
template <typename DstElem>
constexpr void circular_array::Cntr<Elem>::Refer(
    this auto& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, elem_cnt));

    Elem* elem{ idx < elem_cnt
                    ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                    : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <typename Elem>
template <typename DstElem>
constexpr void circular_array::Cntr<Elem>::Derefer(
    this auto& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_elem != nullptr);

    Elem* elem{ static_cast<Elem*>(pos_cursor->elem) };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

namespace circular_array::detail {

template <seq_endpoint::Type type, typename Elem, typename Endpoint>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem>& self, size_t idx, size_t cnt, Endpoint&& endpoint,
     Cursor* dst_cursor) {
    static_assert(type == seq_endpoint::Type::Acceptor ||
                  type == seq_endpoint::Type::Provider);

    (CheckCntr_)(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t rot{ self.rot };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(idx, cnt, elem_cnt));

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        Elem* elem{ (ReferElem)(data, elem_stride, slot_cnt, rot, idx) };

        if constexpr (type == seq_endpoint::Type::Acceptor) {
            seq_endpoint::acceptor::Transfer(
                endpoint, lifecycle::DataTransferSemantics::Copy, elem,
                static_cast<ptrdiff_t>(elem_stride), cur_cnt);
        } else if constexpr (type == seq_endpoint::Type::Provider) {
            seq_endpoint::provider::Transfer(
                endpoint, lifecycle::DataLifeState::Obj, elem,
                static_cast<ptrdiff_t>(elem_stride), cur_cnt);
        } else {
            ZETA_Core_DebugUtils_Diag_Unreachable();
        }

        idx += cur_cnt;
        cnt -= cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
        dst_cursor->elem =
            idx < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                           : nullptr;
    }
}

}  // namespace circular_array::detail

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::Read(this Cntr const& self,
                                                seq_cntr::Tag,
                                                Cursor const* pos_cursor,
                                                size_t cnt, Acceptor&& acceptor,
                                                Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(
        const_cast<Cntr&>(self), pos_cursor->idx, cnt, acceptor, dst_cursor);
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::Write(this Cntr& self, seq_cntr::Tag,
                                                 Cursor const* pos_cursor,
                                                 size_t cnt,
                                                 Provider&& provider,
                                                 Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, pos_cursor->idx, cnt,
                                                     provider, dst_cursor);
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::ReadWrite(
    this Cntr& self, seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(self, pos_cursor->idx, cnt,
                                                     acceptor, dst_cursor);
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxRead(this Cntr const& self,
                                                   size_t idx, size_t cnt,
                                                   Acceptor&& acceptor) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(
        const_cast<Cntr&>(self), idx, cnt, acceptor, nullptr);
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::IdxWrite(this Cntr& self, size_t idx,
                                                    size_t cnt,
                                                    Provider&& provider) {
    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, idx, cnt, provider,
                                                     nullptr);
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxReadWrite(this Cntr& self,
                                                        size_t idx, size_t cnt,
                                                        Acceptor&& acceptor) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(self, idx, cnt, acceptor,
                                                     nullptr);
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::PushL(this Cntr& self, seq_cntr::Tag,
                                                 size_t cnt,
                                                 Provider&& provider,
                                                 Cursor* dst_beg_cursor,
                                                 Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    self.IdxInsert(0, cnt, provider);

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->cntr = &self;
        dst_beg_cursor->idx = 0;
        dst_beg_cursor->elem = 0 < self.elem_cnt
                                   ? (ReferElem)(self.data, self.elem_stride,
                                                 self.slot_cnt, self.rot, 0)
                                   : nullptr;
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->cntr = &self;
        dst_end_cursor->idx = cnt;
        dst_end_cursor->elem = cnt < self.elem_cnt
                                   ? (ReferElem)(self.data, self.elem_stride,
                                                 self.slot_cnt, self.rot, cnt)
                                   : nullptr;
    }
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::PushR(this Cntr& self, seq_cntr::Tag,
                                                 size_t cnt,
                                                 Provider&& provider,
                                                 Cursor* dst_beg_cursor,
                                                 Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    size_t old_elem_cnt{ self.elem_cnt };

    self.IdxInsert(old_elem_cnt, cnt, provider);

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->cntr = &self;
        dst_beg_cursor->idx = old_elem_cnt;
        dst_beg_cursor->elem =
            old_elem_cnt < self.elem_cnt
                ? (ReferElem)(self.data, self.elem_stride, self.slot_cnt,
                              self.rot, old_elem_cnt)
                : nullptr;
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->cntr = &self;
        dst_end_cursor->idx = self.elem_cnt;
        dst_end_cursor->elem = nullptr;
    }
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::Insert(
    this Cntr& self, seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
    Provider&& provider, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    size_t idx{ pos_cursor->idx };

    self.IdxInsert(idx, cnt, provider);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    pos_cursor->elem = (ReferElem)(data, elem_stride, slot_cnt, rot, idx);

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx + cnt;
        dst_cursor->elem =
            idx + cnt < elem_cnt
                ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx + cnt)
                : nullptr;
    }
}

template <typename Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::IdxInsert(this Cntr& self,
                                                     size_t idx, size_t cnt,
                                                     Provider&& provider) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    if (cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanInsert(idx, cnt, elem_cnt, slot_cnt));

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx };

    self.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        self.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;

        if (l_size < cnt) {
            detail::ForwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::MoveConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, 0, cnt, l_size);

            detail::ProviderTransfer_(lifecycle::DataLifeState::Mem, data,
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_size, cnt - l_size, provider);

            detail::ProviderTransfer_(lifecycle::DataLifeState::Obj, data,
                                      elem_stride, rot, elem_cnt, slot_cnt, cnt,
                                      l_size, provider);
        } else {
            detail::ForwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::MoveConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, 0, cnt, cnt);

            if (cnt < l_size) {
                detail::ForwardTransfer_(
                    meta::AutoValueWrapper<
                        lifecycle::DataTransferOp::MoveAssign>{},
                    data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                    elem_cnt, slot_cnt, slot_cnt, cnt, cnt + cnt, l_size - cnt);
            }

            detail::ProviderTransfer_(lifecycle::DataLifeState::Obj, data,
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_size, cnt, provider);
        }
    } else {
        if (r_size < cnt) {
            detail::BackwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::MoveConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, l_size + cnt, l_size, r_size);

            detail::ProviderTransfer_(lifecycle::DataLifeState::Obj, data,
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_size, r_size, provider);

            detail::ProviderTransfer_(lifecycle::DataLifeState::Mem, data,
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_size + r_size, cnt - r_size, provider);
        } else {
            detail::ForwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::MoveConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, l_size + r_size,
                l_size + r_size - cnt, cnt);

            if (cnt < r_size) {
                detail::BackwardTransfer_(
                    meta::AutoValueWrapper<
                        lifecycle::DataTransferOp::MoveAssign>{},
                    data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                    elem_cnt, slot_cnt, slot_cnt, l_size + cnt, l_size,
                    r_size - cnt);
            }

            detail::ProviderTransfer_(lifecycle::DataLifeState::Obj, data,
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_size, cnt, provider);
        }
    }
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::PopL(this Cntr& self, seq_cntr::Tag,
                                                size_t cnt, Acceptor&& acceptor,
                                                Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    self.IdxErase(0, cnt, acceptor, dst_cursor);
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::PopR(this Cntr& self, seq_cntr::Tag,
                                                size_t cnt, Acceptor&& acceptor,
                                                Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    self.IdxErase(elem_cnt - cnt, cnt, acceptor, dst_cursor);
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::Erase(this Cntr& self, seq_cntr::Tag,
                                                 Cursor* pos_cursor, size_t cnt,
                                                 Acceptor&& acceptor) {
    detail::CheckCursor_(self, pos_cursor);

    size_t idx{ pos_cursor->idx };

    self.IdxErase(idx, cnt, acceptor, pos_cursor);

    pos_cursor->elem = idx < self.elem_cnt
                           ? (ReferElem)(self.data, self.elem_stride,
                                         self.slot_cnt, self.rot, idx)
                           : nullptr;
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxErase(this Cntr& self, size_t idx,
                                                    size_t cnt,
                                                    Acceptor&& acceptor,
                                                    Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(idx, cnt, elem_cnt));

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &self;
            dst_cursor->idx = idx;
            dst_cursor->elem = idx < elem_cnt ? (ReferElem)(data, elem_stride,
                                                            slot_cnt, rot, idx)
                                              : nullptr;
        }

        return;
    }

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx - cnt };

    unsigned long long random_seed{ utils::GetRandom() };

    detail::AcceptorTransfer_(lifecycle::DataTransferSemantics::Reloc, data,
                              elem_stride, rot, elem_cnt, slot_cnt, idx, cnt,
                              acceptor);

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        if (0 < l_size) {
            detail::BackwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::RelocConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, cnt, 0, l_size);
        }

        rot += cnt;
        self.rot = rot = rot < slot_cnt ? rot : rot - slot_cnt;
    } else {
        if (0 < r_size) {
            detail::ForwardTransfer_(
                meta::AutoValueWrapper<
                    lifecycle::DataTransferOp::RelocConstruct>{},
                data, data, elem_stride, elem_stride, rot, rot, elem_cnt,
                elem_cnt, slot_cnt, slot_cnt, l_size, l_size + cnt, r_size);
        }
    }

    self.elem_cnt = (elem_cnt -= cnt);

    if (elem_cnt == 0) { self.rot = rot = 0; }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
        dst_cursor->elem =
            idx < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                           : nullptr;
    }
}

template <typename Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::EraseAll(this Cntr& self,
                                                    seq_cntr::Tag,
                                                    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    self.IdxErase(0, self.elem_cnt, acceptor, nullptr);
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::CopyCursor(this Cntr const& self,
                                                      seq_cntr::Tag,
                                                      Cursor const* src_cursor_,
                                                      Cursor* dst_cursor) {
    Cursor const* src_cursor{ static_cast<Cursor const*>(src_cursor_) };

    detail::CheckCursor_(self, src_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &self;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->elem = src_cursor->elem;
}

template <typename Elem>
constexpr bool circular_array::Cntr<Elem>::AreEqualCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

template <typename Elem>
constexpr comparison::Ordering circular_array::Cntr<Elem>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <typename Elem>
constexpr size_t circular_array::Cntr<Elem>::GetCursorIdx(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_) {
    Cursor const* cursor{ static_cast<Cursor const*>(cursor_) };

    detail::CheckCursor_(self, cursor);

    return cursor->idx;
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::CursorStepL(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::CursorStepR(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::CursorAdvanceL(this Cntr const& self,
                                                          seq_cntr::Tag,
                                                          Cursor* cursor,
                                                          size_t step) {
    detail::CheckCursor_(self, cursor);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::CursorAdvanceR(this Cntr const& self,
                                                          seq_cntr::Tag,
                                                          Cursor* cursor,
                                                          size_t step) {
    detail::CheckCursor_(self, cursor);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= elem_cnt - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

template <typename Elem>
constexpr void circular_array::Cntr<Elem>::SanityCheck(
    void const* cntr_, debug_utils::sanity::SanityCheckScope) {
    Cntr const* cntr{ static_cast<Cntr const*>(cntr_) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr != nullptr);

    detail::CheckCntr_(*cntr);
}

}  // namespace zeta::core
