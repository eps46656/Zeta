#pragma once

#include <unordered_map>
#include <vector>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr_ref.hpp>
#include <zeta/core/seq_cntr_ref.ipp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core_test/ptr_iter.hpp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test::seq_cntr_utils {

using VTable = core::seq_cntr::VTable;

inline size_t GetRandomStride(size_t elem_size) {
    ZETA_Core_DebugAssert(1 <= elem_size);

    return elem_size + (GetRandomInt<size_t>)(0, elem_size * 2);
}

inline auto& GetSanitizeFuncs() {
    static std::unordered_map<void const*, void (*)(void const* sc)> instance;
    return instance;
}

inline void AddSanitizeFunc(void const* sc, void (*Sanitize)(void const* sc)) {
    auto& map{ (GetSanitizeFuncs)() };

    auto iter{ map.insert({ sc, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

inline void(Sanitize)(void const* sc) {
    if (sc == nullptr) { return; }

    auto& map{ (GetSanitizeFuncs)() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(sc);
}

inline void(Sanitize)(core::seq_cntr_ref::Ref const* sc) {
    (Sanitize)(sc->cntr);
}

inline auto& GetDestroyFuncs() {
    static std::unordered_map<void*, void (*)(void* sc)> instance;
    return instance;
}

inline void AddDestroyFunc(void* sc, void (*Destroy)(void* sc)) {
    auto& map{ (GetDestroyFuncs)() };

    auto iter{ map.insert({ sc, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

inline void Destroy(void* sc) {
    if (sc == nullptr) { return; }

    auto& map{ (GetDestroyFuncs)() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(sc);
}

inline void Destroy(core::seq_cntr_ref::Ref* sc) { Destroy(sc->cntr); }

template <typename SeqCntr>
void Read_(SeqCntr* sc, size_t idx, size_t cnt, void* dst, size_t dst_stride) {
    core::seq_cntr::CursorLimit pos_cursor;

    size_t size{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugAssert(cnt <= size);

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                          idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    core::seq_cntr::MemReader mem_reader{ .data = dst,
                                          .elem_size =
                                              core::seq_cntr::GetElemSize(*sc),
                                          .elem_stride = dst_stride };

    core::seq_cntr::Read(*sc, &pos_cursor, cnt, mem_reader, dst_cursor);

    (Sanitize)(sc);

    if (&pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                              idx);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, dst_cursor) ==
                          idx + cnt);

    (Sanitize)(sc);
}

template <typename SeqCntr>
void Read(SeqCntr* sc, size_t idx, size_t cnt, void* dst, size_t dst_stride) {
    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    size_t cnt_a{ core::comparison_utils::BasicMin(cnt, elem_cnt - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        (Read_)(sc, idx, cnt_a, dst, dst_stride);

        dst = static_cast<char*>(dst) + dst_stride * cnt_a;
        idx = (idx + cnt_a) % elem_cnt;
    }

    if (0 < cnt_b) { (Read_)(sc, 0, cnt_b, dst, dst_stride); }
}

template <typename SeqCntr>
void Write_(SeqCntr* sc, size_t idx, size_t cnt, void const* src,
            size_t src_stride) {
    core::seq_cntr::CursorLimit pos_cursor;

    size_t elem_size{ core::seq_cntr::GetElemSize(*sc) };

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                          idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ GetRandomInt<int>(0, 1) == 0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    core::seq_cntr::Write(
        *sc, &pos_cursor, cnt,
        core::seq_cntr::MemWriter{
            .data = src, .elem_size = elem_size, .elem_stride = src_stride },
        dst_cursor);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetElemCnt(*sc) == elem_cnt);

    if (&pos_cursor != dst_cursor) {
        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                              idx);

        (Sanitize)(sc);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, dst_cursor) ==
                          idx + cnt);

    (Sanitize)(sc);

    void* buffer{ cnt == 0 ? nullptr
                           : std::malloc(src_stride * (cnt - 1) + elem_size) };

    (Read)(sc, idx, cnt, buffer, src_stride);

    ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                              src, buffer, elem_size, elem_size, src_stride,
                              src_stride, cnt, cnt) == 0);

    std::free(buffer);
}

template <typename SeqCntr>
void Write(SeqCntr* sc, size_t idx, size_t cnt, void const* src,
           size_t src_stride) {
    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    size_t cnt_a{ core::comparison_utils::BasicMin(cnt, elem_cnt - idx) };
    size_t cnt_b{ cnt - cnt_a };

    if (0 < cnt_a) {
        (Write_)(sc, idx, cnt_a, src, src_stride);

        src = static_cast<char const*>(src) + src_stride * cnt_a;
        idx = (idx + cnt_a) % elem_cnt;
    }

    if (0 < cnt_b) { (Write_)(sc, 0, cnt_b, src, src_stride); }
}

template <typename SeqCntr>
void PushL(SeqCntr* sc, size_t cnt, void const* src, size_t src_stride) {
    core::seq_cntr::PushL(*sc, cnt,
                          core::seq_cntr::MemWriter{
                              .data = src,
                              .elem_size = core::seq_cntr::GetElemSize(*sc),
                              .elem_stride = src_stride },
                          nullptr);

    (Sanitize)(sc);
}

template <typename SeqCntr>
void PushR(SeqCntr* sc, size_t cnt, void const* src, size_t src_stride) {
    core::seq_cntr::PushR(*sc, cnt,
                          core::seq_cntr::MemWriter{
                              .data = src,
                              .elem_size = core::seq_cntr::GetElemSize(*sc),
                              .elem_stride = src_stride },
                          nullptr);

    (Sanitize)(sc);
}

template <typename SeqCntr>
void PopL(SeqCntr* sc, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= core::seq_cntr::GetElemCnt(*sc));

    core::seq_cntr::PopL(*sc, cnt, core::seq_cntr::empty_reader);

    (Sanitize)(sc);
}

template <typename SeqCntr>
void PopR(SeqCntr* sc, size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= core::seq_cntr::GetElemCnt(*sc));

    (Sanitize)(sc);

    core::seq_cntr::PopR(*sc, cnt, core::seq_cntr::empty_reader);

    (Sanitize)(sc);
}

template <typename SeqCntr, typename Writer>
void Insert(SeqCntr* sc, size_t idx, size_t cnt, Writer&& writer) {
    core::seq_cntr::CursorLimit pos_cursor;

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                          idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ (GetRandomInt<int>)(0, 1) == 0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    core::seq_cntr::Insert(*sc, &pos_cursor, cnt,
                           core::meta::Forward<Writer>(writer), dst_cursor);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetElemCnt(*sc) ==
                          old_elem_cnt + cnt);

    if (dst_cursor != &pos_cursor) {
        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                              idx);

        (Sanitize)(sc);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, dst_cursor) ==
                          idx + cnt);

    (Sanitize)(sc);
}

