
#include <deque>
#include <zeta/core/debug_deque.decl.hpp>

namespace zeta::core {

inline void DebugDeque::Init() { this->deque = new std::deque<void*>; }

inline void DebugDeque::Deinit() { delete this->deque; }

inline size_t DebugDeque::GetCursorSize() const {
    return sizeof(DebugDeque_Cursor);
}

inline size_t DebugDeque::GetWidth() const { return this->width; }

inline size_t DebugDeque::GetSize() const { return this->deque->size(); }

inline size_t DebugDeque::GetCapacity() const {
    return this->deque->max_size();
}

inline void DebugDeque::GetLBCursor(void* dst_cursor_) const {
    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    dst_cursor->dd = this;
    dst_cursor->idx = -1;
}

inline void DebugDeque::GetRBCursor(void* dst_cursor_) const {
    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    dst_cursor->dd = this;
    dst_cursor->idx = this->deque->size();
}

inline void* DebugDeque::PeekL(void* dst_cursor_, void* dst_elem) {
    this->Check();

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = 0;
    }

    if (!this->deque->empty() && dst_elem != nullptr) {
        MemCopy(dst_elem, deque->front(), this->width);
    }
}

inline void const* DebugDeque::PeekL(void* dst_cursor, void* dst_elem) const {
    return const_cast<DebugDeque*>(this)->PeekL(dst_cursor, dst_elem);
}

inline void* DebugDeque::PeekR(void* dst_cursor_, void* dst_elem) {
    this->Check();

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = 0;
    }

    if (this->deque->empty()) { return nullptr; }

    void* elem{ deque->back() };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, this->width); }

    return elem;
}

inline void const* DebugDeque::PeekR(void* dst_cursor, void* dst_elem) const {
    return const_cast<DebugDeque*>(this)->PeekR(dst_cursor, dst_elem);
}

inline void* DebugDeque::Access(size_t idx, void* dst_cursor_, void* dst_elem) {
    this->Check();

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, this->deque->size()));

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = idx;
    }

    if (!seq_cntr::IsDereferable(idx, 1, this->deque->size())) { return; }

    void* elem{ (*deque)[idx] };

    if (dst_elem != nullptr) { MemCopy(dst_elem, elem, this->width); }

    return elem;
}

inline void const* DebugDeque::Access(size_t idx, void* dst_cursor,
                                      void* dst_elem) const {
    return const_cast<DebugDeque*>(this)->Access(idx, dst_cursor, dst_elem);
}

inline void* DebugDeque::Refer(void const* pos_cursor_) {
    auto pos_cursor{ static_cast<DebugDeque_Cursor const*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, this->deque->size()));

    return seq_cntr::IsDereferable(idx, 1, this->deque->size()) ? (*deque)[idx]
                                                                : nullptr;
}

// -----------------------------------------------------------------------------

template <typename Reader>
void DebugDeque::Read(void const* pos_cursor, size_t cnt, const Reader& reader,
                      void* dst_cursor) const {
    const_cast<DebugDeque*>(this)->ReadWriteTemp_<false>(pos_cursor, cnt,
                                                         reader, dst_cursor);
}

