#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/compare.hpp>
#include <zeta/core/compare_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

inline void* circular_array::ReferElem(void* data, size_t elem_stride,
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

inline size_t circular_array::GetLongestContPred(size_t elem_cnt,
                                                 size_t slot_cnt, size_t rot,
                                                 size_t idx) {
    ZETA_Core_DebugAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugAssert(idx <= elem_cnt);
    ZETA_Core_DebugAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ idx <= k ? idx : idx - k };

    return ret;
}

inline size_t circular_array::GetLongestContSucr(size_t elem_cnt,
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

inline void CheckCntr_  // NOLINT(misc-use-internal-linkage)
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

inline void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& cntr, Cursor const* cursor) {
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

inline void circular_array::AssignFromCircularArray(Cntr& dst_cntr,
                                                    size_t dst_beg,
                                                    Cntr const& src_cntr,
                                                    size_t src_beg,
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

inline void circular_array::Init(Cntr const& cntr) { detail::CheckCntr_(cntr); }

inline void circular_array::Deinit(Cntr& cntr) { detail::CheckCntr_(cntr); }

constexpr size_t circular_array::GetCursorSize(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

inline size_t circular_array::GetElemSize(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

inline size_t circular_array::GetElemStride(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_stride;
}

inline size_t circular_array::GetIdxOffset(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.rot;
}

inline size_t circular_array::GetElemCount(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_cnt;
}

inline size_t circular_array::GetMaxElemCnt(Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.slot_cnt;
}

inline void circular_array::GetLBCursor(Cntr const& cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->elem = nullptr;
}

inline void circular_array::GetRBCursor(Cntr const& cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = cntr.elem_cnt;
    dst_cursor->elem = nullptr;
}

inline void circular_array::PeekL(Cntr& cntr, bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
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
        dst_elem_ptr_view->aliasability =
            elem == nullptr
                ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
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

inline void circular_array::PeekL(Cntr const& cntr, bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
    (PeekL)(const_cast<Cntr&>(cntr), lazy_copy_elem, dst_elem_ptr_view,
            dst_cursor, dst_elem);

    if (dst_elem_ptr_view != nullptr &&
        dst_elem_ptr_view->aliasability ==
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
        dst_elem_ptr_view->aliasability =
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }
}

inline void circular_array::PeekR(Cntr& cntr, bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
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
        dst_elem_ptr_view->aliasability =
            elem == nullptr
                ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
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

inline void circular_array::PeekR(Cntr const& cntr, bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
    (PeekR)(const_cast<Cntr&>(cntr), lazy_copy_elem, dst_elem_ptr_view,
            dst_cursor, dst_elem);

    if (dst_elem_ptr_view != nullptr &&
        dst_elem_ptr_view->aliasability ==
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
        dst_elem_ptr_view->aliasability =
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }
}

inline void circular_array::Refer(Cntr& cntr, size_t idx, bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
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
        dst_elem_ptr_view->aliasability =
            elem == nullptr
                ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
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

inline void circular_array::Refer(Cntr const& cntr, size_t idx,
                                  bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem) {
    (Refer)(const_cast<Cntr&>(cntr), idx, lazy_copy_elem, dst_elem_ptr_view,
            dst_cursor, dst_elem);

    if (dst_elem_ptr_view != nullptr &&
        dst_elem_ptr_view->aliasability ==
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
        dst_elem_ptr_view->aliasability =
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }
}

inline void circular_array::Derefer(Cntr& cntr, Cursor const* pos_cursor,
                                    bool lazy_copy_elem,
                                    seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                    void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr || dst_elem != nullptr);

    void* elem{ pos_cursor->elem };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            elem == nullptr
                ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

inline void circular_array::Derefer(Cntr const& cntr, Cursor const* pos_cursor,
                                    bool lazy_copy_elem,
                                    seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                    void* dst_elem) {
    (Derefer)(const_cast<Cntr&>(cntr), pos_cursor, lazy_copy_elem,
              dst_elem_ptr_view, dst_elem);

    if (dst_elem_ptr_view != nullptr &&
        dst_elem_ptr_view->aliasability ==
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
        dst_elem_ptr_view->aliasability =
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
    }
}

namespace circular_array::detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr& cntr, size_t idx, size_t cnt, ReaderWriter&& reader_writer,
     Cursor* dst_cursor) {
    (CheckCntr_)(cntr);

    void* data{ cntr.data };
    size_t elem_stride{ cntr.elem_stride };
    size_t rot{ cntr.rot };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanDerefer(idx, cnt, elem_cnt));

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        reader_writer(
            static_cast<meta::Conditional<EnWrite, void*, void const*>>(
                (ReferElem)(data, elem_stride, slot_cnt, rot, idx)),
            elem_stride, cur_cnt);

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

template <typename Reader>
void circular_array::Read(Cntr const& cntr, Cursor const* pos_cursor,
                          size_t cnt, Reader&& reader, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<false>(const_cast<Cntr&>(cntr), pos_cursor->idx, cnt,
                              reader, dst_cursor);
}

template <typename Writer>
void circular_array::Write(Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
                           Writer&& writer, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void circular_array::ReadWrite(Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
                               ReaderWriter&& reader_writer,
                               Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, reader_writer,
                             dst_cursor);
}

template <typename ReaderWriter>
void circular_array::IdxRead(Cntr const& cntr, size_t idx, size_t cnt,
                             ReaderWriter&& reader) {
    detail::ReadWrite_<false>(const_cast<Cntr&>(cntr), idx, cnt, reader,
                              nullptr);
}

template <typename ReaderWriter>
void circular_array::IdxWrite(Cntr& cntr, size_t idx, size_t cnt,
                              ReaderWriter&& writer) {
    detail::ReadWrite_<true>(cntr, idx, cnt, writer, nullptr);
}

template <typename ReaderWriter>
void circular_array::IdxReadWrite(Cntr& cntr, size_t idx, size_t cnt,
                                  ReaderWriter&& reader_writer) {
    detail::ReadWrite_<true>(cntr, idx, cnt, reader_writer, nullptr);
}

template <typename Writer>
void circular_array::PushL(Cntr& cntr, size_t cnt, Writer&& writer,
                           Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr.data };
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

        writer((ReferElem)(data, elem_stride, slot_cnt, rot, idx), elem_stride,
               cur_cnt);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

template <typename Writer>
void circular_array::PushR(Cntr& cntr, size_t cnt, Writer&& writer,
                           Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr.data };
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

        writer((ReferElem)(data, elem_stride, slot_cnt, rot, idx), elem_stride,
               cur_cnt);

        idx += cur_cnt;
    }
}

template <typename Writer>
void circular_array::Insert(Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                            Writer&& writer, Cursor* dst_cursor) {
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
        (AssignFromCircularArray)(cntr, 0, cntr, cnt, l_size);
    } else {
        (AssignFromCircularArray)(cntr, l_size + cnt, cntr, l_size, r_size);
    }

    pos_cursor->elem = (ReferElem)(data, elem_stride, slot_cnt, rot, idx);

    (Write)(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename Writer>
void circular_array::IdxInsert(Cntr& cntr, size_t idx, size_t cnt,
                               Writer&& writer) {
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
        (AssignFromCircularArray)(cntr, 0, cntr, cnt, l_size);
    } else {
        (AssignFromCircularArray)(cntr, l_size + cnt, cntr, l_size, r_size);
    }

    (IdxWrite)(cntr, idx, cnt, writer);
}

template <typename Reader>
void circular_array::PopL(Cntr& cntr, size_t cnt, Reader&& reader) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };
    size_t slot_cnt{ cntr.slot_cnt };
    size_t rot{ cntr.rot + cnt };

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanPopL(cnt, elem_cnt));

    (IdxRead)(cntr, 0, cnt, reader);

    cntr.rot = rot < slot_cnt ? rot : rot - slot_cnt;
    cntr.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { cntr.rot = 0; }
}

template <typename Reader>
void circular_array::PopR(Cntr& cntr, size_t cnt, Reader&& reader) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanPopR(cnt, elem_cnt));

    (IdxRead)(cntr, cntr.elem_cnt - cnt, cnt, reader);

    cntr.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { cntr.rot = 0; }
}

