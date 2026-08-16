#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

constexpr void* circular_array::ReferElem(void* data, size_t elem_stride,
                                          size_t slot_cnt, size_t rot,
                                          size_t idx) {
    ZETA_Core_DebugAssert(data != nullptr);
    ZETA_Core_DebugAssert(0 < elem_stride);
    ZETA_Core_DebugAssert(rot < slot_cnt);
    ZETA_Core_DebugAssert(idx < slot_cnt);
    ZETA_Core_DebugAssert(slot_cnt <= ZETA_Core_max_capacity);

    size_t k{ rot + idx };

    return static_cast<char*>(data) +
           elem_stride * (k < slot_cnt ? k : k - slot_cnt);
}

constexpr size_t circular_array::GetLongestContPred(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugAssert(idx <= elem_cnt);
    ZETA_Core_DebugAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ idx <= k ? idx : idx - k };

    return ret;
}

constexpr size_t circular_array::GetLongestContSucr(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugAssert(idx <= elem_cnt);
    ZETA_Core_DebugAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ (elem_cnt <= k || k <= idx) ? elem_cnt - idx : k - idx };

    return ret;
}

namespace circular_array::detail {

constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& cntr) {
    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(0 < elem_size);
    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(elem_cnt <= slot_cnt);
    ZETA_Core_DebugAssert(slot_cnt <= ZETA_Core_max_capacity);
    ZETA_Core_DebugAssert(rot < slot_cnt);
    ZETA_Core_DebugAssert(data != nullptr || slot_cnt == 0);
}

constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& cntr, Cursor const* cursor_) {
    Cursor const* cursor{ static_cast<Cursor const*>(cursor_) };

    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(&cntr == cursor->cntr);

    void* data{ cntr.data };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanRefer(cursor->idx, 1, elem_cnt));

    ZETA_Core_DebugAssert(elem_cnt <= cursor->idx ||
                          cursor->elem == (ReferElem)(data, elem_stride,
                                                      slot_cnt, rot,
                                                      cursor->idx));
}

}  // namespace circular_array::detail

constexpr void circular_array::Cntr::AssignFromCircularArray(
    this Cntr& dst_cntr, size_t dst_beg, Cntr const& src_cntr, size_t src_beg,
    size_t cnt) {
    detail::CheckCntr_(dst_cntr);
    detail::CheckCntr_(src_cntr);

    char* dst_data{ static_cast<char*>(dst_cntr.data) };
    size_t dst_elem_size{ dst_cntr.elem_size };
    size_t dst_elem_stride{ dst_cntr.elem_stride };
    size_t dst_elem_cnt{ dst_cntr.elem_cnt };
    size_t dst_capacity{ dst_cntr.slot_cnt };
    size_t dst_rot{ dst_cntr.rot };

    char* src_data{ static_cast<char*>(src_cntr.data) };
    size_t src_elem_size{ src_cntr.elem_size };
    size_t src_elem_stride{ src_cntr.elem_stride };
    size_t src_elem_cnt{ src_cntr.elem_cnt };
    size_t src_capacity{ src_cntr.slot_cnt };
    size_t src_rot{ src_cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(src_beg, cnt, src_elem_cnt));

    if (cnt == 0) { return; }

    size_t elem_size{ comparison_utils::BasicMin(dst_elem_size,
                                                 src_elem_size) };

    if (&dst_cntr != &src_cntr) {
        ZETA_Core_DebugAssert(
            !ZETA_Core_AreOverlapped(dst_data, dst_data + dst_capacity,
                                     src_data, src_data + src_capacity));

        goto VEC_FW_MOVE;
    }

    if (dst_beg < src_beg) { goto VEC_FW_MOVE; }

    if (src_beg < dst_beg) { goto VEC_BW_MOVE; }

    return;

VEC_FW_MOVE:

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt,
            (GetLongestContSucr)(dst_elem_cnt, dst_capacity, dst_rot, dst_beg),
            (GetLongestContSucr)(src_elem_cnt, src_capacity, src_rot,
                                 src_beg)) };

        utils::ElemMove((ReferElem)(dst_data, dst_elem_stride, dst_capacity,
                                    dst_rot, dst_beg),
                        (ReferElem)(src_data, src_elem_stride, src_capacity,
                                    src_rot, src_beg),
                        elem_size, dst_elem_stride, src_elem_stride, cur_cnt);

        dst_beg += cur_cnt;
        src_beg += cur_cnt;

        cnt -= cur_cnt;
    }

    return;

