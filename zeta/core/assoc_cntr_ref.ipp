#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

#pragma push_macro("TestAbility")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                                     \
    (((ability_flag) & (static_cast<::zeta::core::assoc_cntr::AbilityFlag>(1)  \
                        << ::zeta::core::assoc_cntr::AbilityEnum::ability)) != \
     0)

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...)                             \
    {                                                                \
        ZETA_Core_DebugAssert(                                       \
            TestAbility(ref.dynamic_enabled_ability_flag, ability)); \
                                                                     \
        auto method_ptr{ ref.vtable->method };                       \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                \
                                                                     \
        return method_ptr(ref.cntr, __VA_ARGS__);                    \
    }                                                                \
    ZETA_Core_StaticAssert(true);

inline assoc_cntr::AbilityFlag assoc_cntr_ref::GetDynamicEnabledAbilityFlag(
    Ref& ref) {
    return ref.dynamic_enabled_ability_flag;
}

inline assoc_cntr::AbilityFlag assoc_cntr_ref::GetDynamicEnabledAbilityFlag(
    Ref const& ref) {
    return GetDynamicEnabledAbilityFlag(const_cast<Ref&>(ref)) &
           assoc_cntr::const_ability_flag;
}

inline assoc_cntr::AbilityFlag assoc_cntr_ref::GetDynamicDisabledAbilityFlag(
    Ref& ref) {
    return ref.dynamic_disabled_ability_flag;
}

inline assoc_cntr::AbilityFlag assoc_cntr_ref::GetDynamicDisabledAbilityFlag(
    Ref const& ref) {
    return GetDynamicDisabledAbilityFlag(const_cast<Ref&>(ref)) &
           assoc_cntr::const_ability_flag;
}

inline size_t assoc_cntr_ref::GetCursorSize(Ref const& ref) {
    return ref.cursor_size;
}

inline size_t assoc_cntr_ref::GetElemSize(Ref const& ref) { return ref.width; }

