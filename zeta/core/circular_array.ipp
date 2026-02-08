#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/compare.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core::circular_array {

namespace ops {

inline void* ReferElem(void* data, size_t stride, size_t offset, size_t idx,
                       size_t capacity) {
    ZETA_Core_DebugAssert(data != nullptr);
    ZETA_Core_DebugAssert(0 < stride);
    ZETA_Core_DebugAssert(offset < capacity);
    ZETA_Core_DebugAssert(idx < capacity);
    ZETA_Core_DebugAssert(capacity <= ZETA_Core_max_capacity);

    size_t k{ offset + idx };

    return static_cast<char*>(data) +
           stride * (k < capacity ? k : k - capacity);
}

inline size_t GetLongestContPred(size_t offset, size_t idx, size_t size,
                                 size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return idx <= k ? idx : idx - k;
}

inline size_t GetLongestContSucr(size_t offset, size_t idx, size_t size,
                                 size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return (size <= k || k <= idx) ? size - idx : k - idx;
}

namespace detail {

inline void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    void* data{ cntr->data };
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(size <= capacity);
    ZETA_Core_DebugAssert(capacity <= ZETA_Core_max_capacity);
    ZETA_Core_DebugAssert(data != nullptr || capacity == 0);
}

inline void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const* cntr, Cursor const* cursor) {
    CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(cntr == cursor->cntr);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(cursor->idx, 1, size));

    ZETA_Core_DebugAssert(
        size <= cursor->idx ||
        cursor->elem == ReferElem(data, stride, offset, cursor->idx, capacity));
}

}  // namespace detail

inline void AssignFromCircularArray(Cntr* dst_cntr, size_t dst_beg,
                                    Cntr const* src_cntr, size_t src_beg,
                                    size_t cnt) {
    detail::CheckCntr_(dst_cntr);
    detail::CheckCntr_(src_cntr);

    char* dst_data{ static_cast<char*>(dst_cntr->data) };
    size_t dst_width{ dst_cntr->width };
    size_t dst_stride{ dst_cntr->stride };
    size_t dst_offset{ dst_cntr->offset };
    size_t dst_size{ dst_cntr->size };
    size_t dst_capacity{ dst_cntr->capacity };

    char* src_data{ static_cast<char*>(src_cntr->data) };
    size_t src_width{ src_cntr->width };
    size_t src_stride{ src_cntr->stride };
    size_t src_offset{ src_cntr->offset };
    size_t src_size{ src_cntr->size };
    size_t src_capacity{ src_cntr->capacity };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsDereferable(dst_beg, cnt, dst_size));
    ZETA_Core_DebugAssert(seq_cntr::ops::IsDereferable(src_beg, cnt, src_size));

    if (cnt == 0) { return; }

    size_t width{ Min(dst_width, src_width) };

    if (dst_cntr != src_cntr) {
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
        size_t cur_cnt{ Min(
            cnt,
            GetLongestContSucr(dst_offset, dst_beg, dst_size, dst_capacity),
            GetLongestContSucr(src_offset, src_beg, src_size, src_capacity)) };

        ElemMove(
            ReferElem(dst_data, dst_stride, dst_offset, dst_beg, dst_capacity),
            ReferElem(src_data, src_stride, src_offset, src_beg, src_capacity),
            width, dst_stride, src_stride, cur_cnt);

        dst_beg += cur_cnt;
        src_beg += cur_cnt;

        cnt -= cur_cnt;
    }

    return;

VEC_BW_MOVE:

    for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(
            cnt,
            GetLongestContPred(dst_offset, dst_end, dst_size, dst_capacity),
            GetLongestContPred(src_offset, src_end, src_size, src_capacity)) };

        dst_end -= cur_cnt;
        src_end -= cur_cnt;

        ElemMove(
            ReferElem(dst_data, dst_stride, dst_offset, dst_end, dst_capacity),
            ReferElem(src_data, src_stride, src_offset, src_end, src_capacity),
            width, dst_stride, src_stride, cur_cnt);

        cnt -= cur_cnt;
    }
}

inline void Init(Cntr const* cntr) { detail::CheckCntr_(cntr); }

inline void Deinit(Cntr* cntr) { detail::CheckCntr_(cntr); }

constexpr size_t GetCursorSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

inline size_t GetWidth(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->width;
}

inline size_t GetSride(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->stride;
}

inline size_t GetOffset(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->offset;
}

