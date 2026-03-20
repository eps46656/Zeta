#pragma once

#include <zeta/core/assoc_cntr.hpp>

namespace zeta::core::assoc_cntr_ref {

struct Ref {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    assoc_cntr::AbilityFlag dynamic_enabled_ability_flag;
    assoc_cntr::AbilityFlag dynamic_disabled_ability_flag;

    assoc_cntr::VTable const* vtable;

    void* cntr;
};

assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(Ref* ref);

assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(Ref const* ref);

assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(Ref* ref);

assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(Ref const* ref);

size_t GetCursorSize(Ref const*);

size_t GetWidth(Ref const* ref);

size_t GetSride(Ref const* ref);

size_t GetOffset(Ref const* ref);

size_t GetSize(Ref const* ref);

size_t GetCapacity(Ref const* ref);

void GetLBCursor(Ref const* ref, void* dst_cursor);

void GetRBCursor(Ref const* ref, void* dst_cursor);

void* PeekL(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

void* PeekR(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

void* Access(Ref const* ref, size_t idx, bool lazy_copy_elem, void* dst_cursor,
             void* dst_elem);

void* Derefer(Ref const* ref, void const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename KeyHash, typename KeyElemCompare>
void* Find(Ref const* ref, void const* key, KeyHash&& key_hash,
           KeyElemCompare&& key_elem_compare, bool lazy_copy_elem,
           void* dst_cursor, void* dst_elem);

void* Insert(Ref* ref, void const* elem, void* dst_cursor);

void PopL(Ref* ref, size_t cnt);

void PopR(Ref* ref, size_t cnt);

void Erase(Ref* ref, void* pos_cursor);

void EraseAll(Ref* ref);

void CopyCursor(Ref const* ref, void const* src_cursor, void* dst_cursor);

bool AreEqualCursor(Ref const* ref, void const* cursor_a, void const* cursor_b);

int CompareCursor(Ref const* ref, void const* cursor_a, void const* cursor_b);

size_t GetCursorDist(Ref const* ref, void const* cursor_a,
                     void const* cursor_b);

size_t GetCursorIdx(Ref const* ref, void const* cursor);

void CursorStepL(Ref const* ref, void* cursor);

void CursorStepR(Ref const* ref, void* cursor);

void CursorAdvanceL(Ref const* ref, void* cursor, size_t step);

void CursorAdvanceR(Ref const* ref, void* cursor, size_t step);

void CheckRef(Ref* ref);

template <typename AssocCntrLike>
Ref MakeRef(AssocCntrLike&& cntr);

}  // namespace zeta::core::assoc_cntr_ref

namespace zeta::core {

template <>
struct assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const, void> {
    static void* GetReferedInst(assoc_cntr_ref::Ref const* ref);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        assoc_cntr_ref::Ref const* ref);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        assoc_cntr_ref::Ref const* ref);

    static size_t GetCursorSize(assoc_cntr_ref::Ref const* ref);

    static size_t GetWidth(assoc_cntr_ref::Ref const* ref);

    static size_t GetSize(assoc_cntr_ref::Ref const* ref);

    static size_t GetCapacity(assoc_cntr_ref::Ref const* ref);

    static void GetLBCursor(assoc_cntr_ref::Ref const* ref, void* dst_cursor);

    static void GetRBCursor(assoc_cntr_ref::Ref const* ref, void* dst_cursor);

    static void* PeekL(assoc_cntr_ref::Ref const* ref, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* PeekR(assoc_cntr_ref::Ref const* ref, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* Derefer(assoc_cntr_ref::Ref const* ref, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(assoc_cntr_ref::Ref const* ref, void const* key,
                      KeyHash&& key_hash, KeyElemCompare&& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void CopyCursor(assoc_cntr_ref::Ref const* ref,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(assoc_cntr_ref::Ref const* ref,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(assoc_cntr_ref::Ref const* ref,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(assoc_cntr_ref::Ref const* ref,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(assoc_cntr_ref::Ref const* ref,
                               void const* cursor);

    static void CursorStepL(assoc_cntr_ref::Ref const* ref, void* cursor);

    static void CursorStepR(assoc_cntr_ref::Ref const* ref, void* cursor);

    static void CursorAdvanceL(assoc_cntr_ref::Ref const* ref, void* cursor,
                               size_t step);

    static void CursorAdvanceR(assoc_cntr_ref::Ref const* ref, void* cursor,
                               size_t step);
};

template <>
struct assoc_cntr::CntrTraits<assoc_cntr_ref::Ref, void>
    : public assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const, void> {
    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static void* Insert(assoc_cntr_ref::Ref* ref, void const* elem,
                        void* dst_cursor);

    static void PopL(assoc_cntr_ref::Ref* ref, size_t cnt);

    static void PopR(assoc_cntr_ref::Ref* ref, size_t cnt);

    static void Erase(assoc_cntr_ref::Ref* ref, void* pos_cursor);

    static void EraseAll(assoc_cntr_ref::Ref* ref);
};

}  // namespace zeta::core