template <typename SeqCntr>
void Erase(SeqCntr* sc, size_t idx, size_t cnt) {
    if (sc == nullptr) { return; }

    (Sanitize)(sc);

    size_t elem_size{ core::seq_cntr::GetElemSize(*sc) };

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    core::seq_cntr::CursorLimit pos_cursor;

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                          idx);

    (Sanitize)(sc);

    bool check_reader{ core::utils::GetRandom() % 2 == 0 };

    size_t stride_a{ (GetRandomStride)(elem_size) };
    size_t stride_b{ (GetRandomStride)(elem_size) };

    void* buffer_a{ check_reader && 0 < cnt
                        ? std::malloc(stride_a * (cnt - 1) + elem_size)
                        : nullptr };
    void* buffer_b{ check_reader && 0 < cnt
                        ? std::malloc(stride_b * (cnt - 1) + elem_size)
                        : nullptr };

    size_t origin_cnt{ cnt };

    core::seq_cntr::MemReader reader_a{ .data = buffer_a,
                                        .elem_size = elem_size,
                                        .elem_stride = stride_a };

    core::seq_cntr::MemReader reader_b{ .data = buffer_b,
                                        .elem_size = elem_size,
                                        .elem_stride = stride_b };

    size_t cur_cnt{ std::min(elem_cnt - idx, cnt) };

    {
        size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

        if (check_reader) {
            core::seq_cntr::Read(*sc, &pos_cursor, cur_cnt, reader_a, nullptr);

            (Sanitize)(sc);

            ZETA_Core_DebugAssert(
                core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);
        }

        if (check_reader) {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt, reader_b);
        } else {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt,
                                  core::seq_cntr::empty_reader);
        }

        (Sanitize)(sc);

        ZETA_Core_DebugAssert(core::seq_cntr::GetElemCnt(*sc) ==
                              old_elem_cnt - cur_cnt);
    }

    cnt -= cur_cnt;

    ZETA_Core_DebugAssert(core::seq_cntr::GetElemCnt(*sc) ==
                          elem_cnt - cur_cnt);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) ==
                          idx);

    (Sanitize)(sc);

    core::seq_cntr::PeekL(*sc, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == 0);

    {
        cur_cnt = cnt;

        if (check_reader) {
            core::seq_cntr::Read(*sc, &pos_cursor, cur_cnt, reader_a, nullptr);

            (Sanitize)(sc);

            ZETA_Core_DebugAssert(
                core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == 0);
        }

        size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

        if (check_reader) {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt, reader_b);
        } else {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt,
                                  core::seq_cntr::empty_reader);
        }

        (Sanitize)(sc);

        ZETA_Core_DebugAssert(core::seq_cntr::GetElemCnt(*sc) ==
                              old_elem_cnt - cur_cnt);
    }

    if (check_reader) {
        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride_a, stride_b, origin_cnt,
                                  origin_cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void CheckCursor(SeqCntr* sc, size_t max_op_size) {
    ZETA_Core_DebugAssert(sc != nullptr);

    size_t size{ zeta::core::seq_cntr::GetElemCnt(*sc) };

    zeta::core::seq_cntr::CursorLimit cursor;

    zeta::core::seq_cntr::GetLBCursor(*sc, &cursor);

    for (size_t idx{ static_cast<size_t>(-1) }; idx != size; ++idx) {
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor) == idx);

        zeta::core::seq_cntr::CursorStepR(*sc, &cursor);
    }

    zeta::core::seq_cntr::GetRBCursor(*sc, &cursor);

    for (size_t idx{ size }; idx != static_cast<size_t>(-1); --idx) {
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor) == idx);

        zeta::core::seq_cntr::CursorStepL(*sc, &cursor);
    }

    zeta::core::seq_cntr::CursorLimit cursor_a;
    zeta::core::seq_cntr::CursorLimit cursor_b;
    zeta::core::seq_cntr::CursorLimit cursor_c;

    for (size_t i{ 0 }; i < max_op_size; ++i) {
        size_t idx_a{ static_cast<size_t>(
            (GetRandomInt<long long>)(-1, size)) };
        size_t idx_b{ static_cast<size_t>(
            (GetRandomInt<long long>)(-1, size)) };

        zeta::core::seq_cntr::Refer(*sc, idx_a, true, nullptr, &cursor_a,
                                    nullptr);
        zeta::core::seq_cntr::Refer(*sc, idx_b, true, nullptr, &cursor_b,
                                    nullptr);

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor_a) == idx_a);
        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor_b) == idx_b);

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::AreEqualCursor(*sc, &cursor_a, &cursor_b) ==
            (idx_a == idx_b));

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::CompareCursor(*sc, &cursor_a, &cursor_b) ==
            core::comparison::BasicCompare(
                core::comparison::ComparisonTypeEnum::ThreeWay{}, idx_a + 1,
                idx_b + 1));

        ZETA_Core_DebugAssert(zeta::core::seq_cntr::GetCursorDist(
                                  *sc, &cursor_a, &cursor_b) == idx_b - idx_a);

        zeta::core::seq_cntr::Refer(*sc, idx_a, true, nullptr, &cursor_c,
                                    nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            zeta::core::seq_cntr::CursorAdvanceR(*sc, &cursor_c, idx_b - idx_a);
        } else {
            zeta::core::seq_cntr::CursorAdvanceL(*sc, &cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor_c) == idx_b);

        zeta::core::seq_cntr::Refer(*sc, idx_b, true, nullptr, &cursor_c,
                                    nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            zeta::core::seq_cntr::CursorAdvanceL(*sc, &cursor_c, idx_b - idx_a);
        } else {
            zeta::core::seq_cntr::CursorAdvanceR(*sc, &cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugAssert(
            zeta::core::seq_cntr::GetCursorIdx(*sc, &cursor_c) == idx_a);
    }
}

