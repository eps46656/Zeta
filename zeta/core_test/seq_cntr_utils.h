#pragma once

#include <zeta/core/debugger.h>
#include <zeta/core/seq_cntr.h>
#include <zeta/core/utils.h>
#include <zeta/core_test/ptr_iter.h>
#include <zeta/core_test/random.h>

#include <unordered_map>

template <typename Elem>
size_t Zeta_CoreTest_SeqCntrUtils_GetRandomStride() {
    return sizeof(Elem) +
           alignof(Elem) * Zeta_CoreTest_GetRandomInt<size_t, size_t>(1, 4);
}

// -----------------------------------------------------------------------------

auto& Zeta_CoreTest_SeqCntrUtils_GetDestroyFuncs() {
    static std::unordered_map<Zeta_Core_SeqCntr_VTable const*,
                              void (*)(Zeta_Core_SeqCntr seq_cntr)>
        instance;
    return instance;
}

void Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
    Zeta_Core_SeqCntr_VTable const* seq_cntr_vtable,
    void (*Destroy)(Zeta_Core_SeqCntr seq_cntr)) {
    auto& map{ Zeta_CoreTest_SeqCntrUtils_GetDestroyFuncs() };

    auto iter{ map.insert({ seq_cntr_vtable, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

void Zeta_CoreTest_SeqCntrUtils_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    auto& map{ Zeta_CoreTest_SeqCntrUtils_GetDestroyFuncs() };

    auto iter{ map.find(seq_cntr.vtable) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != NULL);

    iter->second(seq_cntr);
}

// ---

auto& Zeta_CoreTest_SeqCntrUtils_GetSanitizeFuncs() {
    static std::unordered_map<Zeta_Core_SeqCntr_VTable const*,
                              void (*)(Zeta_Core_SeqCntr seq_cntr)>
        instance;
    return instance;
}

void Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
    Zeta_Core_SeqCntr_VTable const* seq_cntr_vtable,
    void (*Sanitize)(Zeta_Core_SeqCntr seq_cntr)) {
    auto& map{ Zeta_CoreTest_SeqCntrUtils_GetSanitizeFuncs() };

    auto iter{ map.insert({ seq_cntr_vtable, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

void Zeta_CoreTest_SeqCntrUtils_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    if (seq_cntr.context == NULL) { return; }

    auto& map{ Zeta_CoreTest_SeqCntrUtils_GetSanitizeFuncs() };

    auto iter{ map.find(seq_cntr.vtable) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != NULL);

    iter->second(seq_cntr);
}

// -----------------------------------------------------------------------------

void Zeta_CoreTest_SeqCntrUtils_Read(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                     size_t cnt, void* dst, size_t dst_stride) {
    void* pos_cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    size_t size{ ZETA_Core_SeqCntr_GetSize(seq_cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ ZETA_Core_GetMinOf(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        ZETA_Core_SeqCntr_Access(seq_cntr, idx, pos_cursor, NULL);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_SeqCntr_Read(seq_cntr, pos_cursor, cnt_a, dst, dst_stride,
                               pos_cursor);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(ZETA_Core_SeqCntr_Cursor_GetIdx(
                                  seq_cntr, pos_cursor) == idx + cnt_a);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        dst = (char*)dst + dst_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) {
        ZETA_Core_SeqCntr_Access(seq_cntr, 0, pos_cursor, NULL);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == 0);

        ZETA_Core_SeqCntr_Read(seq_cntr, pos_cursor, cnt_b, dst, dst_stride,
                               pos_cursor);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == cnt_b);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);
    }
}

void Zeta_CoreTest_SeqCntrUtils_Write(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                      size_t cnt, void const* src,
                                      size_t src_stride) {
    void* pos_cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    size_t width{ ZETA_Core_SeqCntr_GetWidth(seq_cntr) };

    size_t size{ ZETA_Core_SeqCntr_GetSize(seq_cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ ZETA_Core_GetMinOf(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    void* ba{ std::malloc(src_stride * cnt) };

    if (0 < cnt_a) {
        ZETA_Core_SeqCntr_Access(seq_cntr, idx, pos_cursor, NULL);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_SeqCntr_Write(seq_cntr, pos_cursor, cnt_a, src, src_stride,
                                pos_cursor);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        Zeta_CoreTest_SeqCntrUtils_Read(seq_cntr, idx, cnt_a, ba, src_stride);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(src, ba, width, src_stride,
                                                    src_stride, cnt_a) == 0);

        ZETA_Core_DebugAssert(ZETA_Core_SeqCntr_Cursor_GetIdx(
                                  seq_cntr, pos_cursor) == idx + cnt_a);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        src = (char const*)src + src_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) {
        ZETA_Core_SeqCntr_Access(seq_cntr, 0, pos_cursor, NULL);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == 0);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_SeqCntr_Write(seq_cntr, pos_cursor, cnt_b, src, src_stride,
                                pos_cursor);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        Zeta_CoreTest_SeqCntrUtils_Read(seq_cntr, 0, cnt_b, ba, src_stride);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(src, ba, width, src_stride,
                                                    src_stride, cnt_b) == 0);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == cnt_b);

        Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);
    }

    std::free(ba);
}

void Zeta_CoreTest_SeqCntrUtils_PushL(Zeta_Core_SeqCntr seq_cntr, size_t cnt,
                                      void const* src, size_t src_stride) {
    ZETA_Core_SeqCntr_PushL(seq_cntr, cnt, NULL);

    Zeta_CoreTest_SeqCntrUtils_Write(seq_cntr, 0, cnt, src, src_stride);
}

void Zeta_CoreTest_SeqCntrUtils_PushR(Zeta_Core_SeqCntr seq_cntr, size_t cnt,
                                      void const* src, size_t src_stride) {
    size_t size{ ZETA_Core_SeqCntr_GetSize(seq_cntr) };

    ZETA_Core_SeqCntr_PushR(seq_cntr, cnt, NULL);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    Zeta_CoreTest_SeqCntrUtils_Write(seq_cntr, size, cnt, src, src_stride);
}

void Zeta_CoreTest_SeqCntrUtils_PopL(Zeta_Core_SeqCntr seq_cntr, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= ZETA_Core_SeqCntr_GetSize(seq_cntr));

    ZETA_Core_SeqCntr_PopL(seq_cntr, cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);
}

void Zeta_CoreTest_SeqCntrUtils_PopR(Zeta_Core_SeqCntr seq_cntr, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= ZETA_Core_SeqCntr_GetSize(seq_cntr));

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_SeqCntr_PopR(seq_cntr, cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);
}

