#pragma once

#include <unordered_map>
#include <vector>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/poly_seq_cntr.hpp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/ptr_iter.hpp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test::seq_cntr_utils {

using VTable = core::seq_cntr::VTable;

constexpr size_t GetRandomStride(size_t elem_size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(1 <= elem_size);

    return elem_size + (GenUniformRandomInt<size_t>)(0, elem_size * 2);
}

constexpr auto& GetSanitizeFuncs() {
    static std::unordered_map<void const*, void (*)(void const* sc)> instance;
    return instance;
}

constexpr void AddSanitizeFunc(void const* sc,
                               void (*Sanitize)(void const* sc)) {
    auto& map{ (GetSanitizeFuncs)() };

    auto iter{ map.insert({ sc, Sanitize }).first };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second == Sanitize);
}

constexpr void(Sanitize)(void const* sc) {
    if (sc == nullptr) { return; }

    auto& map{ (GetSanitizeFuncs)() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != map.end());
    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second != nullptr);

    iter->second(sc);
}

constexpr void(Sanitize)(core::poly_seq_cntr::Cntr const* sc) {
    (Sanitize)(sc->target_cntr);
}

constexpr auto& GetDestroyFuncs() {
    static std::unordered_map<void*, void (*)(void* sc)> instance;
    return instance;
}

constexpr void AddDestroyFunc(void* sc, void (*Destroy)(void* sc)) {
    auto& map{ (GetDestroyFuncs)() };

    auto iter{ map.insert({ sc, Destroy }).first };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second == Destroy);
}

constexpr void Destroy(void* sc) {
    if (sc == nullptr) { return; }

    auto& map{ (GetDestroyFuncs)() };

    auto iter{ map.find(sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != map.end());
    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second != nullptr);

    iter->second(sc);
}

constexpr void Destroy(core::poly_seq_cntr::Cntr* sc) {
    Destroy(sc->target_cntr);
}

template <typename SeqCntr>
constexpr void Read_(SeqCntr* sc, size_t idx, size_t cnt, void* dst,
                     size_t dst_stride) {
    core::seq_cntr::CursorLimit pos_cursor;

    size_t size{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= size);

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ GenUniformRandomInt<int>(0, 1) == 0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    core::lin_seq_endpoint::acceptor::Acceptor mem_reader{
        .data = dst,
        .elem_size = core::seq_cntr::GetElemSize(*sc),
        .elem_stride = static_cast<ptrdiff_t>(dst_stride),
        .elem_cnt = cnt,
    };

    core::seq_cntr::Read(*sc, &pos_cursor, cnt, mem_reader, dst_cursor);

    (Sanitize)(sc);

    if (&pos_cursor != dst_cursor) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, dst_cursor) == idx + cnt);

    (Sanitize)(sc);
}

template <typename SeqCntr>
constexpr void Read(SeqCntr* sc, size_t idx, size_t cnt, void* dst,
                    size_t dst_stride) {
    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= elem_cnt);

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
constexpr void Write_(SeqCntr* sc, size_t idx, size_t cnt, void const* src,
                      size_t src_stride) {
    core::seq_cntr::CursorLimit pos_cursor;

    size_t elem_size{ core::seq_cntr::GetElemSize(*sc) };

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= elem_cnt);

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ GenUniformRandomInt<int>(0, 1) == 0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    core::seq_cntr::Write(*sc, &pos_cursor, cnt,
                          core::lin_seq_endpoint::provider::Provider{
                              .data = src,
                              .elem_size = elem_size,
                              .elem_stride = static_cast<ptrdiff_t>(src_stride),
                              .elem_cnt = core::seq_cntr::max_max_elem_cnt,
                          },
                          dst_cursor);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(core::seq_cntr::GetElemCnt(*sc) ==
                                            elem_cnt);

    if (&pos_cursor != dst_cursor) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

        (Sanitize)(sc);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, dst_cursor) == idx + cnt);

    (Sanitize)(sc);

    void* buffer{ cnt == 0 ? nullptr
                           : std::malloc(src_stride * (cnt - 1) + elem_size) };

    (Read)(sc, idx, cnt, buffer, src_stride);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::comparison_utils::LinSeqLexCompare(
            core::comparison::OpTags::Equal{}, src, buffer, elem_size,
            elem_size, static_cast<ptrdiff_t>(src_stride),
            static_cast<ptrdiff_t>(src_stride), cnt, cnt));

    std::free(buffer);
}

