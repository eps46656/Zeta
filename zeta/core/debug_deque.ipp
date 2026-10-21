#pragma once

#include <deque>
#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/debug_utils/revision.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_utils.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

namespace debug_deque::detail {

template <meta::IsNonConstObj Elem>
constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.deque != nullptr);
}

template <meta::IsNonConstObj Elem>
constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(cursor->idx, 1, deque->size()));
}

}  // namespace debug_deque::detail

template <meta::IsNonConstObj Elem>
constexpr debug_deque::Cntr<Elem>::Cntr()
    : deque{ new std::deque<Elem*>{} }
#if ZETA_Core_DebugEnable
      ,
      stamp{ debug_utils::revision::GetStamp() }
#endif
{
}

template <meta::IsNonConstObj Elem>
constexpr debug_deque::Cntr<Elem>::~Cntr() {
    delete this->deque;
}

template <meta::IsNonConstObj Elem>
template <meta::IsAnySame<debug_deque::Cntr<Elem>&, debug_deque::Cntr<Elem>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetElemCnt    = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer   = true,
        .Derefer = true,

        .Read      = true,
        .Write     = true,
        .ReadWrite = true,

        .PushL  = true,
        .PushR  = true,
        .Insert = true,

        .PopL     = true,
        .PopR     = true,
        .Erase    = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor  = true,
        .GetCursorDist  = true,
        .GetCursorIdx   = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <meta::IsNonConstObj Elem>
template <meta::IsAnySame<debug_deque::Cntr<Elem> const&,
                          debug_deque::Cntr<Elem> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr&>{}) &
           seq_cntr::capability::const_capability_flag;
}

template <meta::IsNonConstObj Elem>
template <meta::IsAnySame<debug_deque::Cntr<Elem>&, debug_deque::Cntr<Elem>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsNonConstObj Elem>
template <meta::IsAnySame<debug_deque::Cntr<Elem> const&,
                          debug_deque::Cntr<Elem> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::non_const_capability_flag;
}

template <meta::IsNonConstObj Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsNonConstObj Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsNonConstObj Elem>
constexpr void* debug_deque::Cntr<Elem>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    return const_cast<void*>(static_cast<void const*>(&self));
}

template <meta::IsNonConstObj Elem>
template <
    meta::IsAnySame<debug_deque::Cntr<Elem>&, debug_deque::Cntr<Elem> const&,
                    debug_deque::Cntr<Elem>&&, debug_deque::Cntr<Elem> const&&>
        TargetCntr>
constexpr meta::TypeWrapper<Elem> debug_deque::Cntr<Elem>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <meta::IsNonConstObj Elem>
template <
    meta::IsAnySame<debug_deque::Cntr<Elem>&, debug_deque::Cntr<Elem> const&,
                    debug_deque::Cntr<Elem>&&, debug_deque::Cntr<Elem> const&&>
        TargetCntr>
constexpr meta::TypeWrapper<debug_deque::Cursor>
debug_deque::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                       meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <meta::IsNonConstObj Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetElemCnt(this Cntr const& self,
                                                     seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->size();
}

