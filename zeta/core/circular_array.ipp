#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CheckCntr_")
#pragma push_macro("CheckCursor_")

#if ZETA_Core_EnableDebug

#define CheckCntr_(ca) ZETA_Core_DebugAssert(CheckCntr(ca))

#define CheckCursor_(ca, cursor) ZETA_Core_DebugAssert(CheckCursor(ca, cursor))

#else

#define CheckCntr_(ca) ZETA_Core_Unused(ca)

#define CheckCursor_(ca, cursor)  \
    {                             \
        ZETA_Core_Unused(ca);     \
        ZETA_Core_Unused(cursor); \
    }                             \
    ZETA_Core_StaticAssert(true)

#endif

namespace zeta::core {

inline void* CircularArray::ReferElem(void* data, size_t stride, size_t offset,
                                      size_t idx, size_t capacity) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < stride);
    ZETA_Core_DebugAssert(offset < capacity);
    ZETA_Core_DebugAssert(idx < capacity);
    ZETA_Core_DebugAssert(capacity <= ZETA_Core_max_capacity);

    size_t k{ offset + idx };

    return static_cast<char*>(data) +
           stride * (k < capacity ? k : k - capacity);
}

inline size_t CircularArray::GetLongestContPred(size_t offset, size_t idx,
                                                size_t size, size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return idx <= k ? idx : idx - k;
}

inline size_t CircularArray::GetLongestContSucr(size_t offset, size_t idx,
                                                size_t size, size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return (size <= k || k <= idx) ? size - idx : k - idx;
}

inline void CircularArray::AssignFromSeqCntr(void* ca_, size_t dst_beg,
                                             SeqCntr const* src_seq_cntr,
                                             void* src_seq_cntr_cursor,
                                             size_t cnt) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    ZETA_Core_DebugAssert(src_seq_cntr_cursor != nullptr);

    if (src_seq_cntr->vtable == &SeqCntr::MakeVTable<CircularArray>()) {
        auto src_ca_cursor{ static_cast<Cursor const*>(src_seq_cntr_cursor) };

        auto src_ca{ static_cast<CircularArray const*>(
            src_seq_cntr->inst_const) };

        CheckCntr_(src_ca);
        CheckCursor_(src_ca, src_ca_cursor);

        AssignFromCircularArray(ca, dst_beg, src_ca, src_ca_cursor->idx, cnt);

        return;
    }

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    size_t idx{ dst_beg };

    ZETA_Core_DebugAssert(SeqCntr::IsReferable(dst_beg, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        cnt -= cur_cnt;

        SeqCntr::MemRead(src_seq_cntr, src_seq_cntr_cursor, cur_cnt,
                         ReferElem(data, stride, offset, idx, capacity), stride,
                         src_seq_cntr_cursor);

        idx += cur_cnt;
    }
}