template <typename SeqCntr>
constexpr void Write(SeqCntr* sc, size_t idx, size_t cnt, void const* src,
                     size_t src_stride) {
    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= elem_cnt);

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
constexpr void PushL(SeqCntr* sc, size_t cnt, void const* src,
                     size_t src_stride) {
    core::seq_cntr::PushL(*sc, cnt,
                          core::lin_seq_endpoint::provider::Provider{
                              .data = src,
                              .elem_size = core::seq_cntr::GetElemSize(*sc),
                              .elem_stride = static_cast<ptrdiff_t>(src_stride),
                              .elem_cnt = core::seq_cntr::max_max_elem_cnt,
                          },
                          nullptr);

    (Sanitize)(sc);
}

template <typename SeqCntr>
constexpr void PushR(SeqCntr* sc, size_t cnt, void const* src,
                     size_t src_stride) {
    core::seq_cntr::PushR(*sc, cnt,
                          core::lin_seq_endpoint::provider::Provider{
                              .data = src,
                              .elem_size = core::seq_cntr::GetElemSize(*sc),
                              .elem_stride = static_cast<ptrdiff_t>(src_stride),
                              .elem_cnt = core::seq_cntr::max_max_elem_cnt,
                          },
                          nullptr);

    (Sanitize)(sc);
}

template <typename SeqCntr>
constexpr void PopL(SeqCntr* sc, size_t cnt, void* dst, size_t dst_elem_size,
                    size_t dst_elem_stride) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <=
                                            core::seq_cntr::GetElemCnt(*sc));

    (Sanitize)(sc);

    if (dst == nullptr) {
        core::seq_cntr::PopL(*sc, cnt,
                             core::seq_endpoint::acceptor::EmptyAcceptor{});
    } else {
        core::seq_cntr::PopL(
            *sc, cnt,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = dst,
                .elem_size = dst_elem_size,
                .elem_stride = static_cast<ptrdiff_t>(dst_elem_stride),
                .elem_cnt = core::seq_cntr::max_max_elem_cnt,
            });
    }

    (Sanitize)(sc);
}

template <typename SeqCntr>
constexpr void PopR(SeqCntr* sc, size_t cnt, void* dst, size_t dst_elem_size,
                    size_t dst_elem_stride) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <=
                                            core::seq_cntr::GetElemCnt(*sc));

    (Sanitize)(sc);

    if (dst == nullptr) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        core::seq_cntr::PopR(*sc, cnt,
                             core::seq_endpoint::acceptor::EmptyAcceptor{});
    } else {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        core::seq_cntr::PopR(
            *sc, cnt,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = dst,
                .elem_size = dst_elem_size,
                .elem_stride = static_cast<ptrdiff_t>(dst_elem_stride),
                .elem_cnt = core::seq_cntr::max_max_elem_cnt,
            });
    }

    (Sanitize)(sc);
}

template <typename SeqCntr, typename Writer>
constexpr void Insert(SeqCntr* sc, size_t idx, size_t cnt, Writer&& writer) {
    core::seq_cntr::CursorLimit pos_cursor;

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

    (Sanitize)(sc);

    core::seq_cntr::CursorLimit fallback_dst_cursor;

    core::seq_cntr::CursorLimit* dst_cursor{ (GenUniformRandomInt<int>)(0, 1) ==
                                                     0
                                                 ? &pos_cursor
                                                 : &fallback_dst_cursor };

    size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    core::seq_cntr::Insert(*sc, &pos_cursor, cnt,
                           core::meta::Forward<Writer>(writer), dst_cursor);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(core::seq_cntr::GetElemCnt(*sc) ==
                                            old_elem_cnt + cnt);

    if (dst_cursor != &pos_cursor) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

        (Sanitize)(sc);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, dst_cursor) == idx + cnt);

    (Sanitize)(sc);
}