void Zeta_CoreTest_SeqCntrUtils_Insert(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                       size_t cnt, void const* src,
                                       size_t src_stride) {
    void* pos_cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    ZETA_Core_SeqCntr_Access(seq_cntr, idx, pos_cursor, NULL);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_SeqCntr_Insert(seq_cntr, pos_cursor, cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

    Zeta_CoreTest_SeqCntrUtils_Write(seq_cntr, idx, cnt, src, src_stride);
}

void Zeta_CoreTest_SeqCntrUtils_Erase(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                      size_t cnt) {
    if (seq_cntr.context == NULL) { return; }

    size_t size{ ZETA_Core_SeqCntr_GetSize(seq_cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    void* pos_cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    ZETA_Core_SeqCntr_Access(seq_cntr, idx, pos_cursor, NULL);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    size_t cur_cnt{ std::min(size - idx, cnt) };

    ZETA_Core_SeqCntr_Erase(seq_cntr, pos_cursor, cur_cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    cnt -= cur_cnt;

    ZETA_Core_DebugAssert(ZETA_Core_SeqCntr_GetSize(seq_cntr) ==
                          size - cur_cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == idx);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_SeqCntr_PeekL(seq_cntr, pos_cursor, NULL);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, pos_cursor) == 0);

    ZETA_Core_SeqCntr_Erase(seq_cntr, pos_cursor, cnt);

    Zeta_CoreTest_SeqCntrUtils_Sanitize(seq_cntr);
}

void Zeta_CoreTest_SeqCntrUtils_CheckCursor(Zeta_Core_SeqCntr seq_cntr,
                                            size_t max_op_size) {
    ZETA_Core_DebugAssert(seq_cntr.context != NULL);

    size_t size = ZETA_Core_SeqCntr_GetSize(seq_cntr);

    void* cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    ZETA_Core_SeqCntr_GetLBCursor(seq_cntr, cursor);

    for (size_t idx = -1; idx != size; ++idx) {
        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor) == idx);
        ZETA_Core_SeqCntr_Cursor_StepR(seq_cntr, cursor);
    }

    ZETA_Core_SeqCntr_GetRBCursor(seq_cntr, cursor);

    for (size_t idx = size; idx != (size_t)(-1); --idx) {
        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor) == idx);
        ZETA_Core_SeqCntr_Cursor_StepL(seq_cntr, cursor);
    }

    void* cursor_a = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);
    void* cursor_b = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);
    void* cursor_c = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    for (size_t i = 0; i < max_op_size; ++i) {
        size_t idx_a{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(-1, size) };
        size_t idx_b{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(-1, size) };

        ZETA_Core_SeqCntr_Access(seq_cntr, idx_a, cursor_a, NULL);
        ZETA_Core_SeqCntr_Access(seq_cntr, idx_b, cursor_b, NULL);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor_a) == idx_a);
        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor_b) == idx_b);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_AreEqual(seq_cntr, cursor_a, cursor_b) ==
            (idx_a == idx_b));

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_Compare(seq_cntr, cursor_a, cursor_b) ==
            ZETA_Core_ThreeWayCompare(idx_a + 1, idx_b + 1));

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetDist(seq_cntr, cursor_a, cursor_b) ==
            idx_b - idx_a);

        // ---

        ZETA_Core_SeqCntr_Access(seq_cntr, idx_a, cursor_c, NULL);

        if (idx_a <= idx_b) {
            ZETA_Core_SeqCntr_Cursor_AdvanceR(seq_cntr, cursor_c,
                                              idx_b - idx_a);
        } else {
            ZETA_Core_SeqCntr_Cursor_AdvanceL(seq_cntr, cursor_c,
                                              idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor_c) == idx_b);

        // ---

        ZETA_Core_SeqCntr_Access(seq_cntr, idx_b, cursor_c, NULL);

        if (idx_a <= idx_b) {
            ZETA_Core_SeqCntr_Cursor_AdvanceL(seq_cntr, cursor_c,
                                              idx_b - idx_a);
        } else {
            ZETA_Core_SeqCntr_Cursor_AdvanceR(seq_cntr, cursor_c,
                                              idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor_c) == idx_a);
    }
}

