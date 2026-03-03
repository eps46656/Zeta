#pragma once

#include <unordered_map>
#include <vector>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr_ref.hpp>
#include <zeta/core/seq_cntr_ref.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core_test/ptr_iter.hpp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test::seq_cntr_utils {

using VTable = core::seq_cntr::VTable;

template <typename Elem>
size_t GetRandomStride() {
    return sizeof(Elem) + alignof(Elem) * GetRandomInt<size_t>(1, 4);
}

inline auto& GetSanitizeFuncs() {
    static std::unordered_map<void const*, void (*)(void const* sc)> instance;
    return instance;
}

inline void AddSanitizeFunc(void const* sc, void (*Sanitize)(void const* sc)) {
    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.insert({ sc, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

inline void Sanitize(void const* sc) {
    if (sc == nullptr) { return; }

    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(sc);
}

inline void Sanitize(core::seq_cntr_ref::Ref const* sc) { Sanitize(sc->cntr); }

inline auto& GetDestroyFuncs() {
    static std::unordered_map<void*, void (*)(void* sc)> instance;
    return instance;
}

inline void AddDestroyFunc(void* sc, void (*Destroy)(void* sc)) {
    auto& map{ GetDestroyFuncs() };

    auto iter{ map.insert({ sc, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

inline void Destroy(void* sc) {
    if (sc == nullptr) { return; }

    auto& map{ GetDestroyFuncs() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(sc);
}

inline void Destroy(core::seq_cntr_ref::Ref* sc) { Destroy(sc->cntr); }

template <typename SeqCntrView>
void Read_(SeqCntrView* sc, size_t idx, size_t cnt, void* dst,
           size_t dst_stride) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    size_t size{ core::seq_cntr::ops::GetSize(sc) };

    ZETA_Core_DebugAssert(cnt <= size);

    core::seq_cntr::ops::Access(sc, idx, true, pos_cursor, nullptr);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          idx);

    Sanitize(sc);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(sc) };

    core::seq_cntr::ops::Read(
        sc, pos_cursor, cnt,
        core::seq_cntr::MemReader{ .dst = dst,
                                   .dst_width =
                                       core::seq_cntr::ops::GetWidth(sc),
                                   .dst_stride = dst_stride },
        dst_cursor);

    Sanitize(sc);

    if (pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) == idx);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, dst_cursor) ==
                          idx + cnt);

    Sanitize(sc);
}

template <typename SeqCntrView>
void Read(SeqCntrView* sc, size_t idx, size_t cnt, void* dst,
          size_t dst_stride) {
    size_t size{ core::seq_cntr::ops::GetSize(sc) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ core::utils::Min(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        Read_(sc, idx, cnt_a, dst, dst_stride);

        dst = static_cast<char*>(dst) + dst_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) { Read_(sc, 0, cnt_b, dst, dst_stride); }
}

template <typename SeqCntrView>
void Write_(SeqCntrView* sc, size_t idx, size_t cnt, void const* src,
            size_t src_stride) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    size_t width{ core::seq_cntr::ops::GetWidth(sc) };

    size_t size{ core::seq_cntr::ops::GetSize(sc) };

    Sanitize(sc);

    ZETA_Core_DebugAssert(cnt <= size);

    core::seq_cntr::ops::Access(sc, idx, true, pos_cursor, nullptr);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          idx);

    Sanitize(sc);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(sc) };

    core::seq_cntr::ops::Write(
        sc, pos_cursor, cnt,
        core::seq_cntr::MemWriter{
            .src = src, .src_width = width, .src_stride = src_stride },
        dst_cursor);

    Sanitize(sc);

    if (pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) == idx);

        Sanitize(sc);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, dst_cursor) ==
                          idx + cnt);

    Sanitize(sc);

    void* buffer{ std::malloc(src_stride * cnt) };

    Read(sc, idx, cnt, buffer, src_stride);

    ZETA_Core_DebugAssert(
        core::compare::ops::LexElemCompare(
            src, buffer, width, width, src_stride, src_stride, cnt, cnt) == 0);

    std::free(buffer);
}