template <typename SeqCntr>
constexpr void Erase(SeqCntr* sc, size_t idx, size_t cnt, void* dst,
                     size_t dst_elem_size, size_t dst_elem_stride) {
    if (sc == nullptr) { return; }

    (Sanitize)(sc);

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= elem_cnt);

    core::seq_cntr::CursorLimit pos_cursor;

    core::seq_cntr::Refer(*sc, idx, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

    (Sanitize)(sc);

    size_t origin_cnt{ cnt };

    core::lin_seq_endpoint::acceptor::Acceptor reader{
        .data = dst,
        .elem_size = dst_elem_size,
        .elem_stride = static_cast<ptrdiff_t>(dst_elem_stride),
        .elem_cnt = origin_cnt
    };

    size_t cur_cnt{ std::min(elem_cnt - idx, cnt) };

    {
        size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

        if (dst == nullptr) {
            core::seq_cntr::Erase(
                *sc, &pos_cursor, cur_cnt,
                core::seq_endpoint::acceptor::EmptyAcceptor{});
        } else {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt, reader);
        }

        (Sanitize)(sc);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetElemCnt(*sc) == old_elem_cnt - cur_cnt);
    }

    cnt -= cur_cnt;

    ZETA_Core_DebugUtils_Diag_PromiseAssert(core::seq_cntr::GetElemCnt(*sc) ==
                                            elem_cnt - cur_cnt);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == idx);

    (Sanitize)(sc);

    core::seq_cntr::PeekL(*sc, true, nullptr, &pos_cursor, nullptr);

    (Sanitize)(sc);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc, &pos_cursor) == 0);

    {
        cur_cnt = cnt;

        size_t old_elem_cnt{ core::seq_cntr::GetElemCnt(*sc) };

        if (dst == nullptr) {
            core::seq_cntr::Erase(
                *sc, &pos_cursor, cur_cnt,
                core::seq_endpoint::acceptor::EmptyAcceptor{});
        } else {
            core::seq_cntr::Erase(*sc, &pos_cursor, cur_cnt, reader);
        }

        (Sanitize)(sc);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetElemCnt(*sc) == old_elem_cnt - cur_cnt);
    }
}

template <typename SeqCntr>
void CheckCursor(SeqCntr* sc, size_t max_op_size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(sc != nullptr);

    size_t size{ core::seq_cntr::GetElemCnt(*sc) };

    core::seq_cntr::CursorLimit cursor;

    core::seq_cntr::GetLBCursor(*sc, &cursor);

    for (size_t idx{ static_cast<size_t>(-1) }; idx != size; ++idx) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor) == idx);

        core::seq_cntr::CursorStepR(*sc, &cursor);
    }

    core::seq_cntr::GetRBCursor(*sc, &cursor);

    for (size_t idx{ size }; idx != static_cast<size_t>(-1); --idx) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor) == idx);

        core::seq_cntr::CursorStepL(*sc, &cursor);
    }

    core::seq_cntr::CursorLimit cursor_a;
    core::seq_cntr::CursorLimit cursor_b;
    core::seq_cntr::CursorLimit cursor_c;

    for (size_t i{ 0 }; i < max_op_size; ++i) {
        size_t idx_a{ static_cast<size_t>(
            (GenUniformRandomInt<long long>)(-1, size)) };
        size_t idx_b{ static_cast<size_t>(
            (GenUniformRandomInt<long long>)(-1, size)) };

        core::seq_cntr::Refer(*sc, idx_a, true, nullptr, &cursor_a, nullptr);
        core::seq_cntr::Refer(*sc, idx_b, true, nullptr, &cursor_b, nullptr);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor_a) == idx_a);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor_b) == idx_b);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::AreEqualCursor(*sc, &cursor_a, &cursor_b) ==
            (idx_a == idx_b));

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::CompareCursor(*sc, &cursor_a, &cursor_b) ==
            core::comparison::BasicCompare(core::comparison::OpTags::Order{},
                                           idx_a + 1, idx_b + 1));

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorDist(*sc, &cursor_a, &cursor_b) ==
            idx_b - idx_a);

        core::seq_cntr::Refer(*sc, idx_a, true, nullptr, &cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            core::seq_cntr::CursorAdvanceR(*sc, &cursor_c, idx_b - idx_a);
        } else {
            core::seq_cntr::CursorAdvanceL(*sc, &cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor_c) == idx_b);

        core::seq_cntr::Refer(*sc, idx_b, true, nullptr, &cursor_c, nullptr);

        if (idx_a + 1 <= idx_b + 1) {
            core::seq_cntr::CursorAdvanceL(*sc, &cursor_c, idx_b - idx_a);
        } else {
            core::seq_cntr::CursorAdvanceR(*sc, &cursor_c, idx_a - idx_b);
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc, &cursor_c) == idx_a);
    }
}

