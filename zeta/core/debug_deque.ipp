#pragma once

#include <deque>
#include <zeta/core/compare.hpp>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

namespace debug_deque::detail {

inline void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(cntr->deque != nullptr);
    ZETA_Core_DebugAssert(0 < cntr->elem_size);
}

inline void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr, Cursor const* cursor) {
    CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(cursor->idx, 1, deque->size()));
}

}  // namespace debug_deque::detail

inline void debug_deque::Init(Cntr* cntr) {
    ZETA_Core_DebugAssert(0 < cntr->elem_size);

    cntr->deque = new std::deque<void*>;
}

inline void debug_deque::Deinit(Cntr* cntr) {
    detail::CheckCntr_(cntr);

    delete cntr->deque;
}

inline size_t debug_deque::GetCursorSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

inline size_t debug_deque::GetElemSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->elem_size;
}

inline size_t debug_deque::GetElemCnt(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->deque->size();
}

inline size_t debug_deque::GetMaxElemCnt(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->deque->max_size();
}

inline void debug_deque::GetLBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
}

inline void debug_deque::GetRBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    dst_cursor->cntr = cntr;
    dst_cursor->idx = deque->size();
}

inline void* debug_deque::PeekL(Cntr const* cntr, bool lazy_cpy_elem,
                                Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->front() };

    if (!lazy_cpy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }

    return elem;
}

inline void* debug_deque::PeekR(Cntr const* cntr, bool lazy_cpy_elem,
                                Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = deque->size() - 1;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->back() };

    if (!lazy_cpy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }

    return elem;
}

inline void* debug_deque::Access(Cntr const* cntr, size_t idx,
                                 bool lazy_copy_elem, Cursor* dst_cursor,
                                 void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::IsDereferable(idx, 1, deque->size())) { return nullptr; }

    void* elem{ (*deque)[idx] };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }

    return elem;
}

inline void* debug_deque::Derefer(Cntr const* cntr, Cursor const* pos_cursor,
                                  bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    if (!seq_cntr::IsDereferable(idx, 1, deque->size())) { return nullptr; }

    void* elem{ (*deque)[idx] };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr->elem_size);
    }

    return elem;
}

namespace debug_deque::detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
     ReaderWriter&&
         reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
     Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t elem_size{ cntr->elem_size };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        reader_writer(
            static_cast<meta::Conditional<EnWrite, void*, void const*>>(
                (*deque)[idx]),
            elem_size, 1);
    }
}

}  // namespace debug_deque::detail

template <typename Reader>
void debug_deque::Read(
    Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr*>(cntr), pos_cursor, cnt, reader,
                              dst_cursor);
}

template <typename Writer>
void debug_deque::Write(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void debug_deque::ReadWrite(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <typename Writer>
void* debug_deque::PushL(
    Cntr* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[elem_size] };
        writer(elem, elem_size, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
    }

    return deque->empty() ? nullptr : deque->front();
}

template <typename Writer>
void* debug_deque::PushR(
    Cntr* cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    size_t old_cnt{ deque->size() };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_cnt }; idx < deque->size(); ++idx) {
        void* elem{ new unsigned char[elem_size] };
        writer(elem, elem_size, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = old_cnt;
    }

    return old_cnt < deque->size() ? (*deque)[old_cnt] : nullptr;
}

template <typename Writer>
void* debug_deque::Insert(
    Cntr* cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    ZETA_Core_PrintCurPos;

    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };
    size_t elem_size{ cntr->elem_size };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::IsInsertable(idx, cnt, deque->size(), deque->max_size()));

    deque->insert(deque->begin() + static_cast<long long>(idx), cnt, nullptr);

    for (size_t i{ 0 }; i < cnt; ++i) {
        void* elem{ new unsigned char[elem_size] };
        writer(elem, elem_size, 1);
        (*deque)[idx + i] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx + cnt;
    }

    return idx < deque->size() ? (*deque)[idx] : nullptr;
}

inline void debug_deque::PopL(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->front());
        deque->pop_front();
    }
}

inline void debug_deque::PopR(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->back());
        deque->pop_back();
    }
}

inline void debug_deque::Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        delete[] static_cast<unsigned char*>((*deque)[idx]);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

inline void debug_deque::EraseAll(Cntr* cntr) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

inline void debug_deque::CopyCursor(Cntr const* cntr, Cursor const* src_cursor,
                                    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = src_cursor->idx;
}

inline bool debug_deque::AreEqualCursor(Cntr const* cntr,
                                        Cursor const* cursor_a,
                                        Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_a) == (GetCursorIdx)(cntr, cursor_b);
}

inline int debug_deque::CompareCursor(Cntr const* cntr, Cursor const* cursor_a,
                                      Cursor const* cursor_b) {
    return compare::BasicCompare((GetCursorIdx)(cntr, cursor_a) + 1,
                                 (GetCursorIdx)(cntr, cursor_b) + 1);
}

