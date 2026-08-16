#pragma once

#include <deque>
#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

namespace debug_deque::detail {

constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& cntr) {
    ZETA_Core_DebugAssert(cntr.deque != nullptr);
    ZETA_Core_DebugAssert(0 < cntr.elem_size);
}

constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& cntr, Cursor const* cursor) {
    CheckCntr_(cntr);

    auto* deque{ cntr.deque };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanRefer(cursor->idx, 1, deque->size()));
}

}  // namespace debug_deque::detail

constexpr void debug_deque::Cntr::Init(this Cntr& cntr) {
    ZETA_Core_DebugAssert(0 < cntr.elem_size);

    cntr.deque = new std::deque<void*>;
}

constexpr void debug_deque::Cntr::Deinit(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    delete cntr.deque;
}

constexpr void* debug_deque::Cntr::GetReferedInstPtr(this Cntr const& cntr) {
    return const_cast<void*>(static_cast<void const*>(&cntr));
}

constexpr size_t debug_deque::Cntr::GetCursorSize(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

constexpr size_t debug_deque::Cntr::GetElemSize(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

constexpr size_t debug_deque::Cntr::GetElemCnt(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.deque->size();
}

constexpr size_t debug_deque::Cntr::GetMaxElemCnt(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.deque->max_size();
}

constexpr void debug_deque::Cntr::GetLBCursor(this Cntr const& cntr,
                                              Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
}

constexpr void debug_deque::Cntr::GetRBCursor(this Cntr const& cntr,
                                              Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    auto* deque{ cntr.deque };

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = deque->size();
}

constexpr void debug_deque::Cntr::PeekL(
    this auto& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
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
            meta::IsConst<decltype(cntr)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::PeekR(
    this auto& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
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
            meta::IsConst<decltype(cntr)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::Refer(
    this auto& cntr, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
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
            meta::IsConst<decltype(cntr)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void debug_deque::Cntr::Derefer(
    this auto& cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr.deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
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
            meta::IsConst<decltype(cntr)>
                ? seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

namespace debug_deque::detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
     ReaderWriter&& reader_writer, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr.deque };

    size_t elem_size{ cntr.elem_size };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        if constexpr (EnWrite) {
            elem_stream::provider::Transfer(reader_writer,
                                            static_cast<void*>((*deque)[idx]),
                                            elem_size, elem_size, 1);
        } else {
            elem_stream::acceptor::Transfer(
                reader_writer, static_cast<void const*>((*deque)[idx]),
                elem_size, elem_size, 1);
        }
    }
}

}  // namespace debug_deque::detail

template <typename Reader>
constexpr void debug_deque::Cntr::Read(this Cntr const& cntr,
                                       Cursor const* pos_cursor, size_t cnt,
                                       Reader&& reader, Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr&>(cntr), pos_cursor, cnt, reader,
                              dst_cursor);
}

template <typename Writer>
constexpr void debug_deque::Cntr::Write(this Cntr& cntr,
                                        Cursor const* pos_cursor, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
constexpr void debug_deque::Cntr::ReadWrite(this Cntr& cntr,
                                            Cursor const* pos_cursor,
                                            size_t cnt,
                                            ReaderWriter&& reader_writer,
                                            Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <typename Writer>
constexpr void debug_deque::Cntr::PushL(this Cntr& cntr, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[elem_size] };
        elem_stream::provider::Transfer(writer, elem, elem_size, elem_size, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = 0;
    }
}

template <typename Writer>
constexpr void debug_deque::Cntr::PushR(this Cntr& cntr, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    size_t old_cnt{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_cnt }; idx < deque->size(); ++idx) {
        void* elem{ new unsigned char[elem_size] };
        elem_stream::provider::Transfer(writer, elem, elem_size, elem_size, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = old_cnt;
    }
}

template <typename Writer>
constexpr void debug_deque::Cntr::Insert(this Cntr& cntr, Cursor* pos_cursor,
                                         size_t cnt, Writer&& writer,
                                         Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanInsert(
        idx, cnt, deque->size(), deque->max_size()));

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        void* elem{ new unsigned char[elem_size] };
        elem_stream::provider::Transfer(writer, elem, elem_size, elem_size, 1);
        (*deque)[idx + i] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = idx + cnt;
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::PopL(this Cntr& cntr, size_t cnt,
                                       Reader&& reader) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanPopL(cnt, deque->size()));

    while (0 < cnt--) {
        void* elem{ deque->front() };

        elem_stream::acceptor::Transfer(reader, elem, elem_size, elem_size, 1);

        delete[] static_cast<unsigned char*>(elem);

        deque->pop_front();
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::PopR(this Cntr& cntr, size_t cnt,
                                       Reader&& reader) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };
    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanPopR(cnt, deque->size()));

    while (0 < cnt--) {
        void* elem{ deque->back() };

        elem_stream::acceptor::Transfer(reader, elem, elem_size, elem_size, 1);

        delete[] static_cast<unsigned char*>(elem);

        deque->pop_back();
    }
}