template <typename SeqCntrView>
void Write(SeqCntrView* sc, size_t idx, size_t cnt, void const* src,
           size_t src_stride) {
    size_t size{ core::seq_cntr::ops::GetSize(sc) };

    ZETA_Core_DebugAssert(cnt <= size);

    size_t cnt_a{ core::utils::Min(cnt, size - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        Write_(sc, idx, cnt_a, src, src_stride);

        src = static_cast<char const*>(src) + src_stride * cnt_a;
        idx = (idx + cnt_a) % size;
    }

    if (0 < cnt_b) { Write_(sc, 0, cnt_b, src, src_stride); }
}

template <typename SeqCntrView>
void PushL(SeqCntrView* sc, size_t cnt, void const* src, size_t src_stride) {
    core::seq_cntr::ops::PushL(
        sc, cnt,
        core::seq_cntr::MemWriter{ .src = src,
                                   .src_width =
                                       core::seq_cntr::ops::GetWidth(sc),
                                   .src_stride = src_stride },
        nullptr);

    Sanitize(sc);
}

template <typename SeqCntrView>
void PushR(SeqCntrView* sc, size_t cnt, void const* src, size_t src_stride) {
    core::seq_cntr::ops::PushR(
        sc, cnt,
        core::seq_cntr::MemWriter{ .src = src,
                                   .src_width =
                                       core::seq_cntr::ops::GetWidth(sc),
                                   .src_stride = src_stride },
        nullptr);

    Sanitize(sc);
}

template <typename SeqCntrView>
void PopL(SeqCntrView* sc, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= core::seq_cntr::ops::GetSize(sc));

    core::seq_cntr::ops::PopL(sc, cnt);

    Sanitize(sc);
}

template <typename SeqCntrView>
void PopR(SeqCntrView* sc, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= core::seq_cntr::ops::GetSize(sc));

    Sanitize(sc);

    core::seq_cntr::ops::PopR(sc, cnt);

    Sanitize(sc);
}

template <typename SeqCntrView, typename Writer>
void Insert(SeqCntrView* sc, size_t idx, size_t cnt, Writer&& writer) {
    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    core::seq_cntr::ops::Access(sc, idx, true, pos_cursor, nullptr);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          idx);

    Sanitize(sc);

    void* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                          ? pos_cursor
                          : ZETA_Core_SeqCntr_AllocaCursor(sc) };

    core::seq_cntr::ops::Insert(
        sc, pos_cursor, cnt, core::meta::Forward<Writer>(writer), dst_cursor);

    Sanitize(sc);

    if (dst_cursor != pos_cursor) {
        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) == idx);

        Sanitize(sc);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, dst_cursor) ==
                          idx + cnt);

    Sanitize(sc);
}

template <typename SeqCntrView>
void Erase(SeqCntrView* sc, size_t idx, size_t cnt) {
    if (sc == nullptr) { return; }

    Sanitize(sc);

    size_t size{ core::seq_cntr::ops::GetSize(sc) };

    ZETA_Core_DebugAssert(cnt <= size);

    void* pos_cursor{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    core::seq_cntr::ops::Access(sc, idx, true, pos_cursor, nullptr);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          idx);

    Sanitize(sc);

    size_t cur_cnt{ std::min(size - idx, cnt) };

    core::seq_cntr::ops::Erase(sc, pos_cursor, cur_cnt);

    Sanitize(sc);

    cnt -= cur_cnt;

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetSize(sc) == size - cur_cnt);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          idx);

    Sanitize(sc);

    core::seq_cntr::ops::PeekL(sc, true, pos_cursor, nullptr);

    Sanitize(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc, pos_cursor) ==
                          0);

    core::seq_cntr::ops::Erase(sc, pos_cursor, cnt);

    Sanitize(sc);
}