inline size_t GetSize(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->size;
}

inline size_t GetCapacity(Cntr const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->capacity;
}

inline void GetLBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->elem = nullptr;
}

inline void GetRBCursor(Cntr const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = cntr->size;
    dst_cursor->elem = nullptr;
}

inline void* PeekL(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
                   void* dst_elem) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    void* elem{ 0 < size ? ReferElem(data, stride, offset, 0, capacity)
                         : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* PeekR(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
                   void* dst_elem) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    void* elem{ 0 < size ? ReferElem(data, stride, offset, size - 1, capacity)
                         : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = size - 1;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* Access(Cntr const* cntr, size_t idx, bool lazy_copy_elem,
                    Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(idx + 1 < size + 2);

    void* elem{ idx < size ? ReferElem(data, stride, offset, idx, capacity)
                           : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, width);
    }

    return elem;
}

inline void* Derefer(Cntr const* cntr, Cursor const* pos_cursor,
                     bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    void* elem{ pos_cursor->elem };

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

namespace detail {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr* cntr, size_t idx, size_t cnt,
     ReaderWriter&&
         reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
     Cursor* dst_cursor) {
    CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsDereferable(idx, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        reader_writer(static_cast<Conditional<EnWrite, void*, void const*>>(
                          ReferElem(data, stride, offset, idx, capacity)),
                      stride, cur_cnt);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
        dst_cursor->elem = idx < size
                               ? ReferElem(data, stride, offset, idx, capacity)
                               : nullptr;
    }
}

}  // namespace detail

template <typename Reader>
void Read(Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
          Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
          Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(const_cast<Cntr*>(cntr), pos_cursor->idx, cnt,
                             reader, dst_cursor);
}

template <typename Writer>
void Write(Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
           Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
           Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void ReadWrite(
    Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    detail::ReadWrite_<true>(cntr, pos_cursor->idx, cnt, reader_writer,
                             dst_cursor);
}

template <typename ReaderWriter>
void IdxRead(
    Cntr const* cntr, size_t idx, size_t cnt,
    ReaderWriter&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::ReadWrite_<true>(const_cast<Cntr*>(cntr), idx, cnt, reader,
                             nullptr);
}

template <typename ReaderWriter>
void IdxWrite(
    Cntr* cntr, size_t idx, size_t cnt,
    ReaderWriter&& writer  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::ReadWrite_<true>(cntr, idx, cnt, writer, nullptr);
}

template <typename ReaderWriter>
void IdxReadWrite(
    Cntr* cntr, size_t idx, size_t cnt,
    ReaderWriter&&
        reader_writer  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::ReadWrite_<false>(cntr, idx, cnt, reader_writer, nullptr);
}

template <typename Writer>
void* PushL(Cntr* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsInsertable(0, cnt, size, capacity));

    cntr->offset = offset = (offset < cnt ? offset + capacity : offset) - cnt;
    cntr->size = size += cnt;

    void* elem{ 0 < size ? ReferElem(data, stride, offset, 0, capacity)
                         : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ 0 }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        writer(ReferElem(data, stride, offset, idx, capacity), stride, cur_cnt);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }

    return elem;
}

template <typename Writer>
void* PushR(Cntr* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(
        seq_cntr::ops::IsInsertable(size, cnt, size, capacity));

    cntr->size = size += cnt;

    void* elem{ 0 < cnt ? ReferElem(data, stride, offset, size - cnt, capacity)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = size - cnt;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ size - cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        cnt -= cur_cnt;

        writer(ReferElem(data, stride, offset, idx, capacity), stride, cur_cnt);

        idx += cur_cnt;
    }

    return elem;
}

template <typename Writer>
void* Insert(Cntr* cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
             Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    if (cnt == 0) { return pos_cursor->elem; }

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::ops::IsInsertable(idx, cnt, size, capacity));

    size_t l_size{ idx };
    size_t r_size{ size - idx };

    cntr->size = size += cnt;

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        cntr->offset = offset =
            (offset < cnt ? offset + capacity : offset) - cnt;
        AssignFromCircularArray(cntr, 0, cntr, cnt, l_size);
    } else {
        AssignFromCircularArray(cntr, l_size + cnt, cntr, l_size, r_size);
    }

    void* elem{ ReferElem(data, stride, offset, idx, capacity) };

    pos_cursor->elem = elem;

    Write(cntr, pos_cursor, cnt, writer, dst_cursor);

    return elem;
}