// -----------------------------------------------------------------------------

size_t Zeta_CoreTest_SeqCntrUtils_SyncGetSize(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs) {
    ZETA_Core_DebugAssert(!seq_cntrs.empty());

    for (auto seq_cntr : seq_cntrs) {
        ZETA_Core_DebugAssert(seq_cntr.vtable != NULL);
    }

    size_t size{ ZETA_Core_SeqCntr_GetSize(seq_cntrs[0]) };

    for (auto seq_cntr : seq_cntrs) {
        ZETA_Core_DebugAssert(size == ZETA_Core_SeqCntr_GetSize(seq_cntr));
    }

    return size;
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomRead(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, size) };
    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        0, std::min(max_op_size, size)) };

    Elem* ba{ (Elem*)std::malloc(stride * cnt) };
    Elem* bb{ (Elem*)std::malloc(stride * cnt) };

    bool_t read{ FALSE };

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_Read(seq_cntr, idx, cnt, ba, stride);

        if (read) {
            ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                        stride, stride,
                                                        cnt) == 0);
        } else {
            read = TRUE;
            std::memcpy(bb, ba, stride * cnt);
        }
    }

    std::free(ba);
    std::free(bb);
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomWrite(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, size) };
    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        0, std::min(max_op_size, size)) };

    Elem* ba{ (Elem*)std::malloc(stride * cnt) };
    Elem* bb{ (Elem*)std::malloc(stride * cnt) };

    GetRandoms<Elem>(
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) },
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) } +
            cnt);

    std::memcpy(bb, ba, stride * cnt);

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_Write(seq_cntr, idx, cnt, ba, stride);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                    stride, stride, cnt) == 0);
    }

    std::free(ba);
    std::free(bb);
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomPushL(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, max_op_size) };

    Elem* ba{ (Elem*)std::malloc(stride * cnt) };
    Elem* bb{ (Elem*)std::malloc(stride * cnt) };

    GetRandoms<Elem>(
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) },
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) } +
            cnt);

    std::memcpy(bb, ba, stride * cnt);

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_PushL(seq_cntr, cnt, ba, stride);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                    stride, stride, cnt) == 0);
    }

    std::free(ba);
    std::free(bb);
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomPushR(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, max_op_size) };

    Elem* ba{ (Elem*)std::malloc(stride * cnt) };
    Elem* bb{ (Elem*)std::malloc(stride * cnt) };

    GetRandoms<Elem>(
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) },
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) } +
            cnt);

    std::memcpy(bb, ba, stride * cnt);

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_PushR(seq_cntr, cnt, ba, stride);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                    stride, stride, cnt) == 0);
    }

    std::free(ba);
    std::free(bb);
}

