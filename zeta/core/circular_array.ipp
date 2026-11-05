#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/revision.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

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

template <meta::IsObjectType Elem>
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

template <meta::IsObjectType Elem>
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
    ZETA_Core_DebugUtils_Diag_PromiseAssert(slot_cnt == 0 ? rot == 0
                                                          : rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr || slot_cnt == 0);
}

template <meta::IsObjectType Elem>
constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

#if ZETA_Core_DebugEnable
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.stamp == cursor->stamp);
#endif

    size_t elem_cnt{ self.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(cursor->idx, 1, elem_cnt));
}

enum struct Direction_ : unsigned char {
    L = 0,
    R = 1,
};

template <Direction_ direction, lifecycle::IsDstStorageView DstStorageView,
          lifecycle::IsSrcStorageView SrcStorageView>
constexpr void DataTransfer_(meta::ValueWrapper<Direction_, direction>,
                             DstStorageView dst_data, SrcStorageView src_data,
                             size_t dst_elem_stride, size_t src_elem_stride,
                             size_t dst_rot, size_t src_rot,
                             size_t dst_elem_cnt, size_t src_elem_cnt,
                             size_t dst_slot_cnt, size_t src_slot_cnt,
                             size_t dst_beg, size_t src_beg, size_t cnt) {
    size_t dst_idx;
    size_t src_idx;

    if constexpr (direction == Direction_::L) {
        dst_idx = dst_beg + cnt - 1;
        src_idx = src_beg + cnt - 1;
    } else if constexpr (direction == Direction_::R) {
        dst_idx = dst_beg;
        src_idx = src_beg;
    } else {
        static_assert(false);
    }

    while (0 < cnt) {
        size_t cur_cnt;

        if constexpr (direction == Direction_::L) {
            cur_cnt = comparison_utils::BasicMin(
                cnt,
                (GetLongestContPred)(dst_elem_cnt, dst_slot_cnt, dst_rot,
                                     dst_idx + 1),
                (GetLongestContPred)(src_elem_cnt, src_slot_cnt, src_rot,
                                     src_idx + 1));
        } else if constexpr (direction == Direction_::R) {
            cur_cnt = comparison_utils::BasicMin(
                cnt,
                (GetLongestContSucr)(dst_elem_cnt, dst_slot_cnt, dst_rot,
                                     dst_idx),
                (GetLongestContSucr)(src_elem_cnt, src_slot_cnt, src_rot,
                                     src_idx));
        } else {
            static_assert(false);
        }

        auto* dst_iter{ (ReferElem)(dst_data.ptr, dst_elem_stride, dst_slot_cnt,
                                    dst_rot, dst_idx) };

        auto* src_iter{ (ReferElem)(src_data.ptr, src_elem_stride, src_slot_cnt,
                                    src_rot, src_idx) };

        if constexpr (direction == Direction_::L) {
            for (size_t i{ cur_cnt }; 0 < i;
                 --i, dst_iter = utils::PtrDec(dst_iter, dst_elem_stride),
                      src_iter = utils::PtrDec(src_iter, src_elem_stride)) {
                lifecycle::StorageTransfer(DstStorageView{ dst_iter },
                                           SrcStorageView{ src_iter });
            }

            dst_idx -= cur_cnt;
            src_idx -= cur_cnt;
        } else if constexpr (direction == Direction_::R) {
            for (size_t i{ cur_cnt }; 0 < i;
                 --i, dst_iter = utils::PtrInc(dst_iter, dst_elem_stride),
                      src_iter = utils::PtrInc(src_iter, src_elem_stride)) {
                lifecycle::StorageTransfer(DstStorageView{ dst_iter },
                                           SrcStorageView{ src_iter });
            }

            dst_idx += cur_cnt;
            src_idx += cur_cnt;
        } else {
            static_assert(false);
        }

        cnt -= cur_cnt;
    }
}

template <
    meta::IsAnySame<seq_endpoint::acceptor::Tag, seq_endpoint::provider::Tag>
        EndpointTag,
    lifecycle::IsStorageView StorageView, typename Endpoint>
