#pragma once

#include <unordered_map>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/ptr_iter.hpp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test::seq_cntr_utils {

using SeqCntr = core::seq_cntr::SeqCntrRef;
using VTable = core::seq_cntr::SeqCntrVTable;

template <typename Elem>
size_t GetRandomStride() {
    return sizeof(Elem) + alignof(Elem) * GetRandomInt<size_t>(1, 4);
}

// -----------------------------------------------------------------------------

inline auto& GetDestroyFuncs() {
    static std::unordered_map<void const*, void (*)(SeqCntr cntr)> instance;
    return instance;
}

inline void AddDestroyFunc(void const* inst, void (*Destroy)(SeqCntr cntr)) {
    auto& map{ GetDestroyFuncs() };

    auto iter{ map.insert({ inst, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

inline void Destroy(SeqCntr cntr) {
    auto& map{ GetDestroyFuncs() };

    auto iter{ map.find(cntr.inst) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(cntr);
}

// -----------------------------------------------------------------------------

inline auto& GetSanitizeFuncs() {
    static std::unordered_map<void const*, void (*)(SeqCntr cntr)> instance;
    return instance;
}

inline void AddSanitizeFunc(void const* inst, void (*Sanitize)(SeqCntr cntr)) {
    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.insert({ inst, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

inline void Sanitize(SeqCntr cntr) {
    if (cntr.inst == nullptr) { return; }

    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.find(cntr.inst) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(cntr);
}

// -----------------------------------------------------------------------------

inline void Read_(SeqCntr cntr, size_t idx, size_t cnt, void* dst,
                  size_t dst_stride) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    size_t size{ SeqCntr::GetSize(&cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    SeqCntr::Access(&cntr, idx, pos_cursor, nullptr);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    Sanitize(cntr);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::MemRead(&cntr, pos_cursor, cnt, dst, dst_stride, dst_cursor);

    Sanitize(cntr);

    if (pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);
    }

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, dst_cursor) ==
                          idx + cnt);

    Sanitize(cntr);
}

inline void Read(SeqCntr cntr, size_t idx, size_t cnt, void* dst,
                 size_t dst_stride) {
    size_t size{ SeqCntr::GetSize(&cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ core::Min(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        Read_(cntr, idx, cnt_a, dst, dst_stride);

        dst = static_cast<char*>(dst) + dst_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) { Read_(cntr, 0, cnt_b, dst, dst_stride); }
}

inline void Write_(SeqCntr cntr, size_t idx, size_t cnt, void const* src,
                   size_t src_stride) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    size_t width{ cntr.width };

    size_t size{ SeqCntr::GetSize(&cntr) };

    Sanitize(cntr);

    ZETA_Core_DebugAssert(cnt <= size);

    SeqCntr::Access(&cntr, idx, pos_cursor, nullptr);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    Sanitize(cntr);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::MemWrite(&cntr, pos_cursor, cnt, src, src_stride, dst_cursor);

    Sanitize(cntr);

    if (pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

        Sanitize(cntr);
    }

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, dst_cursor) ==
                          idx + cnt);

    Sanitize(cntr);

    void* buffer{ std::malloc(src_stride * cnt) };

    Read(cntr, idx, cnt, buffer, src_stride);

    ZETA_Core_DebugAssert(core::ElemCompare(src, buffer, width, src_stride,
                                            src_stride, cnt) == 0);

    std::free(buffer);
}

inline void Write(SeqCntr cntr, size_t idx, size_t cnt, void const* src,
                  size_t src_stride) {
    size_t size{ SeqCntr::GetSize(&cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ core::Min(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        Write_(cntr, idx, cnt_a, src, src_stride);

        src = static_cast<char const*>(src) + src_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) { Write_(cntr, 0, cnt_b, src, src_stride); }
}

inline void PushL(SeqCntr cntr, size_t cnt, void const* src,
                  size_t src_stride) {
    SeqCntr::MemPushL(&cntr, cnt, src, src_stride, nullptr);

    Sanitize(cntr);
}

inline void PushR(SeqCntr cntr, size_t cnt, void const* src,
                  size_t src_stride) {
    SeqCntr::MemPushR(&cntr, cnt, src, src_stride, nullptr);

    Sanitize(cntr);
}

inline void PopL(SeqCntr cntr, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= SeqCntr::GetSize(&cntr));

    SeqCntr::PopL(&cntr, cnt);

    Sanitize(cntr);
}

inline void PopR(SeqCntr cntr, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= SeqCntr::GetSize(&cntr));

    Sanitize(cntr);

    SeqCntr::PopR(&cntr, cnt);

    Sanitize(cntr);
}

inline void FnInsert(SeqCntr cntr, size_t idx, size_t cnt,
                     zeta::core::seq_cntr::FnWriter writer) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::Access(&cntr, idx, pos_cursor, nullptr);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    Sanitize(cntr);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::FnInsert(&cntr, pos_cursor, cnt, writer, dst_cursor);

    Sanitize(cntr);

    if (dst_cursor != pos_cursor) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

        Sanitize(cntr);
    }

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, dst_cursor) ==
                          idx + cnt);

    Sanitize(cntr);
}

inline void MemInsert(SeqCntr cntr, size_t idx, size_t cnt, void const* src,
                      size_t src_stride) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::Access(&cntr, idx, pos_cursor, nullptr);

    ZETA_Core_PrintCurPos;

    Sanitize(cntr);

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    ZETA_Core_PrintCurPos;

    Sanitize(cntr);

    ZETA_Core_PrintCurPos;

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    ZETA_Core_PrintCurPos;

    size_t old_size{ SeqCntr::GetSize(&cntr) };

    SeqCntr::MemInsert(&cntr, pos_cursor, cnt, src, src_stride, dst_cursor);

    size_t new_size{ SeqCntr::GetSize(&cntr) };

    ZETA_Core_PrintCurPos;

    Sanitize(cntr);

    ZETA_Core_DebugAssert(old_size + cnt == new_size);

    ZETA_Core_PrintCurPos;

    if (dst_cursor != pos_cursor) {
        ZETA_Core_PrintCurPos;

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

        ZETA_Core_PrintCurPos;

        Sanitize(cntr);

        ZETA_Core_PrintCurPos;
    }

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, dst_cursor) ==
                          idx + cnt);

    ZETA_Core_PrintCurPos;

    Sanitize(cntr);

    ZETA_Core_PrintCurPos;

    void* buffer{ std::malloc(src_stride * cnt) };

    ZETA_Core_PrintCurPos;

    Read(cntr, idx, cnt, buffer, src_stride);

    ZETA_Core_PrintCurPos;

    Sanitize(cntr);

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(core::ElemCompare(src, buffer, cntr.width, src_stride,
                                            src_stride, cnt) == 0);

    std::free(buffer);
}

