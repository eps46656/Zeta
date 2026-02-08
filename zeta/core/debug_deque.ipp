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
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::debug_deque {

namespace ops {

namespace detail {

inline void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(cntr->deque != nullptr);
    ZETA_Core_DebugAssert(0 < cntr->width);
}

inline void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr, Cursor const* cursor) {
    CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(
        seq_cntr::ops::IsReferable(cursor->idx, 1, deque->size()));
}

}  // namespace detail

inline void Init(Cntr* cntr) {
    ZETA_Core_DebugAssert(0 < cntr->width);

    cntr->deque = new std::deque<void*>;
}

inline void Deinit(Cntr* cntr) {
    detail::CheckCntr_(cntr);

    delete cntr->deque;
}

inline size_t GetCursorSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

inline size_t GetWidth(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->width;
}

inline size_t GetSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->deque->size();
}

inline size_t GetCapacity(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->deque->max_size();
}

inline void GetLBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
}

inline void GetRBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    dst_cursor->cntr = cntr;
    dst_cursor->idx = deque->size();
}

inline void* PeekL(Cntr const* cntr, bool lazy_cpy_elem, Cursor* dst_cursor,
                   void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->front() };

    if (!lazy_cpy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* PeekR(Cntr const* cntr, bool lazy_cpy_elem, Cursor* dst_cursor,
                   void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = deque->size() - 1;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->back() };

    if (!lazy_cpy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* Access(Cntr const* cntr, size_t idx, bool lazy_copy_elem,
                    Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(idx, 1, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::ops::IsDereferable(idx, 1, deque->size())) {
        return nullptr;
    }

    void* elem{ (*deque)[idx] };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* Derefer(Cntr const* cntr, Cursor const* pos_cursor,
                     bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(idx, 1, deque->size()));

    if (!seq_cntr::ops::IsDereferable(idx, 1, deque->size())) {
        return nullptr;
    }

    void* elem{ (*deque)[idx] };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

namespace detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
     ReaderWriter&&
         reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
     Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t width{ cntr->width };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(beg, cnt, deque->size()));

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

}  // namespace detail

template <typename Reader>
void Read(Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
          Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
          Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr*>(cntr), pos_cursor, cnt, reader,
                              dst_cursor);
}

template <typename Writer>
void Write(Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
           Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
           Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void ReadWrite(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <typename Writer>
void* PushL(Cntr* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

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
void* PushR(Cntr* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

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
void* Insert(Cntr* cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
             Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };
    size_t width{ cntr->width };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsInsertable(idx, cnt, deque->size(),
                                                      deque->max_size()));

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

inline void PopL(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->front());
        deque->pop_front();
    }
}

inline void PopR(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->back());
        deque->pop_back();
    }
}

inline void Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* deque{ cntr->deque };

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        delete[] static_cast<unsigned char*>((*deque)[idx]);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

inline void EraseAll(Cntr* cntr) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

inline void CopyCursor(Cntr const* cntr, Cursor const* src_cursor,
                       Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = src_cursor->idx;
}

inline bool AreEqualCursor(Cntr const* cntr, Cursor const* cursor_a,
                           Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_a) == GetCursorIdx(cntr, cursor_b);
}

inline int CompareCursor(Cntr const* cntr, Cursor const* cursor_a,
                         Cursor const* cursor_b) {
    return compare::Compare(GetCursorIdx(cntr, cursor_a) + 1,
                            GetCursorIdx(cntr, cursor_b) + 1);
}