template <typename SeqCntr>
size_t MirrorGetElemSize(std::vector<SeqCntr*> const& scs) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!scs.empty());

    size_t elem_size{ core::seq_cntr::GetElemSize(*scs[0]) };

    for (auto& sc : scs) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            elem_size == core::seq_cntr::GetElemSize(*sc));
    }

    return elem_size;
}

template <typename SeqCntr>
size_t MirrorGetElemCnt(std::vector<SeqCntr*> const& scs) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!scs.empty());

    size_t elem_cnt{ core::seq_cntr::GetElemCnt(*scs[0]) };

    for (auto& sc : scs) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            elem_cnt == core::seq_cntr::GetElemCnt(*sc));
    }

    return elem_cnt;
}

template <typename SeqCntr>
void MirrorRandomRead(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t elem_cnt{ (MirrorGetElemCnt)(scs) };

    size_t idx{ (GenUniformRandomInt<size_t>)(0, elem_cnt) };
    size_t cnt{ (
        GenUniformRandomInt<size_t>)(0, std::min(max_op_size, elem_cnt)) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    bool read{ false };

    for (auto sc : scs) {
        Read(sc, idx, cnt, buffer_b, stride);

        if (read) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                core::comparison_utils::LinSeqLexCompare(
                    core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                    elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                    static_cast<ptrdiff_t>(stride), cnt, cnt));
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
void MirrorRandomWrite(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize(scs)) };

    size_t elem_cnt{ (MirrorGetElemCnt)(scs) };

    size_t idx{ (GenUniformRandomInt<size_t>)(0, elem_cnt) };
    size_t cnt{ (
        GenUniformRandomInt<size_t>)(0, std::min(max_op_size, elem_cnt)) };

    size_t stride{ (GetRandomStride)(elem_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GenRandomLinSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (Write)(sc, idx, cnt, buffer_a, stride);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::LinSeqLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                static_cast<ptrdiff_t>(stride), cnt, cnt));
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomPushL(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t cnt{ (GenUniformRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GenRandomLinSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (PushL)(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::LinSeqLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                static_cast<ptrdiff_t>(stride), cnt, cnt));
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomPushR(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t cnt{ (GenUniformRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GenRandomLinSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (PushR)(sc, cnt, buffer_a, stride);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::LinSeqLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                static_cast<ptrdiff_t>(stride), cnt, cnt));
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomPopL(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t elem_cnt{ (MirrorGetElemCnt)(scs) };

    size_t cnt{ (
        GenUniformRandomInt<size_t>)(0, std::min(max_op_size, elem_cnt)) };

    bool read{ GenUniformRandomInt<int>(0, 1) == 0 };

    size_t elem_stride_a{ (GetRandomStride)(elem_size) };
    size_t elem_stride_b{ (GetRandomStride)(elem_size) };

    void* buffer_a{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_a * (cnt - 1) + elem_size) };

    void* buffer_b{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_b * (cnt - 1) + elem_size) };

    for (size_t i{ 0 }; i < scs.size(); ++i) {
        (PopL)(scs[i], cnt, i == 0 ? buffer_a : buffer_b, elem_size,
               i == 0 ? elem_stride_a : elem_stride_b);

        if (read && 0 < i) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                core::comparison_utils::LinSeqLexCompare(
                    core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                    elem_size, elem_size, static_cast<ptrdiff_t>(elem_stride_a),
                    static_cast<ptrdiff_t>(elem_stride_b), cnt, cnt));
        }
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomPopR(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t size{ (MirrorGetElemCnt)(scs) };

    size_t cnt{ (GenUniformRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    bool read{ GenUniformRandomInt<int>(0, 1) == 0 };

    size_t elem_stride_a{ (GetRandomStride)(elem_size) };
    size_t elem_stride_b{ (GetRandomStride)(elem_size) };

    ZETA_Core_DebugUtils_Diag_LogVar(elem_size);
    ZETA_Core_DebugUtils_Diag_LogVar(elem_stride_a);
    ZETA_Core_DebugUtils_Diag_LogVar(elem_stride_b);

    void* buffer_a{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_a * (cnt - 1) + elem_size) };

    void* buffer_b{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_b * (cnt - 1) + elem_size) };

    for (size_t i{ 0 }; i < scs.size(); ++i) {
        (PopR)(scs[i], cnt, i == 0 ? buffer_a : buffer_b, elem_size,
               i == 0 ? elem_stride_a : elem_stride_b);

        if (read && 0 < i) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                core::comparison_utils::LinSeqLexCompare(
                    core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                    elem_size, elem_size, static_cast<ptrdiff_t>(elem_stride_a),
                    static_cast<ptrdiff_t>(elem_stride_b), cnt, cnt));
        }
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomInsert(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    size_t size{ (MirrorGetElemCnt)(scs) };

    size_t idx{ (GenUniformRandomInt<size_t>)(0, size) };
    size_t cnt{ (GenUniformRandomInt<size_t>)(0, max_op_size) };

    void* buffer_a{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };
    void* buffer_b{ cnt == 0 ? nullptr
                             : std::malloc(stride * (cnt - 1) + elem_size) };

    GenRandomLinSeq(buffer_a, elem_size, stride, cnt);

    if (0 < cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (cnt - 1) + elem_size);
    }

    for (auto& sc : scs) {
        (Insert)(sc, idx, cnt,
                 core::lin_seq_endpoint::provider::Provider{
                     .data = buffer_b,
                     .elem_size = core::seq_cntr::GetElemSize(*sc),
                     .elem_stride = static_cast<ptrdiff_t>(stride),
                     .elem_cnt = core::seq_cntr::max_max_elem_cnt,
                 });

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::LinSeqLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                static_cast<ptrdiff_t>(stride), cnt, cnt));
    }

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorRandomErase(std::vector<SeqCntr*> const& scs, size_t max_op_size) {
    size_t size{ MirrorGetElemCnt(scs) };

    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t idx{ (GenUniformRandomInt<size_t>)(0, size) };
    size_t cnt{ (GenUniformRandomInt<size_t>)(0, std::min(max_op_size, size)) };

    bool read{ GenUniformRandomInt<int>(0, 1) == 0 };

    size_t elem_stride_a{ (GetRandomStride)(elem_size) };
    size_t elem_stride_b{ (GetRandomStride)(elem_size) };

    void* buffer_a{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_a * (cnt - 1) + elem_size) };

    void* buffer_b{ !read || cnt == 0
                        ? nullptr
                        : std::malloc(elem_stride_b * (cnt - 1) + elem_size) };

    for (size_t i{ 0 }; i < scs.size(); ++i) {
        (Erase)(scs[i], idx, cnt, i == 0 ? buffer_a : buffer_b, elem_size,
                i == 0 ? elem_stride_a : elem_stride_b);

        if (read && 0 < i) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                core::comparison_utils::LinSeqLexCompare(
                    core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                    elem_size, elem_size, static_cast<ptrdiff_t>(elem_stride_a),
                    static_cast<ptrdiff_t>(elem_stride_b), cnt, cnt));
        }
    }
}