template <typename Writer>
void* IdxInsert(
    Cntr* cntr, size_t idx, size_t cnt,
    Writer&& writer  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    if (cnt == 0) { return ReferElem(data, stride, offset, idx, capacity); }

    ZETA_Core_DebugAssert(
        seq_cntr::ops::IsInsertable(idx, cnt, size, capacity));

    size_t l_size{ idx };
    size_t r_size{ size - idx };

    cntr->size = size += cnt;

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        cntr->offset = offset =
            (offset < cnt ? offset + capacity : offset) - cnt;
        AssignFromCircularArray(cntr, 0, cntr, cnt, l_size);
    } else {
        AssignFromCircularArray(cntr, l_size + cnt, cntr, l_size, r_size);
    }

    void* elem{ ReferElem(data, stride, offset, idx, capacity) };

    IdxWrite(cntr, idx, cnt, writer);

    return elem;
}

inline void PopL(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    size_t size{ cntr->size };
    ZETA_Core_DebugAssert(cnt <= size);

    size_t capacity{ cntr->capacity };

    size_t offset{ cntr->offset + cnt };

    cntr->offset = offset < capacity ? offset : offset - capacity;
    cntr->size = size -= cnt;

    if (size == 0) { cntr->offset = 0; }
}

inline void PopR(Cntr* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    size_t size{ cntr->size };
    ZETA_Core_DebugAssert(cnt <= size);

    cntr->size = size -= cnt;

    if (size == 0) { cntr->offset = 0; }
}

inline void Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt) {
    detail::CheckCursor_(cntr, pos_cursor);

    size_t idx{ pos_cursor->idx };

    IdxErase(cntr, idx, cnt);

    pos_cursor->elem = idx < cntr->size
                           ? ReferElem(cntr->data, cntr->stride, cntr->offset,
                                       idx, cntr->capacity)
                           : nullptr;
}

inline void IdxErase(Cntr* cntr, size_t idx, size_t cnt) {
    detail::CheckCntr_(cntr);

    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsErasable(idx, cnt, size));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ size - idx - cnt };

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        AssignFromCircularArray(cntr, cnt, cntr, 0, l_size);

        offset += cnt;
        cntr->offset = offset = offset < capacity ? offset : offset - capacity;
    } else {
        AssignFromCircularArray(cntr, l_size, cntr, l_size + cnt, r_size);
    }

    cntr->size = (size -= cnt);

    if (size == 0) { cntr->offset = offset = 0; }
}

inline void EraseAll(Cntr* cntr) {
    detail::CheckCntr_(cntr);

    cntr->offset = 0;
    cntr->size = 0;
}

inline void CopyCursor(Cntr const* cntr, Cursor const* src_cursor,
                       Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    dst_cursor->cntr = cntr;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->elem = src_cursor->elem;
}

inline bool AreEqualCursor(Cntr const* cntr, Cursor const* cursor_a,
                           Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_a) == GetCursorIdx(cntr, cursor_b);
}

inline int CompareCursor(Cntr const* cntr, Cursor const* cursor_a,
                         Cursor const* cursor_b) {
    return compare::Compare(GetCursorIdx(cntr, cursor_a) + 1,
                            GetCursorIdx(cntr, cursor_b) + 1);
}