void Zeta_CoreTest_SeqCntrUtils_SyncRandomPopL(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        0, std::min(max_op_size, size)) };

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_PopL(seq_cntr, cnt);
    }
}

void Zeta_CoreTest_SeqCntrUtils_SyncRandomPopR(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        0, std::min(max_op_size, size)) };

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_PopR(seq_cntr, cnt);
    }
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomInsert(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, size) };
    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, max_op_size) };

    Elem* ba{ (Elem*)std::malloc(stride * cnt) };
    Elem* bb{ (Elem*)std::malloc(stride * cnt) };

    GetRandoms<Elem>(
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) },
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) } +
            cnt);

    std::memcpy(bb, ba, stride * cnt);

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_Insert(seq_cntr, idx, cnt, ba, stride);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                    stride, stride, cnt) == 0);
    }

    std::free(ba);
    std::free(bb);
}

void Zeta_CoreTest_SeqCntrUtils_SyncRandomErase(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t max_op_size) {
    size_t size{ Zeta_CoreTest_SeqCntrUtils_SyncGetSize(seq_cntrs) };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, size) };
    size_t cnt{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        0, std::min(max_op_size, size)) };

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_Erase(seq_cntr, idx, cnt);
    }
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncRandomInit(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs, size_t size) {
    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    Elem* ba{ (Elem*)std::malloc(stride * size) };
    Elem* bb{ (Elem*)std::malloc(stride * size) };

    GetRandoms<Elem>(
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) },
        Zeta_CoreTest_PtrIter<Elem>{ ba, static_cast<ptrdiff_t>(stride) } +
            size);

    std::memcpy(bb, ba, stride * size);

    for (auto seq_cntr : seq_cntrs) {
        Zeta_CoreTest_SeqCntrUtils_Erase(seq_cntr, 0,
                                         ZETA_Core_SeqCntr_GetSize(seq_cntr));

        Zeta_CoreTest_SeqCntrUtils_PushR(seq_cntr, size, ba, stride);

        ZETA_Core_DebugAssert(Zeta_Core_ElemCompare(ba, bb, sizeof(Elem),
                                                    stride, stride, size) == 0);
    }

    std::free(ba);
    std::free(bb);
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncCompare2_(Zeta_Core_SeqCntr seq_cntr_a,
                                              Zeta_Core_SeqCntr seq_cntr_b) {
    size_t width_a{ ZETA_Core_SeqCntr_GetWidth(seq_cntr_a) };
    size_t width_b{ ZETA_Core_SeqCntr_GetWidth(seq_cntr_b) };

    ZETA_Core_DebugAssert(width_a == width_b);

    size_t stride{ Zeta_CoreTest_SeqCntrUtils_GetRandomStride<Elem>() };

    size_t size_a{ ZETA_Core_SeqCntr_GetSize(seq_cntr_a) };
    size_t size_b{ ZETA_Core_SeqCntr_GetSize(seq_cntr_b) };

    ZETA_Core_DebugAssert(size_a == size_b);

    void* cursor_a = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr_a);
    void* cursor_b = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr_b);

    Elem* ba{ (Elem*)std::malloc(stride * size_a) };
    Elem* bb{ (Elem*)std::malloc(stride * size_b) };

    // ---

    ZETA_Core_SeqCntr_PeekL(seq_cntr_a, cursor_a, NULL);
    ZETA_Core_SeqCntr_PeekL(seq_cntr_b, cursor_b, NULL);

    for (size_t i{ 0 }; i < size_a; ++i) {
        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_a, cursor_a) == i);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_b, cursor_b) == i);

        ZETA_Core_SeqCntr_Read(seq_cntr_a, cursor_a, 1, ba, width_a, cursor_a);

        ZETA_Core_SeqCntr_Read(seq_cntr_b, cursor_b, 1, bb, width_b, cursor_b);

        ZETA_Core_DebugAssert(Zeta_Core_MemCompare(ba, bb, sizeof(Elem)) == 0);
    }

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_a, cursor_a) == size_a);

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_b, cursor_b) == size_b);

    // ---

    ZETA_Core_SeqCntr_PeekR(seq_cntr_a, cursor_a, NULL);
    ZETA_Core_SeqCntr_PeekR(seq_cntr_b, cursor_b, NULL);

    for (size_t i{ size_a }; 0 < i--;) {
        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_a, cursor_a) == i);

        ZETA_Core_DebugAssert(
            ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_b, cursor_b) == i);

        ZETA_Core_SeqCntr_Read(seq_cntr_a, cursor_a, 1, ba, width_a, NULL);

        ZETA_Core_SeqCntr_Read(seq_cntr_b, cursor_b, 1, bb, width_b, NULL);

        ZETA_Core_DebugAssert(Zeta_Core_MemCompare(ba, bb, sizeof(Elem)) == 0);

        ZETA_Core_SeqCntr_Cursor_StepL(seq_cntr_a, cursor_a);

        ZETA_Core_SeqCntr_Cursor_StepL(seq_cntr_b, cursor_b);
    }

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_a, cursor_a) == (size_t)(-1));

    ZETA_Core_DebugAssert(
        ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr_b, cursor_b) == (size_t)(-1));

    std::free(ba);
    std::free(bb);
}