inline void CircularArray::AssignFromCircularArray(void* dst_ca_,
                                                   size_t dst_beg,
                                                   void const* src_ca_,
                                                   size_t src_beg, size_t cnt) {
    auto dst_ca{ static_cast<CircularArray*>(dst_ca_) };
    CheckCntr_(dst_ca);

    auto src_ca{ static_cast<CircularArray const*>(src_ca_) };
    CheckCntr_(src_ca);

    char* dst_data{ static_cast<char*>(dst_ca->data) };
    size_t dst_width{ dst_ca->width };
    size_t dst_stride{ dst_ca->stride };
    size_t dst_offset{ dst_ca->offset };
    size_t dst_size{ dst_ca->size };
    size_t dst_capacity{ dst_ca->capacity };

    char* src_data{ static_cast<char*>(src_ca->data) };
    size_t src_width{ src_ca->width };
    size_t src_stride{ src_ca->stride };
    size_t src_offset{ src_ca->offset };
    size_t src_size{ src_ca->size };
    size_t src_capacity{ src_ca->capacity };

    ZETA_Core_DebugAssert(SeqCntr::IsDereferable(dst_beg, cnt, dst_size));

    ZETA_Core_DebugAssert(SeqCntr::IsDereferable(src_beg, cnt, src_size));

    if (cnt == 0) { return; }

    size_t width{ Min(dst_width, src_width) };

    if (dst_ca != src_ca) {
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

        cnt -= cur_cnt;

        ElemMove(
            ReferElem(dst_data, dst_stride, dst_offset, dst_beg, dst_capacity),
            ReferElem(src_data, src_stride, src_offset, src_beg, src_capacity),
            width, dst_stride, src_stride, cur_cnt);

        dst_beg += cur_cnt;
        src_beg += cur_cnt;
    }

    return;

VEC_BW_MOVE:

    for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(
            cnt,
            GetLongestContPred(dst_offset, dst_end, dst_size, dst_capacity),
            GetLongestContPred(src_offset, src_end, src_size, src_capacity)) };

        cnt -= cur_cnt;

        dst_end -= cur_cnt;
        src_end -= cur_cnt;

        ElemMove(
            ReferElem(dst_data, dst_stride, dst_offset, dst_end, dst_capacity),
            ReferElem(src_data, src_stride, src_offset, src_end, src_capacity),
            width, dst_stride, src_stride, cur_cnt);
    }

    return;
}

template <bool EnWrite, typename ReaderWriter>
void CircularArray::CoreReadWrite(void* ca_, size_t idx, size_t cnt,
                                  ReaderWriter&& reader_writer,
                                  void* dst_cursor_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(SeqCntr::IsDereferable(idx, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        cnt -= cur_cnt;

        reader_writer(ReferElem(data, stride, offset, idx, capacity), stride,
                      cur_cnt);

        idx += cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = idx;
        dst_cursor->ref = idx < size
                              ? ReferElem(data, stride, offset, idx, capacity)
                              : nullptr;
    }
}

// -----------------------------------------------------------------------------

inline void CircularArray::Init(void const* ca) { CheckCntr_(ca); }

inline void CircularArray::Deinit(void* ca_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);
}

inline size_t CircularArray::GetWidth(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return ca->width;
}

inline size_t CircularArray::GetSride(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return ca->stride;
}

inline size_t CircularArray::GetOffset(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return ca->offset;
}

inline size_t CircularArray::GetSize(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return ca->size;
}

inline size_t CircularArray::GetCapacity(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return ca->capacity;
}

inline void CircularArray::GetLBCursor(void const* ca_, void* dst_cursor_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    Cursor* dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->ca = ca;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->ref = nullptr;
}

inline void CircularArray::GetRBCursor(void const* ca_, void* dst_cursor_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    Cursor* dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->ca = ca;
    dst_cursor->idx = ca->size;
    dst_cursor->ref = nullptr;
}

inline void* CircularArray::PeekL(void* ca_, void* dst_cursor_,
                                  void* dst_elem) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t width{ ca->width };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    Cursor* dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    void* ref{ 0 < size ? ReferElem(data, stride, offset, 0, capacity)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = 0;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }

    return ref;
}

inline void const* CircularArray::ConstPeekL(void const* ca, void* dst_cursor,
                                             void* dst_elem) {
    return PeekL(const_cast<void*>(ca), dst_cursor, dst_elem);
}

inline void* CircularArray::PeekR(void* ca_, void* dst_cursor_,
                                  void* dst_elem) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t width{ ca->width };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    void* ref{ 0 < size ? ReferElem(data, stride, offset, size - 1, capacity)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = size - 1;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }

    return ref;
}

inline void const* CircularArray::ConstPeekR(void const* ca, void* dst_cursor,
                                             void* dst_elem) {
    return PeekR(const_cast<void*>(ca), dst_cursor, dst_elem);
}

inline void* CircularArray::Access(void* ca_, size_t idx, void* dst_cursor_,
                                   void* dst_elem) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t width{ ca->width };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    Cursor* dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(idx + 1 < size + 2);

    void* ref{ idx < size ? ReferElem(data, stride, offset, idx, capacity)
                          : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = idx;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }

    return ref;
}