template <typename SeqCntr>
void MirrorRandomInit(std::vector<SeqCntr*> const& scs, size_t elem_cnt) {
    size_t elem_size{ (MirrorGetElemSize)(scs) };

    size_t stride{ (GetRandomStride)(elem_size) };

    ZETA_Core_DebugUtils_Logging_ImmLogVar(elem_size);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(stride);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(elem_cnt);

    void* buffer_a{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride * (elem_cnt - 1) + elem_size) };
    void* buffer_b{ elem_cnt == 0
                        ? nullptr
                        : std::malloc(stride * (elem_cnt - 1) + elem_size) };

    GenRandomLinSeq(buffer_a, elem_size, stride, elem_cnt);

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    if (0 < elem_cnt) {
        std::memcpy(buffer_b, buffer_a, stride * (elem_cnt - 1) + elem_size);
    }

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    for (auto& sc : scs) {
        (Erase)(sc, 0, core::seq_cntr::GetElemCnt(*sc), nullptr, 0, 0);

        (PushR)(sc, elem_cnt, buffer_b, stride);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::LinSeqLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size, static_cast<ptrdiff_t>(stride),
                static_cast<ptrdiff_t>(stride), elem_cnt, elem_cnt));
    }

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntrA, typename SeqCntrB>
void MirrorCompare2_(SeqCntrA* sc_a, SeqCntrB* sc_b) {
    size_t elem_size{ (MirrorGetElemSize)(std::vector<SeqCntrA*>{ sc_a }) };

    size_t elem_cnt{ (MirrorGetElemCnt)(std::vector<SeqCntrA*>{ sc_a }) };

    core::seq_cntr::CursorLimit cursor_a;
    core::seq_cntr::CursorLimit cursor_b;

    size_t elem_stride_a{ (GetRandomStride)(elem_size) };
    size_t elem_stride_b{ (GetRandomStride)(elem_size) };

    void* buffer_a{ elem_cnt == 0 ? nullptr
                                  : std::malloc(elem_stride_a * (elem_cnt - 1) +
                                                elem_size) };
    void* buffer_b{ elem_cnt == 0 ? nullptr
                                  : std::malloc(elem_stride_b * (elem_cnt - 1) +
                                                elem_size) };

    core::seq_cntr::PeekL(*sc_a, true, nullptr, &cursor_a, nullptr);
    core::seq_cntr::PeekL(*sc_b, true, nullptr, &cursor_b, nullptr);

    for (size_t i{ 0 }; i < elem_cnt; ++i) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) == i);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) == i);

        core::seq_cntr::Read(
            *sc_a, &cursor_a, 1,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = buffer_a,
                .elem_size = elem_size,
                .elem_stride = static_cast<ptrdiff_t>(elem_stride_a),
                .elem_cnt = 1,
            },
            &cursor_a);

        core::seq_cntr::Read(
            *sc_b, &cursor_b, 1,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = buffer_b,
                .elem_size = elem_size,
                .elem_stride = static_cast<ptrdiff_t>(elem_stride_b),
                .elem_cnt = 1,
            },
            &cursor_b);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::MemLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size));
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) == elem_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) == elem_cnt);

    core::seq_cntr::PeekR(*sc_a, true, nullptr, &cursor_a, nullptr);
    core::seq_cntr::PeekR(*sc_b, true, nullptr, &cursor_b, nullptr);

    for (size_t i{ elem_cnt }; 0 < i--;) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) == i);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) == i);

        core::seq_cntr::Read(
            *sc_a, &cursor_a, 1,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = buffer_a,
                .elem_size = elem_size,
                .elem_stride = static_cast<ptrdiff_t>(elem_stride_a),
                .elem_cnt = 1,
            },
            nullptr);

        core::seq_cntr::Read(
            *sc_b, &cursor_b, 1,
            core::lin_seq_endpoint::acceptor::Acceptor{
                .data = buffer_b,
                .elem_size = elem_size,
                .elem_stride = static_cast<ptrdiff_t>(elem_stride_b),
                .elem_cnt = 1,
            },
            nullptr);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            core::comparison_utils::MemLexCompare(
                core::comparison::OpTags::Equal{}, buffer_a, buffer_b,
                elem_size, elem_size));

        core::seq_cntr::CursorStepL(*sc_a, &cursor_a);

        core::seq_cntr::CursorStepL(*sc_b, &cursor_b);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc_a, &cursor_a) ==
        static_cast<size_t>(-1));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        core::seq_cntr::GetCursorIdx(*sc_b, &cursor_b) ==
        static_cast<size_t>(-1));

    std::free(buffer_a);
    std::free(buffer_b);
}