template <typename Elem>
void Zeta_CoreTest_SeqCntrUtils_SyncCompare(
    const std::vector<Zeta_Core_SeqCntr>& seq_cntrs) {
    for (auto seq_cntr : seq_cntrs) {
        ZETA_Core_DebugAssert(seq_cntr.vtable != NULL);
    }

    size_t seq_cntr_cnt{ seq_cntrs.size() };

    if (seq_cntr_cnt == 1) { return; }

    for (size_t i{ 0 }; i < seq_cntr_cnt; ++i) {
        Zeta_CoreTest_SeqCntrUtils_SyncCompare2_<Elem>(
            seq_cntrs[i], seq_cntrs[(i + 1) % seq_cntr_cnt]);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

enum Zeta_CoreTest_SeqCntrUtils_OpEnum : int {
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
void Zeta_CoreTest_SeqCntrUtils_DoRandomOperations(
    std::vector<Zeta_Core_SeqCntr> seq_cntrs,
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

    if (0 < read_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::READ);
    }

    if (0 < write_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::WRITE);
    }

    if (0 < push_l_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::PUSH_L);
    }

    if (0 < push_r_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::PUSH_R);
    }

    if (0 < pop_l_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::POP_L);
    }

    if (0 < pop_r_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::POP_R);
    }

    if (0 < insert_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::INSERT);
    }

    if (0 < erase_max_op_size) {
        ops.push_back(Zeta_CoreTest_SeqCntrUtils_OpEnum::ERASE);
    }

    ZETA_Core_DebugAssert(!ops.empty());

    for (size_t iter_i{ 0 }; iter_i < iter_cnt; ++iter_i) {
        switch (ops[Zeta_CoreTest_GetRandomInt<size_t, size_t>(
            0, ops.size() - 1)]) {
            case Zeta_CoreTest_SeqCntrUtils_OpEnum::READ:
                ZETA_Core_PrintVar("READ");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomRead<Elem>(
                    seq_cntrs, read_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::WRITE:
                ZETA_Core_PrintVar("WRITE");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomWrite<Elem>(
                    seq_cntrs, write_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::PUSH_L:
                ZETA_Core_PrintVar("PUSH_L");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomPushL<Elem>(
                    seq_cntrs, push_l_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::PUSH_R:
                ZETA_Core_PrintVar("PUSH_R");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomPushR<Elem>(
                    seq_cntrs, push_r_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::POP_L:
                ZETA_Core_PrintVar("POP_L");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomPopL(seq_cntrs,
                                                          pop_l_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::POP_R:
                ZETA_Core_PrintVar("POP_R");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomPopR(seq_cntrs,
                                                          pop_r_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::INSERT:
                ZETA_Core_PrintVar("INSERT");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomInsert<Elem>(
                    seq_cntrs, insert_max_op_size);
                break;

            case Zeta_CoreTest_SeqCntrUtils_OpEnum::ERASE:
                ZETA_Core_PrintVar("ERASE");
                Zeta_CoreTest_SeqCntrUtils_SyncRandomErase(seq_cntrs,
                                                           erase_max_op_size);
                break;
        }

        /*
        for (auto seq_cntr : seq_cntrs) {
            Zeta_CoreTest_SeqCntrUtils_CheckCursor(seq_cntr, 16);
        }
        */

        // Zeta_CoreTest_SeqCntrUtils_SyncCompare<Elem>(seq_cntrs);

        Zeta_Core_Debugger_ClearPipe();
    }
}
