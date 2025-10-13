#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("Call_")
#pragma push_macro("ConstCall_")

#define Call_(func, ...)                                                \
    ZETA_Core_StaticAssert(!IsConst::value);                            \
                                                                        \
    auto seq_cntr_ref{ static_cast<SeqCntrRef*>(seq_cntr_ref_) };       \
    ZETA_Core_DebugAssert(CheckCntr(seq_cntr_ref));                     \
                                                                        \
    return seq_cntr_ref->vtable->func(seq_cntr_ref->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

#define ConstCall_(func, ...)                                           \
    auto seq_cntr_ref{ static_cast<SeqCntrRef const*>(seq_cntr_ref_) }; \
    ZETA_Core_DebugAssert(CheckCntr(seq_cntr_ref));                     \
                                                                        \
    return seq_cntr_ref->vtable->func(seq_cntr_ref->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

namespace zeta::core::seq_cntr {

inline bool IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

inline bool IsDereferable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

inline bool IsInsertable(size_t idx, size_t cnt, size_t size, size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

inline bool IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

// -----------------------------------------------------------------------------

template <typename DstSeqCntrImpl, typename SrcSeqCntrImpl>
void RangeAssign(DstSeqCntrImpl* dst_seq_cntr, SrcSeqCntrImpl* src_seq_cntr,
                 size_t dst_beg, size_t src_beg, size_t cnt) {
    ZETA_Core_DebugAssert(DstSeqCntrImpl::CheckCntr(dst_seq_cntr));
    ZETA_Core_DebugAssert(SrcSeqCntrImpl::CheckCntr(src_seq_cntr));

    ZETA_Core_DebugAssert(DstSeqCntrImpl::GetWidth(dst_seq_cntr) ==
                          SrcSeqCntrImpl::GetWidth(src_seq_cntr));

    size_t width{ DstSeqCntrImpl::GetWidth(dst_seq_cntr) };

    size_t buffer_capacity{ Max(1ULL, sizeof(void*) * 1024 / width) };

    size_t dst_size{ DstSeqCntrImpl::GetSize(dst_seq_cntr) };
    size_t src_size{ SrcSeqCntrImpl::GetSize(src_seq_cntr) };

    ZETA_Core_DebugAssert(IsDereferable(dst_beg, cnt, dst_size));
    ZETA_Core_DebugAssert(IsDereferable(src_beg, cnt, src_size));

    void const* real_dst_seq_cntr{ dst_seq_cntr };
    void const* real_src_seq_cntr{ src_seq_cntr };

    if constexpr (IsAnyOf<RemoveCVRef<DstSeqCntrImpl>, SeqCntrRef>) {
        real_dst_seq_cntr = dst_seq_cntr->const_inst;
    }

    if constexpr (IsAnyOf<RemoveCVRef<SrcSeqCntrImpl>, SeqCntrRef>) {
        real_src_seq_cntr = src_seq_cntr->const_inst;
    }

    if (cnt == 0 || real_dst_seq_cntr == real_src_seq_cntr) { return; }

    size_t dst_end{ dst_beg + cnt };
    size_t src_end{ src_beg + cnt };

    void* buffer{ __builtin_alloca_with_align(
        width * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t)) };

    void* dst_cursor{ ZETA_Core_SeqCntr_AllocaCursor(dst_seq_cntr) };
    void* src_cursor{ ZETA_Core_SeqCntr_AllocaCursor(src_seq_cntr) };

    if (real_dst_seq_cntr != real_src_seq_cntr || dst_beg <= src_beg ||
        src_end <= dst_beg) {
        DstSeqCntrImpl::Access(dst_seq_cntr, dst_beg, dst_cursor, nullptr);
        SrcSeqCntrImpl::Access(src_seq_cntr, src_beg, src_cursor, nullptr);

        while (0 < cnt) {
            size_t cur_cnt{ Min(buffer_capacity, cnt) };

            SrcSeqCntrImpl::MemRead(src_seq_cntr, src_cursor, cur_cnt, buffer,
                                    width, src_cursor);
            DstSeqCntrImpl::MemWrite(dst_seq_cntr, dst_cursor, cur_cnt, buffer,
                                     width, dst_cursor);

            cnt -= cur_cnt;
        }

        return;
    }

    DstSeqCntrImpl::Access(dst_seq_cntr, dst_end, dst_cursor, nullptr);
    SrcSeqCntrImpl::Access(src_seq_cntr, src_end, src_cursor, nullptr);

    while (0 < cnt) {
        size_t cur_cnt{ Min(buffer_capacity, cnt) };

        SrcSeqCntrImpl::CursorAdvanceL(src_seq_cntr, src_cursor, cur_cnt);
        DstSeqCntrImpl::CursorAdvanceL(dst_seq_cntr, dst_cursor, cur_cnt);

        SrcSeqCntrImpl::MemRead(src_seq_cntr, src_cursor, cur_cnt, buffer,
                                width, nullptr);

        DstSeqCntrImpl::MemWrite(dst_seq_cntr, dst_cursor, cur_cnt, buffer,
                                 width, nullptr);

        cnt -= cur_cnt;
    }
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::Deinit(void* seq_cntr_ref_) {
    Call_(Deinit);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
size_t SeqCntrRefTpl<IsConst>::GetSize(void const* seq_cntr_ref_) {
    ConstCall_(GetSize);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::GetLBCursor(void const* seq_cntr_ref_,
                                         void* dst_cursor) {
    ConstCall_(GetLBCursor, dst_cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::GetRBCursor(void const* seq_cntr_ref_,
                                         void* dst_cursor) {
    ConstCall_(GetRBCursor, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::PeekL(void* seq_cntr_ref_, void* dst_cursor,
                                    void* dst_elem) {
    Call_(PeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* SeqCntrRefTpl<IsConst>::ConstPeekL(void const* seq_cntr_ref_,
                                               void* dst_cursor,
                                               void* dst_elem) {
    ConstCall_(ConstPeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::PeekR(void* seq_cntr_ref_, void* dst_cursor,
                                    void* dst_elem) {
    Call_(PeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* SeqCntrRefTpl<IsConst>::ConstPeekR(void const* seq_cntr_ref_,
                                               void* dst_cursor,
                                               void* dst_elem) {
    ConstCall_(ConstPeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::Access(void* seq_cntr_ref_, size_t idx,
                                     void* dst_cursor, void* dst_elem) {
    Call_(Access, idx, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* SeqCntrRefTpl<IsConst>::ConstAccess(void const* seq_cntr_ref_,
                                                size_t idx, void* dst_cursor,
                                                void* dst_elem) {
    ConstCall_(ConstAccess, idx, dst_cursor, dst_elem);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::Refer(void* seq_cntr_ref_,
                                    void const* pos_cursor) {
    Call_(Refer, pos_cursor);
}

template <typename IsConst>
void const* SeqCntrRefTpl<IsConst>::ConstRefer(void const* seq_cntr_ref_,
                                               void const* pos_cursor) {
    ConstCall_(ConstRefer, pos_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::FnRead(void const* seq_cntr_ref_,
                                    void const* pos_cursor, size_t cnt,
                                    FnReader reader, void* dst_cursor) {
    ConstCall_(FnRead, pos_cursor, cnt, reader, dst_cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::FnWrite(void* seq_cntr_ref_, void* pos_cursor,
                                     size_t cnt, FnWriter writer,
                                     void* dst_cursor) {
    Call_(FnWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::FnReadWrite(void* seq_cntr_ref_, void* pos_cursor,
                                         size_t cnt,
                                         FnReaderWriter reader_writer,
                                         void* dst_cursor) {
    Call_(FnReadWrite, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::MemRead(void const* seq_cntr_ref_,
                                     void const* pos_cursor, size_t cnt,
                                     void* dst, size_t dst_stride,
                                     void* dst_cursor) {
    ConstCall_(MemRead, pos_cursor, cnt, dst, dst_stride, dst_cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::MemWrite(void* seq_cntr_ref_, void* pos_cursor,
                                      size_t cnt, void const* src,
                                      size_t src_stride, void* dst_cursor) {
    Call_(MemWrite, pos_cursor, cnt, src, src_stride, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::FnPushL(void* seq_cntr_ref_, size_t cnt,
                                      FnWriter writer, void* dst_cursor) {
    Call_(FnPushL, cnt, writer, dst_cursor);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::FnPushR(void* seq_cntr_ref_, size_t cnt,
                                      FnWriter writer, void* dst_cursor) {
    Call_(FnPushR, cnt, writer, dst_cursor);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::FnInsert(void* seq_cntr_ref_, void* pos_cursor,
                                       size_t cnt, FnWriter writer,
                                       void* dst_cursor) {
    Call_(FnInsert, pos_cursor, cnt, writer, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::MemPushL(void* seq_cntr_ref_, size_t cnt,
                                       void const* src, size_t src_stride,
                                       void* dst_cursor) {
    Call_(MemPushL, cnt, src, src_stride, dst_cursor);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::MemPushR(void* seq_cntr_ref_, size_t cnt,
                                       void const* src, size_t src_stride,
                                       void* dst_cursor) {
    Call_(MemPushR, cnt, src, src_stride, dst_cursor);
}

template <typename IsConst>
void* SeqCntrRefTpl<IsConst>::MemInsert(void* seq_cntr_ref_, void* pos_cursor,
                                        size_t cnt, void const* src,
                                        size_t src_stride, void* dst_cursor) {
    Call_(MemInsert, pos_cursor, cnt, src, src_stride, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::PopL(void* seq_cntr_ref_, size_t cnt) {
    Call_(PopL, cnt);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::PopR(void* seq_cntr_ref_, size_t cnt) {
    Call_(PopR, cnt);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::Erase(void* seq_cntr_ref_, void* pos_cursor,
                                   size_t cnt) {
    Call_(Erase, pos_cursor, cnt);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::EraseAll(void* seq_cntr_ref_) {
    Call_(EraseAll);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
bool SeqCntrRefTpl<IsConst>::AreEqualCursor(void const* seq_cntr_ref_,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    ConstCall_(AreEqualCursor, cursor_a, cursor_b);
}

template <typename IsConst>
int SeqCntrRefTpl<IsConst>::CompareCursor(void const* seq_cntr_ref_,
                                          void const* cursor_a,
                                          void const* cursor_b) {
    ConstCall_(CompareCursor, cursor_a, cursor_b);
}

template <typename IsConst>
size_t SeqCntrRefTpl<IsConst>::GetCursorDist(void const* seq_cntr_ref_,
                                             void const* cursor_a,
                                             void const* cursor_b) {
    ConstCall_(GetCursorDist, cursor_a, cursor_b);
}

template <typename IsConst>
size_t SeqCntrRefTpl<IsConst>::GetCursorIdx(void const* seq_cntr_ref_,
                                            void const* cursor) {
    ConstCall_(GetCursorIdx, cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::CursorStepL(void const* seq_cntr_ref_,
                                         void* cursor) {
    ConstCall_(CursorStepL, cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::CursorStepR(void const* seq_cntr_ref_,
                                         void* cursor) {
    ConstCall_(CursorStepR, cursor);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::CursorAdvanceL(void const* seq_cntr_ref_,
                                            void* cursor, size_t step) {
    ConstCall_(CursorAdvanceL, cursor, step);
}

template <typename IsConst>
void SeqCntrRefTpl<IsConst>::CursorAdvanceR(void const* seq_cntr_ref_,
                                            void* cursor, size_t step) {
    ConstCall_(CursorAdvanceR, cursor, step);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
bool SeqCntrRefTpl<IsConst>::CheckCntr(void const* seq_cntr_ref_) {
    auto seq_cntr{ static_cast<SeqCntrRefTpl const*>(seq_cntr_ref_) };

    if (!(seq_cntr != nullptr)) { return false; }
    if (!(seq_cntr->inst != nullptr)) { return false; }
    if (!(seq_cntr->vtable != nullptr)) { return false; }

    return true;
}

}  // namespace zeta::core::seq_cntr

#pragma pop_macro("Call_")
#pragma pop_macro("ConstCall_")
