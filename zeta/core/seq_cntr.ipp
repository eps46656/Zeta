#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/seq_cntr.hpp>

#pragma push_macro("Call_")
#pragma push_macro("ConstCall_")

#define Call_(func, ...)                                        \
    auto seq_cntr{ static_cast<SeqCntr*>(seq_cntr_) };          \
    CheckCntr(seq_cntr);                                        \
    return seq_cntr->vtable->func(seq_cntr->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

#define ConstCall_(func, ...)                                         \
    auto seq_cntr{ static_cast<SeqCntr const*>(seq_cntr_) };          \
    CheckCntr(seq_cntr);                                              \
    return seq_cntr->vtable->func(seq_cntr->inst_const, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

namespace zeta::core {

inline bool SeqCntr::IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

inline bool SeqCntr::IsDereferable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

inline bool SeqCntr::IsInsertable(size_t idx, size_t cnt, size_t size,
                                  size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

inline bool SeqCntr::IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

// -----------------------------------------------------------------------------

template <typename ImplCntr>
SeqCntr::VTable const& SeqCntr::MakeVTable() {
    static VTable const vtable{
        &ImplCntr::Deinit,

        // ---------------------------------------------------------------------

        &ImplCntr::GetSize,

        &ImplCntr::GetCapacity,

        // ---------------------------------------------------------------------

        &ImplCntr::GetLBCursor,

        &ImplCntr::GetRBCursor,

        // ---------------------------------------------------------------------

        &ImplCntr::PeekL,

        &ImplCntr::ConstPeekL,

        &ImplCntr::PeekR,

        &ImplCntr::ConstPeekR,

        &ImplCntr::Access,

        &ImplCntr::ConstAccess,

        &ImplCntr::Refer,

        &ImplCntr::ConstRefer,

        // ---------------------------------------------------------------------

        &ImplCntr::FnRead,

        &ImplCntr::FnWrite,

        &ImplCntr::FnReadWrite,

        // ---------------------------------------------------------------------

        &ImplCntr::MemRead,

        &ImplCntr::MemWrite,

        // ---------------------------------------------------------------------

        &ImplCntr::FnPushL,

        &ImplCntr::FnPushR,

        &ImplCntr::FnInsert,

        // ---------------------------------------------------------------------

        &ImplCntr::MemPushL,

        &ImplCntr::MemPushR,

        &ImplCntr::MemInsert,

        // ---------------------------------------------------------------------

        &ImplCntr::PopL,

        &ImplCntr::PopR,

        &ImplCntr::Erase,

        &ImplCntr::EraseAll,

        // ---------------------------------------------------------------------

        &ImplCntr::CopyCursor,

        &ImplCntr::AreEqualCursor,

        &ImplCntr::CompareCursor,

        &ImplCntr::GetCursorDist,

        &ImplCntr::GetCursorIdx,

        &ImplCntr::CursorStepL,

        &ImplCntr::CursorStepR,

        &ImplCntr::CursorAdvanceL,

        &ImplCntr::CursorAdvanceR,
    };

    return vtable;
}

// -----------------------------------------------------------------------------

inline void SeqCntr::CheckCntr(void* seq_cntr_) {
    auto seq_cntr{ static_cast<SeqCntr const*>(seq_cntr_) };
    CheckCntr(seq_cntr);

    ZETA_Core_DebugAssert(seq_cntr->inst != nullptr);
}

inline void SeqCntr::CheckCntr(void const* seq_cntr_) {
    auto seq_cntr{ static_cast<SeqCntr const*>(seq_cntr_) };
    ZETA_Core_DebugAssert(seq_cntr != nullptr);

    ZETA_Core_DebugAssert(seq_cntr->inst_const != nullptr);

    ZETA_Core_DebugAssert(seq_cntr->vtable != nullptr);
}

// -----------------------------------------------------------------------------

inline void SeqCntr::Deinit(void* seq_cntr_) { Call_(Deinit); }

// -----------------------------------------------------------------------------

inline size_t SeqCntr::GetSize(void const* seq_cntr_) { ConstCall_(GetSize); }

// -----------------------------------------------------------------------------

inline void SeqCntr::GetLBCursor(void const* seq_cntr_, void* dst_cursor) {
    ConstCall_(GetLBCursor, dst_cursor);
}

inline void SeqCntr::GetRBCursor(void const* seq_cntr_, void* dst_cursor) {
    ConstCall_(GetRBCursor, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* SeqCntr::PeekL(void* seq_cntr_, void* dst_cursor, void* dst_elem) {
    Call_(PeekL, dst_cursor, dst_elem);
}

inline void const* SeqCntr::ConstPeekL(void const* seq_cntr_, void* dst_cursor,
                                       void* dst_elem) {
    ConstCall_(ConstPeekL, dst_cursor, dst_elem);
}

inline void* SeqCntr::PeekR(void* seq_cntr_, void* dst_cursor, void* dst_elem) {
    Call_(PeekR, dst_cursor, dst_elem);
}

inline void const* SeqCntr::ConstPeekR(void const* seq_cntr_, void* dst_cursor,
                                       void* dst_elem) {
    ConstCall_(ConstPeekR, dst_cursor, dst_elem);
}

inline void* SeqCntr::Access(void* seq_cntr_, size_t idx, void* dst_cursor,
                             void* dst_elem) {
    Call_(Access, idx, dst_cursor, dst_elem);
}

inline void const* SeqCntr::ConstAccess(void const* seq_cntr_, size_t idx,
                                        void* dst_cursor, void* dst_elem) {
    ConstCall_(ConstAccess, idx, dst_cursor, dst_elem);
}

inline void* SeqCntr::Refer(void* seq_cntr_, void const* pos_cursor) {
    Call_(Refer, pos_cursor);
}

inline void const* SeqCntr::ConstRefer(void const* seq_cntr_,
                                       void const* pos_cursor) {
    ConstCall_(ConstRefer, pos_cursor);
}

// -----------------------------------------------------------------------------

inline void SeqCntr::FnRead(void const* seq_cntr_, void const* pos_cursor,
                            size_t cnt, FnReader reader, void* dst_cursor) {
    ConstCall_(FnRead, pos_cursor, cnt, reader, dst_cursor);
}

inline void SeqCntr::FnWrite(void* seq_cntr_, void* pos_cursor, size_t cnt,
                             FnWriter writer, void* dst_cursor) {
    Call_(FnWrite, pos_cursor, cnt, writer, dst_cursor);
}

inline void SeqCntr::FnReadWrite(void* seq_cntr_, void const* pos_cursor,
                                 size_t cnt, FnReaderWriter reader_writer,
                                 void* dst_cursor) {
    Call_(FnReadWrite, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void SeqCntr::MemRead(void const* seq_cntr_, void const* pos_cursor,
                             size_t cnt, void* dst, size_t dst_stride,
                             void* dst_cursor) {
    ConstCall_(MemRead, pos_cursor, cnt, dst, dst_stride, dst_cursor);
}

inline void SeqCntr::MemWrite(void* seq_cntr_, void* pos_cursor, size_t cnt,
                              void const* src, size_t src_stride,
                              void* dst_cursor) {
    Call_(MemWrite, pos_cursor, cnt, src, src_stride, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* SeqCntr::FnPushL(void* seq_cntr_, size_t cnt,
                              FnWriter const& writer, void* dst_cursor) {
    Call_(FnPushL, cnt, writer, dst_cursor);
}

inline void* SeqCntr::FnPushR(void* seq_cntr_, size_t cnt,
                              FnWriter const& writer, void* dst_cursor) {
    Call_(FnPushR, cnt, writer, dst_cursor);
}

inline void* SeqCntr::FnInsert(void* seq_cntr_, void* pos_cursor, size_t cnt,
                               FnWriter const& writer, void* dst_cursor) {
    Call_(FnInsert, pos_cursor, cnt, writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* SeqCntr::MemPushL(void* seq_cntr_, size_t cnt, void const* src,
                               size_t src_stride, void* dst_cursor) {
    Call_(MemPushL, cnt, src, src_stride, dst_cursor);
}

inline void* SeqCntr::MemPushR(void* seq_cntr_, size_t cnt, void const* src,
                               size_t src_stride, void* dst_cursor) {
    Call_(MemPushR, cnt, src, src_stride, dst_cursor);
}

inline void* SeqCntr::MemInsert(void* seq_cntr_, void* pos_cursor, size_t cnt,
                                void const* src, size_t src_stride,
                                void* dst_cursor) {
    Call_(MemInsert, pos_cursor, cnt, src, src_stride, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void SeqCntr::PopL(void* seq_cntr_, size_t cnt) { Call_(PopL, cnt); }

inline void SeqCntr::PopR(void* seq_cntr_, size_t cnt) { Call_(PopR, cnt); }

inline void SeqCntr::Erase(void* seq_cntr_, void* pos_cursor, size_t cnt) {
    Call_(Erase, pos_cursor, cnt);
}

inline void SeqCntr::EraseAll(void* seq_cntr_) { Call_(EraseAll); }

// -----------------------------------------------------------------------------

inline bool SeqCntr::AreEqualCursor(void const* seq_cntr_, void const* cursor_a,
                                    void const* cursor_b) {
    ConstCall_(AreEqualCursor, cursor_a, cursor_b);
}

inline int SeqCntr::CompareCursor(void const* seq_cntr_, void const* cursor_a,
                                  void const* cursor_b) {
    ConstCall_(CompareCursor, cursor_a, cursor_b);
}

inline size_t SeqCntr::GetCursorDist(void const* seq_cntr_,
                                     void const* cursor_a,
                                     void const* cursor_b) {
    ConstCall_(GetCursorDist, cursor_a, cursor_b);
}

inline size_t SeqCntr::GetCursorIdx(void const* seq_cntr_, void const* cursor) {
    ConstCall_(GetCursorIdx, cursor);
}

inline void SeqCntr::CursorStepL(void const* seq_cntr_, void* cursor) {
    ConstCall_(CursorStepL, cursor);
}

inline void SeqCntr::CursorStepR(void const* seq_cntr_, void* cursor) {
    ConstCall_(CursorStepR, cursor);
}

inline void SeqCntr::CursorAdvanceL(void const* seq_cntr_, void* cursor,
                                    size_t step) {
    ConstCall_(CursorAdvanceL, cursor, step);
}

inline void SeqCntr::CursorAdvanceR(void const* seq_cntr_, void* cursor,
                                    size_t step) {
    ConstCall_(CursorAdvanceR, cursor, step);
}

}  // namespace zeta::core

#pragma pop_macro("Call_")
#pragma pop_macro("ConstCall_")
