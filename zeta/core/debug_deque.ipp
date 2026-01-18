#pragma once

#include <deque>
#include <zeta/core/compare.hpp>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::debug_deque {

template <bool EnWrite, typename ReaderWriter>
void ops::ReadWrite_(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor) ==
                          CheckResultCode::Success);

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        reader_writer(static_cast<Conditional<EnWrite, void*, void const*>>(
                          (*deque)[idx]),
                      width, 1);
    }
}

// -----------------------------------------------------------------------------

inline void ops::Init(Cntr* cntr) {
    ZETA_Core_DebugAssert(0 < cntr->width);

    cntr->deque = new std::deque<void*>;
}

inline void ops::Deinit(Cntr* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    delete cntr->deque;
}

inline size_t ops::GetCursorSize(Cntr const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    return sizeof(Cursor);
}

inline size_t ops::GetWidth(Cntr const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    return cntr->width;
}

inline size_t ops::GetSize(Cntr const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    return cntr->deque->size();
}

inline size_t ops::GetCapacity(Cntr const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    return cntr->deque->max_size();
}

inline void ops::GetLBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
}

inline void ops::GetRBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    dst_cursor->cntr = cntr;
    dst_cursor->idx = deque->size();
}

inline void* ops::PeekL(Cntr* cntr, Cursor* dst_cursor, void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->front() };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* ops::PeekL(Cntr const* cntr, Cursor* dst_cursor,
                              void* dst_elem) {
    return PeekL(const_cast<Cntr*>(cntr), dst_cursor, dst_elem);
}

inline void* ops::PeekR(Cntr* cntr, Cursor* dst_cursor, void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = deque->size() - 1;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->back() };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* ops::PeekR(Cntr const* cntr, Cursor* dst_cursor,
                              void* dst_elem) {
    return PeekR(const_cast<Cntr*>(cntr), dst_cursor, dst_elem);
}

inline void* ops::Access(Cntr* cntr, size_t idx, Cursor* dst_cursor,
                         void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::IsDereferable(idx, 1, deque->size())) { return nullptr; }

    void* elem{ (*deque)[idx] };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* ops::Access(Cntr const* cntr, size_t idx, Cursor* dst_cursor,
                               void* dst_elem) {
    return Access(const_cast<Cntr*>(cntr), idx, dst_cursor, dst_elem);
}

inline void* ops::Derefer(Cntr* cntr, Cursor const* pos_cursor,
                          void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor) ==
                          CheckResultCode::Success);

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    if (!seq_cntr::IsDereferable(idx, 1, deque->size())) { return nullptr; }

    void* elem{ (*deque)[idx] };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, cntr->width); }

    return elem;
}

inline void const* ops::Derefer(Cntr const* cntr, Cursor const* pos_cursor,
                                void* dst_elem) {
    return Derefer(const_cast<Cntr*>(cntr), pos_cursor, dst_elem);
}

// -----------------------------------------------------------------------------

template <typename Reader>
void ops::Read(
    Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ReadWrite_<false>(const_cast<Cntr*>(cntr), pos_cursor, cnt, reader,
                      dst_cursor);
}

template <typename Writer>
void ops::Write(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void ops::ReadWrite(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename Writer>
void* ops::PushL(
    Cntr* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[width] };
        writer(elem, width, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
    }

    return deque->empty() ? nullptr : deque->front();
}

template <typename Writer>
void* ops::PushR(
    Cntr* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    size_t old_size{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_size }; idx < deque->size(); ++idx) {
        void* elem{ new unsigned char[width] };
        writer(elem, width, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = old_size;
    }

    return old_size < deque->size() ? (*deque)[old_size] : nullptr;
}

template <typename Writer>
void* ops::Insert(
    Cntr* cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor) ==
                          CheckResultCode::Success);

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::IsInsertable(idx, cnt, deque->size(), deque->max_size()));

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        void* elem{ new unsigned char[width] };
        writer(elem, width, 1);
        (*deque)[idx + i] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx + cnt;
    }

    return idx < deque->size() ? (*deque)[idx] : nullptr;
}

// -----------------------------------------------------------------------------

inline void ops::PopL(Cntr* cntr, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->front());
        deque->pop_front();
    }
}

inline void ops::PopR(Cntr* cntr, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->back());
        deque->pop_back();
    }
}

inline void ops::Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor) ==
                          CheckResultCode::Success);

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        delete[] static_cast<unsigned char*>((*deque)[idx]);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

inline void ops::EraseAll(Cntr* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

// -----------------------------------------------------------------------------

inline void ops::CopyCursor(Cntr const* cntr, Cursor* dst_cursor,
                            Cursor const* src_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    ZETA_Core_DebugAssert(CheckCursor(cntr, src_cursor) ==
                          CheckResultCode::Success);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = src_cursor->idx;
}

inline bool ops::AreEqualCursor(Cntr const* cntr, Cursor const* cursor_a,
                                Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_a) == GetCursorIdx(cntr, cursor_b);
}

