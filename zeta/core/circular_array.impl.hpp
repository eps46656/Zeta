#include <zeta/core/define.h>

#include <zeta/core/circular_array.decl.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

inline void CircularArray::Init() { this->Check(); }

inline void CircularArray::Deinit() {}

inline size_t CircularArray::GetWidth() const {
    this->Check();

    return this->width;
}

inline size_t CircularArray::GetSride() const {
    this->Check();

    return this->stride;
}

inline size_t CircularArray::GetOffset() const {
    this->Check();

    return this->offset;
}

inline size_t CircularArray::GetSize() const {
    this->Check();

    return this->size;
}

inline size_t CircularArray::GetCapacity() const {
    this->Check();

    return this->capacity;
}

inline void CircularArray::GetLBCursor(void* dst_cursor_) const {
    this->Check();

    CircularArray_Cursor* dst_cursor{ static_cast<CircularArray_Cursor*>(
        dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->ca = const_cast<CircularArray*>(this);
    dst_cursor->idx = -1;
    dst_cursor->ref = nullptr;
}

inline void CircularArray::GetRBCursor(void* dst_cursor_) const {
    this->Check();

    CircularArray_Cursor* dst_cursor{ static_cast<CircularArray_Cursor*>(
        dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->ca = this;
    dst_cursor->idx = this->size;
    dst_cursor->ref = nullptr;
}

inline void* CircularArray::PeekL(void* dst_cursor_, void* dst_elem) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    CircularArray_Cursor* dst_cursor{ static_cast<CircularArray_Cursor*>(
        dst_cursor_) };

    void* ref{ 0 < size ? Refer_(data, stride, offset, 0, capacity) : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = 0;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }
}

inline void const* CircularArray::PeekL(void* dst_cursor,
                                        void* dst_elem) const {
    return const_cast<CircularArray*>(this)->PeekL(dst_cursor, dst_elem);
}

inline void* CircularArray::PeekR(void* dst_cursor_, void* dst_elem) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto dst_cursor{ static_cast<CircularArray_Cursor*>(dst_cursor_) };

    void* ref{ 0 < size ? Refer_(this->data, this->stride, this->offset,
                                 this->size - 1, this->capacity)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = this->size - 1;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }

    return ref;
}

inline void const* CircularArray::PeekR(void* dst_cursor,
                                        void* dst_elem) const {
    return const_cast<CircularArray*>(this)->PeekR(dst_cursor, dst_elem);
}

inline void* CircularArray::Access(size_t idx, void* dst_cursor_,
                                   void* dst_elem) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    CircularArray_Cursor* dst_cursor{ static_cast<CircularArray_Cursor*>(
        dst_cursor_) };

    ZETA_Core_DebugAssert(idx + 1 < size + 2);

    void* ref{ idx < this->size ? Refer_(data, stride, offset, idx, capacity)
                                : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = idx;
        dst_cursor->ref = ref;
    }

    if (ref != nullptr && dst_elem != nullptr) {
        MemCopy(dst_elem, ref, width);
    }

    return ref;
}

inline void const* CircularArray::Access(size_t idx, void* dst_cursor,
                                         void* dst_elem) const {
    return const_cast<CircularArray*>(this)->Access(idx, dst_cursor, dst_elem);
}

inline void* CircularArray::Refer(void const* pos_cursor_) {
    this->Check();

    auto pos_cursor{ static_cast<CircularArray_Cursor const*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    return pos_cursor->ref;
}

inline void const* CircularArray::Refer(void const* pos_cursor_) const {
    return const_cast<CircularArray*>(this)->Refer(pos_cursor_);
}

// -----------------------------------------------------------------------------

template <typename Reader>
void CircularArray::Read(void const* pos_cursor, size_t cnt,
                         const Reader& reader, void* dst_cursor) const {
    const_cast<CircularArray*>(this)->ReadWriteTemp_<false>(pos_cursor, cnt,
                                                            reader, dst_cursor);
}

template <typename Writer>
void CircularArray::Write(void* pos_cursor, size_t cnt, const Writer& writer,
                          void* dst_cursor) {
    this->ReadWriteTemp_<true>(pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void CircularArray::ReadWrite(void* pos_cursor, size_t cnt,
                              const ReaderWriter& reader_writer,
                              void* dst_cursor) {
    this->ReadWriteTemp_<true>(pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void CircularArray::Read(void const* pos_cursor, size_t cnt, void* dst,
                                size_t dst_stride, void* dst_cursor) const {
    ZETA_Core_DebugAssert(dst != nullptr);

    size_t width{ this->width };

    this->Read(
        pos_cursor, cnt,
        [=](void* src) mutable {
            MemCopy(dst, src, width);
            dst = static_cast<char*>(dst) + dst_stride;
        },
        dst_cursor);
}

inline void CircularArray::Write(void* pos_cursor, size_t cnt, void const* src,
                                 size_t src_stride, void* dst_cursor) {
    ZETA_Core_DebugAssert(src != nullptr);

    size_t width{ this->width };

    this->Write(
        pos_cursor, cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename Writer>
void* CircularArray::PushL(size_t cnt, const Writer& writer,
                           void* dst_cursor_) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto dst_cursor{ static_cast<CircularArray_Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(0, cnt, size, capacity));

    this->offset = offset = (offset + capacity - cnt) % capacity;
    this->size = size += cnt;

    void* ref{ cnt == 0 ? nullptr : Refer_(data, stride, offset, 0, capacity) };

    for (size_t idx{ 0 }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr_(offset, idx, size, capacity)) };
        cnt -= cur_cnt;

        auto cur_data{ static_cast<char*>(
            Refer_(data, stride, offset, idx, capacity)) };

        for (size_t i{ 0 }; i < cur_cnt; ++i) { writer(cur_data + stride * i); }

        idx += cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = 0;
        dst_cursor->ref = ref;
    }

    return ref;
}

template <typename Writer>
void* CircularArray::PushR(size_t cnt, const Writer& writer,
                           void* dst_cursor_) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto dst_cursor{ static_cast<CircularArray_Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(size, cnt, size, capacity));

    this->size = size += cnt;

    void* ref{ cnt == 0 ? nullptr
                        : Refer_(data, stride, offset, size - cnt, capacity) };

    for (size_t idx{ size - cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr_(offset, idx, size, capacity)) };
        cnt -= cur_cnt;

        auto cur_data{ static_cast<char*>(
            Refer_(data, stride, offset, idx, capacity)) };

        for (size_t i{ 0 }; i < cur_cnt; ++i) { writer(cur_data + stride * i); }

        idx += cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = size;
        dst_cursor->ref = ref;
    }

    return ref;
}

template <typename Writer>
void* CircularArray::Insert(void const* pos_cursor_, size_t cnt,
                            const Writer& writer, void* dst_cursor_) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto pos_cursor{ static_cast<CircularArray_Cursor*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(idx, cnt, size, capacity));

    size_t l_size{ idx };
    size_t r_size{ size - idx };

    this->size = size += cnt;

    unsigned long long random_seed{ GetRandom() };

    if (Zeta_Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        this->offset = offset = (offset + capacity - cnt) % capacity;
        this->Assign(this, 0, cnt, l_size);
    } else {
        this->Assign(this, l_size + cnt, l_size, r_size);
    }

    pos_cursor->ref = Refer_(data, stride, offset, idx, capacity);

    return pos_cursor->ref;
}

// -------------------------------------------------------------------------

inline void* CircularArray::PushL(size_t cnt, void const* src,
                                  size_t src_stride, void* dst_cursor) {
    size_t width{ this->width };

    if (src == nullptr) {
        return this->PushL(cnt, [=](void*) {}, dst_cursor);
    }

    return this->PushL(
        cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

inline void* CircularArray::PushR(size_t cnt, void const* src,
                                  size_t src_stride, void* dst_cursor) {
    size_t width{ this->width };

    if (src == nullptr) {
        return this->PushR(cnt, [=](void*) {}, dst_cursor);
    }

    return this->PushR(
        cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

inline void* CircularArray::Insert(void const* pos_cursor, size_t cnt,
                                   void const* src, size_t src_stride,
                                   void* dst_cursor) {
    size_t width{ this->width };

    if (src == nullptr) {
        return this->Insert(pos_cursor, cnt, [=](void*) {}, dst_cursor);
    }

    return this->Insert(
        pos_cursor, cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

// -----------------------------------------------------------------------------

inline void CircularArray::PopL(size_t cnt) {
    this->Check();

    size_t size{ this->size };
    ZETA_Core_DebugAssert(cnt <= size);

    this->offset = (this->offset + cnt) % this->capacity;
    this->size = size -= cnt;

    if (size == 0) { this->offset = 0; }
}

inline void CircularArray::PopR(size_t cnt) {
    this->Check();

    size_t size{ this->size };
    ZETA_Core_DebugAssert(cnt <= size);

    this->size = size -= cnt;

    if (size == 0) { this->offset = 0; }
}

inline void CircularArray::Erase(void* pos_cursor_, size_t cnt) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto pos_cursor{ static_cast<CircularArray_Cursor*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    size_t idx = pos_cursor->idx;

    ZETA_Core_DebugAssert(seq_cntr::IsErasable(idx, cnt, size));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ size - idx - cnt };

    unsigned long long random_seed = GetRandom();

    if (Choose2(l_size <= r_size, r_size <= l_size, &random_seed)) {
        this->AssignFromCircularArray_(l_size, this, 0, cnt);
        this->offset = offset = (offset + cnt) % capacity;
    } else {
        this->AssignFromCircularArray_(l_size, this, l_size + cnt, r_size);
    }

    this->size = (size -= cnt);

    if (size == 0) { this->offset = offset = 0; }

    pos_cursor->ref =
        idx < size ? Refer_(data, stride, offset, idx, capacity) : nullptr;
}

inline void CircularArray::EraseAll() {
    this->Check();

    this->offset = 0;
    this->size = 0;
}

// -----------------------------------------------------------------------------

inline void CircularArray::Cursor_Copy(void* dst_cursor_,
                                       void const* src_cursor_) const {
    this->Check();

    auto dst_cursor{ static_cast<CircularArray_Cursor*>(dst_cursor_) };
    ZETA_Core_DebugAssert(dst_cursor != nullptr);

    auto src_cursor{ static_cast<CircularArray_Cursor const*>(src_cursor_) };
    this->CheckCursor(src_cursor);

    dst_cursor->ca = this;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->ref = src_cursor->ref;
}

inline bool CircularArray::Cursor_AreEqual(void const* cursor_a,
                                           void const* cursor_b) const {
    return this->Cursor_GetIdx(cursor_a) == this->Cursor_GetIdx(cursor_b);
}

inline int CircularArray::Cursor_Compare(void const* cursor_a,
                                         void const* cursor_b) const {
    return ThreeWayCompare(this->Cursor_GetIdx(cursor_a) + 1,
                           this->Cursor_GetIdx(cursor_b) + 1);
}

inline size_t CircularArray::Cursor_GetDist(void const* cursor_a,
                                            void const* cursor_b) const {
    return this->Cursor_GetIdx(cursor_b) - this->Cursor_GetIdx(cursor_a);
}

inline size_t CircularArray::Cursor_GetIdx(void const* cursor_) const {
    this->Check();

    auto cursor{ static_cast<CircularArray_Cursor const*>(cursor_) };
    this->CheckCursor(cursor);

    return cursor->idx;
}

inline void CircularArray::Cursor_StepL(void* cursor) const {
    this->Cursor_AdvanceL(cursor, 1);
}

inline void CircularArray::Cursor_StepR(void* cursor) const {
    this->Cursor_AdvanceR(cursor, 1);
}

inline void CircularArray::Cursor_AdvanceL(void* cursor_, size_t step) const {
    this->Check();

    void* data{ this->data };
    size_t width{ this->width };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto cursor{ static_cast<CircularArray_Cursor*>(cursor_) };
    this->CheckCursor(cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->ref =
        idx < size ? Refer_(data, width, offset, idx, capacity) : nullptr;
}

inline void CircularArray::Cursor_AdvanceR(void* cursor_, size_t step) const {
    this->Check();

    void* data{ this->data };
    size_t width{ this->width };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto cursor{ static_cast<CircularArray_Cursor*>(cursor_) };
    this->CheckCursor(cursor);

    ZETA_Core_DebugAssert(step <= size - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->ref =
        idx < size ? Refer_(data, width, offset, idx, capacity) : nullptr;
}

// -----------------------------------------------------------------------------

inline void CircularArray::Check() const {
    void* data{ this->data };
    size_t width{ this->width };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(size <= capacity);
    ZETA_Core_DebugAssert(capacity <= ZETA_Core_max_capacity);

    if (data == nullptr) { ZETA_Core_DebugAssert(capacity == 0); }
}

inline void CircularArray::CheckCursor(void const* cursor_) const {
    this->Check();

    auto cursor{ static_cast<CircularArray_Cursor const*>(cursor_) };

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(this == cursor->ca);

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(cursor->idx, 1, size));

    if (cursor->idx < size) {
        ZETA_Core_DebugAssert(
            cursor->ref == Refer_(data, stride, offset, cursor->idx, capacity));
    }
}

// -----------------------------------------------------------------------------

inline void* CircularArray::Refer_(void* data, size_t stride, size_t offset,
                                   size_t idx, size_t capacity) {
    return (static_cast<char*>(data) + stride * ((offset + idx) % capacity));
}

inline size_t CircularArray::GetLongestContPred_(size_t offset, size_t idx,
                                                 size_t size, size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return idx <= k ? idx : idx - k;
}

inline size_t CircularArray::GetLongestContSucr_(size_t offset, size_t idx,
                                                 size_t size, size_t capacity) {
    ZETA_Core_DebugAssert(offset == 0 || offset < capacity);
    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(size <= capacity);

    size_t k{ capacity - offset };

    return (size <= k || k <= idx) ? size - idx : k - idx;
}

template <bool EnWrite, typename ReaderWriter>
void ReadWriteTemp_(void* pos_cursor_, size_t cnt,
                    const ReaderWriter& reader_writer, void* dst_cursor_) {
    this->Check();

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    auto pos_cursor{ static_cast<CircularArray_Cursor const*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    auto dst_cursor{ static_cast<CircularArray_Cursor*>(dst_cursor_) };

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->ca = this;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReadable(idx, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt{ Min(cnt,
                            GetLongestContSucr_(offset, idx, size, capacity)) };
        cnt -= cur_cnt;

        auto cur_data{ static_cast<conditional_t<EnWrite, char*, char const*>>(
            Refer_(data, stride, offset, idx, capacity)) };

        for (size_t i{ 0 }; i < cur_cnt; ++i) { reader(cur_data + stride * i); }

        idx += cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->ca = this;
        dst_cursor->idx = idx;
        dst_cursor->ref = idx == this->size
                              ? nullptr
                              : Refer_(this->data, this->stride, this->offset,
                                       idx, this->capacity);
    }
}

inline void CircularArray::AssignFromSeqCntr_(
    size_t dst_beg, seq_cntr::SeqCntr const* src_seq_cntr,
    void const* src_seq_cntr_cursor, size_t cnt) {
    this->Check();

    ZETA_Core_DebugAssert(src_seq_cntr_cursor != nullptr);

    auto src_ca{ dynamic_cast<CircularArray const*>(src_seq_cntr) };

    if (src_ca != nullptr) {
        auto src_ca_cursor{ static_cast<CircularArray_Cursor const*>(
            src_seq_cntr_cursor) };

        src_ca->Check();
        src_ca->CheckCursor(src_ca_cursor);

        this->AssignFromCircularArray_(dst_beg, src_ca, src_ca_cursor->idx,
                                       cnt);

        return;
    }

    void* data{ this->data };
    size_t stride{ this->stride };
    size_t offset{ this->offset };
    size_t size{ this->size };
    size_t capacity{ this->capacity };

    size_t idx{ dst_beg };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(dst_beg, cnt, size));

    while (0 < cnt) {
        size_t cur_cnt =
            Min(cnt, GetLongestContSucr_(offset, idx, size, capacity));
        cnt -= cur_cnt;

        src_seq_cntr->Read(src_seq_cntr_cursor, cur_cnt,
                           Refer_(data, stride, offset, idx, capacity), stride,
                           src_seq_cntr_cursor);

        idx += cur_cnt;
    }
}

inline void CircularArray::AssignFromCircularArray_(size_t dst_beg,
                                                    CircularArray const* src_ca,
                                                    size_t src_beg,
                                                    size_t cnt) {
    this->Check();
    src_ca->Check();

    void* dst_data{ this->data };
    size_t dst_width{ this->width };
    size_t dst_stride{ this->stride };
    size_t dst_offset{ this->offset };
    size_t dst_size{ this->size };
    size_t dst_capacity{ this->capacity };

    void* src_data{ src_ca->data };
    size_t src_width{ src_ca->width };
    size_t src_stride{ src_ca->stride };
    size_t src_offset{ src_ca->offset };
    size_t src_size{ src_ca->size };
    size_t src_capacity{ src_ca->capacity };

    ZETA_Core_DebugAssert(seq_cntr::IsDereferable(dst_beg, cnt, dst_size));
    ZETA_Core_DebugAssert(seq_cntr::IsDereferable(src_beg, cnt, src_size));

    if (cnt == 0) { return; }

    size_t width{ Min(dst_width, src_width) };

    if (this != src_ca) {
        ZETA_Core_DebugAssert(
            !ZETA_Core_AreOverlapped(dst_data, dst_data + dst_capacity,
                                     src_data, src_data + src_capacity));

        goto VEC_MOVE;
    }

    if (dst_beg == src_beg) { return; }

    if (dst_beg < src_beg) {
        goto VEC_FW_MOVE;
    } else {
        goto VEC_BW_MOVE;
    }

VEC_MOVE:

VEC_FW_MOVE:
    while (0 < cnt) {
        size_t cur_cnt{ Min(
            cnt,
            GetLongestContSucr_(dst_offset, dst_beg, dst_size, dst_capacity),
            GetLongestContSucr_(src_offset, src_beg, src_size, src_capacity)) };
        cnt -= cur_cnt;

        ElemMove(
            Refer_(dst_data, dst_stride, dst_offset, dst_beg, dst_capacity),
            Refer_(src_data, src_stride, src_offset, src_beg, src_capacity),
            width, dst_stride, src_stride, cur_cnt);

        dst_beg += cur_cnt;
        src_beg += cur_cnt;
    }

    return;

VEC_BW_MOVE:
    for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt }; 0 < cnt;) {
        size_t cur_cnt{ Min(
            cnt,
            GetLongestContSucr_(dst_offset, dst_beg, dst_size, dst_capacity),
            GetLongestContSucr_(src_offset, src_beg, src_size, src_capacity)) };

        cnt -= cur_cnt;

        dst_end -= cur_cnt;
        src_end -= cur_cnt;

        ElemMove(
            Refer_(dst_data, dst_stride, dst_offset, dst_end, dst_capacity),
            Refer_(src_data, src_stride, src_offset, src_end, src_capacity),
            width, dst_stride, src_stride, cur_cnt);
    }

    return;
}

}  // namespace zeta::core
