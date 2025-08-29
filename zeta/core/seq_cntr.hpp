

#pragma once

#include <zeta/core/utils.hpp>

namespace zeta::core::seq_cntr {

struct SeqCntr;

bool IsReferable(size_t idx, size_t cnt, size_t size);

bool IsDereferable(size_t idx, size_t cnt, size_t size);

bool IsInsertable(size_t idx, size_t cnt, size_t size, size_t capacity);

bool IsErasable(size_t idx, size_t cnt, size_t size);

// -----------------------------------------------------------------------------

struct SeqCntr {
    virtual void Deinit() = 0;

    // -------------------------------------------------------------------------

    virtual size_t GetCursorSize() const = 0;

    virtual size_t GetWidth() const = 0;

    virtual size_t GetSize() const = 0;

    virtual size_t GetCapacity() const = 0;

    // -------------------------------------------------------------------------

    virtual void GetLBCursor(void* dst_cursor) const = 0;

    virtual void GetRBCursor(void* dst_cursor) const = 0;

    // -------------------------------------------------------------------------

    virtual void* PeekL(void* dst_cursor, void* dst_elem) = 0;

    virtual void const* PeekL(void* dst_cursor, void* dst_elem) const = 0;

    virtual void* PeekR(void* dst_cursor, void* dst_elem) = 0;

    virtual void const* PeekR(void* dst_cursor, void* dst_elem) const = 0;

    virtual void* Access(size_t idx, void* dst_cursor, void* dst_elem) = 0;

    virtual void const* Access(size_t idx, void* dst_cursor,
                               void* dst_elem) const = 0;

    virtual void* Refer(void const* pos_cursor) = 0;

    virtual void const* Refer(void const* pos_cursor) const = 0;

    // -------------------------------------------------------------------------

    virtual void Read(void const* pos_cursor, size_t cnt, void* dst,
                      size_t dst_stride, void* dst_cursor) const = 0;

    virtual void Write(void* pos_cursor, size_t cnt, void const* src,
                       size_t src_stride, void* dst_cursor) = 0;

    // -------------------------------------------------------------------------

    virtual void* PushL(size_t cnt, void const* src, size_t src_stride,
                        void* dst_cursor) = 0;

    virtual void* PushR(size_t cnt, void const* src, size_t src_stride,
                        void* dst_cursor) = 0;

    virtual void* Insert(void const* pos_cursor, size_t cnt, void const* src,
                         size_t src_stride, void* dst_cursor) = 0;

    // -------------------------------------------------------------------------

    virtual void PopL(size_t cnt) = 0;

    virtual void PopR(size_t cnt) = 0;

    virtual void Erase(void* pos_cursor, size_t cnt) = 0;

    virtual void EraseAll() = 0;

    // -------------------------------------------------------------------------

    virtual bool Cursor_AreEqual(void const* cursor_a,
                                 void const* cursor_b) const = 0;

    virtual int Cursor_Compare(void const* cursor_a,
                               void const* cursor_b) const = 0;

    virtual size_t Cursor_GetDist(void const* cursor_a,
                                  void const* cursor_b) const = 0;

    virtual size_t Cursor_GetIdx(void const* cursor) const = 0;

    virtual void Cursor_StepL(void* cursor) const = 0;

    virtual void Cursor_StepR(void* cursor) const = 0;

    virtual void Cursor_AdvanceL(void* cursor, size_t step) const = 0;

    virtual void Cursor_AdvanceR(void* cursor, size_t step) const = 0;
};

// -----------------------------------------------------------------------------

inline bool IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

inline bool IsDereferable(size_t idx, size_t cnt, size_t size) {
    return idx < size && cnt <= size - idx;
}

inline bool IsInsertable(size_t idx, size_t cnt, size_t size, size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

inline bool IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

}  // namespace zeta::core::seq_cntr
