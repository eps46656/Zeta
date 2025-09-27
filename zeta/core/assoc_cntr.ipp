#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>

namespace zeta::core::assoc_cntr {

#pragma push_macro("Call_")
#pragma push_macro("ConstCall_")

#define Call_(func, ...)                                \
    auto cntr{ static_cast<Cntr*>(cntr_) };             \
    CheckCntr(cntr);                                    \
    return cntr->vtable->func(cntr->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

#define ConstCall_(func, ...)                                 \
    auto cntr{ static_cast<Cntr const*>(cntr_) };             \
    CheckCntr(cntr);                                          \
    return cntr->vtable->func(cntr->inst_const, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

// -----------------------------------------------------------------------------

inline void Deinit(void* cntr_) { Call_(Deinit); }

// -----------------------------------------------------------------------------

inline size_t GetSize(void const* cntr_) { ConstCall_(GetSize); }

// -----------------------------------------------------------------------------

inline void GetLBCursor(void const* cntr_, void* dst_cursor) {
    ConstCall_(GetLBCursor, dst_cursor);
}

inline void GetRBCursor(void const* cntr_, void* dst_cursor) {
    ConstCall_(GetRBCursor, dst_cursor);
}

// -----------------------------------------------------------------------------

inline void* PeekL(void* cntr_, void* dst_cursor, void* dst_elem) {
    Call_(PeekL, dst_cursor, dst_elem);
}

inline void const* ConstPeekL(void const* cntr_, void* dst_cursor,
                              void* dst_elem) {
    ConstCall_(ConstPeekL, dst_cursor, dst_elem);
}

inline void* PeekR(void* cntr_, void* dst_cursor, void* dst_elem) {
    Call_(PeekR, dst_cursor, dst_elem);
}

inline void const* ConstPeekR(void const* cntr_, void* dst_cursor,
                              void* dst_elem) {
    ConstCall_(ConstPeekR, dst_cursor, dst_elem);
}

inline void* Refer(void* cntr_, void const* pos_cursor) {
    Call_(Refer, pos_cursor);
}

inline void const* ConstRefer(void const* cntr_, void const* pos_cursor) {
    ConstCall_(ConstRefer, pos_cursor);
}

// -----------------------------------------------------------------------------

inline void* Find(void* cntr_, void const* key, ContextualHash const& key_hash,
                  ContextualCompare const& key_elem_compare, void* dst_cursor) {
    Call_(Find, key, key_hash, key_elem_compare, dst_cursor);
}

inline void const* ConstFind(void const* cntr_, void const* key,
                             ContextualHash const& key_hash,
                             ContextualCompare const& key_elem_compare,
                             void* dst_cursor) {
    ConstCall_(ConstFind, key, key_hash, key_elem_compare, dst_cursor);
}

inline void* Insert(void* cntr_, void* elem, void* dst_cursor) {
    Call_(Insert, elem, dst_cursor);
}

inline void Erase(void* cntr_, void* pos_cursor) { Call_(Erase, pos_cursor); }

inline void EraseAll(void* cntr_) { Call_(EraseAll); }

// -----------------------------------------------------------------------------

inline bool AreEqualCursor(void const* cntr_, void const* cursor_a,
                           void const* cursor_b) {
    ConstCall_(AreEqualCursor, cursor_a, cursor_b);
}

inline int CompareCursor(void const* cntr_, void const* cursor_a,
                         void const* cursor_b) {
    ConstCall_(CompareCursor, cursor_a, cursor_b);
}

inline size_t GetCursorDist(void const* cntr_, void const* cursor_a,
                            void const* cursor_b) {
    ConstCall_(GetCursorDist, cursor_a, cursor_b);
}

inline size_t GetCursorIdx(void const* cntr_, void const* cursor) {
    ConstCall_(GetCursorIdx, cursor);
}

inline void CursorStepL(void const* cntr_, void* cursor) {
    ConstCall_(CursorStepL, cursor);
}

inline void CursorStepR(void const* cntr_, void* cursor) {
    ConstCall_(CursorStepR, cursor);
}

inline void CursorAdvanceL(void const* cntr_, void* cursor, size_t step) {
    ConstCall_(CursorAdvanceL, cursor, step);
}

inline void CursorAdvanceR(void const* cntr_, void* cursor, size_t step) {
    ConstCall_(CursorAdvanceR, cursor, step);
}

// -----------------------------------------------------------------------------

inline void CheckCntr(void* cntr_) {
    auto cntr{ static_cast<Cntr const*>(cntr_) };
    CheckCntr(cntr);

    ZETA_Core_DebugAssert(cntr->inst != nullptr);
}

inline void CheckCntr(void const* cntr_) {
    auto cntr{ static_cast<Cntr const*>(cntr_) };
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(cntr->inst_const != nullptr);

    ZETA_Core_DebugAssert(cntr->vtable != nullptr);
}

#pragma pop_macro("Call_")
#pragma pop_macro("ConstCall_")

}  // namespace zeta::core::assoc_cntr
