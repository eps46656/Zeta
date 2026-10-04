#pragma once

#include <deque>
#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

namespace debug_deque::detail {

template <meta::IsContainerElem Elem>
constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.deque != nullptr);
}

template <meta::IsContainerElem Elem>
constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<Elem> const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(cursor->idx, 1, deque->size()));
}

}  // namespace debug_deque::detail

template <meta::IsContainerElem Elem>
constexpr debug_deque::Cntr<Elem>::Cntr() : deque{ new std::deque<Elem*>{} } {
    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);
}

template <meta::IsContainerElem Elem>
constexpr debug_deque::Cntr<Elem>::~Cntr() {
    delete this->deque;

    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
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

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           seq_cntr::capability::const_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
debug_deque::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr void* debug_deque::Cntr<Elem>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    return const_cast<void*>(static_cast<void const*>(&self));
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<Elem> debug_deque::Cntr<Elem>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<debug_deque::Cursor>
debug_deque::Cntr<Elem>::GetCursorType(seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<debug_deque::Cursor>
debug_deque::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                       meta::TypeWrapper<Cntr const>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetElemCnt(this Cntr const& self,
                                                     seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->size();
}

template <meta::IsContainerElem Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetMaxElemCnt(this Cntr const& self,
                                                        seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->max_size();
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::GetLBCursor(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &self;
    dst_cursor->idx = static_cast<size_t>(-1);
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::GetRBCursor(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    auto* deque{ self.deque };

    dst_cursor->cntr = &self;
    dst_cursor->idx = deque->size();
}

template <meta::IsContainerElem Elem>
template <typename DstElem>
constexpr void debug_deque::Cntr<Elem>::PeekL(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
    }

    if (deque->empty()) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    Elem* elem{ deque->front() };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <meta::IsContainerElem Elem>
template <typename DstElem>
constexpr void debug_deque::Cntr<Elem>::PeekR(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = deque->size() - 1;
    }

    if (deque->empty()) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    Elem* elem{ deque->back() };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <meta::IsContainerElem Elem>
template <typename DstElem>
constexpr void debug_deque::Cntr<Elem>::Refer(
    this auto& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::op_check::CanDerefer(idx, 1, deque->size())) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    Elem* elem{ (*deque)[idx] };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

template <meta::IsContainerElem Elem>
template <typename DstElem>
constexpr void debug_deque::Cntr<Elem>::Derefer(
    this auto& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    lifecycle::DataLifeState dst_elem_life_state, DstElem* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, deque->size()));

    if (!seq_cntr::op_check::CanDerefer(idx, 1, deque->size())) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    Elem* elem{ (*deque)[idx] };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        lifecycle::DataTransfer(
            lifecycle::DeriveDataTransferOp(
                dst_elem_life_state, lifecycle::DataTransferSemantics::Copy),
            dst_elem, *elem);
    }
}

namespace debug_deque::detail {

template <seq_endpoint::Type type,
          typename Endpoint>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (auto&& self, Cursor const* pos_cursor, size_t cnt, Endpoint&& endpoint,
     Cursor* dst_cursor) {
    using Elem = meta::RemoveRef<decltype(self)>::Elem;

    static_assert(type == seq_endpoint::Type::Acceptor ||
                  type == seq_endpoint::Type::Provider);

    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    constexpr size_t elem_size{ sizeof(Elem) };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        Elem* elem{ (*deque)[idx] };

        if constexpr (type == seq_endpoint::Type::Acceptor) {
            seq_endpoint::acceptor::Transfer(
                endpoint,
                meta::IsLValueRef<decltype(self)>
                    ? lifecycle::DataTransferSemantics::Copy
                    : lifecycle::DataTransferSemantics::Move,
                elem, static_cast<ptrdiff_t>(elem_size), 1);
        } else if constexpr (type == seq_endpoint::Type::Provider) {
            seq_endpoint::provider::Transfer(
                endpoint, lifecycle::DataLifeState::Obj, elem,
                static_cast<ptrdiff_t>(elem_size), 1);
        } else {
            ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    }
}

}  // namespace debug_deque::detail

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::Read(this auto&& self, seq_cntr::Tag,
                                             Cursor const* pos_cursor,
                                             size_t cnt, Acceptor&& acceptor,
                                             Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(
        meta::Forward<Cntr>(self), pos_cursor, cnt, acceptor, dst_cursor);
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::Write(this Cntr& self, seq_cntr::Tag,
                                              Cursor const* pos_cursor,
                                              size_t cnt, Provider&& provider,
                                              Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, pos_cursor, cnt,
                                                     provider, dst_cursor);
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::ReadWrite(
    this Cntr& self, seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(self, pos_cursor, cnt,
                                                     acceptor, dst_cursor);
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::PushL(this Cntr& self, seq_cntr::Tag,
                                              size_t cnt, Provider&& provider,
                                              Cursor* dst_beg_cursor,
                                              Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    auto* deque{ self.deque };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        Elem* elem{ static_cast<Elem*>(std::malloc(sizeof(Elem))) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::DataLifeState::Mem, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx] = elem;
    }

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->cntr = &self;
        dst_beg_cursor->idx = 0;
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->cntr = &self;
        dst_end_cursor->idx = cnt;
    }
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::PushR(this Cntr& self, seq_cntr::Tag,
                                              size_t cnt, Provider&& provider,
                                              Cursor* dst_beg_cursor,
                                              Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    auto* deque{ self.deque };

    size_t old_elem_cnt{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_elem_cnt }; idx < deque->size(); ++idx) {
        Elem* elem{ static_cast<Elem*>(std::malloc(sizeof(Elem))) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::DataLifeState::Mem, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx] = elem;
    }

    if (dst_beg_cursor != nullptr) {
        dst_beg_cursor->cntr = &self;
        dst_beg_cursor->idx = old_elem_cnt;
    }

    if (dst_end_cursor != nullptr) {
        dst_end_cursor->cntr = &self;
        dst_end_cursor->idx = old_elem_cnt + cnt;
    }
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void debug_deque::Cntr<Elem>::Insert(this Cntr& self, seq_cntr::Tag,
                                               Cursor* pos_cursor, size_t cnt,
                                               Provider&& provider,
                                               Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(seq_cntr::op_check::CanInsert(
        idx, cnt, deque->size(), deque->max_size()));

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        Elem* elem{ static_cast<Elem*>(std::malloc(sizeof(Elem))) };

        seq_endpoint::provider::Transfer(
            provider, lifecycle::DataLifeState::Mem, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        (*deque)[idx + i] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx + cnt;
    }
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::PopL(this Cntr& self, seq_cntr::Tag,
                                             size_t cnt, Acceptor&& acceptor,
                                             Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= deque->size());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopL(cnt, deque->size()));

    while (0 < cnt--) {
        Elem* elem{ deque->front() };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DataTransferSemantics::Reloc, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        std::free(elem);

        deque->pop_front();
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
    }
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::PopR(this Cntr& self, seq_cntr::Tag,
                                             size_t cnt, Acceptor&& acceptor,
                                             Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopR(cnt, deque->size()));

    for (size_t i{ deque->size() - cnt }; i < deque->size(); ++i) {
        Elem* elem{ (*deque)[i] };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DataTransferSemantics::Reloc, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        std::free(elem);
    }

    deque->erase(deque->end() - static_cast<long long>(cnt), deque->end());

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = deque->size();
    }
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::Erase(this Cntr& self, seq_cntr::Tag,
                                              Cursor* pos_cursor, size_t cnt,
                                              Acceptor&& acceptor) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        Elem* elem{ (*deque)[idx] };

        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DataTransferSemantics::Reloc, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        std::free(elem);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void debug_deque::Cntr<Elem>::EraseAll(this Cntr& self, seq_cntr::Tag,
                                                 Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    for (Elem* elem : *deque) {
        seq_endpoint::acceptor::Transfer(
            acceptor, lifecycle::DataTransferSemantics::Reloc, elem,
            static_cast<ptrdiff_t>(sizeof(Elem)), 1);

        std::free(elem);
    }

    deque->clear();
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::CopyCursor(this Cntr const& self,
                                                   seq_cntr::Tag,
                                                   Cursor const* src_cursor,
                                                   Cursor* dst_cursor) {
    detail::CheckCursor_(self, src_cursor);

    dst_cursor->cntr = &self;
    dst_cursor->idx = src_cursor->idx;
}

template <meta::IsContainerElem Elem>
constexpr bool debug_deque::Cntr<Elem>::AreEqualCursor(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor const* cursor_a,
                                                       Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

template <meta::IsContainerElem Elem>
constexpr comparison::Ordering debug_deque::Cntr<Elem>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <meta::IsContainerElem Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <meta::IsContainerElem Elem>
constexpr size_t debug_deque::Cntr<Elem>::GetCursorIdx(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor const* cursor) {
    detail::CheckCntr_(self);

    return cursor->idx;
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::CursorStepL(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::CursorStepR(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <meta::IsContainerElem Elem>
constexpr void debug_deque::Cntr<Elem>::CursorAdvanceL(this Cntr const& self,
                                                       seq_cntr::Tag,
                                                       Cursor* cursor,
                                                       size_t step) {
    detail::CheckCntr_(self);

    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

template <meta::IsContainerElem Elem>
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