constexpr void EndpointTransfer_(EndpointTag, StorageView data,
                                 size_t elem_stride, size_t rot,
                                 size_t elem_cnt, size_t slot_cnt, size_t beg,
                                 size_t cnt, Endpoint&& endpoint) {
    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, beg)) };

        auto elem{ (ReferElem)(data.ptr, elem_stride, slot_cnt, rot, beg) };

        if constexpr (meta::IsSame<EndpointTag, seq_endpoint::acceptor::Tag>) {
            seq_endpoint::acceptor::Transfer(
                endpoint, StorageView{ elem },
                static_cast<ptrdiff_t>(elem_stride), cur_cnt);
        } else if constexpr (meta::IsSame<EndpointTag,
                                          seq_endpoint::provider::Tag>) {
            seq_endpoint::provider::Transfer(
                endpoint, StorageView{ elem },
                static_cast<ptrdiff_t>(elem_stride), cur_cnt);
        } else {
            static_assert(false);
        }

        beg += cur_cnt;
        cnt -= cur_cnt;
    }
}

}  // namespace circular_array::detail

template <typename DstCntr, typename SrcCntr>
constexpr void circular_array::SegTransfer(DstCntr&& dst_cntr,
                                           SrcCntr&& src_cntr, size_t dst_beg,
                                           size_t src_beg, size_t cnt)
    requires requires {
        requires IsCntr<meta::RemoveConstVolatileReference<DstCntr>>;
        requires IsCntr<meta::RemoveConstVolatileReference<SrcCntr>>;
    }
{
    using DstElem = meta::RemoveConst<typename DstCntr::Elem>;
    using SrcElem = meta::RemoveConst<typename SrcCntr::Elem>;

    using DstStorageView = lifecycle::ForwardStorageView<DstCntr, DstElem>;
    using SrcStorageView = lifecycle::ForwardStorageView<SrcCntr, SrcElem>;

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
        detail::DataTransfer_(
            meta::AutoValueWrapper<detail::Direction_::R>{},
            DstStorageView{ dst_data }, SrcStorageView{ src_data },
            dst_elem_stride, src_elem_stride, dst_rot, src_rot, dst_elem_cnt,
            src_elem_cnt, dst_slot_cnt, src_slot_cnt, dst_beg, src_beg, cnt);

        return;
    }

VEC_BW_MOVE:
    {
        detail::DataTransfer_(
            meta::AutoValueWrapper<detail::Direction_::L>{},
            DstStorageView{ dst_data }, SrcStorageView{ src_data },
            dst_elem_stride, src_elem_stride, dst_rot, src_rot, dst_elem_cnt,
            src_elem_cnt, dst_slot_cnt, src_slot_cnt, dst_beg, src_beg, cnt);

        return;
    }
}

template <meta::IsObjectType Elem>
template <
    meta::IsAnySame<circular_array::Cntr<Elem>&, circular_array::Cntr<Elem>&&>
        TargetCntr>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetElemCnt{ true },
        .GetMaxElemCnt{ true },

        .GetLBCursor{ true },
        .GetRBCursor{ true },

        .PeekL{ true },
        .PeekR{ true },

        .Refer{ true },
        .Derefer{ true },

        .Read{ true },
        .Write{ true },
        .ReadWrite{ true },

        .PushL{ true },
        .PushR{ true },
        .Insert{ true },

        .PopL{ true },
        .PopR{ true },
        .Erase{ true },
        .EraseAll{ true },

        .CopyCursor{ true },

        .AreEqualCursor{ true },
        .CompareCursor{ true },
        .GetCursorDist{ true },
        .GetCursorIdx{ true },

        .CursorStepL{ true },
        .CursorStepR{ true },

        .CursorAdvanceL{ true },
        .CursorAdvanceR{ true },
    }();
}

template <meta::IsObjectType Elem>
template <meta::IsAnySame<circular_array::Cntr<Elem> const&,
                          circular_array::Cntr<Elem> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr&>{}) &
           seq_cntr::capability::const_cap_flag;
}