template <typename Writer>
void DebugDeque::Write(void const* pos_cursor, size_t cnt, const Writer& writer,
                       void* dst_cursor) {
    this->ReadWriteTemp_<true>(pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void DebugDeque::ReadWrite(void const* pos_cursor, size_t cnt,
                           const ReaderWriter& reader_writer,
                           void* dst_cursor) {
    this->ReadWriteTemp_<true>(pos_cursor, cnt, reader_writer, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void DebugDeque::Read(void const* pos_cursor, size_t cnt, void* dst,
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

inline void DebugDeque::Write(void* pos_cursor, size_t cnt, void const* src,
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
void* DebugDeque::PushL(size_t cnt, const Writer& writer, void* dst_cursor_) {
    this->Check();

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    this->deque->insert(this->deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        void* elem{ new unsigned char[this->width] };
        writer(elem);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = 0;
    }

    return this->deque->empty() ? nullptr : this->deque->front();
}

template <typename Writer>
void* DebugDeque::PushR(size_t cnt, const Writer& writer, void* dst_cursor_) {
    this->Check();

    size_t old_size{ this->deque->size() };

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    this->deque->insert(this->deque->end(), cnt, nullptr);

    for (size_t idx{ old_size }; idx < this->deque->size(); ++idx) {
        void* elem{ new unsigned char[this->width] };
        writer(elem);
        (*deque)[idx] = elem;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = old_size;
    }

    return old_size < this->deque->size() ? (*this->deque)[old_size] : nullptr;
}

// -----------------------------------------------------------------------------

void* DebugDeque::PushL(size_t cnt, void const* src, size_t src_stride,
                        void* dst_cursor) {
    if (src == nullptr) {
        return this->PushL(cnt, [=](void*) {}, dst_cursor);
    }

    return this->PushL(
        cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, this->width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

void* DebugDeque::PushR(size_t cnt, void const* src, size_t src_stride,
                        void* dst_cursor) {
    if (src == nullptr) {
        return this->PushR(cnt, [=](void*) {}, dst_cursor);
    }

    return this->PushR(
        cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, this->width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

void* DebugDeque::Insert(void const* pos_cursor, size_t cnt, void const* src,
                         size_t src_stride, void* dst_cursor) {
    if (src == nullptr) {
        return this->Insert(pos_cursor, cnt, [=](void*) {}, dst_cursor);
    }

    return this->Insert(
        pos_cursor, cnt,
        [=](void* dst) mutable {
            MemCopy(dst, src, this->width);
            src = static_cast<char const*>(src) + src_stride;
        },
        dst_cursor);
}

inline void DebugDeque::PopL(size_t cnt) {
    this->Check();

    ZETA_Core_DebugAssert(cnt <= this->deque->size());

    while (0 < cnt--) {
        delete[] (unsigned char*)this->deque->front();
        this->deque->pop_front();
    }
}

inline void DebugDeque::PopR(size_t cnt) {
    this->Check();

    ZETA_Core_DebugAssert(cnt <= this->deque->size());

    while (0 < cnt--) {
        delete[] (unsigned char*)this->deque->back();
        this->deque->pop_back();
    }
}

inline void DebugDeque::Erase(void* pos_cursor_, size_t cnt) {
    this->Check();

    auto pos_cursor{ static_cast<DebugDeque_Cursor*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    size_t beg{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, this->deque->size()));

    size_t end{ beg + cnt };

    for (size_t idx{ beg }; idx < end; ++idx) {
        delete[] (unsigned char*)((*deque)[idx]);
    }

    deque->erase(deque->begin() + beg, deque->begin() + end);
}

inline void DebugDeque::EraseAll() {
    this->Check();

    for (void* elem : *this->deque) { delete[] (unsigned char*)elem; }

    this->deque->clear();
}

inline bool DebugDeque::Cursor_AreEqual(void const* cursor_a,
                                        void const* cursor_b) const {
    size_t idx_a{ this->Cursor_GetIdx(cursor_a) };
    size_t idx_b{ this->Cursor_GetIdx(cursor_b) };

    return idx_a == idx_b;
}

inline int DebugDeque::Cursor_Compare(void const* cursor_a,
                                      void const* cursor_b) const {
    size_t idx_a{ this->Cursor_GetIdx(cursor_a) };
    size_t idx_b{ this->Cursor_GetIdx(cursor_b) };

    return ThreeWayCompare(idx_a + 1, idx_b + 1);
}

inline size_t DebugDeque::Cursor_GetDist(void const* cursor_a,
                                         void const* cursor_b) const {
    return this->Cursor_GetIdx(cursor_b) - this->Cursor_GetIdx(cursor_a);
}

inline size_t DebugDeque::Cursor_GetIdx(void const* cursor_) const {
    this->Check();

    auto cursor{ static_cast<DebugDeque_Cursor const*>(cursor_) };

    return cursor->idx;
}

inline void DebugDeque::Cursor_StepL(void* cursor) const {
    this->Cursor_AdvanceL(cursor, 1);
}

inline void DebugDeque::Cursor_StepR(void* cursor) const {
    this->Cursor_AdvanceR(cursor, 1);
}

inline void DebugDeque::Cursor_AdvanceL(void* cursor_, size_t step) const {
    this->Check();

    auto cursor{ static_cast<DebugDeque_Cursor*>(cursor_) };
    this->CheckCursor(cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

inline void DebugDeque::Cursor_AdvanceR(void* cursor_, size_t step) const {
    this->Check();

    auto cursor{ static_cast<DebugDeque_Cursor*>(cursor_) };
    this->CheckCursor(cursor);

    ZETA_Core_DebugAssert(step <= this->deque->size() - cursor->idx);

    cursor->idx += step;
}

// -----------------------------------------------------------------------------

inline void DebugDeque::Check() const {
    ZETA_Core_DebugAssert(this->deque != nullptr);
}

inline void DebugDeque::CheckCursor(void const* cursor_) const {
    this->Check();

    auto cursor{ static_cast<DebugDeque_Cursor const*>(cursor_) };

    ZETA_Core_DebugAssert(
        seq_cntr::IsReferable(cursor->idx, 1, this->deque->size()));
}

// -----------------------------------------------------------------------------

template <bool EnWrite, typename ReaderWriter>
void DebugDeque::ReadWriteTemp_(void const* pos_cursor, size_t cnt,
                                const ReaderWriter& reader_writer,
                                void* dst_cursor) {
    this->Check();

    auto pos_cursor{ static_cast<DebugDeque_Cursor const*>(pos_cursor_) };
    this->CheckCursor(pos_cursor);

    size_t beg{ pos_cursor->idx };

    auto dst_cursor{ static_cast<DebugDeque_Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(beg, cnt, this->deque->size()));

    if (dst_cursor != nullptr) {
        dst_cursor->dd = this;
        dst_cursor->idx = beg + cnt;
    }

    for (size_t idx{ beg }, end{ beg + cnt }; idx < end; ++idx) {
        reader_writer(
            conditional_t<EnWrite, void*, void const*>((*this->deque)[idx]));
    }
}

}  // namespace zeta::core