inline void Erase(SeqCntr cntr, size_t idx, size_t cnt) {
    if (cntr.inst == nullptr) { return; }

    size_t size{ SeqCntr::GetSize(&cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::Access(&cntr, idx, pos_cursor, nullptr);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    Sanitize(cntr);

    size_t cur_cnt{ std::min(size - idx, cnt) };

    SeqCntr::Erase(&cntr, pos_cursor, cur_cnt);

    Sanitize(cntr);

    cnt -= cur_cnt;

    ZETA_Core_DebugAssert(SeqCntr::GetSize(&cntr) == size - cur_cnt);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == idx);

    Sanitize(cntr);

    SeqCntr::PeekL(&cntr, pos_cursor, nullptr);

    Sanitize(cntr);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, pos_cursor) == 0);

    SeqCntr::Erase(&cntr, pos_cursor, cnt);

    Sanitize(cntr);
}

inline void CheckCursor(SeqCntr cntr, size_t max_op_size) {
    ZETA_Core_DebugAssert(cntr.inst != nullptr);

    size_t size{ SeqCntr::GetSize(&cntr) };

    void* cursor{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    SeqCntr::GetLBCursor(&cntr, cursor);

    for (size_t idx{ static_cast<size_t>(-1) }; idx != size; ++idx) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor) == idx);

        SeqCntr::CursorStepR(&cntr, cursor);
    }

    SeqCntr::GetRBCursor(&cntr, cursor);

    for (size_t idx{ size }; idx != static_cast<size_t>(-1); --idx) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor) == idx);

        SeqCntr::CursorStepL(&cntr, cursor);
    }

    void* cursor_a{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };
    void* cursor_b{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };
    void* cursor_c{ ZETA_Core_SeqCntr_AllocaCursor(&cntr) };

    for (size_t i{ 0 }; i < max_op_size; ++i) {
        size_t idx_a{ static_cast<size_t>(GetRandomInt<long long>(-1, size)) };
        size_t idx_b{ static_cast<size_t>(GetRandomInt<long long>(-1, size)) };

        SeqCntr::Access(&cntr, idx_a, cursor_a, nullptr);
        SeqCntr::Access(&cntr, idx_b, cursor_b, nullptr);

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor_a) == idx_a);
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor_b) == idx_b);

        ZETA_Core_DebugAssert(
            SeqCntr::AreEqualCursor(&cntr, cursor_a, cursor_b) ==
            (idx_a == idx_b));

        ZETA_Core_DebugAssert(
            SeqCntr::CompareCursor(&cntr, cursor_a, cursor_b) ==
            core::compare::Compare(idx_a + 1, idx_b + 1));

        ZETA_Core_DebugAssert(
            SeqCntr::GetCursorDist(&cntr, cursor_a, cursor_b) == idx_b - idx_a);

        // ---------------------------------------------------------------------

        SeqCntr::Access(&cntr, idx_a, cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            SeqCntr::CursorAdvanceR(&cntr, cursor_c, idx_b - idx_a);
        } else {
            SeqCntr::CursorAdvanceL(&cntr, cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor_c) == idx_b);

        // ---------------------------------------------------------------------

        SeqCntr::Access(&cntr, idx_b, cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            SeqCntr::CursorAdvanceL(&cntr, cursor_c, idx_b - idx_a);
        } else {
            SeqCntr::CursorAdvanceR(&cntr, cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr, cursor_c) == idx_a);
    }
}