template <meta::IsObjectType Elem>
template <
    meta::IsAnySame<circular_array::Cntr<Elem>&, circular_array::Cntr<Elem>&&>
        TargetCntr>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::empty_cap_flag;
}

template <meta::IsObjectType Elem>
template <meta::IsAnySame<circular_array::Cntr<Elem> const&,
                          circular_array::Cntr<Elem> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::non_const_cap_flag;
}

template <meta::IsObjectType Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_cap_flag;
}

template <meta::IsObjectType Elem>
constexpr seq_cntr::capability::Flag
circular_array::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_cap_flag;
}

template <meta::IsObjectType Elem>
template <meta::IsAnySame<
    circular_array::Cntr<Elem>&, circular_array::Cntr<Elem> const&,
    circular_array::Cntr<Elem>&&, circular_array::Cntr<Elem> const&&>
              TargetCntr>
constexpr meta::TypeWrapper<Elem> circular_array::Cntr<Elem>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <meta::IsObjectType Elem>
template <meta::IsAnySame<
    circular_array::Cntr<Elem>&, circular_array::Cntr<Elem> const&,
    circular_array::Cntr<Elem>&&, circular_array::Cntr<Elem> const&&>
              TargetCntr>
constexpr meta::TypeWrapper<circular_array::Cursor>
circular_array::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                          meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <meta::IsObjectType Elem>
constexpr void* circular_array::Cntr<Elem>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return const_cast<void*>(static_cast<void const*>(&self));
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetElemStride(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_stride;
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetIdxOffset(this Cntr const& self,
                                                          seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.rot;
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetElemCnt(this Cntr const& self,
                                                        seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_cnt;
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetMaxElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.slot_cnt;
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::GetLBCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->idx = static_cast<size_t>(-1);

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::GetRBCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->idx = self.elem_cnt;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::PeekL(
    this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
    resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
        resolving_packet) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = 0;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet == nullptr) { return; }

    Elem* elem{ elem_cnt == 0
                    ? nullptr
                    : (ReferElem)(data, elem_stride, slot_cnt, rot, 0) };

    if (elem == nullptr) {
        resolving_packet->state.resolved = true;
    } else {
        resolving_packet->ResolveFromView(
            lifecycle::ForwardStorageView<decltype(self), Elem>{ elem },
            meta::AutoValueWrapper<true>{}, meta::AutoValueWrapper<true>{});
    }
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::PeekR(
    this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
    resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
        resolving_packet) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = elem_cnt - 1;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet == nullptr) { return; }

    Elem* elem{ elem_cnt == 0 ? nullptr
                              : (ReferElem)(data, elem_stride, slot_cnt, rot,
                                            elem_cnt - 1) };

    if (elem == nullptr) {
        resolving_packet->state.resolved = true;
    } else {
        resolving_packet->ResolveFromView(
            lifecycle::ForwardStorageView<decltype(self), Elem>{ elem },
            meta::AutoValueWrapper<true>{}, meta::AutoValueWrapper<true>{});
    }
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::Refer(
    this auto&& self, seq_cntr::Tag, size_t idx, Cursor* dst_cursor,
    resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
        resolving_packet) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, elem_cnt));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = idx;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet == nullptr) { return; }

    Elem* elem{ idx < elem_cnt
                    ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                    : nullptr };

    if (elem == nullptr) {
        resolving_packet->state.resolved = true;
    } else {
        resolving_packet->ResolveFromView(
            lifecycle::ForwardStorageView<decltype(self), Elem>{ elem },
            meta::AutoValueWrapper<true>{}, meta::AutoValueWrapper<true>{});
    }
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::Derefer(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor,
    resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
        resolving_packet) {
    detail::CheckCursor_(self, pos_cursor);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(resolving_packet != nullptr);

    resolving_packet->AssertIsNotResovled();

    Elem* elem{ idx < elem_cnt
                    ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                    : nullptr };

    if (elem == nullptr) {
        resolving_packet->state.resolved = true;
    } else {
        resolving_packet->ResolveFromView(
            lifecycle::ForwardStorageView<decltype(self), Elem>{ elem },
            meta::AutoValueWrapper<true>{}, meta::AutoValueWrapper<true>{});
    }
}