template <meta::IsNonConstObj Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetMaxElemCnt(this Cntr const& self,
                                                        seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->max_size();
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::GetLBCursor(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->idx = static_cast<size_t>(-1);

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::GetRBCursor(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    auto* deque{ self.deque };

    dst_cursor->idx = deque->size();

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsNonConstObj Elem>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void debug_deque::Cntr<Elem>::PeekL(
    this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    if (dst_cursor != nullptr) {
        dst_cursor->idx = 0;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    Elem* elem{ deque->empty() ? nullptr : deque->back() };

    lifecycle::ForwardStorageView<decltype(self), Elem> elem_view{ elem };

    if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

    if (!lazy_copy_elem && dst_elem.ptr != nullptr) {
        lifecycle::StorageTransfer(dst_elem, elem_view);
    }
}

template <meta::IsNonConstObj Elem>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void debug_deque::Cntr<Elem>::PeekR(
    this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    if (dst_cursor != nullptr) {
        dst_cursor->idx = deque->size() - 1;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    Elem* elem{ deque->empty() ? nullptr : deque->back() };

    lifecycle::ForwardStorageView<decltype(self), Elem> elem_view{ elem };

    if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

    if (!lazy_copy_elem && dst_elem.ptr != nullptr) {
        lifecycle::StorageTransfer(dst_elem, elem_view);
    }
}

template <meta::IsNonConstObj Elem>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void debug_deque::Cntr<Elem>::Refer(
    this auto&& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->idx = idx;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    Elem* elem{ seq_cntr::op_check::CanDerefer(idx, 1, deque->size())
                    ? (*deque)[idx]
                    : nullptr };

    lifecycle::ForwardStorageView<decltype(self), Elem> elem_view{ elem };

    if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

    if (!lazy_copy_elem && dst_elem.ptr != nullptr) {
        lifecycle::StorageTransfer(dst_elem, elem_view);
    }
}

template <meta::IsNonConstObj Elem>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void debug_deque::Cntr<Elem>::Derefer(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, lifecycle::DynamicStorageView<Elem>* dst_elem_view,
    DstStorageView dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, deque->size()));

    Elem* elem{ (*deque)[idx] };

    lifecycle::ForwardStorageView<decltype(self), Elem> elem_view{ elem };

    if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

    if (!lazy_copy_elem && dst_elem.ptr != nullptr) {
        lifecycle::StorageTransfer(dst_elem, elem_view);
    }
}

namespace debug_deque::detail {

enum struct ReadWriteOpType_ : unsigned char {
    Read,
    Write,
    ReadWrite,
};

template <lifecycle::StorageSemantics cntr_semantics, meta::IsNonConstObj Elem,
          ReadWriteOpType_ op_type,
          seq_endpoint::IsEndpoint<Elem> Endpoint>
constexpr void ReadWriteOp_  // NOLINT(misc-use-internal-linkage)
    (lifecycle::StorageView<cntr_semantics, Cntr<Elem>> self_view,
     meta::ValueWrapper<ReadWriteOpType_, op_type>, Cursor const* pos_cursor,
     size_t cnt, Endpoint&& endpoint, Cursor* dst_cursor) {
    static_assert(cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                  cntr_semantics == lifecycle::StorageSemantics::ConstLeftObj ||
                  cntr_semantics == lifecycle::StorageSemantics::RightObj ||
                  cntr_semantics == lifecycle::StorageSemantics::ConstRightObj);

    static_assert(op_type == ReadWriteOpType_::Read ||
                  op_type == ReadWriteOpType_::Write ||
                  op_type == ReadWriteOpType_::ReadWrite);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_view.ptr != nullptr);

    auto& self{ *self_view.ptr };

    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(beg, cnt, deque->size()));

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        Elem* elem{ (*deque)[idx] };

        if constexpr (op_type == ReadWriteOpType_::Read ||
                      op_type == ReadWriteOpType_::ReadWrite) {
            seq_endpoint::acceptor::Transfer(
                endpoint, lifecycle::StorageView<cntr_semantics, Elem>{ elem },
                sizeof(Elem), 1);
        } else if constexpr (op_type == ReadWriteOpType_::Write) {
            seq_endpoint::provider::Transfer(
                endpoint, lifecycle::StorageView<cntr_semantics, Elem>{ elem },
                sizeof(Elem), 1);
        } else {
            static_assert(false);
        }
    }

    if constexpr ((op_type == ReadWriteOpType_::Write ||
                   op_type == ReadWriteOpType_::ReadWrite) &&
                  (cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                   cntr_semantics == lifecycle::StorageSemantics::RightObj)) {
#if ZETA_Core_DebugEnable
        self.stamp        = debug_utils::revision::GetStamp();
        pos_cursor->stamp = self.stamp;
#endif
    }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = beg + cnt;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

}  // namespace debug_deque::detail

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::Read(this auto&& self, seq_cntr::Tag,
                                             Cursor const* pos_cursor,
                                             size_t cnt, Acceptor&& acceptor,
                                             Cursor* dst_cursor) {
    detail::ReadWriteOp_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOpType_::Read>{}, pos_cursor,
        cnt, acceptor, dst_cursor);
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::Write(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor const* pos_cursor, size_t cnt, Provider&& provider,
    Cursor* dst_cursor) {
    detail::ReadWriteOp_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOpType_::Write>{}, pos_cursor,
        cnt, provider, dst_cursor);
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::ReadWrite(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor const* pos_cursor, size_t cnt, Acceptor&& acceptor,
    Cursor* dst_cursor) {
    detail::ReadWriteOp_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOpType_::ReadWrite>{},
        pos_cursor, cnt, acceptor, dst_cursor);
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::PushL(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Provider&& provider, Cursor* dst_beg_cursor, Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    auto* deque{ self.deque };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        Elem* elem{ debug_utils::memory::Malloc<Elem>(sizeof(Elem)) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx] = elem;
    }

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

template <meta::IsNonConstObj Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::PushR(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Provider&& provider, Cursor* dst_beg_cursor, Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    auto* deque{ self.deque };

    size_t old_elem_cnt{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_elem_cnt }; idx < deque->size(); ++idx) {
        Elem* elem{ debug_utils::memory::Malloc<Elem>(sizeof(Elem)) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx] = elem;
    }

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->idx = old_elem_cnt;

#if ZETA_Core_DebugEnable
        dst_beg_cursor->stamp = self.stamp;
#endif
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->idx = old_elem_cnt + cnt;

#if ZETA_Core_DebugEnable
        dst_end_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::Insert(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& provider, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(seq_cntr::op_check::CanInsert(
        idx, cnt, deque->size(), deque->max_size()));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        Elem* elem{ debug_utils::memory::Malloc<Elem>(sizeof(Elem)) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx + i] = elem;
    }

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

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::PopL(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopL(cnt, deque->size()));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    while (0 < cnt--) {
        Elem* elem{ deque->front() };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DestructingObjToMemView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        debug_utils::memory::Free(elem);

        deque->pop_front();
    }

    if (dst_cursor != nullptr) {
        dst_cursor->idx = 0;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::PopR(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopR(cnt, deque->size()));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    for (size_t i{ deque->size() - cnt }; i < deque->size(); ++i) {
        Elem* elem{ (*deque)[i] };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DestructingObjToMemView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        debug_utils::memory::Free(elem);
    }

    deque->erase(deque->end() - static_cast<long long>(cnt), deque->end());

    if (dst_cursor != nullptr) {
        dst_cursor->idx = deque->size();

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::Erase(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(beg, cnt, deque->size()));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        Elem* elem{ (*deque)[idx] };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DestructingObjToMemView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        debug_utils::memory::Free(elem);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif
}

template <meta::IsNonConstObj Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::EraseAll(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    for (Elem* elem : *deque) {
        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DestructingObjToMemView{ elem },
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        debug_utils::memory::Free(elem);
    }

    deque->clear();
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::CopyCursor(this Cntr const& self,
                                                   seq_cntr::Tag,
                                                   Cursor const* src_cursor,
                                                   Cursor* dst_cursor) {
    detail::CheckCursor_(self, src_cursor);

    dst_cursor->idx = src_cursor->idx;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <meta::IsNonConstObj Elem>
constexpr bool debug_deque::Cntr<Elem>::AreEqualCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor const* cursor_a,
                                                       Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

template <meta::IsNonConstObj Elem>
constexpr comparison::Ordering debug_deque::Cntr<Elem>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <meta::IsNonConstObj Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <meta::IsNonConstObj Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetCursorIdx(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor const* cursor) {
    detail::CheckCntr_(self);

    return cursor->idx;
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::CursorStepL(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::CursorStepR(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::CursorAdvanceL(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor,
                                                       size_t step) {
    detail::CheckCntr_(self);

    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

template <meta::IsNonConstObj Elem>
constexpr void debug_deque::Cntr<Elem>::CursorAdvanceR(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor,
                                                       size_t step) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <=
                                            deque->size() - cursor->idx);

    cursor->idx += step;
}

constexpr debug_deque::Cursor::Cursor(seq_cntr::CursorLimit const& src_cursor) {
    *this = src_cursor;
}

constexpr debug_deque::Cursor::operator seq_cntr::CursorLimit(
    this Cursor const& self) {
    seq_cntr::CursorLimit dst_cursor;

    std::memcpy(&dst_cursor, &self, sizeof(Cursor));

    return dst_cursor;
}

constexpr debug_deque::Cursor& debug_deque::Cursor::operator=(
    seq_cntr::CursorLimit const& src_cursor) {
    std::memcpy(this, &src_cursor, sizeof(Cursor));
    return *this;
}

}  // namespace zeta::core