template <typename Reader>
constexpr void debug_deque::Cntr::Erase(this Cntr& cntr, Cursor* pos_cursor,
                                        size_t cnt, Reader&& reader) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr.deque };

    size_t elem_size{ cntr.elem_size };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanErase(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        void* elem{ (*deque)[idx] };

        elem_stream::acceptor::Transfer(reader, elem, elem_size, elem_size, 1);

        delete[] static_cast<unsigned char*>(elem);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

constexpr void debug_deque::Cntr::EraseAll(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

constexpr void debug_deque::Cntr::CopyCursor(this Cntr const& cntr,
                                             Cursor const* src_cursor,
                                             Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = src_cursor->idx;
}

constexpr bool debug_deque::Cntr::AreEqualCursor(this Cntr const& cntr,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b) {
    return cntr.GetCursorIdx(cursor_a) == cntr.GetCursorIdx(cursor_b);
}

constexpr comparison::Ordering debug_deque::Cntr::CompareCursor(
    this Cntr const& cntr, Cursor const* cursor_a, Cursor const* cursor_b) {
    return comparison::BasicCompare(
        meta::AutoValueWrapper<comparison::OpEnum::Order>{},
        cntr.GetCursorIdx(cursor_a) + 1, cntr.GetCursorIdx(cursor_b) + 1);
}

constexpr size_t debug_deque::Cntr::GetCursorDist(this Cntr const& cntr,
                                                  Cursor const* cursor_a,
                                                  Cursor const* cursor_b) {
    return cntr.GetCursorIdx(cursor_b) - cntr.GetCursorIdx(cursor_a);
}

constexpr size_t debug_deque::Cntr::GetCursorIdx(this Cntr const& cntr,
                                                 Cursor const* cursor) {
    detail::CheckCntr_(cntr);

    return cursor->idx;
}

constexpr void debug_deque::Cntr::CursorStepL(this Cntr const& cntr,
                                              Cursor* cursor) {
    cntr.CursorAdvanceL(cursor, 1);
}

constexpr void debug_deque::Cntr::CursorStepR(this Cntr const& cntr,
                                              Cursor* cursor) {
    cntr.CursorAdvanceR(cursor, 1);
}

constexpr void debug_deque::Cntr::CursorAdvanceL(this Cntr const& cntr,
                                                 Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

constexpr void debug_deque::Cntr::CursorAdvanceR(this Cntr const& cntr,
                                                 Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr.deque };

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= deque->size() - cursor->idx);

    cursor->idx += step;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<debug_deque::Cntr>::GetStaticEnabledCapabilityFlag() {
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
seq_cntr::CntrTraits<debug_deque::Cntr>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    debug_deque::Cntr>::GetDynamicEnabledCapabilityFlag(debug_deque::Cntr&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    debug_deque::Cntr>::GetDynamicDisabledCapabilityFlag(debug_deque::Cntr&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    debug_deque::Cntr const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CntrTraits<
               debug_deque::Cntr>::GetStaticEnabledCapabilityFlag() &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    debug_deque::Cntr const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<debug_deque::Cntr const>::GetDynamicEnabledCapabilityFlag(
    debug_deque::Cntr const&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<debug_deque::Cntr const>::GetDynamicDisabledCapabilityFlag(
    debug_deque::Cntr const&) {
    return seq_cntr::capability::empty_capability_flag;
}

}  // namespace zeta::core