inline void const* CircularArray::ConstAccess(void const* ca, size_t idx,
                                              void* dst_cursor,
                                              void* dst_elem) {
    return Access(const_cast<void*>(ca), idx, dst_cursor, dst_elem);
}

inline void* CircularArray::Refer(void* ca_, void const* pos_cursor_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    return pos_cursor->ref;
}

inline void const* CircularArray::ConstRefer(void const* ca,
                                             void const* pos_cursor) {
    return Refer(const_cast<void*>(ca), pos_cursor);
}

// -----------------------------------------------------------------------------

template <typename Reader>
void CircularArray::TplRead(void const* ca_, void const* pos_cursor_,
                            size_t cnt, Reader&& reader, void* dst_cursor) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    CoreReadWrite<false>(const_cast<CircularArray*>(ca), pos_cursor->idx, cnt,
                         reader, dst_cursor);
}

template <typename Writer>
void CircularArray::TplWrite(void* ca_, void const* pos_cursor_, size_t cnt,
                             Writer&& writer, void* dst_cursor) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    CoreReadWrite<true>(ca, pos_cursor->idx, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void CircularArray::TplReadWrite(void* ca_, void const* pos_cursor_, size_t cnt,
                                 ReaderWriter&& reader_writer,
                                 void* dst_cursor) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    CoreReadWrite<true>(ca, pos_cursor->idx, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void CircularArray::MemRead(void const* ca_, void const* pos_cursor,
                                   size_t cnt, void* dst, size_t dst_stride,
                                   void* dst_cursor) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    ZETA_Core_DebugAssert(dst != nullptr);

    size_t width{ ca->width };

    TplRead(
        ca, pos_cursor, cnt,
        [=](void const* src, size_t src_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            dst = static_cast<char*>(dst) + dst_stride * cnt;
        },
        dst_cursor);
}

inline void CircularArray::MemWrite(void* ca_, void* pos_cursor, size_t cnt,
                                    void const* src, size_t src_stride,
                                    void* dst_cursor) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    ZETA_Core_DebugAssert(src != nullptr);

    size_t width{ ca->width };

    TplWrite(
        ca_, pos_cursor, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, 1);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

// -----------------------------------------------------------------------------

inline void CircularArray::FnRead(void const* ca, void const* pos_cursor,
                                  size_t cnt, SeqCntr::FnReader reader,
                                  void* dst_cursor) {
    TplRead(ca, pos_cursor, cnt, reader, dst_cursor);
}

inline void CircularArray::FnWrite(void* ca, void* pos_cursor, size_t cnt,
                                   SeqCntr::FnWriter writer, void* dst_cursor) {
    TplWrite(ca, pos_cursor, cnt, writer, dst_cursor);
}

inline void CircularArray::FnReadWrite(void* ca, void const* pos_cursor,
                                       size_t cnt,
                                       SeqCntr::FnReaderWriter reader_writer,
                                       void* dst_cursor) {
    TplReadWrite(ca, pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename ReaderWriter>
void CircularArray::IdxRead(void const* ca, size_t idx, size_t cnt,
                            ReaderWriter&& reader_writer) {
    CoreReadWrite<false>(const_cast<void*>(ca), idx, cnt, reader_writer,
                         nullptr);
}

template <typename ReaderWriter>
void CircularArray::IdxWrite(void* ca, size_t idx, size_t cnt,
                             ReaderWriter&& reader_writer) {
    CoreReadWrite<true>(ca, idx, cnt, reader_writer, nullptr);
}

template <typename ReaderWriter>
void CircularArray::IdxReadWrite(void* ca, size_t idx, size_t cnt,
                                 ReaderWriter&& reader_writer) {
    CoreReadWrite<true>(ca, idx, cnt, reader_writer, nullptr);
}

// -----------------------------------------------------------------------------

template <typename Writer>
void* CircularArray::TplPushL(void* ca_, size_t cnt, Writer&& writer,
                              void* dst_cursor_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(SeqCntr::IsInsertable(0, cnt, size, capacity));

    ca->offset = offset = (offset < cnt ? offset + capacity : offset) - cnt;
    ca->size = size += cnt;

    void* ref{ 0 < size ? ReferElem(data, stride, offset, 0, capacity)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = 0;
        dst_cursor->ref = ref;
    }

    for (size_t idx{ 0 }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        cnt -= cur_cnt;

        writer(ReferElem(data, stride, offset, idx, capacity), stride, cur_cnt);

        idx += cur_cnt;
    }

    return ref;
}

template <typename Writer>
void* CircularArray::TplPushR(void* ca_, size_t cnt, Writer&& writer,
                              void* dst_cursor_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(SeqCntr::IsInsertable(size, cnt, size, capacity));

    ca->size = size += cnt;

    void* ref{ 0 < cnt ? ReferElem(data, stride, offset, size - cnt, capacity)
                       : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = ca;
        dst_cursor->idx = size - cnt;
        dst_cursor->ref = ref;
    }

    for (size_t idx{ size - cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr(offset, idx, size, capacity)) };

        cnt -= cur_cnt;

        writer(ReferElem(data, stride, offset, idx, capacity), stride, cur_cnt);

        idx += cur_cnt;
    }

    return ref;
}

template <typename Writer>
void* CircularArray::TplInsert(void* ca_, void* pos_cursor_, size_t cnt,
                               Writer&& writer, void* dst_cursor) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto pos_cursor{ static_cast<Cursor*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    if (cnt == 0) { return pos_cursor->ref; }

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(SeqCntr::IsInsertable(idx, cnt, size, capacity));

    size_t l_size{ idx };
    size_t r_size{ size - idx };

    ca->size = size += cnt;

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        ca->offset = offset = (offset < cnt ? offset + capacity : offset) - cnt;
        AssignFromCircularArray(ca, 0, ca, cnt, l_size);
    } else {
        AssignFromCircularArray(ca, l_size + cnt, ca, l_size, r_size);
    }

    void* ref{ ReferElem(data, stride, offset, idx, capacity) };

    pos_cursor->ref = ref;

    TplWrite(ca, pos_cursor, cnt, writer, dst_cursor);

    return ref;
}

template <typename Writer>
void* CircularArray::IdxInsert(void* ca_, size_t idx, size_t cnt,
                               Writer&& writer) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    if (cnt == 0) { return ReferElem(data, stride, offset, idx, capacity); }

    ZETA_Core_DebugAssert(SeqCntr::IsInsertable(idx, cnt, size, capacity));

    size_t l_size{ idx };
    size_t r_size{ size - idx };

    ca->size = size += cnt;

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        ca->offset = offset = (offset < cnt ? offset + capacity : offset) - cnt;
        AssignFromCircularArray(ca, 0, ca, cnt, l_size);
    } else {
        AssignFromCircularArray(ca, l_size + cnt, ca, l_size, r_size);
    }

    void* ref{ ReferElem(data, stride, offset, idx, capacity) };

    IdxWrite(ca, idx, cnt, writer);

    return ref;
}