VEC_BW_MOVE:

    for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt,
            (GetLongestContPred)(dst_elem_cnt, dst_capacity, dst_rot, dst_end),
            (GetLongestContPred)(src_elem_cnt, src_capacity, src_rot,
                                 src_end)) };

        dst_end -= cur_cnt;
        src_end -= cur_cnt;

        utils::ElemMove((ReferElem)(dst_data, dst_elem_stride, dst_capacity,
                                    dst_rot, dst_end),
                        (ReferElem)(src_data, src_elem_stride, src_capacity,
                                    src_rot, src_end),
                        elem_size, dst_elem_stride, src_elem_stride, cur_cnt);

        cnt -= cur_cnt;
    }
}

constexpr void circular_array::Cntr::Init(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);
}

constexpr void circular_array::Cntr::Deinit(this Cntr& cntr) {
    detail::CheckCntr_(cntr);
}

constexpr void* circular_array::Cntr::GetReferedInstPtr(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return const_cast<void*>(static_cast<void const*>(&cntr));
}

constexpr size_t circular_array::Cntr::GetCursorSize(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

constexpr size_t circular_array::Cntr::GetElemSize(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

constexpr size_t circular_array::Cntr::GetElemStride(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_stride;
}

constexpr size_t circular_array::Cntr::GetIdxOffset(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.rot;
}

constexpr size_t circular_array::Cntr::GetElemCnt(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_cnt;
}

constexpr size_t circular_array::Cntr::GetMaxElemCnt(this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.slot_cnt;
}

constexpr void circular_array::Cntr::GetLBCursor(this Cntr const& cntr,
                                                 Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->elem = nullptr;
}

constexpr void circular_array::Cntr::GetRBCursor(this Cntr const& cntr,
                                                 Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = cntr.elem_cnt;
    dst_cursor->elem = nullptr;
}

constexpr void circular_array::Cntr::PeekL(
    this auto&& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr ||
                          dst_cursor != nullptr || dst_elem != nullptr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, 0)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::PeekR(
    this auto&& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr ||
                          dst_cursor != nullptr || dst_elem != nullptr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot,
                                           elem_cnt - 1)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = elem_cnt - 1;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::Refer(
    this auto&& cntr, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr ||
                          dst_cursor != nullptr || dst_elem != nullptr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, elem_cnt));

    void* elem{ idx < elem_cnt
                    ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                    : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = idx;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::Derefer(
    this auto&& cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr || dst_elem != nullptr);

    void* elem{ pos_cursor->elem };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

namespace circular_array::detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr& cntr, size_t idx, size_t cnt, ReaderWriter&& reader_writer,
     Cursor* dst_cursor) {
    (CheckCntr_)(cntr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t rot{ cntr.rot };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(idx, cnt, elem_cnt));

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        if constexpr (EnWrite) {
            elem_stream::provider::Transfer(
                reader_writer,
                static_cast<void*>(
                    (ReferElem)(data, elem_stride, slot_cnt, rot, idx)),
                elem_size, elem_stride, cur_cnt);
        } else {
            elem_stream::acceptor::Transfer(
                reader_writer,
                static_cast<void const*>(
                    (ReferElem)(data, elem_stride, slot_cnt, rot, idx)),
                elem_size, elem_stride, cur_cnt);
        }

        idx += cur_cnt;
        cnt -= cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = idx;
        dst_cursor->elem =
            idx < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                           : nullptr;
    }
}

}  // namespace circular_array::detail

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::Read(this Cntr const& cntr,
                                          Cursor const* pos_cursor, size_t cnt,
                                          Reader&& reader, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<false>(const_cast<Cntr&>(cntr), pos_cursor->idx, cnt,
                              reader, dst_cursor);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::Write(this Cntr& cntr,
                                           Cursor const* pos_cursor, size_t cnt,
                                           Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, writer, dst_cursor);
}