inline size_t GetCursorDist(Cntr const* cntr, Cursor const* cursor_a,
                            Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

inline size_t GetCursorIdx(Cntr const* cntr, Cursor const* cursor) {
    detail::CheckCntr_(cntr);

    return cursor->idx;
}

inline void CursorStepL(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceL(cntr, cursor, 1);
}

inline void CursorStepR(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceR(cntr, cursor, 1);
}

inline void CursorAdvanceL(Cntr const* cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

inline void CursorAdvanceR(Cntr const* cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);

    auto* deque{ cntr->deque };

    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(step <= deque->size() - cursor->idx);

    cursor->idx += step;
}

}  // namespace ops

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView*>) {
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

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView const*>) {
    return GetStaticEnabledAbilityFlag(
               type_wrapper::TypeWrapper<SeqCntrView*>{}) &
           seq_cntr::const_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView const*>) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetDynamicEnabledAbilityFlag(
    SeqCntrView const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetDynamicDisabledAbilityFlag(
    SeqCntrView const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr size_t SeqCntrView::GetCursorSize(SeqCntrView const* seq_cntr_view) {
    return ops::GetCursorSize(
        static_cast<Cntr const*>(static_cast<void const*>(seq_cntr_view)));
}

inline size_t SeqCntrView::GetWidth(SeqCntrView const* seq_cntr_view) {
    return ops::GetWidth(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline size_t SeqCntrView::GetSize(SeqCntrView const* seq_cntr_view) {
    return ops::GetSize(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline size_t SeqCntrView::GetCapacity(SeqCntrView const* seq_cntr_view) {
    return ops::GetCapacity(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline void SeqCntrView::GetLBCursor(SeqCntrView const* seq_cntr_view,
                                     void* dst_cursor) {
    ops::GetLBCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(dst_cursor));
}

inline void SeqCntrView::GetRBCursor(SeqCntrView const* seq_cntr_view,
                                     void* dst_cursor) {
    ops::GetRBCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(dst_cursor));
}

inline void* SeqCntrView::PeekL(SeqCntrView const* seq_cntr_view,
                                bool lazy_cpy_elem, void* dst_cursor,
                                void* dst_elem) {
    return ops::PeekL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                      lazy_cpy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

inline void* SeqCntrView::PeekR(SeqCntrView const* seq_cntr_view,
                                bool lazy_cpy_elem, void* dst_cursor,
                                void* dst_elem) {
    return ops::PeekR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                      lazy_cpy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

inline void* SeqCntrView::Access(SeqCntrView const* seq_cntr_view, size_t idx,
                                 bool lazy_copy_elem, void* dst_cursor,
                                 void* dst_elem) {
    return ops::Access(reinterpret_cast<Cntr const*>(seq_cntr_view), idx,
                       lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                       dst_elem);
}

inline void* SeqCntrView::Derefer(SeqCntrView const* seq_cntr_view,
                                  void const* pos_cursor, bool lazy_copy_elem,
                                  void* dst_elem) {
    return ops::Derefer(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor const*>(pos_cursor), lazy_copy_elem,
                        dst_elem);
}

template <typename Reader>
void SeqCntrView::Read(
    SeqCntrView const* seq_cntr_view, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Read(reinterpret_cast<Cntr const*>(seq_cntr_view),
              static_cast<Cursor const*>(pos_cursor), cnt,
              Forward<Reader>(reader), static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void SeqCntrView::Write(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Write(reinterpret_cast<Cntr*>(seq_cntr_view),
               static_cast<Cursor*>(pos_cursor), cnt, Forward<Writer>(writer),
               static_cast<Cursor*>(dst_cursor));
}

template <typename ReaderWriter>
void SeqCntrView::ReadWrite(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::ReadWrite(reinterpret_cast<Cntr*>(seq_cntr_view),
                   static_cast<Cursor*>(pos_cursor), cnt,
                   Forward<ReaderWriter>(reader_writer),
                   static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::PushL(
    SeqCntrView* seq_cntr_view, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushL(reinterpret_cast<Cntr*>(seq_cntr_view), cnt,
                      Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::PushR(
    SeqCntrView* seq_cntr_view, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushR(reinterpret_cast<Cntr*>(seq_cntr_view), cnt,
                      Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::Insert(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::Insert(reinterpret_cast<Cntr*>(seq_cntr_view),
                       static_cast<Cursor*>(pos_cursor), cnt,
                       Forward<Writer>(writer),
                       static_cast<Cursor*>(dst_cursor));
}

inline void SeqCntrView::PopL(SeqCntrView* seq_cntr_view, size_t cnt) {
    ops::PopL(reinterpret_cast<Cntr*>(seq_cntr_view), cnt);
}

inline void SeqCntrView::PopR(SeqCntrView* seq_cntr_view, size_t cnt) {
    ops::PopR(reinterpret_cast<Cntr*>(seq_cntr_view), cnt);
}

inline void SeqCntrView::Erase(SeqCntrView* seq_cntr_view, void* pos_cursor,
                               size_t cnt) {
    ops::Erase(reinterpret_cast<Cntr*>(seq_cntr_view),
               static_cast<Cursor*>(pos_cursor), cnt);
}

inline void SeqCntrView::EraseAll(SeqCntrView* seq_cntr_view) {
    ops::EraseAll(reinterpret_cast<Cntr*>(seq_cntr_view));
}

inline void SeqCntrView::CopyCursor(SeqCntrView const* seq_cntr_view,
                                    void const* src_cursor, void* dst_cursor) {
    ops::CopyCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                    static_cast<Cursor const*>(src_cursor),
                    static_cast<Cursor*>(dst_cursor));
}

inline bool SeqCntrView::AreEqualCursor(SeqCntrView const* seq_cntr_view,
                                        void const* cursor_a,
                                        void const* cursor_b) {
    return ops::AreEqualCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                               static_cast<Cursor const*>(cursor_a),
                               static_cast<Cursor const*>(cursor_b));
}

inline int SeqCntrView::CompareCursor(SeqCntrView const* seq_cntr_view,
                                      void const* cursor_a,
                                      void const* cursor_b) {
    return ops::CompareCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                              static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

inline size_t SeqCntrView::GetCursorDist(SeqCntrView const* seq_cntr_view,
                                         void const* cursor_a,
                                         void const* cursor_b) {
    return ops::GetCursorDist(reinterpret_cast<Cntr const*>(seq_cntr_view),
                              static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

inline size_t SeqCntrView::GetCursorIdx(SeqCntrView const* seq_cntr_view,
                                        void const* cursor) {
    return ops::GetCursorIdx(reinterpret_cast<Cntr const*>(seq_cntr_view),
                             static_cast<Cursor const*>(cursor));
}

inline void SeqCntrView::CursorStepL(SeqCntrView const* seq_cntr_view,
                                     void* cursor) {
    ops::CursorStepL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(cursor));
}

inline void SeqCntrView::CursorStepR(SeqCntrView const* seq_cntr_view,
                                     void* cursor) {
    ops::CursorStepR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(cursor));
}

inline void SeqCntrView::CursorAdvanceL(SeqCntrView const* seq_cntr_view,
                                        void* cursor, size_t step) {
    ops::CursorAdvanceL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor*>(cursor), step);
}

inline void SeqCntrView::CursorAdvanceR(SeqCntrView const* seq_cntr_view,
                                        void* cursor, size_t step) {
    ops::CursorAdvanceR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor*>(cursor), step);
}

}  // namespace zeta::core::debug_deque