template <typename SeqCntr>
void MirrorCompare(std::vector<SeqCntr*> const& scs) {
    size_t cntr_cnt{ scs.size() };

    if (cntr_cnt == 1) { return; }

    for (size_t i{ 0 }; i < cntr_cnt; ++i) {
        auto& sc_a{ scs[i] };
        auto& sc_b{ scs[(i + 1) % cntr_cnt] };
        (MirrorCompare2_)(sc_a, sc_b);
    }
}

enum Op : int {
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

    if (0 < read_max_op_size) { ops.push_back(Op::READ); }

    if (0 < write_max_op_size) { ops.push_back(Op::WRITE); }

    if (0 < push_l_max_op_size) { ops.push_back(Op::PUSH_L); }

    if (0 < push_r_max_op_size) { ops.push_back(Op::PUSH_R); }

    if (0 < pop_l_max_op_size) { ops.push_back(Op::POP_L); }

    if (0 < pop_r_max_op_size) { ops.push_back(Op::POP_R); }

    if (0 < insert_max_op_size) { ops.push_back(Op::INSERT); }

    if (0 < erase_max_op_size) { ops.push_back(Op::ERASE); }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!ops.empty());

    for (size_t iter_i{ 0 }; iter_i < iter_cnt; ++iter_i) {
        switch (ops[(GenUniformRandomInt<size_t>)(0, ops.size() - 1)]) {
        case Op::READ:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("READ");
            (MirrorRandomRead)(scs, read_max_op_size);
            break;

        case Op::WRITE:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("WRITE");
            (MirrorRandomWrite)(scs, write_max_op_size);
            break;

        case Op::PUSH_L:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("PUSH_L");
            (MirrorRandomPushL)(scs, push_l_max_op_size);
            break;

        case Op::PUSH_R:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("PUSH_R");
            (MirrorRandomPushR)(scs, push_r_max_op_size);
            break;

        case Op::POP_L:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("POP_L");
            (MirrorRandomPopL)(scs, pop_l_max_op_size);
            break;

        case Op::POP_R:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("POP_R");
            (MirrorRandomPopR)(scs, pop_r_max_op_size);
            break;

        case Op::INSERT:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("INSERT");
            (MirrorRandomInsert)(scs, insert_max_op_size);
            break;

        case Op::ERASE:
            ZETA_Core_DebugUtils_Logging_ImmLogMsg("ERASE");
            (MirrorRandomErase)(scs, erase_max_op_size);
            break;
        }

        for (auto& sc : scs) { (Sanitize)(sc); }

        for (auto& sc : scs) { (CheckCursor)(sc, 16); }

        (MirrorCompare)(scs);
    }
}

}  // namespace zeta::core_test::seq_cntr_utils
