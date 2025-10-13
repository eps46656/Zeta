#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>

#pragma push_macro("Call_")
#pragma push_macro("ConstCall_")

#define Call_(func, ...)                                                    \
    ZETA_Core_StaticAssert(!IsConst::value);                                \
                                                                            \
    auto assoc_cntr_ref{ static_cast<AssocCntrRefTpl*>(assoc_cntr_ref_) };  \
    ZETA_Core_DebugAssert(CheckCntr(assoc_cntr_ref));                       \
                                                                            \
    return assoc_cntr_ref->vtable->func(assoc_cntr_ref->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

#define ConstCall_(func, ...)                                               \
    auto assoc_cntr_ref{ static_cast<AssocCntrRefTpl const*>(               \
        assoc_cntr_ref_) };                                                 \
                                                                            \
    ZETA_Core_DebugAssert(CheckCntr(assoc_cntr_ref));                       \
                                                                            \
    return assoc_cntr_ref->vtable->func(assoc_cntr_ref->inst, __VA_ARGS__); \
    ZETA_Core_StaticAssert(true);

namespace zeta::core::assoc_cntr {

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void, _> AssocCntrRefTpl<IsConst>::Deinit(
    void* assoc_cntr_ref_) {
    Call_(Deinit);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetSize(void const* assoc_cntr_ref_) {
    ConstCall_(GetSize);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::GetLBCursor(void const* assoc_cntr_ref_,
                                           void* dst_cursor) {
    ConstCall_(GetLBCursor, dst_cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::GetRBCursor(void const* assoc_cntr_ref_,
                                           void* dst_cursor) {
    ConstCall_(GetRBCursor, dst_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AssocCntrRefTpl<IsConst>::PeekL(
    void* assoc_cntr_ref_, void* dst_cursor, void* dst_elem) {
    Call_(PeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstPeekL(void const* assoc_cntr_ref_,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    ConstCall_(ConstPeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AssocCntrRefTpl<IsConst>::PeekR(
    void* assoc_cntr_ref_, void* dst_cursor, void* dst_elem) {
    Call_(PeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstPeekR(void const* assoc_cntr_ref_,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    ConstCall_(ConstPeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AssocCntrRefTpl<IsConst>::Refer(
    void* assoc_cntr_ref_, void const* pos_cursor) {
    Call_(Refer, pos_cursor);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstRefer(void const* assoc_cntr_ref_,
                                                 void const* pos_cursor) {
    ConstCall_(ConstRefer, pos_cursor);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AssocCntrRefTpl<IsConst>::Find(
    void* assoc_cntr_ref_, void const* key, FnHash const& key_hash,
    FnCompare const& key_elem_compare, void* dst_cursor) {
    Call_(Find, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstFind(
    void const* assoc_cntr_ref_, void const* key, FnHash const& key_hash,
    FnCompare const& key_elem_compare, void* dst_cursor) {
    ConstCall_(ConstFind, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void*, _> AssocCntrRefTpl<IsConst>::Insert(
    void* assoc_cntr_ref_, void const* elem, void* dst_cursor) {
    Call_(Insert, elem, dst_cursor);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void, _> AssocCntrRefTpl<IsConst>::Erase(
    void* assoc_cntr_ref_, void* pos_cursor) {
    Call_(Erase, pos_cursor);
}

template <typename IsConst>
template <typename _>
EnableIf<!IsConst::value, void, _> AssocCntrRefTpl<IsConst>::EraseAll(
    void* assoc_cntr_ref_) {
    Call_(EraseAll);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
template <typename OtherIsConst, typename>
AssocCntrRefTpl<IsConst>::AssocCntrRefTpl(
    AssocCntrRefTpl<OtherIsConst> const& other_assoc_cntr_ref)
    : inst{ other_assoc_cntr_ref.inst },
      cursor_size{ other_assoc_cntr_ref.cursor_size },
      width{ other_assoc_cntr_ref.width },
      capacity{ other_assoc_cntr_ref.capacity },
      vtable{ other_assoc_cntr_ref.vtable } {}

// -----------------------------------------------------------------------------

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CopyCursor(void const* assoc_cntr_ref_,
                                          void const* cursor,
                                          void* dst_cursor) {
    ConstCall_(CopyCursor, cursor, dst_cursor);
}

template <typename IsConst>
bool AssocCntrRefTpl<IsConst>::AreEqualCursor(void const* assoc_cntr_ref_,
                                              void const* cursor_a,
                                              void const* cursor_b) {
    ConstCall_(AreEqualCursor, cursor_a, cursor_b);
}

template <typename IsConst>
int AssocCntrRefTpl<IsConst>::CompareCursor(void const* assoc_cntr_ref_,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    ConstCall_(CompareCursor, cursor_a, cursor_b);
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetCursorDist(void const* assoc_cntr_ref_,
                                               void const* cursor_a,
                                               void const* cursor_b) {
    ConstCall_(GetCursorDist, cursor_a, cursor_b);
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetCursorIdx(void const* assoc_cntr_ref_,
                                              void const* cursor) {
    ConstCall_(GetCursorIdx, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorStepL(void const* assoc_cntr_ref_,
                                           void* cursor) {
    ConstCall_(CursorStepL, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorStepR(void const* assoc_cntr_ref_,
                                           void* cursor) {
    ConstCall_(CursorStepR, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorAdvanceL(void const* assoc_cntr_ref_,
                                              void* cursor, size_t step) {
    ConstCall_(CursorAdvanceL, cursor, step);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorAdvanceR(void const* assoc_cntr_ref_,
                                              void* cursor, size_t step) {
    ConstCall_(CursorAdvanceR, cursor, step);
}

// -----------------------------------------------------------------------------

template <typename IsConst>
bool AssocCntrRefTpl<IsConst>::CheckCntr(void const* assoc_cntr_ref_) {
    auto assoc_cntr_ref{ static_cast<AssocCntrRef const*>(assoc_cntr_ref_) };

    if (!(assoc_cntr_ref != nullptr)) { return false; }
    if (!(assoc_cntr_ref->inst != nullptr)) { return false; }
    if (!(assoc_cntr_ref->vtable != nullptr)) { return false; }

    return true;
}

}  // namespace zeta::core::assoc_cntr

#pragma pop_macro("Call_")
#pragma pop_macro("ConstCall_")