template <typename SeqCntr>
size_t SyncGetElemSize(std::vector<SeqCntr*> const& scs) {
    ZETA_Core_DebugAssert(!scs.empty());

    size_t elem_size{ core::seq_cntr::GetElemSize(*scs[0]) };

    for (auto& sc : scs) {
        ZETA_Core_DebugAssert(elem_size == core::seq_cntr::GetElemSize(*sc));
    }

    return elem_size;
}

template <typename SeqCntr>
size_t SyncGetElemCnt(std::vector<SeqCntr*> const& scs) {
    ZETA_Core_DebugAssert(!scs.empty());

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*scs[0]) };

    for (auto& sc : scs) {
        ZETA_Core_DebugAssert(elem_cnt == core::seq_cntr::GetElemCnt(*sc));
    }

    return elem_cnt;
}

template <typename SeqCntr>
void SyncRandomRead(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (SyncGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t size{ (SyncGetElemCnt)(scs) };

    size_t idx{ (GetRandomInt<size_t>)(0, size) };
    size_t cnt{ (GetRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    bool read{ false };

    for (auto sc : scs) {
        Read(sc, idx, cnt, buffer_b, stride);

        if (read) {
            ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                      buffer_a, buffer_b, elem_size, elem_size,
                                      stride, stride, cnt, cnt) == 0);
        } else {
            read = true;

            if (0 < cnt) {
                std::memcpy(buffer_a, buffer_b, stride * (cnt - 1) + elem_size);
            }
        }
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncRandomWrite(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (SyncGetElemSize(scs)) };

    size_t elem_cnt{ (SyncGetElemCnt)(scs) };

    size_t idx{ (GetRandomInt<size_t>)(0, elem_cnt) };
    size_t cnt{ (GetRandomInt<size_t>)(0, std::min(max_op_size, elem_cnt)) };

    size_t stride{ (GetRandomStride)(elem_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GetRandomMemSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (Write)(sc, idx, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncRandomPushL(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (SyncGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t cnt{ (GetRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GetRandomMemSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (PushL)(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncRandomPushR(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (SyncGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t cnt{ (GetRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GetRandomMemSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (PushR)(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncRandomPopL(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t size{ (SyncGetElemCnt)(scs) };

    size_t cnt{ (GetRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { (PopL)(sc, cnt); }
}

template <typename SeqCntr>
void SyncRandomPopR(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t size{ (SyncGetElemCnt)(scs) };

    size_t cnt{ (GetRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { (PopR)(sc, cnt); }
}

template <typename SeqCntr>
void SyncRandomInsert(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (SyncGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t size{ (SyncGetElemCnt)(scs) };

    size_t idx{ (GetRandomInt<size_t>)(0, size) };
    size_t cnt{ (GetRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GetRandomMemSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (Insert)(sc, idx, cnt,
                 core::seq_cntr::MemWriter{
                     .data = buffer_b,
                     .elem_size = core::seq_cntr::GetElemSize(*sc),
                     .elem_stride = stride });

        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride, stride, cnt, cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncRandomErase(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t size{ SyncGetElemCnt(scs) };

    size_t idx{ (GetRandomInt<size_t>)(0, size) };
    size_t cnt{ (GetRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    for (auto& sc : scs) { (Erase)(sc, idx, cnt); }
}

template <typename SeqCntr>
void SyncRandomInit(std::vector<SeqCntr*> const& scs, size_t elem_cnt) {
    size_t elem_size{ (SyncGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    ZETA_Core_PrintVar(elem_size);
    ZETA_Core_PrintVar(stride);

    void* buffer_a{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride * (elem_cnt - 1) + elem_size) };
    void* buffer_b{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride * (elem_cnt - 1) + elem_size) };

    GetRandomMemSeq(buffer_a, elem_size, stride, elem_cnt);

    if (0 < elem_cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (elem_cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (Erase)(sc, 0, core::seq_cntr::GetElemCnt(*sc));

        (PushR)(sc, elem_cnt, buffer_b, stride);

        ZETA_Core_DebugAssert(core::comparison_utils::MemSeqLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size,
                                  stride, stride, elem_cnt, elem_cnt) == 0);
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntrA, typename SeqCntrB>
void SyncCompare2_(SeqCntrA* sc_a, SeqCntrB* sc_b) {
    size_t elem_size{ (SyncGetElemSize)(std::vector<SeqCntrA*>{ sc_a }) };

    size_t elem_cnt{ (SyncGetElemCnt)(std::vector<SeqCntrA*>{ sc_a }) };

    core::seq_cntr::CursorLimit cursor_a;
    core::seq_cntr::CursorLimit cursor_b;

    size_t stride_a{ (GetRandomStride)(elem_size) };
    size_t stride_b{ (GetRandomStride)(elem_size) };

    void* buffer_a{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride_a * (elem_cnt - 1) + elem_size) };
    void* buffer_b{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride_b * (elem_cnt - 1) + elem_size) };

    core::seq_cntr::PeekL(*sc_a, true, nullptr, &cursor_a, nullptr);
    core::seq_cntr::PeekL(*sc_b, true, nullptr, &cursor_b, nullptr);

    for (size_t i{ 0 }; i < elem_cnt; ++i) {
        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) ==
                              i);

        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) ==
                              i);

        core::seq_cntr::Read(*sc_a, &cursor_a, 1,
                             core::seq_cntr::MemReader{
                                 .data = buffer_a,
                                 .elem_size = elem_size,
                                 .elem_stride = stride_a,
                             },
                             &cursor_a);

        core::seq_cntr::Read(*sc_b, &cursor_b, 1,
                             core::seq_cntr::MemReader{
                                 .data = buffer_b,
                                 .elem_size = elem_size,
                                 .elem_stride = stride_b,
                             },
                             &cursor_b);

        ZETA_Core_DebugAssert(core::comparison_utils::MemLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size) ==
                              0);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) ==
                          elem_cnt);

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) ==
                          elem_cnt);

    core::seq_cntr::PeekR(*sc_a, true, nullptr, &cursor_a, nullptr);
    core::seq_cntr::PeekR(*sc_b, true, nullptr, &cursor_b, nullptr);

    for (size_t i{ elem_cnt }; 0 < i--;) {
        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) ==
                              i);

        ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) ==
                              i);

        core::seq_cntr::Read(*sc_a, &cursor_a, 1,
                             core::seq_cntr::MemReader{
                                 .data = buffer_a,
                                 .elem_size = elem_size,
                                 .elem_stride = stride_a,
                             },
                             nullptr);

        core::seq_cntr::Read(*sc_b, &cursor_b, 1,
                             core::seq_cntr::MemReader{
                                 .data = buffer_b,
                                 .elem_size = elem_size,
                                 .elem_stride = stride_b,
                             },
                             nullptr);

        ZETA_Core_DebugAssert(core::comparison_utils::MemLexCompare(
                                  buffer_a, buffer_b, elem_size, elem_size) ==
                              0);

        core::seq_cntr::CursorStepL(*sc_a, &cursor_a);

        core::seq_cntr::CursorStepL(*sc_b, &cursor_b);
    }

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) ==
                          static_cast<size_t>(-1));

    ZETA_Core_DebugAssert(core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) ==
                          static_cast<size_t>(-1));

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void SyncCompare(std::vector<SeqCntr*> const& scs) {
    size_t cntr_cnt{ scs.size() };

    if (cntr_cnt == 1) { return; }

    for (size_t i{ 0 }; i < cntr_cnt; ++i) {
        auto& sc_a{ scs[i] };
        auto& sc_b{ scs[(i + 1) % cntr_cnt] };
        (SyncCompare2_)(sc_a, sc_b);
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

template <typename SeqCntr>
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
        switch (ops[(GetRandomInt<size_t>)(0, ops.size() - 1)]) {
        case OpEnum::READ:
            ZETA_Core_PrintVar("READ");
            (SyncRandomRead)(scs, read_max_op_size);
            break;

        case OpEnum::WRITE:
            ZETA_Core_PrintVar("WRITE");
            (SyncRandomWrite)(scs, write_max_op_size);
            break;

        case OpEnum::PUSH_L:
            ZETA_Core_PrintVar("PUSH_L");
            (SyncRandomPushL)(scs, push_l_max_op_size);
            break;

        case OpEnum::PUSH_R:
            ZETA_Core_PrintVar("PUSH_R");
            (SyncRandomPushR)(scs, push_r_max_op_size);
            break;

        case OpEnum::POP_L:
            ZETA_Core_PrintVar("POP_L");
            (SyncRandomPopL)(scs, pop_l_max_op_size);
            break;

        case OpEnum::POP_R:
            ZETA_Core_PrintVar("POP_R");
            (SyncRandomPopR)(scs, pop_r_max_op_size);
            break;

        case OpEnum::INSERT:
            ZETA_Core_PrintVar("INSERT");
            (SyncRandomInsert)(scs, insert_max_op_size);
            break;

        case OpEnum::ERASE:
            ZETA_Core_PrintVar("ERASE");
            (SyncRandomErase)(scs, erase_max_op_size);
            break;
        }

        for (auto& sc : scs) { (Sanitize)(sc); }

        for (auto& sc : scs) { (CheckCursor)(sc, 16); }

        (SyncCompare)(scs);
    }
}

}  // namespace zeta::core_test::seq_cntr_utils