template <typename SeqCntrView>
void CheckCursor(SeqCntrView* sc, size_t max_op_size) {
    ZETA_Core_DebugAssert(sc != nullptr);

    size_t size{ zeta::core::seq_cntr::ops::GetSize(sc) };

    void* cursor{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    zeta::core::seq_cntr::ops::GetLBCursor(sc, cursor);

    for (size_t idx{ static_cast<size_t>(-1) }; idx != size; ++idx) {
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor) == idx);

        zeta::core::seq_cntr::ops::CursorStepR(sc, cursor);
    }

    zeta::core::seq_cntr::ops::GetRBCursor(sc, cursor);

    for (size_t idx{ size }; idx != static_cast<size_t>(-1); --idx) {
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor) == idx);

        zeta::core::seq_cntr::ops::CursorStepL(sc, cursor);
    }

    void* cursor_a{ ZETA_Core_SeqCntr_AllocaCursor(sc) };
    void* cursor_b{ ZETA_Core_SeqCntr_AllocaCursor(sc) };
    void* cursor_c{ ZETA_Core_SeqCntr_AllocaCursor(sc) };

    for (size_t i{ 0 }; i < max_op_size; ++i) {
        size_t idx_a{ static_cast<size_t>(GetRandomInt<long long>(-1, size)) };
        size_t idx_b{ static_cast<size_t>(GetRandomInt<long long>(-1, size)) };

        zeta::core::seq_cntr::ops::Access(sc, idx_a, true, cursor_a, nullptr);
        zeta::core::seq_cntr::ops::Access(sc, idx_b, true, cursor_b, nullptr);

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor_a) == idx_a);
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor_b) == idx_b);

        ZETA_Core_DebugAssert(zeta::core::seq_cntr::ops::AreEqualCursor(
                                  sc, cursor_a, cursor_b) == (idx_a == idx_b));

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::CompareCursor(sc, cursor_a, cursor_b) ==
            core::compare::ops::BasicCompare(idx_a + 1, idx_b + 1));

        ZETA_Core_DebugAssert(zeta::core::seq_cntr::ops::GetCursorDist(
                                  sc, cursor_a, cursor_b) == idx_b - idx_a);

        zeta::core::seq_cntr::ops::Access(sc, idx_a, true, cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            zeta::core::seq_cntr::ops::CursorAdvanceR(sc, cursor_c,
                                                      idx_b - idx_a);
        } else {
            zeta::core::seq_cntr::ops::CursorAdvanceL(sc, cursor_c,
                                                      idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor_c) == idx_b);

        zeta::core::seq_cntr::ops::Access(sc, idx_b, true, cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            zeta::core::seq_cntr::ops::CursorAdvanceL(sc, cursor_c,
                                                      idx_b - idx_a);
        } else {
            zeta::core::seq_cntr::ops::CursorAdvanceR(sc, cursor_c,
                                                      idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::ops::GetCursorIdx(sc, cursor_c) == idx_a);
    }
}

template <typename SeqCntrView>
size_t SyncGetSize(std::vector<SeqCntrView*> const& scs) {
    ZETA_Core_DebugAssert(!scs.empty());

    size_t size{ core::seq_cntr::ops::GetSize(scs[0]) };

    for (auto& sc : scs) {
        ZETA_Core_DebugAssert(size == core::seq_cntr::ops::GetSize(sc));
    }

    return size;
}