template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr void circular_array::Cntr::ReadWrite(this Cntr& cntr,
                                               Cursor const* pos_cursor,
                                               size_t cnt,
                                               ReaderWriter&& reader_writer,
                                               Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, reader_writer,
                             dst_cursor);
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::IdxRead(this Cntr const& cntr, size_t idx,
                                             size_t cnt, Reader&& reader) {
    detail::ReadWrite_<false>(const_cast<Cntr&>(cntr), idx, cnt, reader,
                              nullptr);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::IdxWrite(this Cntr& cntr, size_t idx,
                                              size_t cnt, Writer&& writer) {
    detail::ReadWrite_<true>(cntr, idx, cnt, writer, nullptr);
}

template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr void circular_array::Cntr::IdxReadWrite(
    this Cntr& cntr, size_t idx, size_t cnt, ReaderWriter&& reader_writer) {
    detail::ReadWrite_<true>(cntr, idx, cnt, reader_writer, nullptr);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::PushL(this Cntr& cntr, size_t cnt,
                                           Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanPushL(cnt, elem_cnt, slot_cnt));

    cntr.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
    cntr.elem_cnt = elem_cnt += cnt;

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, 0)
                             : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ 0 }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        elem_stream::provider::Transfer(
            writer, (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
            elem_size, elem_stride, cur_cnt);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::PushR(this Cntr& cntr, size_t cnt,
                                           Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanPushL(cnt, elem_cnt, slot_cnt));

    cntr.elem_cnt = elem_cnt += cnt;

    void* elem{ 0 < cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot,
                                      elem_cnt - cnt)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = elem_cnt - cnt;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ elem_cnt - cnt }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        cnt -= cur_cnt;

        elem_stream::provider::Transfer(
            writer, (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
            elem_size, elem_stride, cur_cnt);

        idx += cur_cnt;
    }
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::Insert(this Cntr& cntr, Cursor* pos_cursor,
                                            size_t cnt, Writer&& writer,
                                            Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    void* data{ cntr.data };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanInsert(idx, cnt, elem_cnt, slot_cnt));

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx };

    cntr.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        cntr.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
        cntr.AssignFromCircularArray(0, cntr, cnt, l_size);
    } else {
        cntr.AssignFromCircularArray(l_size + cnt, cntr, l_size, r_size);
    }

    pos_cursor->elem = (ReferElem)(data, elem_stride, slot_cnt, rot, idx);

    cntr.Write(pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::IdxInsert(this Cntr& cntr, size_t idx,
                                               size_t cnt, Writer&& writer) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanInsert(idx, cnt, elem_cnt, slot_cnt));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx };

    cntr.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        cntr.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
        cntr.AssignFromCircularArray(0, cntr, cnt, l_size);
    } else {
        cntr.AssignFromCircularArray(l_size + cnt, cntr, l_size, r_size);
    }

    cntr.IdxWrite(idx, cnt, writer);
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::PopL(this Cntr& cntr, size_t cnt,
                                          Reader&& reader) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot + cnt };

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanPopL(cnt, elem_cnt));

    cntr.IdxRead(0, cnt, reader);

    cntr.rot = rot < slot_cnt ? rot : rot - slot_cnt;
    cntr.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { cntr.rot = 0; }
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::PopR(this Cntr& cntr, size_t cnt,
                                          Reader&& reader) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanPopR(cnt, elem_cnt));

    cntr.IdxRead(cntr.elem_cnt - cnt, cnt, reader);

    cntr.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { cntr.rot = 0; }
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::Erase(this Cntr& cntr, Cursor* pos_cursor,
                                           size_t cnt, Reader&& reader) {
    detail::CheckCursor_(cntr, pos_cursor);

    size_t idx{ pos_cursor->idx };

    cntr.IdxErase(idx, cnt, reader);

    pos_cursor->elem = idx < cntr.elem_cnt
                           ? (ReferElem)(cntr.data, cntr.elem_stride,
                                         cntr.slot_cnt, cntr.rot, idx)
                           : nullptr;
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::IdxErase(this Cntr& cntr, size_t idx,
                                              size_t cnt, Reader&& reader) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanErase(idx, cnt, elem_cnt));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx - cnt };

    unsigned long long random_seed{ utils::GetRandom() };

    cntr.IdxRead(idx, cnt, reader);

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        cntr.AssignFromCircularArray(cnt, cntr, 0, l_size);

        rot += cnt;
        cntr.rot = rot = rot < slot_cnt ? rot : rot - slot_cnt;
    } else {
        cntr.AssignFromCircularArray(l_size, cntr, l_size + cnt, r_size);
    }

    cntr.elem_cnt = (elem_cnt -= cnt);

    if (elem_cnt == 0) { cntr.rot = rot = 0; }
}

constexpr void circular_array::Cntr::EraseAll(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    cntr.rot = 0;
    cntr.elem_cnt = 0;
}

constexpr void circular_array::Cntr::CopyCursor(this Cntr const& cntr,
                                                void const* src_cursor_,
                                                Cursor* dst_cursor) {
    Cursor const* src_cursor{ static_cast<Cursor const*>(src_cursor_) };

    detail::CheckCursor_(cntr, src_cursor);

    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->elem = src_cursor->elem;
}