inline int ops::CompareCursor(Cntr const* cntr, Cursor const* cursor_a,
                              Cursor const* cursor_b) {
    return compare::Compare(GetCursorIdx(cntr, cursor_a) + 1,
                            GetCursorIdx(cntr, cursor_b) + 1);
}

inline size_t ops::GetCursorDist(Cntr const* cntr, Cursor const* cursor_a,
                                 Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

inline size_t ops::GetCursorIdx(Cntr const* cntr, Cursor const* cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    return cursor->idx;
}

inline void ops::CursorStepL(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceL(cntr, cursor, 1);
}

inline void ops::CursorStepR(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceR(cntr, cursor, 1);
}

inline void ops::CursorAdvanceL(Cntr const* cntr, Cursor* cursor, size_t step) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor) ==
                          CheckResultCode::Success);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

inline void ops::CursorAdvanceR(Cntr const* cntr, Cursor* cursor, size_t step) {
    ZETA_Core_DebugAssert(CheckCntr(cntr) == CheckResultCode::Success);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor) ==
                          CheckResultCode::Success);

    ZETA_Core_DebugAssert(step <= deque->size() - cursor->idx);

    cursor->idx += step;
}

// -----------------------------------------------------------------------------

inline int ops::CheckCntr(Cntr const* cntr) {
    if (!(cntr != nullptr)) { return CheckResultCode::NullCntr; }

    if (!(cntr->deque != nullptr)) { return CheckResultCode::NullCntr; }
    if (!(0 < cntr->width)) { return CheckResultCode::ZeroWidth; }

    return CheckResultCode::Success;
}

inline int ops::CheckCursor(Cntr const* cntr, Cursor const* cursor) {
    {
        int rc{ CheckCntr(cntr) };
        if (rc != CheckResultCode::Success) { return rc; }
    }

    auto* deque{ cntr->deque };

    if (!(seq_cntr::IsReferable(cursor->idx, 1, deque->size()))) {
        return CheckResultCode::CursorIdxOutOfRange;
    }

    return CheckResultCode::Success;
}

// -----------------------------------------------------------------------------

template <typename CntrImpl>
constexpr bool SeqCntrView<CntrImpl>::IsConst() {
    return core::IsConst<RemovePointer<CntrImpl>>;
}

template <typename CntrImpl>
constexpr seq_cntr::AbilityFlag
SeqCntrView<CntrImpl>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetWidth = true,
        .GetSize = true,
        .GetCapacity = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
        .Derefer = true,

        .Read = true,
        .Write = !IsConst(),
        .ReadWrite = !IsConst(),

        .PushL = !IsConst(),
        .PushR = !IsConst(),
        .Insert = !IsConst(),

        .PopL = !IsConst(),
        .PopR = !IsConst(),
        .Erase = !IsConst(),
        .EraseAll = !IsConst(),

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

template <typename CntrImpl>
constexpr seq_cntr::AbilityFlag
SeqCntrView<CntrImpl>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

template <typename CntrImpl>
constexpr seq_cntr::AbilityFlag
SeqCntrView<CntrImpl>::GetDynamicEnabledAbilityFlag(CntrImpl*) {
    return seq_cntr::empty_ability_flag;
}

template <typename CntrImpl>
constexpr seq_cntr::AbilityFlag
SeqCntrView<CntrImpl>::GetDynamicDisabledAbilityFlag(CntrImpl*) {
    return seq_cntr::empty_ability_flag;
}

template <typename CntrImpl>
constexpr size_t SeqCntrView<CntrImpl>::GetCursorSize(CntrImpl* cntr) {
    return ops::GetCursorSize(cntr);
}

template <typename CntrImpl>
size_t SeqCntrView<CntrImpl>::GetWidth(CntrImpl* cntr) {
    return ops::GetWidth(cntr);
}

template <typename CntrImpl>
size_t SeqCntrView<CntrImpl>::GetSize(CntrImpl* cntr) {
    return ops::GetSize(cntr);
}