template <typename Elem, typename SeqCntrView>
void SyncRandomRead(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(scs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    bool read{ false };

    for (auto sc : scs) {
        Read(sc, idx, cnt, buffer_b, stride);

        if (read) {
            ZETA_Core_DebugAssert(core::compare::ops::LexElemCompare(
                                      buffer_a, buffer_b, sizeof(Elem),
                                      sizeof(Elem), stride, stride, cnt,
                                      cnt) == 0);
        } else {
            read = true;
            std::memcpy(buffer_a, buffer_b, stride * cnt);
        }
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem, typename SeqCntrView>
void SyncRandomWrite(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(scs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto& sc : scs) {
        Write(sc, idx, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::compare::ops::LexElemCompare(
                                  buffer_a, buffer_b, sizeof(Elem),
                                  sizeof(Elem), stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem, typename SeqCntrView>
void SyncRandomPushL(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto& sc : scs) {
        PushL(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::compare::ops::LexElemCompare(
                                  buffer_a, buffer_b, sizeof(Elem),
                                  sizeof(Elem), stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem, typename SeqCntrView>
void SyncRandomPushR(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto& sc : scs) {
        PushR(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::compare::ops::LexElemCompare(
                                  buffer_a, buffer_b, sizeof(Elem),
                                  sizeof(Elem), stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntrView>
void SyncRandomPopL(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t size{ SyncGetSize(scs) };

    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { PopL(sc, cnt); }
}

template <typename SeqCntrView>
void SyncRandomPopR(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t size{ SyncGetSize(scs) };

    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { PopR(sc, cnt); }
}

template <typename Elem, typename SeqCntrView>
void SyncRandomInsert(std::vector<SeqCntrView*> const& scs,
                      size_t max_op_size) {
    size_t stride{ GetRandomStride<Elem>() };

    size_t size{ SyncGetSize(scs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, max_op_size) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * cnt)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * cnt)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(cnt));

    std::memcpy(buffer_b, buffer_a, stride * cnt);

    for (auto& sc : scs) {
        Insert(sc, idx, cnt,
               core::seq_cntr::MemWriter{ .src = buffer_b,
                                          .src_width =
                                              core::seq_cntr::ops::GetWidth(sc),
                                          .src_stride = stride });

        ZETA_Core_DebugAssert(core::compare::ops::LexElemCompare(
                                  buffer_a, buffer_b, sizeof(Elem),
                                  sizeof(Elem), stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntrView>
void SyncRandomErase(std::vector<SeqCntrView*> const& scs, size_t max_op_size) {
    size_t size{ SyncGetSize(scs) };

    size_t idx{ GetRandomInt<size_t>(0, size) };
    size_t cnt{ GetRandomInt<size_t>(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { Erase(sc, idx, cnt); }
}

template <typename Elem, typename SeqCntrView>
void SyncRandomInit(std::vector<SeqCntrView*> const& scs, size_t size) {
    size_t stride{ GetRandomStride<Elem>() };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * size)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * size)) };

    GetRandoms<Elem>(PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) },
                     PtrIter<Elem>{ buffer_a, static_cast<ptrdiff_t>(stride) } +
                         static_cast<long long>(size));

    std::memcpy(buffer_b, buffer_a, stride * size);

    for (auto& sc : scs) {
        Erase(sc, 0, core::seq_cntr::ops::GetSize(sc));

        PushR(sc, size, buffer_b, stride);

        ZETA_Core_DebugAssert(
            core::compare::ops::LexElemCompare(buffer_a, buffer_b, sizeof(Elem),
                                               sizeof(Elem), stride, stride,
                                               size, size) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem, typename SeqCntrA, typename SeqCntrB>
void SyncCompare2_(SeqCntrA* sc_a, SeqCntrB* sc_b) {
    size_t width_a{ core::seq_cntr::ops::GetWidth(sc_a) };
    size_t width_b{ core::seq_cntr::ops::GetWidth(sc_b) };

    ZETA_Core_DebugAssert(width_a == width_b);

    size_t stride{ GetRandomStride<Elem>() };

    size_t size_a{ core::seq_cntr::ops::GetSize(sc_a) };
    size_t size_b{ core::seq_cntr::ops::GetSize(sc_b) };

    ZETA_Core_DebugAssert(size_a == size_b);

    void* cursor_a{ ZETA_Core_SeqCntr_AllocaCursor(sc_a) };
    void* cursor_b{ ZETA_Core_SeqCntr_AllocaCursor(sc_b) };

    Elem* buffer_a{ static_cast<Elem*>(std::malloc(stride * size_a)) };
    Elem* buffer_b{ static_cast<Elem*>(std::malloc(stride * size_b)) };

    core::seq_cntr::ops::PeekL(sc_a, true, cursor_a, nullptr);
    core::seq_cntr::ops::PeekL(sc_b, true, cursor_b, nullptr);

    for (size_t i{ 0 }; i < size_a; ++i) {
        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc_a, cursor_a) == i);

        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc_b, cursor_b) == i);

        core::seq_cntr::ops::Read(sc_a, cursor_a, 1,
                                  core::seq_cntr::MemReader{
                                      .dst = buffer_a,
                                      .dst_width = width_a,
                                      .dst_stride = width_a,
                                  },
                                  cursor_a);

        core::seq_cntr::ops::Read(sc_b, cursor_b, 1,
                                  core::seq_cntr::MemReader{
                                      .dst = buffer_b,
                                      .dst_width = width_b,
                                      .dst_stride = width_b,
                                  },
                                  cursor_b);

        ZETA_Core_DebugAssert(
            core::compare::ops::LexMemCompare(buffer_a, buffer_b, sizeof(Elem),
                                              sizeof(Elem)) == 0);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc_a, cursor_a) ==
                          size_a);

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc_b, cursor_b) ==
                          size_b);

    core::seq_cntr::ops::PeekR(sc_a, true, cursor_a, nullptr);
    core::seq_cntr::ops::PeekR(sc_b, true, cursor_b, nullptr);

    for (size_t i{ size_a }; 0 < i--;) {
        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc_a, cursor_a) == i);

        ZETA_Core_DebugAssert(
            core::seq_cntr::ops::GetCursorIdx(sc_b, cursor_b) == i);

        core::seq_cntr::ops::Read(sc_a, cursor_a, 1,
                                  core::seq_cntr::MemReader{
                                      .dst = buffer_a,
                                      .dst_width = width_a,
                                      .dst_stride = width_a,
                                  },
                                  nullptr);

        core::seq_cntr::ops::Read(sc_b, cursor_b, 1,
                                  core::seq_cntr::MemReader{
                                      .dst = buffer_b,
                                      .dst_width = width_b,
                                      .dst_stride = width_b,
                                  },
                                  nullptr);

        ZETA_Core_DebugAssert(
            core::compare::ops::LexMemCompare(buffer_a, buffer_b, sizeof(Elem),
                                              sizeof(Elem)) == 0);

        core::seq_cntr::ops::CursorStepL(sc_a, cursor_a);

        core::seq_cntr::ops::CursorStepL(sc_b, cursor_b);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc_a, cursor_a) ==
                          static_cast<size_t>(-1));

    ZETA_Core_DebugAssert(core::seq_cntr::ops::GetCursorIdx(sc_b, cursor_b) ==
                          static_cast<size_t>(-1));

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename Elem, typename SeqCntr>
void SyncCompare(std::vector<SeqCntr*> const& scs) {
    size_t cntr_cnt{ scs.size() };

    if (cntr_cnt == 1) { return; }

    for (size_t i{ 0 }; i < cntr_cnt; ++i) {
        auto& sc_a{ scs[i] };
        auto& sc_b{ scs[(i + 1) % cntr_cnt] };
        SyncCompare2_<Elem>(sc_a, sc_b);
    }
}

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

template <typename Elem, typename SeqCntr>
void DoRandomOperations(std::vector<SeqCntr*> scs,
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
            SyncRandomRead<Elem>(scs, read_max_op_size);
            break;

        case OpEnum::WRITE:
            ZETA_Core_PrintVar("WRITE");
            SyncRandomWrite<Elem>(scs, write_max_op_size);
            break;

        case OpEnum::PUSH_L:
            ZETA_Core_PrintVar("PUSH_L");
            SyncRandomPushL<Elem>(scs, push_l_max_op_size);
            break;

        case OpEnum::PUSH_R:
            ZETA_Core_PrintVar("PUSH_R");
            SyncRandomPushR<Elem>(scs, push_r_max_op_size);
            break;

        case OpEnum::POP_L:
            ZETA_Core_PrintVar("POP_L");
            SyncRandomPopL(scs, pop_l_max_op_size);
            break;

        case OpEnum::POP_R:
            ZETA_Core_PrintVar("POP_R");
            SyncRandomPopR(scs, pop_r_max_op_size);
            break;

        case OpEnum::INSERT:
            ZETA_Core_PrintVar("INSERT");
            SyncRandomInsert<Elem>(scs, insert_max_op_size);
            break;

        case OpEnum::ERASE:
            ZETA_Core_PrintVar("ERASE");
            SyncRandomErase(scs, erase_max_op_size);
            break;
        }

        for (auto& sc : scs) { CheckCursor(sc, 16); }

        SyncCompare<Elem>(scs);

        for (auto& sc : scs) { Sanitize(sc); }
    }
}

}  // namespace zeta::core_test::seq_cntr_utils