inline size_t debug_deque::GetCursorDist(Cntr const* cntr,
                                         Cursor const* cursor_a,
                                         Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_b) - (GetCursorIdx)(cntr, cursor_a);
}

inline size_t debug_deque::GetCursorIdx(Cntr const* cntr,
                                        Cursor const* cursor) {
    detail::CheckCntr_(cntr);

    return cursor->idx;
}

inline void debug_deque::CursorStepL(Cntr const* cntr, Cursor* cursor) {
    (CursorAdvanceL)(cntr, cursor, 1);
}

inline void debug_deque::CursorStepR(Cntr const* cntr, Cursor* cursor) {
    (CursorAdvanceR)(cntr, cursor, 1);
}

inline void debug_deque::CursorAdvanceL(Cntr const* cntr, Cursor* cursor,
                                        size_t step) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

inline void debug_deque::CursorAdvanceR(Cntr const* cntr, Cursor* cursor,
                                        size_t step) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= deque->size() - cursor->idx);

    cursor->idx += step;
}

inline void*
seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetReferedInst(
    debug_deque::Cntr const* cntr) {
    return const_cast<debug_deque::Cntr*>(cntr);
}

constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<debug_deque::Cntr, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
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

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    debug_deque::Cntr const, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::CntrTraits<debug_deque::Cntr,
                                void>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<debug_deque::Cntr, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    debug_deque::Cntr const, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    debug_deque::Cntr const,
    void>::GetDynamicEnabledAbilityFlag(debug_deque::Cntr const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    debug_deque::Cntr const,
    void>::GetDynamicDisabledAbilityFlag(debug_deque::Cntr const*) {
    return seq_cntr::empty_ability_flag;
}

inline size_t
seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetCursorSize(
    debug_deque::Cntr const* cntr) {
    return debug_deque::GetCursorSize(cntr);
}

inline size_t seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetElemSize(
    debug_deque::Cntr const* cntr) {
    return debug_deque::GetElemSize(cntr);
}

inline size_t seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetElemCnt(
    debug_deque::Cntr const* cntr) {
    return debug_deque::GetElemCnt(cntr);
}

inline size_t
seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetMaxElemCnt(
    debug_deque::Cntr const* cntr) {
    return debug_deque::GetMaxElemCnt(cntr);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetLBCursor(
    debug_deque::Cntr const* cntr, void* dst_cursor) {
    debug_deque::GetLBCursor(cntr,
                             static_cast<debug_deque::Cursor*>(dst_cursor));
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetRBCursor(
    debug_deque::Cntr const* cntr, void* dst_cursor) {
    debug_deque::GetRBCursor(cntr,
                             static_cast<debug_deque::Cursor*>(dst_cursor));
}

inline void* seq_cntr::CntrTraits<debug_deque::Cntr const, void>::PeekL(
    debug_deque::Cntr const* cntr, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return debug_deque::PeekL(cntr, lazy_copy_elem,
                              static_cast<debug_deque::Cursor*>(dst_cursor),
                              dst_elem);
}

inline void* seq_cntr::CntrTraits<debug_deque::Cntr const, void>::PeekR(
    debug_deque::Cntr const* cntr, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return debug_deque::PeekR(cntr, lazy_copy_elem,
                              static_cast<debug_deque::Cursor*>(dst_cursor),
                              dst_elem);
}

inline void* seq_cntr::CntrTraits<debug_deque::Cntr const, void>::Access(
    debug_deque::Cntr const* cntr, size_t idx, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return debug_deque::Access(cntr, idx, lazy_copy_elem,
                               static_cast<debug_deque::Cursor*>(dst_cursor),
                               dst_elem);
}

inline void* seq_cntr::CntrTraits<debug_deque::Cntr const, void>::Derefer(
    debug_deque::Cntr const* cntr, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return debug_deque::Derefer(
        cntr, static_cast<debug_deque::Cursor const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <typename Reader>
void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::Read(
    debug_deque::Cntr const* cntr, void const* pos_cursor, size_t cnt,
    Reader&& reader, void* dst_cursor) {
    debug_deque::Read(cntr, static_cast<debug_deque::Cursor const*>(pos_cursor),
                      cnt, meta::Forward<Reader>(reader),
                      static_cast<debug_deque::Cursor*>(dst_cursor));
}

template <typename Writer>
void seq_cntr::CntrTraits<debug_deque::Cntr, void>::Write(
    debug_deque::Cntr* cntr, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    debug_deque::Write(cntr, static_cast<debug_deque::Cursor*>(pos_cursor), cnt,
                       meta::Forward<Writer>(writer),
                       static_cast<debug_deque::Cursor*>(dst_cursor));
}

template <typename ReaderWriter>
void seq_cntr::CntrTraits<debug_deque::Cntr, void>::ReadWrite(
    debug_deque::Cntr* cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    debug_deque::ReadWrite(cntr, static_cast<debug_deque::Cursor*>(pos_cursor),
                           cnt, meta::Forward<ReaderWriter>(reader_writer),
                           static_cast<debug_deque::Cursor*>(dst_cursor));
}

template <typename Writer>
void* seq_cntr::CntrTraits<debug_deque::Cntr, void>::PushL(
    debug_deque::Cntr* cntr, size_t cnt, Writer&& writer, void* dst_cursor) {
    return debug_deque::PushL(cntr, cnt, meta::Forward<Writer>(writer),
                              static_cast<debug_deque::Cursor*>(dst_cursor));
}

template <typename Writer>
void* seq_cntr::CntrTraits<debug_deque::Cntr, void>::PushR(
    debug_deque::Cntr* cntr, size_t cnt, Writer&& writer, void* dst_cursor) {
    return debug_deque::PushR(cntr, cnt, meta::Forward<Writer>(writer),
                              static_cast<debug_deque::Cursor*>(dst_cursor));
}

template <typename Writer>
void* seq_cntr::CntrTraits<debug_deque::Cntr, void>::Insert(
    debug_deque::Cntr* cntr, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    return debug_deque::Insert(cntr,
                               static_cast<debug_deque::Cursor*>(pos_cursor),
                               cnt, meta::Forward<Writer>(writer),
                               static_cast<debug_deque::Cursor*>(dst_cursor));
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr, void>::PopL(
    debug_deque::Cntr* cntr, size_t cnt) {
    debug_deque::PopL(cntr, cnt);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr, void>::PopR(
    debug_deque::Cntr* cntr, size_t cnt) {
    debug_deque::PopR(cntr, cnt);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr, void>::Erase(
    debug_deque::Cntr* cntr, void* pos_cursor, size_t cnt) {
    debug_deque::Erase(cntr, static_cast<debug_deque::Cursor*>(pos_cursor),
                       cnt);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr, void>::EraseAll(
    debug_deque::Cntr* cntr) {
    debug_deque::EraseAll(cntr);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CopyCursor(
    debug_deque::Cntr const* cntr, void const* src_cursor, void* dst_cursor) {
    debug_deque::CopyCursor(cntr,
                            static_cast<debug_deque::Cursor const*>(src_cursor),
                            static_cast<debug_deque::Cursor*>(dst_cursor));
}

inline bool seq_cntr::CntrTraits<debug_deque::Cntr const, void>::AreEqualCursor(
    debug_deque::Cntr const* cntr, void const* cursor_a, void const* cursor_b) {
    return debug_deque::AreEqualCursor(
        cntr, static_cast<debug_deque::Cursor const*>(cursor_a),
        static_cast<debug_deque::Cursor const*>(cursor_b));
}

inline int seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CompareCursor(
    debug_deque::Cntr const* cntr, void const* cursor_a, void const* cursor_b) {
    return debug_deque::CompareCursor(
        cntr, static_cast<debug_deque::Cursor const*>(cursor_a),
        static_cast<debug_deque::Cursor const*>(cursor_b));
}

inline size_t
seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetCursorDist(
    debug_deque::Cntr const* cntr, void const* cursor_a, void const* cursor_b) {
    return debug_deque::GetCursorDist(
        cntr, static_cast<debug_deque::Cursor const*>(cursor_a),
        static_cast<debug_deque::Cursor const*>(cursor_b));
}

inline size_t seq_cntr::CntrTraits<debug_deque::Cntr const, void>::GetCursorIdx(
    debug_deque::Cntr const* cntr, void const* cursor) {
    return debug_deque::GetCursorIdx(
        cntr, static_cast<debug_deque::Cursor const*>(cursor));
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CursorStepL(
    debug_deque::Cntr const* cntr, void* cursor) {
    debug_deque::CursorStepL(cntr, static_cast<debug_deque::Cursor*>(cursor));
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CursorStepR(
    debug_deque::Cntr const* cntr, void* cursor) {
    debug_deque::CursorStepR(cntr, static_cast<debug_deque::Cursor*>(cursor));
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CursorAdvanceL(
    debug_deque::Cntr const* cntr, void* cursor, size_t step) {
    debug_deque::CursorAdvanceL(cntr, static_cast<debug_deque::Cursor*>(cursor),
                                step);
}

inline void seq_cntr::CntrTraits<debug_deque::Cntr const, void>::CursorAdvanceR(
    debug_deque::Cntr const* cntr, void* cursor, size_t step) {
    debug_deque::CursorAdvanceR(cntr, static_cast<debug_deque::Cursor*>(cursor),
                                step);
}

}  // namespace zeta::core