template <typename CntrImpl>
size_t SeqCntrView<CntrImpl>::GetCapacity(CntrImpl* cntr) {
    return ops::GetCapacity(cntr);
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::GetLBCursor(CntrImpl* cntr, void* dst_cursor) {
    ops::GetLBCursor(cntr, static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::GetRBCursor(CntrImpl* cntr, void* dst_cursor) {
    ops::GetRBCursor(cntr, static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
Conditional<SeqCntrView<CntrImpl>::IsConst(), void const*, void*>
SeqCntrView<CntrImpl>::PeekL(CntrImpl* cntr, void* dst_cursor, void* dst_elem) {
    return ops::PeekL(cntr, static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename CntrImpl>
Conditional<SeqCntrView<CntrImpl>::IsConst(), void const*, void*>
SeqCntrView<CntrImpl>::PeekR(CntrImpl* cntr, void* dst_cursor, void* dst_elem) {
    return ops::PeekR(cntr, static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename CntrImpl>
Conditional<SeqCntrView<CntrImpl>::IsConst(), void const*, void*>
SeqCntrView<CntrImpl>::Access(CntrImpl* cntr, size_t idx, void* dst_cursor,
                              void* dst_elem) {
    return ops::Access(cntr, idx, static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename CntrImpl>
Conditional<SeqCntrView<CntrImpl>::IsConst(), void const*, void*>
SeqCntrView<CntrImpl>::Derefer(CntrImpl* cntr, void const* pos_cursor,
                               void* dst_elem) {
    return ops::Derefer(cntr, static_cast<Cursor const*>(pos_cursor), dst_elem);
}

template <typename CntrImpl>
template <typename Reader>
void SeqCntrView<CntrImpl>::Read(
    CntrImpl* cntr, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Read(cntr, static_cast<Cursor const*>(pos_cursor), cnt,
              Forward<Reader>(reader), static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename Writer, typename>
void SeqCntrView<CntrImpl>::Write(
    CntrImpl* cntr, void const* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Write(cntr, static_cast<Cursor const*>(pos_cursor), cnt,
               Forward<Writer>(writer), static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename ReaderWriter, typename>
void SeqCntrView<CntrImpl>::ReadWrite(
    CntrImpl* cntr, void const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::ReadWrite(cntr, static_cast<Cursor const*>(pos_cursor), cnt,
                   Forward<ReaderWriter>(reader_writer),
                   static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename Writer, typename>
void* SeqCntrView<CntrImpl>::PushL(
    CntrImpl* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushL(cntr, cnt, Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename Writer, typename>
void* SeqCntrView<CntrImpl>::PushR(
    CntrImpl* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushR(cntr, cnt, Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename Writer, typename>
void* SeqCntrView<CntrImpl>::Insert(
    CntrImpl* cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::Insert(cntr, static_cast<Cursor*>(pos_cursor), cnt,
                       Forward<Writer>(writer),
                       static_cast<Cursor*>(dst_cursor));
}

template <typename CntrImpl>
template <typename, typename>
void SeqCntrView<CntrImpl>::PopL(CntrImpl* cntr, size_t cnt) {
    ops::PopL(cntr, cnt);
}

template <typename CntrImpl>
template <typename, typename>
void SeqCntrView<CntrImpl>::PopR(CntrImpl* cntr, size_t cnt) {
    ops::PopR(cntr, cnt);
}

template <typename CntrImpl>
template <typename, typename>
void SeqCntrView<CntrImpl>::Erase(CntrImpl* cntr, void* pos_cursor,
                                  size_t cnt) {
    ops::Erase(cntr, static_cast<Cursor*>(pos_cursor), cnt);
}

template <typename CntrImpl>
template <typename, typename>
void SeqCntrView<CntrImpl>::EraseAll(CntrImpl* cntr) {
    ops::EraseAll(cntr);
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::CopyCursor(CntrImpl* cntr, void* dst_cursor,
                                       void const* src_cursor) {
    ops::CopyCursor(cntr, static_cast<Cursor*>(dst_cursor),
                    static_cast<Cursor const*>(src_cursor));
}

template <typename CntrImpl>
bool SeqCntrView<CntrImpl>::AreEqualCursor(CntrImpl* cntr, void const* cursor_a,
                                           void const* cursor_b) {
    return ops::AreEqualCursor(cntr, static_cast<Cursor const*>(cursor_a),
                               static_cast<Cursor const*>(cursor_b));
}

template <typename CntrImpl>
int SeqCntrView<CntrImpl>::CompareCursor(CntrImpl* cntr, void const* cursor_a,
                                         void const* cursor_b) {
    return ops::CompareCursor(cntr, static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

template <typename CntrImpl>
size_t SeqCntrView<CntrImpl>::GetCursorDist(CntrImpl* cntr,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    return ops::GetCursorDist(cntr, static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

template <typename CntrImpl>
size_t SeqCntrView<CntrImpl>::GetCursorIdx(CntrImpl* cntr, void const* cursor) {
    return ops::GetCursorIdx(cntr, static_cast<Cursor const*>(cursor));
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::CursorStepL(CntrImpl* cntr, void* cursor) {
    ops::CursorStepL(cntr, static_cast<Cursor*>(cursor));
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::CursorStepR(CntrImpl* cntr, void* cursor) {
    ops::CursorStepR(cntr, static_cast<Cursor*>(cursor));
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::CursorAdvanceL(CntrImpl* cntr, void* cursor,
                                           size_t step) {
    ops::CursorAdvanceL(cntr, static_cast<Cursor*>(cursor), step);
}

template <typename CntrImpl>
void SeqCntrView<CntrImpl>::CursorAdvanceR(CntrImpl* cntr, void* cursor,
                                           size_t step) {
    ops::CursorAdvanceR(cntr, static_cast<Cursor*>(cursor), step);
}

}  // namespace zeta::core::debug_deque