inline size_t assoc_cntr_ref::GetElemCnt(Ref const& ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

inline size_t assoc_cntr_ref::GetMaxElemCnt(Ref const& ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

inline void assoc_cntr_ref::GetLBCursor(Ref const& ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void assoc_cntr_ref::GetRBCursor(Ref const& ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void* assoc_cntr_ref::PeekL(Ref const& ref, bool lazy_copy_elem,
                                   void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* assoc_cntr_ref::PeekR(Ref const& ref, bool lazy_copy_elem,
                                   void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* assoc_cntr_ref::Derefer(Ref const& ref, void const* pos_cursor,
                                     bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename KeyHash, typename KeyElemCompare>
void* assoc_cntr_ref::Find(Ref const& ref, void const* key, KeyHash&& key_hash,
                           KeyElemCompare&& key_elem_compare,
                           bool lazy_copy_elem, void* dst_cursor,
                           void* dst_elem) {
    CallMethod(Find, FnFind, key, meta::Forward<KeyHash>(key_hash),
               meta::Forward<KeyElemCompare>(key_elem_compare), lazy_copy_elem,
               dst_cursor, dst_elem);
}

inline void* assoc_cntr_ref::Insert(Ref& ref, void const* elem,
                                    void* dst_cursor) {
    CallMethod(Insert, FnInsert, elem, dst_cursor);
}

inline void assoc_cntr_ref::PopL(Ref& ref, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

inline void assoc_cntr_ref::PopR(Ref& ref, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

inline void assoc_cntr_ref::Erase(Ref& ref, void* pos_cursor) {
    CallMethod(Erase, Erase, pos_cursor);
}

inline void assoc_cntr_ref::EraseAll(Ref& ref) {
    CallMethod(EraseAll, EraseAll);
}

inline void assoc_cntr_ref::CopyCursor(Ref const& ref, void const* src_cursor,
                                       void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool assoc_cntr_ref::AreEqualCursor(Ref const& ref, void const* cursor_a,
                                           void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int assoc_cntr_ref::CompareCursor(Ref const& ref, void const* cursor_a,
                                         void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t assoc_cntr_ref::GetCursorDist(Ref const& ref,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t assoc_cntr_ref::GetCursorIdx(Ref const& ref, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void assoc_cntr_ref::CursorStepL(Ref const& ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void assoc_cntr_ref::CursorStepR(Ref const& ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void assoc_cntr_ref::CursorAdvanceL(Ref const& ref, void* cursor,
                                           size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void assoc_cntr_ref::CursorAdvanceR(Ref const& ref, void* cursor,
                                           size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

inline void assoc_cntr_ref::CheckRef(Ref& ref) {
    ZETA_Core_DebugAssert(0 < ref.width);
    ZETA_Core_DebugAssert(ref.vtable != nullptr);
    ZETA_Core_DebugAssert(ref.cntr != nullptr);

    assoc_cntr::AbilityFlag enabled_ability_flag{
        ref.dynamic_enabled_ability_flag
    };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(ability, method)                                     \
    ZETA_Core_DebugAssert(!TestAbility(enabled_ability_flag, ability) || \
                          ref.vtable->method != nullptr);

    CheckMethod(GetElemCnt, GetElemCnt);
    CheckMethod(GetMaxElemCnt, GetMaxElemCnt);

    CheckMethod(GetLBCursor, GetLBCursor);
    CheckMethod(GetRBCursor, GetRBCursor);
    CheckMethod(PeekL, PeekL);
    CheckMethod(PeekR, PeekR);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Find, FnFind);

    CheckMethod(Insert, FnInsert);

    CheckMethod(PopL, PopL);
    CheckMethod(PopR, PopR);
    CheckMethod(Erase, Erase);
    CheckMethod(EraseAll, EraseAll);

    CheckMethod(CopyCursor, CopyCursor);
    CheckMethod(AreEqualCursor, AreEqualCursor);
    CheckMethod(CompareCursor, CompareCursor);
    CheckMethod(GetCursorDist, GetCursorDist);
    CheckMethod(GetCursorIdx, GetCursorIdx);
    CheckMethod(CursorStepL, CursorStepL);
    CheckMethod(CursorStepR, CursorStepR);
    CheckMethod(CursorAdvanceL, CursorAdvanceL);
    CheckMethod(CursorAdvanceR, CursorAdvanceR);

#pragma pop_macro("CheckMethod")
}

template <typename AssocCntr>
assoc_cntr_ref::Ref
    assoc_cntr_ref::MakeRef  // NOLINT(misc-use-internal-linkage)
    (AssocCntr& cntr) {
    assoc_cntr::CheckContract(cntr);

    return {
        .cursor_size = assoc_cntr::GetCursorSize(cntr),

        .width = assoc_cntr::GetElemSize(cntr),
        .capacity = assoc_cntr::GetMaxElemCnt(cntr),

        .dynamic_enabled_ability_flag =
            assoc_cntr::GetStaticEnabledAbilityFlag<AssocCntr>() |
            assoc_cntr::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            assoc_cntr::GetStaticDisabledAbilityFlag<AssocCntr>() |
            assoc_cntr::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &assoc_cntr::GetVTable<AssocCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(&cntr)),
    };
}

inline void*
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetReferedInstPtr(
    assoc_cntr_ref::Ref const& ref) {
    return ref.cntr;
}

constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Derefer = true,

        .Find = true,
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

constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Ref const>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::CntrTraits<
               assoc_cntr_ref::Ref>::GetStaticEnabledAbilityFlag() &
           assoc_cntr::const_ability_flag;
}

constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::empty_ability_flag;
}

constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Ref const>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::non_const_ability_flag;
}

constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetDynamicEnabledAbilityFlag(
    assoc_cntr_ref::Ref const&) {
    return assoc_cntr::empty_ability_flag;
}

constexpr assoc_cntr::AbilityFlag assoc_cntr::
    CntrTraits<assoc_cntr_ref::Ref const>::GetDynamicDisabledAbilityFlag(
        assoc_cntr_ref::Ref const&) {
    return assoc_cntr::empty_ability_flag;
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetCursorSize(
    assoc_cntr_ref::Ref const& ref) {
    return assoc_cntr_ref::GetCursorSize(ref);
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetElemSize(
    assoc_cntr_ref::Ref const& ref) {
    return assoc_cntr_ref::GetElemSize(ref);
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetElemCnt(
    assoc_cntr_ref::Ref const& ref) {
    return assoc_cntr_ref::GetElemCnt(ref);
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetMaxElemCnt(
    assoc_cntr_ref::Ref const& ref) {
    return assoc_cntr_ref::GetMaxElemCnt(ref);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetLBCursor(
    assoc_cntr_ref::Ref const& ref, void* dst_cursor) {
    assoc_cntr_ref::GetLBCursor(ref, dst_cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetRBCursor(
    assoc_cntr_ref::Ref const& ref, void* dst_cursor) {
    assoc_cntr_ref::GetRBCursor(ref, dst_cursor);
}

inline void* assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::PeekL(
    assoc_cntr_ref::Ref const& ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return assoc_cntr_ref::PeekL(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::PeekR(
    assoc_cntr_ref::Ref const& ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return assoc_cntr_ref::PeekR(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::Derefer(
    assoc_cntr_ref::Ref const& ref, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return assoc_cntr_ref::Derefer(ref, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::Find(
    assoc_cntr_ref::Ref const& ref, void const* key, KeyHash&& key_hash,
    KeyElemCompare&& key_elem_compare, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return assoc_cntr_ref::Find(ref, key, meta::Forward<KeyHash>(key_hash),
                                meta::Forward<KeyElemCompare>(key_elem_compare),
                                lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::Insert(
    assoc_cntr_ref::Ref& ref, void const* elem, void* dst_cursor) {
    return assoc_cntr_ref::Insert(ref, elem, dst_cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::PopL(
    assoc_cntr_ref::Ref& ref, size_t cnt) {
    assoc_cntr_ref::PopL(ref, cnt);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::PopR(
    assoc_cntr_ref::Ref& ref, size_t cnt) {
    assoc_cntr_ref::PopR(ref, cnt);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::Erase(
    assoc_cntr_ref::Ref& ref, void* pos_cursor) {
    assoc_cntr_ref::Erase(ref, pos_cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::EraseAll(
    assoc_cntr_ref::Ref& ref) {
    assoc_cntr_ref::EraseAll(ref);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CopyCursor(
    assoc_cntr_ref::Ref const& ref, void const* src_cursor, void* dst_cursor) {
    assoc_cntr_ref::CopyCursor(ref, src_cursor, dst_cursor);
}

inline bool assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::AreEqualCursor(
    assoc_cntr_ref::Ref const& ref, void const* cursor_a,
    void const* cursor_b) {
    return assoc_cntr_ref::AreEqualCursor(ref, cursor_a, cursor_b);
}

inline int assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CompareCursor(
    assoc_cntr_ref::Ref const& ref, void const* cursor_a,
    void const* cursor_b) {
    return assoc_cntr_ref::CompareCursor(ref, cursor_a, cursor_b);
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetCursorDist(
    assoc_cntr_ref::Ref const& ref, void const* cursor_a,
    void const* cursor_b) {
    return assoc_cntr_ref::GetCursorDist(ref, cursor_a, cursor_b);
}

inline size_t assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::GetCursorIdx(
    assoc_cntr_ref::Ref const& ref, void const* cursor) {
    return assoc_cntr_ref::GetCursorIdx(ref, cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CursorStepL(
    assoc_cntr_ref::Ref const& ref, void* cursor) {
    assoc_cntr_ref::CursorStepL(ref, cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CursorStepR(
    assoc_cntr_ref::Ref const& ref, void* cursor) {
    assoc_cntr_ref::CursorStepR(ref, cursor);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CursorAdvanceL(
    assoc_cntr_ref::Ref const& ref, void* cursor, size_t step) {
    assoc_cntr_ref::CursorAdvanceL(ref, cursor, step);
}

inline void assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>::CursorAdvanceR(
    assoc_cntr_ref::Ref const& ref, void* cursor, size_t step) {
    assoc_cntr_ref::CursorAdvanceR(ref, cursor, step);
}

#pragma pop_macro("TestAbility")

}  // namespace zeta::core