// -----------------------------------------------------------------------------

inline void* CircularArray::FnPushL(void* ca, size_t cnt,
                                    SeqCntr::FnWriter writer,
                                    void* dst_cursor) {
    return TplPushL(ca, cnt, writer, dst_cursor);
}

inline void* CircularArray::FnPushR(void* ca, size_t cnt,
                                    SeqCntr::FnWriter writer,
                                    void* dst_cursor) {
    return TplPushR(ca, cnt, writer, dst_cursor);
}

inline void* CircularArray::FnInsert(void* ca, void* pos_cursor, size_t cnt,
                                     SeqCntr::FnWriter writer,
                                     void* dst_cursor) {
    return TplInsert(ca, pos_cursor, cnt, writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* CircularArray::MemPushL(void* ca_, size_t cnt, void const* src,
                                     size_t src_stride, void* dst_cursor) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    size_t width{ ca->width };

    if (src == nullptr) {
        return TplPushL(ca_, cnt, [=](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplPushL(
        ca_, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

inline void* CircularArray::MemPushR(void* ca_, size_t cnt, void const* src,
                                     size_t src_stride, void* dst_cursor) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    size_t width{ ca->width };

    if (src == nullptr) {
        return TplPushR(ca_, cnt, [=](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplPushR(
        ca_, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

inline void* CircularArray::MemInsert(void* ca_, void* pos_cursor, size_t cnt,
                                      void const* src, size_t src_stride,
                                      void* dst_cursor) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    size_t width{ ca->width };

    if (src == nullptr) {
        return TplInsert(
            ca_, pos_cursor, cnt, [=](void*, size_t, size_t) {}, dst_cursor);
    }

    return TplInsert(
        ca_, pos_cursor, cnt,
        [=](void* dst, size_t dst_stride, size_t cnt) mutable {
            ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
            src = static_cast<char const*>(src) + src_stride * cnt;
        },
        dst_cursor);
}

// -----------------------------------------------------------------------------

inline void CircularArray::PopL(void* ca_, size_t cnt) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    size_t size{ ca->size };
    ZETA_Core_DebugAssert(cnt <= size);

    size_t capacity{ ca->capacity };

    size_t offset{ ca->offset + cnt };

    ca->offset = offset < capacity ? offset : offset - capacity;
    ca->size = size -= cnt;

    if (size == 0) { ca->offset = 0; }
}

inline void CircularArray::PopR(void* ca_, size_t cnt) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    size_t size{ ca->size };
    ZETA_Core_DebugAssert(cnt <= size);

    ca->size = size -= cnt;

    if (size == 0) { ca->offset = 0; }
}

inline void CircularArray::Erase(void* ca_, void* pos_cursor_, size_t cnt) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    auto pos_cursor{ static_cast<Cursor*>(pos_cursor_) };
    CheckCursor_(ca, pos_cursor);

    size_t idx{ pos_cursor->idx };

    IdxErase(ca, idx, cnt);

    pos_cursor->ref = idx < ca->size ? ReferElem(ca->data, ca->stride,
                                                 ca->offset, idx, ca->capacity)
                                     : nullptr;
}

inline void CircularArray::IdxErase(void* ca_, size_t idx, size_t cnt) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    ZETA_Core_DebugAssert(SeqCntr::IsErasable(idx, cnt, size));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ size - idx - cnt };

    unsigned long long random_seed{ GetRandom() };

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        AssignFromCircularArray(ca, cnt, ca, 0, l_size);

        offset += cnt;
        ca->offset = offset = offset < capacity ? offset : offset - capacity;
    } else {
        AssignFromCircularArray(ca, l_size, ca, l_size + cnt, r_size);
    }

    ca->size = (size -= cnt);

    if (size == 0) { ca->offset = offset = 0; }
}

inline void CircularArray::EraseAll(void* ca_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    ca->offset = 0;
    ca->size = 0;
}

// -----------------------------------------------------------------------------

inline void CircularArray::CopyCursor(void const* ca_, void* dst_cursor_,
                                      void const* src_cursor_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };
    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    auto src_cursor{ static_cast<Cursor const*>(src_cursor_) };
    CheckCursor_(ca, src_cursor);

    dst_cursor->ca = ca;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->ref = src_cursor->ref;
}

inline bool CircularArray::AreEqualCursor(void const* ca, void const* cursor_a,
                                          void const* cursor_b) {
    return GetCursorIdx(ca, cursor_a) == GetCursorIdx(ca, cursor_b);
}

inline int CircularArray::CompareCursor(void const* ca, void const* cursor_a,
                                        void const* cursor_b) {
    return ThreeWayCompare(GetCursorIdx(ca, cursor_a) + 1,
                           GetCursorIdx(ca, cursor_b) + 1);
}

inline size_t CircularArray::GetCursorDist(void const* ca, void const* cursor_a,
                                           void const* cursor_b) {
    return GetCursorIdx(ca, cursor_b) - GetCursorIdx(ca, cursor_a);
}

inline size_t CircularArray::GetCursorIdx(void const* ca_,
                                          void const* cursor_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    auto cursor{ static_cast<Cursor const*>(cursor_) };
    CheckCursor_(ca, cursor);

    return cursor->idx;
}

inline void CircularArray::CursorStepL(void const* ca, void* cursor) {
    CursorAdvanceL(ca, cursor, 1);
}

inline void CircularArray::CursorStepR(void const* ca, void* cursor) {
    CursorAdvanceR(ca, cursor, 1);
}

inline void CircularArray::CursorAdvanceL(void const* ca_, void* cursor_,
                                          size_t step) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto cursor{ static_cast<Cursor*>(cursor_) };
    CheckCursor(ca, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->ref =
        idx < size ? ReferElem(data, stride, offset, idx, capacity) : nullptr;
}

inline void CircularArray::CursorAdvanceR(void const* ca_, void* cursor_,
                                          size_t step) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    auto cursor{ static_cast<Cursor*>(cursor_) };
    CheckCursor(ca, cursor);

    ZETA_Core_DebugAssert(step <= size - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->ref =
        idx < size ? ReferElem(data, stride, offset, idx, capacity) : nullptr;
}

// -----------------------------------------------------------------------------

inline bool CircularArray::CheckCntr(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    if (!(ca != nullptr)) { return false; }

    void* data{ ca->data };
    size_t width{ ca->width };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    if (!(0 < width)) { return false; }
    if (!(width <= stride)) { return false; }
    if (!(offset == 0 || offset < capacity)) { return false; }
    if (!(size <= capacity)) { return false; }
    if (!(capacity <= ZETA_Core_max_capacity)) { return false; }

    if (data == nullptr && !(capacity == 0)) { return false; }

    return true;
}

inline bool CircularArray::CheckCursor(void const* ca_, void const* cursor_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    if (!CheckCntr(ca)) { return false; }

    auto cursor{ static_cast<Cursor const*>(cursor_) };
    if (!(cursor != nullptr)) { return false; }

    if (!(ca == cursor->ca)) { return false; }

    void* data{ ca->data };
    size_t stride{ ca->stride };
    size_t offset{ ca->offset };
    size_t size{ ca->size };
    size_t capacity{ ca->capacity };

    if (!(SeqCntr::IsReferable(cursor->idx, 1, size))) { return false; }

    if (cursor->idx < size &&
        !(cursor->ref == CircularArray::ReferElem(data, stride, offset,
                                                  cursor->idx, capacity))) {
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------

inline SeqCntr CircularArray::ToSeqCntr(void* ca_) {
    auto ca{ static_cast<CircularArray*>(ca_) };
    CheckCntr_(ca);

    return {
        .inst = ca,
        .inst_const = ca,

        .cursor_size = sizeof(Cursor),
        .width = ca->width,
        .capacity = ca->capacity,

        .vtable = &SeqCntr::MakeVTable<CircularArray>(),
    };
}

inline SeqCntr CircularArray::ToSeqCntr(void const* ca_) {
    auto ca{ static_cast<CircularArray const*>(ca_) };
    CheckCntr_(ca);

    return {
        .inst = nullptr,
        .inst_const = ca,

        .cursor_size = sizeof(Cursor),
        .width = ca->width,
        .capacity = ca->capacity,

        .vtable = &SeqCntr::MakeVTable<CircularArray>(),
    };
}

}  // namespace zeta::core

#pragma pop_macro("CheckCntr_")
#pragma pop_macro("CheckCursor_")