namespace circular_array::detail {

enum struct ReadWriteOp_ : unsigned char {
    Read,
    Write,
    ReadWrite,
};

template <lifecycle::StorageSemantics cntr_semantics, meta::IsObjectType Elem,
          ReadWriteOp_ op,
          typename Endpoint>
constexpr void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (lifecycle::StorageView<cntr_semantics, Cntr<Elem>> self_view,
     meta::ValueWrapper<ReadWriteOp_, op>, size_t idx, size_t cnt,
     Endpoint&& endpoint, Cursor* dst_cursor)
    requires requires {
        requires(cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                 cntr_semantics == lifecycle::StorageSemantics::ConstLeftObj ||
                 cntr_semantics == lifecycle::StorageSemantics::RightObj ||
                 cntr_semantics == lifecycle::StorageSemantics::ConstRightObj);

        requires(op == ReadWriteOp_::Read || op == ReadWriteOp_::Write ||
                 op == ReadWriteOp_::ReadWrite);

        requires meta::Implies(
            op == ReadWriteOp_::Read,
            seq_endpoint::acceptor::IsAcceptorLike<Endpoint, Elem>);

        requires meta::Implies(
            op == ReadWriteOp_::Write,
            seq_endpoint::provider::IsProviderLike<Endpoint, Elem>);

        requires meta::Implies(
            op == ReadWriteOp_::ReadWrite,
            seq_endpoint::acceptor::IsAcceptorLike<Endpoint, Elem>);
    }
{
    auto&& self{ self_view.GetRef() };

    (CheckCntr_)(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(idx, cnt, elem_cnt));

    if constexpr ((op == ReadWriteOp_::Write ||
                   op == ReadWriteOp_::ReadWrite) &&
                  (cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                   cntr_semantics == lifecycle::StorageSemantics::RightObj)) {
#if ZETA_Core_DebugEnable
        self.stamp = debug_utils::revision::GetStamp();
#endif
    }

    if constexpr (op == ReadWriteOp_::Read || op == ReadWriteOp_::ReadWrite) {
        (EndpointTransfer_)(
            seq_endpoint::acceptor::Tag{},
            lifecycle::StorageView<cntr_semantics, Elem>{ data }, elem_stride,
            rot, elem_cnt, slot_cnt, idx, cnt, endpoint);
    } else if constexpr (op == ReadWriteOp_::Write) {
        (EndpointTransfer_)(
            seq_endpoint::provider::Tag{},
            lifecycle::StorageView<cntr_semantics, Elem>{ data }, elem_stride,
            rot, elem_cnt, slot_cnt, idx, cnt, endpoint);
    } else {
        static_assert(false);
    }

    idx += cnt;

    if (dst_cursor != nullptr) {
        dst_cursor->idx = idx;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

}  // namespace circular_array::detail

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::Read(this auto&& self, seq_cntr::Tag,
                                                Cursor const* pos_cursor,
                                                size_t cnt, Acceptor&& acceptor,
                                                Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::Read>{}, pos_cursor->idx,
        cnt, acceptor, dst_cursor);

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::Write(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor const* pos_cursor, size_t cnt, Provider&& provider,
    Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::Write>{}, pos_cursor->idx,
        cnt, provider, dst_cursor);

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::ReadWrite(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor const* pos_cursor, size_t cnt, Acceptor&& acceptor,
    Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::ReadWrite>{},
        pos_cursor->idx, cnt, acceptor, dst_cursor);

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxRead(this auto&& self, size_t idx,
                                                   size_t cnt,
                                                   Acceptor&& acceptor) {
    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::Read>{}, idx, cnt,
        acceptor, nullptr);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::IdxWrite(
    this meta::IsNonConstWithoutReference auto&& self, size_t idx, size_t cnt,
    Provider&& provider) {
    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::Write>{}, idx, cnt,
        provider, nullptr);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxReadWrite(
    this meta::IsNonConstWithoutReference auto&& self, size_t idx, size_t cnt,
    Acceptor&& acceptor) {
    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::ReadWrite>{}, idx, cnt,
        acceptor, nullptr);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::PushL(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Provider&& provider, Cursor* dst_beg_cursor,
    Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    self.IdxInsert(0, cnt, provider);

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->idx = 0;

#if ZETA_Core_DebugEnable
        dst_beg_cursor->stamp = self.stamp;
#endif
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->idx = cnt;

#if ZETA_Core_DebugEnable
        dst_end_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::PushR(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Provider&& provider, Cursor* dst_beg_cursor,
    Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    size_t old_elem_cnt{ self.elem_cnt };

    self.IdxInsert(old_elem_cnt, cnt, provider);

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->idx = old_elem_cnt;

#if ZETA_Core_DebugEnable
        dst_beg_cursor->stamp = self.stamp;
#endif
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->idx = self.elem_cnt;

#if ZETA_Core_DebugEnable
        dst_end_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::Insert(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& provider, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    size_t idx{ pos_cursor->idx };

    self.IdxInsert(idx, cnt, provider);

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif

    if (dst_cursor != nullptr) {
        dst_cursor->idx = idx + cnt;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void circular_array::Cntr<Elem>::IdxInsert(
    this meta::IsNonConstWithoutReference auto&& self, size_t idx, size_t cnt,
    Provider&& provider) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanInsert(idx, cnt, elem_cnt, slot_cnt));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    if (cnt == 0) { return; }

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx };

    self.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case utils::Choose2Result::Case0: {
        self.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;

        if (l_cnt < cnt) {
            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::ConstructingMemToObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, 0, cnt,
                l_cnt);

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                lifecycle::ConstructingMemToObjView<Elem>{ data }, elem_stride,
                rot, elem_cnt, slot_cnt, l_cnt, cnt - l_cnt, provider);

            detail::EndpointTransfer_(seq_endpoint::provider::Tag{},
                                      lifecycle::LeftObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt, cnt,
                                      l_cnt, provider);
        } else {
            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::ConstructingMemToObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, 0, cnt, cnt);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, cnt,
                cnt + cnt, l_cnt - cnt);

            detail::EndpointTransfer_(seq_endpoint::provider::Tag{},
                                      lifecycle::LeftObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, cnt, provider);
        }