template <typename Reader>
void circular_array::Erase(Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                           Reader&& reader) {
    detail::CheckCursor_(cntr, pos_cursor);

    size_t idx{ pos_cursor->idx };

    (IdxErase)(cntr, idx, cnt, reader);

    pos_cursor->elem = idx < cntr.elem_cnt
                           ? (ReferElem)(cntr.data, cntr.elem_stride,
                                         cntr.slot_cnt, cntr.rot, idx)
                           : nullptr;
}

template <typename Reader>
void circular_array::IdxErase(Cntr& cntr, size_t idx, size_t cnt,
                              Reader&& reader) {
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

    (IdxRead)(cntr, idx, cnt, reader);

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        (AssignFromCircularArray)(cntr, cnt, cntr, 0, l_size);

        rot += cnt;
        cntr.rot = rot = rot < slot_cnt ? rot : rot - slot_cnt;
    } else {
        (AssignFromCircularArray)(cntr, l_size, cntr, l_size + cnt, r_size);
    }

    cntr.elem_cnt = (elem_cnt -= cnt);

    if (elem_cnt == 0) { cntr.rot = rot = 0; }
}

inline void circular_array::EraseAll(Cntr& cntr) {
    detail::CheckCntr_(cntr);

    cntr.rot = 0;
    cntr.elem_cnt = 0;
}