constexpr bool circular_array::Cntr::AreEqualCursor(this Cntr const& cntr,
                                                    Cursor const* cursor_a,
                                                    Cursor const* cursor_b) {
    return cntr.GetCursorIdx(cursor_a) == cntr.GetCursorIdx(cursor_b);
}

constexpr comparison::Ordering circular_array::Cntr::CompareCursor(
    this Cntr const& cntr, Cursor const* cursor_a, Cursor const* cursor_b) {
    return comparison::BasicCompare(
        meta::AutoValueWrapper<comparison::OpEnum::Order>{},
        cntr.GetCursorIdx(cursor_a) + 1, cntr.GetCursorIdx(cursor_b) + 1);
}

constexpr size_t circular_array::Cntr::GetCursorDist(this Cntr const& cntr,
                                                     Cursor const* cursor_a,
                                                     Cursor const* cursor_b) {
    return cntr.GetCursorIdx(cursor_b) - cntr.GetCursorIdx(cursor_a);
}

constexpr size_t circular_array::Cntr::GetCursorIdx(this Cntr const& cntr,
                                                    Cursor const* cursor_) {
    Cursor const* cursor{ static_cast<Cursor const*>(cursor_) };

    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

constexpr void circular_array::Cntr::CursorStepL(this Cntr const& cntr,
                                                 Cursor* cursor) {
    cntr.CursorAdvanceL(cursor, 1);
}

constexpr void circular_array::Cntr::CursorStepR(this Cntr const& cntr,
                                                 Cursor* cursor) {
    cntr.CursorAdvanceR(cursor, 1);
}

constexpr void circular_array::Cntr::CursorAdvanceL(this Cntr const& cntr,
                                                    Cursor* cursor_,
                                                    size_t step) {
    Cursor* cursor{ static_cast<Cursor*>(cursor_) };

    detail::CheckCursor_(cntr, cursor);

    void* data{ cntr.data };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

constexpr void circular_array::Cntr::CursorAdvanceR(this Cntr const& cntr,
                                                    Cursor* cursor_,
                                                    size_t step) {
    Cursor* cursor{ static_cast<Cursor*>(cursor_) };

    detail::CheckCursor_(cntr, cursor);

    void* data{ cntr.data };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    ZETA_Core_DebugAssert(step <= elem_cnt - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

template <seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void circular_array::Cntr::AssignFromSeqCntr(
    this Cntr& cntr, size_t dst_beg, SrcSeqCntr const& src_seq_cntr,
    void* src_seq_cntr_cursor, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(src_seq_cntr_cursor != nullptr);

    /*
    TODO
    if constexpr (IsSame<RemoveCVRef<SrcSeqCntr>, seq_cntr_ref::Ref>) {
        if (src_seq_cntr.vtable ==
            &seq_cntr::GetVTable(type_wrapper::TypeWrapper<Cntr>{})) {
            auto const* src_ca_cursor{ static_cast<Cursor*>(
                src_seq_cntr_cursor) };

            auto const* src_ca{ static_cast<Cntr const*>(src_seq_cntr.cntr) };

            detail::CheckCursor_(src_ca, src_ca_cursor);

            AssignFromCircularArray(cntr, dst_beg, src_ca, src_ca_cursor->idx,
                                    cnt);

            return;
        }
    }
    */

    void* data{ cntr.data };
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot };

    size_t idx{ dst_beg };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(dst_beg, cnt, elem_cnt));

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        seq_cntr::Read(
            src_seq_cntr, src_seq_cntr_cursor, cur_cnt,
            lin_seq_elem_stream::Acceptor{
                .data = (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
                .elem_size = elem_size,
                .elem_stride = elem_stride,
                .elem_cnt = cur_cnt,
            },
            src_seq_cntr_cursor);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<circular_array::Cntr>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::FlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer = true,
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

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<circular_array::Cntr>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<circular_array::Cntr>::GetDynamicEnabledCapabilityFlag(
    circular_array::Cntr&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<circular_array::Cntr>::GetDynamicDisabledCapabilityFlag(
    circular_array::Cntr&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    circular_array::Cntr const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::GetStaticEnabledCapabilityFlag<circular_array::Cntr>() &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    circular_array::Cntr const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::GetStaticDisabledCapabilityFlag<circular_array::Cntr>() |
           seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::
    CntrTraits<circular_array::Cntr const>::GetDynamicEnabledCapabilityFlag(
        circular_array::Cntr const&) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::
    CntrTraits<circular_array::Cntr const>::GetDynamicDisabledCapabilityFlag(
        circular_array::Cntr const&) {
    return seq_cntr::capability::empty_capability_flag;
}

}  // namespace zeta::core
