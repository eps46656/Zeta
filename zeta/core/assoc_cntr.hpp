#pragma once

#include <zeta/core/utils.hpp>

namespace zeta::core::assoc_cntr {

#define ZETA_Core_AssocCntr_AllocaCursor(tmp_cntr, cntr)             \
    ({                                                               \
        void const* tmp_cntr{ cntr };                                \
        ZETA_Core_DebugAssert(tmp_cntr != nullptr);                  \
                                                                     \
        __builtin_alloca_with_align(                                 \
            static_cast<::zeta::core::assoc_cntr::Cntr const*>(cntr) \
                ->cursor_size,                                       \
            __CHAR_BIT__ * alignof(max_align_t));                    \
    })

#define ZETA_Core_AssocCntr_AllocaCursor(cntr) \
    ZETA_Core_AssocCntr_AllocaCursor(ZETA_Core_TmpName, cntr)

struct Cntr;
struct VTable;

struct Cntr {
    void* inst;
    void const* inst_const;

    size_t cursor_size;

    size_t width;
    size_t capacity;

    VTable const* vtable;
};

// -----------------------------------------------------------------------------

struct VTable {
    void (*Deinit)(void* cntr);

    // -------------------------------------------------------------------------

    size_t (*GetSize)(void const* cntr);

    size_t (*GetCapacity)(void const* cntr);

    // -------------------------------------------------------------------------

    void (*GetLBCursor)(void const* cntr, void* dst_cursor);

    void (*GetRBCursor)(void const* cntr, void* dst_cursor);

    // -------------------------------------------------------------------------

    void* (*PeekL)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekL)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*PeekR)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekR)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*Refer)(void* cntr, void const* pos_cursor);

    void const* (*ConstRefer)(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    void* (*Find)(void* cntr, void const* key, ContextualHash const& key_hash,
                  ContextualCompare const& key_elem_compare, void* dst_cursor);

    void const* (*ConstFind)(void const* cntr, void const* key,
                             ContextualHash const& key_hash,
                             ContextualCompare const& key_elem_compare,
                             void* dst_cursor);

    void* (*Insert)(void* cntr, void* elem, void* dst_cursor);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    // -------------------------------------------------------------------------

    void (*CopyCursor)(void const* cntr, void* dst_cursor,
                       void const* src_cursor);

    bool (*AreEqualCursor)(void const* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void const* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void const* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void const* cntr, void const* cursor);

    void (*CursorStepL)(void const* cntr, void* cursor);

    void (*CursorStepR)(void const* cntr, void* cursor);

    void (*CursorAdvanceL)(void const* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void const* cntr, void* cursor, size_t step);
};

// -----------------------------------------------------------------------------

void Deinit(void* cntr);

// -----------------------------------------------------------------------------

size_t GetSize(void const* cntr);

size_t GetCapacity(void const* cntr);

// -----------------------------------------------------------------------------

void GetLBCursor(void const* cntr, void* dst_cursor);

void GetRBCursor(void const* cntr, void* dst_cursor);

// -----------------------------------------------------------------------------

void* PeekL(void* cntr, void* dst_cursor, void* dst_elem);

void const* ConstPeekL(void const* cntr, void* dst_cursor, void* dst_elem);

void* PeekR(void* cntr, void* dst_cursor, void* dst_elem);

void const* ConstPeekR(void const* cntr, void* dst_cursor, void* dst_elem);

void* Refer(void* cntr, void const* pos_cursor);

void const* ConstRefer(void const* cntr, void const* pos_cursor);

// -----------------------------------------------------------------------------

void* Find(void* cntr, void const* key, ContextualHash const& key_hash,
           ContextualCompare const& key_elem_compare, void* dst_cursor);

void const* ConstFind(void const* cntr, void const* key,
                      ContextualHash const& key_hash,
                      ContextualCompare const& key_elem_compare,
                      void* dst_cursor);

void* Insert(void* cntr, void* elem, void* dst_cursor);

void Erase(void* cntr, void* pos_cursor);

void EraseAll(void* cntr);

// -----------------------------------------------------------------------------

bool AreEqualCursor(void const* cntr, void const* cursor_a,
                    void const* cursor_b);

int CompareCursor(void const* cntr, void const* cursor_a, void const* cursor_b);

size_t GetCursorDist(void const* cntr, void const* cursor_a,
                     void const* cursor_b);

size_t GetCursorIdx(void const* cntr, void const* cursor);

void CursorStepL(void const* cntr, void* cursor);

void CursorStepR(void const* cntr, void* cursor);

void CursorAdvanceL(void const* cntr, void* cursor, size_t step);

void CursorAdvanceR(void const* cntr, void* cursor, size_t step);

// -----------------------------------------------------------------------------

void CheckCntr(void* cntr);

void CheckCntr(void const* cntr);

}  // namespace zeta::core::assoc_cntr