// -----------------------------------------------------------------------------

inline size_t SyncGetSize(std::vector<SeqCntr> const& cntrs) {
    ZETA_Core_DebugAssert(!cntrs.empty());

    for (auto cntr : cntrs) { ZETA_Core_DebugAssert(cntr.vtable != nullptr); }

    size_t size{ SeqCntr::GetSize(&cntrs[0]) };

    for (auto cntr : cntrs) {
        ZETA_Core_DebugAssert(size == SeqCntr::GetSize(&cntr));
    }

    return size;
}

template <typename Elem>
void SyncRandomRead(std::vector<SeqCntr> const& cntrs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(cntrs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    bool read{ false };

    for (auto cntr : cntrs) {
        Read(cntr, idx, cnt, buffer_b, stride);

        if (read) {
            ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                    sizeof(Elem), stride,
                                                    stride, cnt) == 0);
        } else {
            read = true;
            std::memcpy(buffer_a, buffer_b, stride * cnt);
        }
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem>
void SyncRandomWrite(std::vector<SeqCntr> const& cntrs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(cntrs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto cntr : cntrs) {
        Write(cntr, idx, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                sizeof(Elem), stride, stride,
                                                cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem>
void SyncRandomPushL(std::vector<SeqCntr> const& cntrs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto cntr : cntrs) {
        PushL(cntr, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                sizeof(Elem), stride, stride,
                                                cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem>
void SyncRandomPushR(std::vector<SeqCntr> const& cntrs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto cntr : cntrs) {
        PushR(cntr, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                sizeof(Elem), stride, stride,
                                                cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

inline void SyncRandomPopL(std::vector<SeqCntr> const& cntrs,
                           size_t max_op_size) {
    size_t size{ SyncGetSize(cntrs) };

    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto cntr : cntrs) { PopL(cntr, cnt); }
}

inline void SyncRandomPopR(std::vector<SeqCntr> const& cntrs,
                           size_t max_op_size) {
    size_t size{ SyncGetSize(cntrs) };

    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto cntr : cntrs) { PopR(cntr, cnt); }
}

template <typename Elem>
void SyncRandomInsert(std::vector<SeqCntr> const& cntrs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(cntrs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto cntr : cntrs) {
        MemInsert(cntr, idx, cnt, buffer_b, stride);

        ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                sizeof(Elem), stride, stride,
                                                cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

inline void SyncRandomErase(std::vector<SeqCntr> const& cntrs,
                            size_t max_op_size) {
    size_t size{ SyncGetSize(cntrs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto cntr : cntrs) { Erase(cntr, idx, cnt); }
}

template <typename Elem>
void SyncRandomInit(std::vector<SeqCntr> const& cntrs, size_t size) {
    size_t stride{ GetRandomStride<Elem>() };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * size)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * size)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(size));

    std::memcpy(buffer_b, buffer_a, stride * size);

    for (auto cntr : cntrs) {
        Erase(cntr, 0, SeqCntr::GetSize(&cntr));

        PushR(cntr, size, buffer_b, stride);

        ZETA_Core_DebugAssert(core::ElemCompare(buffer_a, buffer_b,
                                                sizeof(Elem), stride, stride,
                                                size) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem>
void SyncCompare2_(SeqCntr cntr_a, SeqCntr cntr_b) {
    size_t width_a{ cntr_a.width };
    size_t width_b{ cntr_b.width };

    ZETA_Core_DebugAssert(width_a == width_b);

    size_t stride{ GetRandomStride<Elem>() };

    size_t size_a{ SeqCntr::GetSize(&cntr_a) };
    size_t size_b{ SeqCntr::GetSize(&cntr_b) };

    ZETA_Core_DebugAssert(size_a == size_b);

    void* cursor_a{ ZETA_Core_SeqCntr_AllocaCursor(&cntr_a) };
    void* cursor_b{ ZETA_Core_SeqCntr_AllocaCursor(&cntr_b) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * size_a)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * size_b)) };

    // -------------------------------------------------------------------------

    SeqCntr::PeekL(&cntr_a, cursor_a, nullptr);
    SeqCntr::PeekL(&cntr_b, cursor_b, nullptr);

    for (size_t i{ 0 }; i < size_a; ++i) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_a, cursor_a) == i);

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_b, cursor_b) == i);

        SeqCntr::MemRead(&cntr_a, cursor_a, 1, buffer_a, width_a, cursor_a);

        SeqCntr::MemRead(&cntr_b, cursor_b, 1, buffer_b, width_b, cursor_b);

        ZETA_Core_DebugAssert(
            core::MemCompare(buffer_a, buffer_b, sizeof(Elem)) == 0);
    }

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_a, cursor_a) == size_a);

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_b, cursor_b) == size_b);

    // -------------------------------------------------------------------------

    SeqCntr::PeekR(&cntr_a, cursor_a, nullptr);
    SeqCntr::PeekR(&cntr_b, cursor_b, nullptr);

    for (size_t i{ size_a }; 0 < i--;) {
        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_a, cursor_a) == i);

        ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_b, cursor_b) == i);

        SeqCntr::MemRead(&cntr_a, cursor_a, 1, buffer_a, width_a, nullptr);

        SeqCntr::MemRead(&cntr_b, cursor_b, 1, buffer_b, width_b, nullptr);

        ZETA_Core_DebugAssert(
            core::MemCompare(buffer_a, buffer_b, sizeof(Elem)) == 0);

        SeqCntr::CursorStepL(&cntr_a, cursor_a);

        SeqCntr::CursorStepL(&cntr_b, cursor_b);
    }

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_a, cursor_a) ==
                          static_cast<size_t>(-1));

    ZETA_Core_DebugAssert(SeqCntr::GetCursorIdx(&cntr_b, cursor_b) ==
                          static_cast<size_t>(-1));

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem>
void SyncCompare(std::vector<SeqCntr> const& cntrs) {
    for (auto cntr : cntrs) { ZETA_Core_DebugAssert(cntr.vtable != nullptr); }

    size_t cntr_cnt{ cntrs.size() };

    if (cntr_cnt == 1) { return; }

    for (size_t i{ 0 }; i < cntr_cnt; ++i) {
        SyncCompare2_<Elem>(cntrs[i], cntrs[(i + 1) % cntr_cnt]);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

enum OpEnum : int {
    READ,
    WRITE,
    PUSH_L,
    PUSH_R,
    POP_L,
    POP_R,
    INSERT,
    ERASE,

    CURSOR_STEP_L,
    CURSOR_STEP_R,
    CURSOR_ADVANCE_L,
    CURSOR_ADVANCE_R,
};

template <typename Elem>
void DoRandomOperations(std::vector<SeqCntr> cntrs,
                        size_t iter_cnt,  //

                        size_t read_max_op_size,   //
                        size_t write_max_op_size,  //
                        size_t push_l_max_op_size,
                        size_t push_r_max_op_size,  //
                        size_t pop_l_max_op_size,   //
                        size_t pop_r_max_op_size,   //
                        size_t insert_max_op_size,  //
                        size_t erase_max_op_size,   //

                        size_t cursor_step_l_max_op_size,  //
                        size_t cursor_step_r_max_op_size,  //
                        size_t cursor_advance_l_op_size,   //
                        size_t cursor_advance_r_op_size    //
) {
    ZETA_Core_Unused(cursor_step_l_max_op_size);
    ZETA_Core_Unused(cursor_step_r_max_op_size);
    ZETA_Core_Unused(cursor_advance_l_op_size);
    ZETA_Core_Unused(cursor_advance_r_op_size);

    std::vector<int> ops;

    if (0 < read_max_op_size) { ops.push_back(OpEnum::READ); }

    if (0 < write_max_op_size) { ops.push_back(OpEnum::WRITE); }

    if (0 < push_l_max_op_size) { ops.push_back(OpEnum::PUSH_L); }

    if (0 < push_r_max_op_size) { ops.push_back(OpEnum::PUSH_R); }

    if (0 < pop_l_max_op_size) { ops.push_back(OpEnum::POP_L); }

    if (0 < pop_r_max_op_size) { ops.push_back(OpEnum::POP_R); }

    if (0 < insert_max_op_size) { ops.push_back(OpEnum::INSERT); }

    if (0 < erase_max_op_size) { ops.push_back(OpEnum::ERASE); }

    ZETA_Core_DebugAssert(!ops.empty());

    for (size_t iter_i{ 0 }; iter_i < iter_cnt; ++iter_i) {
        switch (ops[GetRandomInt<size_t>(0, ops.size() - 1)]) {
        case OpEnum::READ:
            ZETA_Core_PrintVar("READ");
            SyncRandomRead<Elem>(cntrs, read_max_op_size);
            break;

        case OpEnum::WRITE:
            ZETA_Core_PrintVar("WRITE");
            SyncRandomWrite<Elem>(cntrs, write_max_op_size);
            break;

        case OpEnum::PUSH_L:
            ZETA_Core_PrintVar("PUSH_L");
            SyncRandomPushL<Elem>(cntrs, push_l_max_op_size);
            break;

        case OpEnum::PUSH_R:
            ZETA_Core_PrintVar("PUSH_R");
            SyncRandomPushR<Elem>(cntrs, push_r_max_op_size);
            break;

        case OpEnum::POP_L:
            ZETA_Core_PrintVar("POP_L");
            SyncRandomPopL(cntrs, pop_l_max_op_size);
            break;

        case OpEnum::POP_R:
            ZETA_Core_PrintVar("POP_R");
            SyncRandomPopR(cntrs, pop_r_max_op_size);
            break;

        case OpEnum::INSERT:
            ZETA_Core_PrintVar("INSERT");
            SyncRandomInsert<Elem>(cntrs, insert_max_op_size);
            break;

        case OpEnum::ERASE:
            ZETA_Core_PrintVar("ERASE");
            SyncRandomErase(cntrs, erase_max_op_size);
            break;
        }

        for (auto cntr : cntrs) { CheckCursor(cntr, 16); }

        SyncCompare<Elem>(cntrs);
    }
}

}  // namespace zeta::core_test::seq_cntr_utils