inline size_t GetCursorDist(Cntr const* cntr, Cursor const* cursor_a,
                            Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

inline size_t GetCursorIdx(Cntr const* cntr, Cursor const* cursor) {
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

inline void CursorStepL(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceL(cntr, cursor, 1);
}

inline void CursorStepR(Cntr const* cntr, Cursor* cursor) {
    CursorAdvanceR(cntr, cursor, 1);
}

inline void CursorAdvanceL(Cntr const* cntr, Cursor* cursor, size_t step) {
    detail::CheckCursor_(cntr, cursor);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->elem =
        idx < size ? ReferElem(data, stride, offset, idx, capacity) : nullptr;
}

inline void CursorAdvanceR(Cntr const* cntr, Cursor* cursor, size_t step) {
    detail::CheckCursor_(cntr, cursor);

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    ZETA_Core_DebugAssert(step <= size - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->elem =
        idx < size ? ReferElem(data, stride, offset, idx, capacity) : nullptr;
}

}  // namespace ops

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView*>) {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetWidth = true,
        .GetSize = true,
        .GetCapacity = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
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

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView const*>) {
    return GetStaticEnabledAbilityFlag(
               type_wrapper::TypeWrapper<SeqCntrView*>{}) &
           seq_cntr::const_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrView const*>) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetDynamicEnabledAbilityFlag(
    SeqCntrView const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag SeqCntrView::GetDynamicDisabledAbilityFlag(
    SeqCntrView const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr size_t SeqCntrView::GetCursorSize(SeqCntrView const* seq_cntr_view) {
    return ops::GetCursorSize(
        static_cast<Cntr const*>(static_cast<void const*>(seq_cntr_view)));
}

inline size_t SeqCntrView::GetWidth(SeqCntrView const* seq_cntr_view) {
    return ops::GetWidth(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline size_t SeqCntrView::GetSize(SeqCntrView const* seq_cntr_view) {
    return ops::GetSize(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline size_t SeqCntrView::GetCapacity(SeqCntrView const* seq_cntr_view) {
    return ops::GetCapacity(reinterpret_cast<Cntr const*>(seq_cntr_view));
}

inline void SeqCntrView::GetLBCursor(SeqCntrView const* seq_cntr_view,
                                     void* dst_cursor) {
    ops::GetLBCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(dst_cursor));
}

inline void SeqCntrView::GetRBCursor(SeqCntrView const* seq_cntr_view,
                                     void* dst_cursor) {
    ops::GetRBCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(dst_cursor));
}

inline void* SeqCntrView::PeekL(SeqCntrView const* seq_cntr_view,
                                bool lazy_copy_elem, void* dst_cursor,
                                void* dst_elem) {
    return ops::PeekL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

inline void* SeqCntrView::PeekR(SeqCntrView const* seq_cntr_view,
                                bool lazy_copy_elem, void* dst_cursor,
                                void* dst_elem) {
    return ops::PeekR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

inline void* SeqCntrView::Access(SeqCntrView const* seq_cntr_view, size_t idx,
                                 bool lazy_copy_elem, void* dst_cursor,
                                 void* dst_elem) {
    return ops::Access(reinterpret_cast<Cntr const*>(seq_cntr_view), idx,
                       lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                       dst_elem);
}

inline void* SeqCntrView::Derefer(SeqCntrView const* seq_cntr_view,
                                  void const* pos_cursor, bool lazy_copy_elem,
                                  void* dst_elem) {
    return ops::Derefer(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor const*>(pos_cursor), lazy_copy_elem,
                        dst_elem);
}

template <typename Reader>
void SeqCntrView::Read(
    SeqCntrView const* seq_cntr_view, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Read(reinterpret_cast<Cntr const*>(seq_cntr_view),
              static_cast<Cursor const*>(pos_cursor), cnt,
              Forward<Reader>(reader), static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void SeqCntrView::Write(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::Write(reinterpret_cast<Cntr*>(seq_cntr_view),
               static_cast<Cursor*>(pos_cursor), cnt, Forward<Writer>(writer),
               static_cast<Cursor*>(dst_cursor));
}

template <typename ReaderWriter>
void SeqCntrView::ReadWrite(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    ops::ReadWrite(reinterpret_cast<Cntr*>(seq_cntr_view),
                   static_cast<Cursor*>(pos_cursor), cnt,
                   Forward<ReaderWriter>(reader_writer),
                   static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::PushL(
    SeqCntrView* seq_cntr_view, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushL(reinterpret_cast<Cntr*>(seq_cntr_view), cnt,
                      Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::PushR(
    SeqCntrView* seq_cntr_view, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::PushR(reinterpret_cast<Cntr*>(seq_cntr_view), cnt,
                      Forward<Writer>(writer),
                      static_cast<Cursor*>(dst_cursor));
}

template <typename Writer>
void* SeqCntrView::Insert(
    SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return ops::Insert(reinterpret_cast<Cntr*>(seq_cntr_view),
                       static_cast<Cursor*>(pos_cursor), cnt,
                       Forward<Writer>(writer),
                       static_cast<Cursor*>(dst_cursor));
}

inline void SeqCntrView::PopL(SeqCntrView* seq_cntr_view, size_t cnt) {
    ops::PopL(reinterpret_cast<Cntr*>(seq_cntr_view), cnt);
}

inline void SeqCntrView::PopR(SeqCntrView* seq_cntr_view, size_t cnt) {
    ops::PopR(reinterpret_cast<Cntr*>(seq_cntr_view), cnt);
}

inline void SeqCntrView::Erase(SeqCntrView* seq_cntr_view, void* pos_cursor,
                               size_t cnt) {
    ops::Erase(reinterpret_cast<Cntr*>(seq_cntr_view),
               static_cast<Cursor*>(pos_cursor), cnt);
}

inline void SeqCntrView::EraseAll(SeqCntrView* seq_cntr_view) {
    ops::EraseAll(reinterpret_cast<Cntr*>(seq_cntr_view));
}

inline void SeqCntrView::CopyCursor(SeqCntrView const* seq_cntr_view,
                                    void const* src_cursor, void* dst_cursor) {
    ops::CopyCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                    static_cast<Cursor const*>(src_cursor),
                    static_cast<Cursor*>(dst_cursor));
}

inline bool SeqCntrView::AreEqualCursor(SeqCntrView const* seq_cntr_view,
                                        void const* cursor_a,
                                        void const* cursor_b) {
    return ops::AreEqualCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                               static_cast<Cursor const*>(cursor_a),
                               static_cast<Cursor const*>(cursor_b));
}

inline int SeqCntrView::CompareCursor(SeqCntrView const* seq_cntr_view,
                                      void const* cursor_a,
                                      void const* cursor_b) {
    return ops::CompareCursor(reinterpret_cast<Cntr const*>(seq_cntr_view),
                              static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

inline size_t SeqCntrView::GetCursorDist(SeqCntrView const* seq_cntr_view,
                                         void const* cursor_a,
                                         void const* cursor_b) {
    return ops::GetCursorDist(reinterpret_cast<Cntr const*>(seq_cntr_view),
                              static_cast<Cursor const*>(cursor_a),
                              static_cast<Cursor const*>(cursor_b));
}

inline size_t SeqCntrView::GetCursorIdx(SeqCntrView const* seq_cntr_view,
                                        void const* cursor) {
    return ops::GetCursorIdx(reinterpret_cast<Cntr const*>(seq_cntr_view),
                             static_cast<Cursor const*>(cursor));
}

inline void SeqCntrView::CursorStepL(SeqCntrView const* seq_cntr_view,
                                     void* cursor) {
    ops::CursorStepL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(cursor));
}

inline void SeqCntrView::CursorStepR(SeqCntrView const* seq_cntr_view,
                                     void* cursor) {
    ops::CursorStepR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                     static_cast<Cursor*>(cursor));
}

inline void SeqCntrView::CursorAdvanceL(SeqCntrView const* seq_cntr_view,
                                        void* cursor, size_t step) {
    ops::CursorAdvanceL(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor*>(cursor), step);
}

inline void SeqCntrView::CursorAdvanceR(SeqCntrView const* seq_cntr_view,
                                        void* cursor, size_t step) {
    ops::CursorAdvanceR(reinterpret_cast<Cntr const*>(seq_cntr_view),
                        static_cast<Cursor*>(cursor), step);
}

namespace ops {

template <typename SrcSeqCntr>
void AssignFromSeqCntr(Cntr* cntr, size_t dst_beg,
                       SrcSeqCntr const* src_seq_cntr,
                       void* src_seq_cntr_cursor, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(src_seq_cntr_cursor != nullptr);

    if (src_seq_cntr->vtable == &seq_cntr::GetVTable<SeqCntrView>()) {
        auto const* src_ca_cursor{ static_cast<Cursor const*>(
            src_seq_cntr_cursor) };

        auto const* src_ca{ static_cast<Cntr const*>(src_seq_cntr->inst) };

        detail::CheckCursor_(src_ca, src_ca_cursor);

        AssignFromCircularArray(cntr, dst_beg, src_ca, src_ca_cursor->idx, cnt);

        return;
    }

    void* data{ cntr->data };
    size_t stride{ cntr->stride };
    size_t offset{ cntr->offset };
    size_t size{ cntr->size };
    size_t capacity{ cntr->capacity };

    size_t idx{ dst_beg };

    ZETA_Core_DebugAssert(seq_cntr::ops::IsReferable(dst_beg, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        SrcSeqCntr::Read(src_seq_cntr, src_seq_cntr_cursor, cur_cnt,
                         ReferElem(data, stride, offset, idx, capacity), stride,
                         src_seq_cntr_cursor);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

}  // namespace ops

}  // namespace zeta::core::circular_array
