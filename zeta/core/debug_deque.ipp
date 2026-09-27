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

constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.deque != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < self.elem_size);
}

constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    auto* deque{ self.deque };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanRefer(cursor->idx, 1, deque->size()));
}

}  // namespace debug_deque::detail

constexpr debug_deque::Cntr::Cntr(size_t elem_size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_size);

    this->elem_size = elem_size;

    this->deque = new std::deque<void*>;

    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);
}

constexpr debug_deque::Cntr::~Cntr() {
    delete this->deque;

    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetStaticEnabledCapabilityFlag(seq_cntr::Tag,
                                                  meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
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

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetStaticDisabledCapabilityFlag(seq_cntr::Tag,
                                                   meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
debug_deque::Cntr::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr void* debug_deque::Cntr::GetReferedInstPtr(this Cntr const& self,
                                                     seq_cntr::Tag) {
    return const_cast<void*>(static_cast<void const*>(&self));
}

constexpr meta::TypeWrapper<debug_deque::Cursor>
debug_deque::Cntr::GetCursorType(seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

constexpr meta::TypeWrapper<debug_deque::Cursor>
debug_deque::Cntr::GetCursorType(seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return {};
}

constexpr size_t debug_deque::Cntr::GetCursorSize(seq_cntr::Tag) {
    return sizeof(Cursor);
}

constexpr size_t debug_deque::Cntr::GetElemSize(this Cntr const& self,
                                                seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_size;
}

constexpr size_t debug_deque::Cntr::GetElemCnt(this Cntr const& self,
                                               seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->size();
}

constexpr size_t debug_deque::Cntr::GetMaxElemCnt(this Cntr const& self,
                                                  seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.deque->max_size();
}

constexpr void debug_deque::Cntr::GetLBCursor(this Cntr const& self,
                                              seq_cntr::Tag,
                                              Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &self;
    dst_cursor->idx = static_cast<size_t>(-1);
}

constexpr void debug_deque::Cntr::GetRBCursor(this Cntr const& self,
                                              seq_cntr::Tag,
                                              Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    auto* deque{ self.deque };

    dst_cursor->cntr = &self;
    dst_cursor->idx = deque->size();
}

constexpr void debug_deque::Cntr::PeekL(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

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

    void* elem{ deque->front() };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::PeekR(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

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

    void* elem{ deque->back() };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::Refer(
    this auto& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::check_operation::CanDerefer(idx, 1, deque->size())) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    void* elem{ (*deque)[idx] };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::Derefer(
    this auto& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    void* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, deque->size()));

    if (!seq_cntr::check_operation::CanDerefer(idx, 1, deque->size())) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        return;
    }

    void* elem{ (*deque)[idx] };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            meta::IsConst<decltype(self)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

namespace debug_deque::detail {

template <seq_endpoint::Type type, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr& self, Cursor const* pos_cursor, size_t cnt,
     ReaderWriter&& reader_writer, Cursor* dst_cursor) {
    static_assert(type == seq_endpoint::Type::Acceptor ||
                  type == seq_endpoint::Type::Provider ||
                  type == seq_endpoint::Type::AcceptorProvider);

    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t elem_size{ self.elem_size };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanDerefer(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        void* elem{ (*deque)[idx] };

        if constexpr (type == seq_endpoint::Type::Acceptor) {
            seq_endpoint::acceptor::Transfer(reader_writer, elem, elem_size,
                                             static_cast<ptrdiff_t>(elem_size),
                                             1);
        } else if constexpr (type == seq_endpoint::Type::Provider) {
            seq_endpoint::provider::Transfer(reader_writer, elem, elem_size,
                                             static_cast<ptrdiff_t>(elem_size),
                                             1);
        } else if constexpr (type == seq_endpoint::Type::AcceptorProvider) {
            seq_endpoint::acceptor_provider::Transfer(
                reader_writer, elem, elem_size,
                static_cast<ptrdiff_t>(elem_size), 1);
        } else {
            ZETA_Core_Unreachable();
        }
    }
}

}  // namespace debug_deque::detail

template <typename Reader>
constexpr void debug_deque::Cntr::Read(this Cntr const& self, seq_cntr::Tag,
                                       Cursor const* pos_cursor, size_t cnt,
                                       Reader&& reader, Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(
        const_cast<Cntr&>(self), pos_cursor, cnt, reader, dst_cursor);
}

template <typename Writer>
constexpr void debug_deque::Cntr::Write(this Cntr& self, seq_cntr::Tag,
                                        Cursor const* pos_cursor, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, pos_cursor, cnt,
                                                     writer, dst_cursor);
}

template <typename ReaderWriter>
constexpr void debug_deque::Cntr::ReadWrite(this Cntr& self, seq_cntr::Tag,
                                            Cursor const* pos_cursor,
                                            size_t cnt,
                                            ReaderWriter&& reader_writer,
                                            Cursor* dst_cursor) {
    detail::ReadWrite_<seq_endpoint::Type::AcceptorProvider>(
        self, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <typename Writer>
constexpr void debug_deque::Cntr::PushL(this Cntr& self, seq_cntr::Tag,
                                        size_t cnt, Writer&& writer,
                                        Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[elem_size] };
        seq_endpoint::provider::Transfer(writer, elem, elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
    }
}

template <typename Writer>
constexpr void debug_deque::Cntr::PushR(this Cntr& self, seq_cntr::Tag,
                                        size_t cnt, Writer&& writer,
                                        Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    size_t old_cnt{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_cnt }; idx < deque->size(); ++idx) {
        void* elem{ new unsigned char[elem_size] };
        seq_endpoint::provider::Transfer(writer, elem, elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = old_cnt;
    }
}

template <typename Writer>
constexpr void debug_deque::Cntr::Insert(this Cntr& self, seq_cntr::Tag,
                                         Cursor* pos_cursor, size_t cnt,
                                         Writer&& writer, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanInsert(idx, cnt, deque->size(),
                                             deque->max_size()));

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        void* elem{ new unsigned char[elem_size] };
        seq_endpoint::provider::Transfer(writer, elem, elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);
        (*deque)[idx + i] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx + cnt;
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::PopL(this Cntr& self, seq_cntr::Tag,
                                       size_t cnt, Reader&& reader) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= deque->size());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPopL(cnt, deque->size()));

    while (0 < cnt--) {
        void* elem{ deque->front() };

        seq_endpoint::acceptor::Transfer(reader, elem, elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);

        delete[] static_cast<unsigned char*>(elem);

        deque->pop_front();
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::PopR(this Cntr& self, seq_cntr::Tag,
                                       size_t cnt, Reader&& reader) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };
    size_t elem_size{ self.elem_size };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPopR(cnt, deque->size()));

    for (size_t i{ deque->size() - cnt }; i < deque->size(); ++i) {
        seq_endpoint::acceptor::Transfer(reader, (*deque)[i], elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);
    }

    while (0 < cnt--) {
        void* elem{ deque->back() };

        delete[] static_cast<unsigned char*>(elem);

        deque->pop_back();
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::Erase(this Cntr& self, seq_cntr::Tag,
                                        Cursor* pos_cursor, size_t cnt,
                                        Reader&& reader) {
    detail::CheckCursor_(self, pos_cursor);

    auto* deque{ self.deque };

    size_t elem_size{ self.elem_size };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanErase(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        void* elem{ (*deque)[idx] };

        seq_endpoint::acceptor::Transfer(reader, elem, elem_size,
                                         static_cast<ptrdiff_t>(elem_size), 1);

        delete[] static_cast<unsigned char*>(elem);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

constexpr void debug_deque::Cntr::EraseAll(this Cntr& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

constexpr void debug_deque::Cntr::CopyCursor(this Cntr const& self,
                                             seq_cntr::Tag,
                                             Cursor const* src_cursor,
                                             Cursor* dst_cursor) {
    detail::CheckCursor_(self, src_cursor);

    dst_cursor->cntr = &self;
    dst_cursor->idx = src_cursor->idx;
}

constexpr bool debug_deque::Cntr::AreEqualCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

constexpr comparison::Ordering debug_deque::Cntr::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

constexpr size_t debug_deque::Cntr::GetCursorDist(this Cntr const& self,
                                                  seq_cntr::Tag,
                                                  Cursor const* cursor_a,
                                                  Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

constexpr size_t debug_deque::Cntr::GetCursorIdx(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor) {
    detail::CheckCntr_(self);

    return cursor->idx;
}

constexpr void debug_deque::Cntr::CursorStepL(this Cntr const& self,
                                              seq_cntr::Tag, Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

constexpr void debug_deque::Cntr::CursorStepR(this Cntr const& self,
                                              seq_cntr::Tag, Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

constexpr void debug_deque::Cntr::CursorAdvanceL(this Cntr const& self,
                                                 seq_cntr::Tag, Cursor* cursor,
                                                 size_t step) {
    detail::CheckCntr_(self);

    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

constexpr void debug_deque::Cntr::CursorAdvanceR(this Cntr const& self,
                                                 seq_cntr::Tag, Cursor* cursor,
                                                 size_t step) {
    detail::CheckCntr_(self);

    auto* deque{ self.deque };

    detail::CheckCursor_(self, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <=
                                            deque->size() - cursor->idx);

    cursor->idx += step;
}

}  // namespace zeta::core
