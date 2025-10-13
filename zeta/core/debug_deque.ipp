#pragma once

#include <deque>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>

namespace zeta::core {

template <bool EnWrite, typename ReaderWriter>
void DebugDeque::ReadWrite_(void* dd_, void const* pos_cursor_, size_t cnt,
                            ReaderWriter&& reader_writer, void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    size_t width{ dd->width };

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor(dd, pos_cursor);

    size_t beg{ pos_cursor->idx };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        reader_writer(static_cast<Conditional<EnWrite, void*, void const*>>(
                          (*deque)[idx]),
                      width, 1);
    }
}

seq_cntr::SeqCntrVTable const DebugDeque::seq_cntr_vtable{
    .Deinit = &DebugDeque::Deinit,

    .GetSize = &DebugDeque::GetSize,
    .GetCapacity = &DebugDeque::GetCapacity,

    .GetLBCursor = &DebugDeque::GetLBCursor,
    .GetRBCursor = &DebugDeque::GetRBCursor,

    .PeekL = &DebugDeque::PeekL,
    .ConstPeekL = &DebugDeque::ConstPeekL,
    .PeekR = &DebugDeque::PeekR,
    .ConstPeekR = &DebugDeque::ConstPeekR,
    .Access = &DebugDeque::Access,
    .ConstAccess = &DebugDeque::ConstAccess,
    .Refer = &DebugDeque::Refer,
    .ConstRefer = &DebugDeque::ConstRefer,

    .FnRead = &DebugDeque::FnRead,
    .FnWrite = &DebugDeque::FnWrite,
    .FnReadWrite = &DebugDeque::FnReadWrite,

    .MemRead = &DebugDeque::MemRead,
    .MemWrite = &DebugDeque::MemWrite,

    .FnPushL = &DebugDeque::FnPushL,
    .FnPushR = &DebugDeque::FnPushR,
    .FnInsert = &DebugDeque::FnInsert,

    .MemPushL = &DebugDeque::MemPushL,
    .MemPushR = &DebugDeque::MemPushR,
    .MemInsert = &DebugDeque::MemInsert,

    .PopL = &DebugDeque::PopL,
    .PopR = &DebugDeque::PopR,
    .Erase = &DebugDeque::Erase,
    .EraseAll = &DebugDeque::EraseAll,

    .CopyCursor = &DebugDeque::CopyCursor,
    .AreEqualCursor = &DebugDeque::AreEqualCursor,
    .CompareCursor = &DebugDeque::CompareCursor,
    .GetCursorDist = &DebugDeque::GetCursorDist,
    .GetCursorIdx = &DebugDeque::GetCursorIdx,
    .CursorStepL = &DebugDeque::CursorStepL,
    .CursorStepR = &DebugDeque::CursorStepR,
    .CursorAdvanceL = &DebugDeque::CursorAdvanceL,
    .CursorAdvanceR = &DebugDeque::CursorAdvanceR,
};

// -----------------------------------------------------------------------------

inline void DebugDeque::Init(void* dd_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };

    ZETA_Core_DebugAssert(0 < dd->width);

    dd->deque = new std::deque<void*>;
}

inline void DebugDeque::Deinit(void* dd_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    delete dd->deque;
}

inline size_t DebugDeque::GetSize(void const* dd_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    return dd->deque->size();
}

inline size_t DebugDeque::GetCapacity(void const* dd_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    return dd->deque->max_size();
}

inline void DebugDeque::GetLBCursor(void const* dd_, void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    dst_cursor->dd = dd;
    dst_cursor->idx = static_cast<size_t>(-1);
}

inline void DebugDeque::GetRBCursor(void const* dd_, void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    dst_cursor->dd = dd;
    dst_cursor->idx = deque->size();
}

inline void* DebugDeque::PeekL(void* dd_, void* dst_cursor_, void* dst_elem) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = 0;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->front() };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* DebugDeque::ConstPeekL(void const* dd, void* dst_cursor,
                                          void* dst_elem) {
    return PeekL(const_cast<void*>(dd), dst_cursor, dst_elem);
}

inline void* DebugDeque::PeekR(void* dd_, void* dst_cursor_, void* dst_elem) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = deque->size() - 1;
    }

    if (deque->empty()) { return nullptr; }

    void* elem{ deque->back() };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* DebugDeque::ConstPeekR(void const* dd, void* dst_cursor,
                                          void* dst_elem) {
    return PeekR(const_cast<void*>(dd), dst_cursor, dst_elem);
}

inline void* DebugDeque::Access(void* dd_, size_t idx, void* dst_cursor_,
                                void* dst_elem) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::IsDereferable(idx, 1, deque->size())) { return nullptr; }

    void* elem{ (*deque)[idx] };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, width); }

    return elem;
}

inline void const* DebugDeque::ConstAccess(void const* dd, size_t idx,
                                           void* dst_cursor, void* dst_elem) {
    return Access(const_cast<void*>(dd), idx, dst_cursor, dst_elem);
}