        break;
    }

    case utils::Choose2Result::Case1: {
        if (r_cnt < cnt) {
            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::ConstructingMemToObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, l_cnt + cnt,
                l_cnt, r_cnt);

            detail::EndpointTransfer_(seq_endpoint::provider::Tag{},
                                      lifecycle::LeftObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, r_cnt, provider);

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                lifecycle::ConstructingMemToObjView<Elem>{ data }, elem_stride,
                rot, elem_cnt, slot_cnt, l_cnt + r_cnt, cnt - r_cnt, provider);
        } else {
            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::ConstructingMemToObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, l_cnt + r_cnt,
                l_cnt + r_cnt - cnt, cnt);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, l_cnt + cnt,
                l_cnt, r_cnt - cnt);

            detail::EndpointTransfer_(seq_endpoint::provider::Tag{},
                                      lifecycle::LeftObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, cnt, provider);
        }

        break;
    }

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::PopL(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    self.IdxErase(0, cnt, acceptor, dst_cursor);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::PopR(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    self.IdxErase(elem_cnt - cnt, cnt, acceptor, dst_cursor);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::Erase(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor) {
    detail::CheckCursor_(self, pos_cursor);

    size_t idx{ pos_cursor->idx };

    self.IdxErase(idx, cnt, acceptor, pos_cursor);
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::IdxErase(
    this meta::IsNonConstWithoutReference auto&& self, size_t idx, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    Elem* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(idx, cnt, elem_cnt));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->idx = idx;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        return;
    }

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx - cnt };

    unsigned long long random_seed{ utils::GetRandom() };

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case utils::Choose2Result::Case0: {
        if (l_cnt < cnt) {
            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                rot, elem_cnt, slot_cnt, l_cnt, cnt - l_cnt, acceptor);

            detail::EndpointTransfer_(seq_endpoint::acceptor::Tag{},
                                      lifecycle::RightObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt, cnt,
                                      l_cnt, acceptor);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                elem_stride, rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt,
                cnt, 0, l_cnt);
        } else {
            detail::EndpointTransfer_(seq_endpoint::acceptor::Tag{},
                                      lifecycle::RightObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, cnt, acceptor);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, cnt + cnt,
                cnt, l_cnt - cnt);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::L>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                elem_stride, rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt,
                cnt, 0, cnt);
        }

        rot += cnt;
        self.rot = rot = rot < slot_cnt ? rot : rot - slot_cnt;

        break;
    }

    case utils::Choose2Result::Case1: {
        if (r_cnt < cnt) {
            detail::EndpointTransfer_(seq_endpoint::acceptor::Tag{},
                                      lifecycle::RightObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, r_cnt, acceptor);

            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                rot, elem_cnt, slot_cnt, l_cnt + r_cnt, cnt - r_cnt, acceptor);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                elem_stride, rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt,
                l_cnt, l_cnt + cnt, r_cnt);
        } else {
            detail::EndpointTransfer_(seq_endpoint::acceptor::Tag{},
                                      lifecycle::RightObjView<Elem>{ data },
                                      elem_stride, rot, elem_cnt, slot_cnt,
                                      l_cnt, cnt, acceptor);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::RightObjView<Elem>{ data }, elem_stride, elem_stride,
                rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt, l_cnt,
                l_cnt + cnt, r_cnt - cnt);

            detail::DataTransfer_(
                meta::AutoValueWrapper<detail::Direction_::R>{},
                lifecycle::LeftObjView<Elem>{ data },
                lifecycle::DestructingObjToMemView<Elem>{ data }, elem_stride,
                elem_stride, rot, rot, elem_cnt, elem_cnt, slot_cnt, slot_cnt,
                l_cnt + r_cnt - cnt, l_cnt + r_cnt, cnt);
        }

        break;
    }

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    self.elem_cnt = (elem_cnt -= cnt);

    if (elem_cnt == 0) { self.rot = rot = 0; }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = idx;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void circular_array::Cntr<Elem>::EraseAll(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    self.IdxErase(0, self.elem_cnt, acceptor, nullptr);
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::CopyCursor(this Cntr const& self,
                                                      seq_cntr::Tag,
                                                      Cursor const* src_cursor_,
                                                      Cursor* dst_cursor) {
    Cursor const* src_cursor{ static_cast<Cursor const*>(src_cursor_) };

    detail::CheckCursor_(self, src_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->idx = src_cursor->idx;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
constexpr bool circular_array::Cntr<Elem>::AreEqualCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

template <meta::IsObjectType Elem>
constexpr comparison::Ordering circular_array::Cntr<Elem>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <meta::IsObjectType Elem>
constexpr size_t circular_array::Cntr<Elem>::GetCursorIdx(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_) {
    Cursor const* cursor{ static_cast<Cursor const*>(cursor_) };

    detail::CheckCursor_(self, cursor);

    return cursor->idx;
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::CursorStepL(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::CursorStepR(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::CursorAdvanceL(this Cntr const& self,
                                                          seq_cntr::Tag,
                                                          Cursor* cursor,
                                                          size_t step) {
    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

#if ZETA_Core_DebugEnable
    cursor->stamp = self.stamp;
#endif
}

template <meta::IsObjectType Elem>
constexpr void circular_array::Cntr<Elem>::CursorAdvanceR(this Cntr const& self,
                                                          seq_cntr::Tag,
                                                          Cursor* cursor,
                                                          size_t step) {
    detail::CheckCursor_(self, cursor);

    size_t elem_cnt{ self.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= elem_cnt - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

#if ZETA_Core_DebugEnable
    cursor->stamp = self.stamp;
#endif
}

static_assert(seq_cntr::IsSeqCntr<circular_array::Cntr<int>&>);
static_assert(seq_cntr::IsSeqCntr<circular_array::Cntr<int> const&>);
static_assert(seq_cntr::IsSeqCntr<circular_array::Cntr<int>&&>);
static_assert(seq_cntr::IsSeqCntr<circular_array::Cntr<int> const&&>);

}  // namespace zeta::core