inline void circular_array::CopyCursor(Cntr const& cntr,
                                       Cursor const* src_cursor,
                                       Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->elem = src_cursor->elem;
}

inline bool circular_array::AreEqualCursor(Cntr const& cntr,
                                           Cursor const* cursor_a,
                                           Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_a) == (GetCursorIdx)(cntr, cursor_b);
}

inline int circular_array::CompareCursor(Cntr const& cntr,
                                         Cursor const* cursor_a,
                                         Cursor const* cursor_b) {
    return comparison::BasicCompare(comparison::ComparisonTypeEnum::ThreeWay{},
                                    (GetCursorIdx)(cntr, cursor_a) + 1,
                                    (GetCursorIdx)(cntr, cursor_b) + 1);
}

inline size_t circular_array::GetCursorDist(Cntr const& cntr,
                                            Cursor const* cursor_a,
                                            Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_b) - (GetCursorIdx)(cntr, cursor_a);
}

inline size_t circular_array::GetCursorIdx(Cntr const& cntr,
                                           Cursor const* cursor) {
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

inline void circular_array::CursorStepL(Cntr const& cntr, Cursor* cursor) {
    (CursorAdvanceL)(cntr, cursor, 1);
}

inline void circular_array::CursorStepR(Cntr const& cntr, Cursor* cursor) {
    (CursorAdvanceR)(cntr, cursor, 1);
}

inline void circular_array::CursorAdvanceL(Cntr const& cntr, Cursor* cursor,
                                           size_t step) {
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

inline void circular_array::CursorAdvanceR(Cntr const& cntr, Cursor* cursor,
                                           size_t step) {
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

template <typename SrcSeqCntr>
void circular_array::AssignFromSeqCntr(Cntr& cntr, size_t dst_beg,
                                       SrcSeqCntr const& src_seq_cntr,
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
            seq_cntr::MemReader{
                .data = (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
                .elem_size = elem_size,
                .elem_stride = elem_stride,
            },
            src_seq_cntr_cursor);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

inline void*
seq_cntr::CntrTraits<circular_array::Cntr const>::GetReferedInstPtr(
    circular_array::Cntr const& cntr) {
    return &const_cast<circular_array::Cntr&>(cntr);
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<circular_array::Cntr>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CapabilityFlagBuilder{
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

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    circular_array::Cntr const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CntrTraits<
               circular_array::Cntr>::GetStaticEnabledCapabilityFlag() &
           seq_cntr::const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<circular_array::Cntr>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::empty_capability_flag;
}

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    circular_array::Cntr const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::non_const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag seq_cntr::
    CntrTraits<circular_array::Cntr const>::GetDynamicEnabledCapabilityFlag(
        circular_array::Cntr const&) {
    return seq_cntr::empty_capability_flag;
}

constexpr seq_cntr::CapabilityFlag seq_cntr::
    CntrTraits<circular_array::Cntr const>::GetDynamicDisabledCapabilityFlag(
        circular_array::Cntr const&) {
    return seq_cntr::empty_capability_flag;
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetCursorSize(
    circular_array::Cntr const& cntr) {
    return circular_array::GetCursorSize(cntr);
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetElemSize(
    circular_array::Cntr const& cntr) {
    return circular_array::GetElemSize(cntr);
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetElemCnt(
    circular_array::Cntr const& cntr) {
    return circular_array::GetElemCount(cntr);
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetMaxElemCnt(
    circular_array::Cntr const& cntr) {
    return circular_array::GetMaxElemCnt(cntr);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::GetLBCursor(
    circular_array::Cntr const& cntr, void* dst_cursor) {
    circular_array::GetLBCursor(
        cntr, static_cast<circular_array::Cursor*>(dst_cursor));
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::GetRBCursor(
    circular_array::Cntr const& cntr, void* dst_cursor) {
    circular_array::GetRBCursor(
        cntr, static_cast<circular_array::Cursor*>(dst_cursor));
}

inline void seq_cntr::CntrTraits<circular_array::Cntr>::PeekL(
    circular_array::Cntr& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::PeekL(cntr, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::PeekL(
    circular_array::Cntr const& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::PeekL(cntr, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr>::PeekR(
    circular_array::Cntr& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::PeekR(cntr, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::PeekR(
    circular_array::Cntr const& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::PeekR(cntr, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr>::Refer(
    circular_array::Cntr& cntr, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::Refer(cntr, idx, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::Refer(
    circular_array::Cntr const& cntr, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    circular_array::Refer(cntr, idx, lazy_copy_elem, dst_elem_ptr_view,
                          static_cast<circular_array::Cursor*>(dst_cursor),
                          dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr>::Derefer(
    circular_array::Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    circular_array::Derefer(cntr,
                            static_cast<circular_array::Cursor*>(pos_cursor),
                            lazy_copy_elem, dst_elem_ptr_view, dst_elem);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::Derefer(
    circular_array::Cntr const& cntr, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    circular_array::Derefer(cntr,
                            static_cast<circular_array::Cursor*>(pos_cursor),
                            lazy_copy_elem, dst_elem_ptr_view, dst_elem);
}

template <typename Reader>
void seq_cntr::CntrTraits<circular_array::Cntr const>::Read(
    circular_array::Cntr const& cntr, void* pos_cursor, size_t cnt,
    Reader&& reader, void* dst_cursor) {
    circular_array::Read(cntr, static_cast<circular_array::Cursor*>(pos_cursor),
                         cnt, reader,
                         static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename Writer>
void seq_cntr::CntrTraits<circular_array::Cntr>::Write(
    circular_array::Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    circular_array::Write(
        cntr, static_cast<circular_array::Cursor*>(pos_cursor), cnt, writer,
        static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename ReaderWriter>
void seq_cntr::CntrTraits<circular_array::Cntr>::ReadWrite(
    circular_array::Cntr& cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    circular_array::ReadWrite(
        cntr, static_cast<circular_array::Cursor*>(pos_cursor), cnt,
        reader_writer, static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename Writer>
void seq_cntr::CntrTraits<circular_array::Cntr>::PushL(
    circular_array::Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor) {
    circular_array::PushL(cntr, cnt, writer,
                          static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename Writer>
void seq_cntr::CntrTraits<circular_array::Cntr>::PushR(
    circular_array::Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor) {
    circular_array::PushR(cntr, cnt, writer,
                          static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename Writer>
void seq_cntr::CntrTraits<circular_array::Cntr>::Insert(
    circular_array::Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    circular_array::Insert(
        cntr, static_cast<circular_array::Cursor*>(pos_cursor), cnt, writer,
        static_cast<circular_array::Cursor*>(dst_cursor));
}

template <typename Reader>
void seq_cntr::CntrTraits<circular_array::Cntr>::PopL(
    circular_array::Cntr& cntr, size_t cnt, Reader&& reader) {
    circular_array::PopL(cntr, cnt, reader);
}

template <typename Reader>
void seq_cntr::CntrTraits<circular_array::Cntr>::PopR(
    circular_array::Cntr& cntr, size_t cnt, Reader&& reader) {
    circular_array::PopR(cntr, cnt, reader);
}

template <typename Reader>
void seq_cntr::CntrTraits<circular_array::Cntr>::Erase(
    circular_array::Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader) {
    circular_array::Erase(
        cntr, static_cast<circular_array::Cursor*>(pos_cursor), cnt, reader);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr>::EraseAll(
    circular_array::Cntr& cntr) {
    circular_array::EraseAll(cntr);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::CopyCursor(
    circular_array::Cntr const& cntr, void const* src_cursor,
    void* dst_cursor) {
    circular_array::CopyCursor(
        cntr, static_cast<circular_array::Cursor const*>(src_cursor),
        static_cast<circular_array::Cursor*>(dst_cursor));
}

inline bool seq_cntr::CntrTraits<circular_array::Cntr const>::AreEqualCursor(
    circular_array::Cntr const& cntr, void const* cursor_a,
    void const* cursor_b) {
    return circular_array::AreEqualCursor(
        cntr, static_cast<circular_array::Cursor const*>(cursor_a),
        static_cast<circular_array::Cursor const*>(cursor_b));
}

inline int seq_cntr::CntrTraits<circular_array::Cntr const>::CompareCursor(
    circular_array::Cntr const& cntr, void const* cursor_a,
    void const* cursor_b) {
    return circular_array::CompareCursor(
        cntr, static_cast<circular_array::Cursor const*>(cursor_a),
        static_cast<circular_array::Cursor const*>(cursor_b));
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetCursorDist(
    circular_array::Cntr const& cntr, void const* cursor_a,
    void const* cursor_b) {
    return circular_array::GetCursorDist(
        cntr, static_cast<circular_array::Cursor const*>(cursor_a),
        static_cast<circular_array::Cursor const*>(cursor_b));
}

inline size_t seq_cntr::CntrTraits<circular_array::Cntr const>::GetCursorIdx(
    circular_array::Cntr const& cntr, void const* cursor) {
    return circular_array::GetCursorIdx(
        cntr, static_cast<circular_array::Cursor const*>(cursor));
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::CursorStepL(
    circular_array::Cntr const& cntr, void* cursor) {
    circular_array::CursorStepL(cntr,
                                static_cast<circular_array::Cursor*>(cursor));
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::CursorStepR(
    circular_array::Cntr const& cntr, void* cursor) {
    circular_array::CursorStepR(cntr,
                                static_cast<circular_array::Cursor*>(cursor));
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::CursorAdvanceL(
    circular_array::Cntr const& cntr, void* cursor, size_t step) {
    circular_array::CursorAdvanceL(
        cntr, static_cast<circular_array::Cursor*>(cursor), step);
}

inline void seq_cntr::CntrTraits<circular_array::Cntr const>::CursorAdvanceR(
    circular_array::Cntr const& cntr, void* cursor, size_t step) {
    circular_array::CursorAdvanceR(
        cntr, static_cast<circular_array::Cursor*>(cursor), step);
}

}  // namespace zeta::core