inline void* DebugDeque::Refer(void* dd_, void const* pos_cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor(dd, pos_cursor);

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, deque->size()));

    return seq_cntr::IsDereferable(idx, 1, deque->size()) ? (*deque)[idx]
                                                          : nullptr;
}

inline void const* DebugDeque::ConstRefer(void const* dd,
                                          void const* pos_cursor_) {
    return Refer(const_cast<void*>(dd), pos_cursor_);
}

// -----------------------------------------------------------------------------

template <typename Reader>
void DebugDeque::TplRead(void const* dd, void const* pos_cursor, size_t cnt,
                         Reader&& reader, void* dst_cursor) {
    ReadWrite_<false>(const_cast<void*>(dd), pos_cursor, cnt, reader,
                      dst_cursor);
}

template <typename Writer>
void DebugDeque::TplWrite(void* dd, void const* pos_cursor, size_t cnt,
                          Writer&& writer, void* dst_cursor) {
    ReadWrite_<true>(dd, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void DebugDeque::TplReadWrite(void* dd, void const* pos_cursor, size_t cnt,
                              ReaderWriter&& reader_writer, void* dst_cursor) {
    ReadWrite_<true>(dd, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void DebugDeque::FnRead(void const* dd, void const* pos_cursor,
                               size_t cnt, seq_cntr::FnReader reader,
                               void* dst_cursor) {
    TplRead(dd, pos_cursor, cnt, reader, dst_cursor);
}

inline void DebugDeque::FnWrite(void* dd, void* pos_cursor, size_t cnt,
                                seq_cntr::FnWriter writer, void* dst_cursor) {
    TplWrite(dd, pos_cursor, cnt, writer, dst_cursor);
}

inline void DebugDeque::FnReadWrite(void* dd, void* pos_cursor, size_t cnt,
                                    seq_cntr::FnReaderWriter reader_writer,
                                    void* dst_cursor) {
    TplReadWrite(dd, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void DebugDeque::MemRead(void const* dd_, void const* pos_cursor,
                                size_t cnt, void* dst, size_t dst_stride,
                                void* dst_cursor) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    size_t width{ dd->width };

    TplRead(
        dd_, pos_cursor, cnt,
        [=](void const* src, size_t src_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            dst = static_cast<char*>(dst) + dst_stride * cnt;
        },
        dst_cursor);
}

inline void DebugDeque::MemWrite(void* dd_, void* pos_cursor, size_t cnt,
                                 void const* src, size_t src_stride,
                                 void* dst_cursor) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    size_t width{ dd->width };

    ZETA_Core_DebugAssert(src != nullptr);

    TplWrite(
        dd_, pos_cursor, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename Writer>
void* DebugDeque::TplPushL(void* dd_, size_t cnt, Writer&& writer,
                           void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[width] };
        writer(elem, width, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = 0;
    }

    return deque->empty() ? nullptr : deque->front();
}

template <typename Writer>
void* DebugDeque::TplPushR(void* dd_, size_t cnt, Writer&& writer,
                           void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    size_t old_size{ deque->size() };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ old_size }; idx < deque->size(); ++idx) {
        void* elem{ new unsigned char[width] };
        writer(elem, width, 1);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->dd = dd;
        dst_cursor->idx = old_size;
    }

    return old_size < deque->size() ? (*deque)[old_size] : nullptr;
}

template <typename Writer>
void* DebugDeque::TplInsert(void* dd_, void* pos_cursor_, size_t cnt,
                            Writer&& writer, void* dst_cursor_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };
    size_t width{ dd->width };

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor(dd, pos_cursor);

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

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
        dst_cursor->dd = dd;
        dst_cursor->idx = idx + cnt;
    }

    return idx < deque->size() ? (*deque)[idx] : nullptr;
}

// -----------------------------------------------------------------------------

inline void* DebugDeque::FnPushL(void* dd, size_t cnt,
                                 seq_cntr::FnWriter writer, void* dst_cursor) {
    return TplPushL(dd, cnt, writer, dst_cursor);
}

inline void* DebugDeque::FnPushR(void* dd, size_t cnt,
                                 seq_cntr::FnWriter writer, void* dst_cursor) {
    return TplPushR(dd, cnt, writer, dst_cursor);
}

inline void* DebugDeque::FnInsert(void* dd, void* pos_cursor, size_t cnt,
                                  seq_cntr::FnWriter writer, void* dst_cursor) {
    return TplInsert(dd, pos_cursor, cnt, writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* DebugDeque::MemPushL(void* dd_, size_t cnt, void const* src,
                                  size_t src_stride, void* dst_cursor) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    size_t width{ dd->width };

    if (src == nullptr) {
        return TplPushL(dd_, cnt, [](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplPushL(
        dd_, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

inline void* DebugDeque::MemPushR(void* dd_, size_t cnt, void const* src,
                                  size_t src_stride, void* dst_cursor) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    size_t width{ dd->width };

    if (src == nullptr) {
        return TplPushR(dd, cnt, [](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplPushR(
        dd, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

inline void* DebugDeque::MemInsert(void* dd_, void* pos_cursor, size_t cnt,
                                   void const* src, size_t src_stride,
                                   void* dst_cursor) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    size_t width{ dd->width };

    if (src == nullptr) {
        return TplInsert(
            dd, pos_cursor, cnt, [](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplInsert(
        dd, pos_cursor, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

inline void DebugDeque::PopL(void* dd_, size_t cnt) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->front());
        deque->pop_front();
    }
}

inline void DebugDeque::PopR(void* dd_, size_t cnt) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] static_cast<unsigned char*>(deque->back());
        deque->pop_back();
    }
}

inline void DebugDeque::Erase(void* dd_, void* pos_cursor_, size_t cnt) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    auto pos_cursor{ static_cast<Cursor*>(pos_cursor_) };
    CheckCursor(dd, pos_cursor);

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        delete[] static_cast<unsigned char*>((*deque)[idx]);
    }

    deque->erase(deque->begin() + static_cast<long long>(beg),
                 deque->begin() + static_cast<long long>(end));
}

inline void DebugDeque::EraseAll(void* dd_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    for (void* elem : *deque) { delete[] static_cast<unsigned char*>(elem); }

    deque->clear();
}

// -----------------------------------------------------------------------------

inline void DebugDeque::CopyCursor(void const* dd_, void* dst_cursor_,
                                   void const* src_cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    auto src_cursor{ static_cast<Cursor const*>(src_cursor_) };
    CheckCursor(dd, src_cursor);

    dst_cursor->dd = dd;
    dst_cursor->idx = src_cursor->idx;
}

inline bool DebugDeque::AreEqualCursor(void const* dd, void const* cursor_a,
                                       void const* cursor_b) {
    return GetCursorIdx(dd, cursor_a) == GetCursorIdx(dd, cursor_b);
}

inline int DebugDeque::CompareCursor(void const* dd, void const* cursor_a,
                                     void const* cursor_b) {
    return compare::Compare(GetCursorIdx(dd, cursor_a) + 1,
                            GetCursorIdx(dd, cursor_b) + 1);
}

inline size_t DebugDeque::GetCursorDist(void const* dd, void const* cursor_a,
                                        void const* cursor_b) {
    return GetCursorIdx(dd, cursor_b) - GetCursorIdx(dd, cursor_a);
}

inline size_t DebugDeque::GetCursorIdx(void const* dd_, void const* cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto cursor{ static_cast<Cursor const*>(cursor_) };

    return cursor->idx;
}

inline void DebugDeque::CursorStepL(void const* dd, void* cursor) {
    CursorAdvanceL(dd, cursor, 1);
}

inline void DebugDeque::CursorStepR(void const* dd, void* cursor) {
    CursorAdvanceR(dd, cursor, 1);
}

inline void DebugDeque::CursorAdvanceL(void const* dd_, void* cursor_,
                                       size_t step) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto cursor{ static_cast<Cursor*>(cursor_) };
    CheckCursor(dd, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

inline void DebugDeque::CursorAdvanceR(void const* dd_, void* cursor_,
                                       size_t step) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    auto deque{ dd->deque };

    auto cursor{ static_cast<Cursor*>(cursor_) };
    CheckCursor(dd, cursor);

    ZETA_Core_DebugAssert(step <= deque->size() - cursor->idx);

    cursor->idx += step;
}

// -----------------------------------------------------------------------------

inline bool DebugDeque::CheckCntr(void const* dd_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    if (!(dd != nullptr)) { return false; }

    if (!(dd->deque != nullptr)) { return false; }
    if (!(0 < dd->width)) { return false; }

    return true;
}

inline bool DebugDeque::CheckCursor(void const* dd_, void const* cursor_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    if (!(CheckCntr(dd))) { return false; }

    auto deque{ dd->deque };

    auto cursor{ static_cast<Cursor const*>(cursor_) };

    if (!(seq_cntr::IsReferable(cursor->idx, 1, deque->size()))) {
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------

inline seq_cntr::SeqCntrRef DebugDeque::GetSeqCntrRef(void* dd_) {
    auto dd{ static_cast<DebugDeque*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    return {
        .inst = dd,

        .cursor_size = sizeof(Cursor),
        .width = dd->width,
        .capacity = ZETA_Core_max_capacity,

        .vtable = &seq_cntr_vtable,
    };
}

inline seq_cntr::ConstSeqCntrRef DebugDeque::GetSeqCntrRef(void const* dd_) {
    auto dd{ static_cast<DebugDeque const*>(dd_) };
    ZETA_Core_DebugAssert(CheckCntr(dd));

    return {
        .inst = dd,

        .cursor_size = sizeof(Cursor),
        .width = dd->width,
        .capacity = ZETA_Core_max_capacity,

        .vtable = &seq_cntr_vtable,
    };
}

}  // namespace zeta::core
