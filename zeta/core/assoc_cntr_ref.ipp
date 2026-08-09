#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

#pragma push_macro("TestCapability")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)          \
    (((cap_flag) &                                  \
      (static_cast<assoc_cntr::capability::Flag>(1) \
       << meta::ToUnderlying(assoc_cntr::capability::Kind::cap_name))) != 0)

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, cap_name, method, ...)                      \
    {                                                                       \
        ZETA_Core_DebugAssert(                                              \
            TestCapability(ref.dynamic_enabled_capability_flag, cap_name)); \
                                                                            \
        auto method_ptr{ ref.vtable->method };                              \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                       \
                                                                            \
        return method_ptr(ref.cntr, __VA_ARGS__);                           \
    }                                                                       \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method, ...) \
    CallMethod_(ZETA_Core_TmpName, cap_name, method, __VA_ARGS__)

template <assoc_cntr::IsAssocCntr Cntr>
constexpr assoc_cntr_ref::Ref::Ref(Cntr& cntr) {
    this->Init(cntr);
}

template <assoc_cntr::IsAssocCntr Cntr>
constexpr void assoc_cntr_ref::Ref::Init(this Ref& ref, Cntr& cntr) {
    ref.cursor_size = assoc_cntr::GetCursorSize(cntr);
    ref.elem_size = assoc_cntr::GetElemSize(cntr);
    ref.max_elem_cnt = assoc_cntr::GetMaxElemCnt(cntr);
    ref.dynamic_enabled_capability_flag =
        assoc_cntr::GetStaticEnabledCapabilityFlag<Cntr>() |
        assoc_cntr::GetDynamicEnabledCapabilityFlag(cntr);
    ref.dynamic_disabled_capability_flag =
        assoc_cntr::GetStaticDisabledCapabilityFlag<Cntr>() |
        assoc_cntr::GetDynamicDisabledCapabilityFlag(cntr);
    ref.vtable = &assoc_cntr::GetVTable<Cntr>();
    ref.cntr = const_cast<void*>(static_cast<void const*>(&cntr));
}

constexpr void* assoc_cntr_ref::Ref::GetReferedInstPtr(this Ref const& ref) {
    return ref.cntr;
}

constexpr size_t assoc_cntr_ref::Ref::GetCursorSize(this Ref const& ref) {
    return ref.cursor_size;
}

constexpr size_t assoc_cntr_ref::Ref::GetElemSize(this Ref const& ref) {
    return ref.elem_size;
}

constexpr size_t assoc_cntr_ref::Ref::GetElemCnt(this Ref const& ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

constexpr size_t assoc_cntr_ref::Ref::GetMaxElemCnt(this Ref const& ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

constexpr void assoc_cntr_ref::Ref::GetLBCursor(this Ref const& ref,
                                                void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

constexpr void assoc_cntr_ref::Ref::GetRBCursor(this Ref const& ref,
                                                void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

constexpr void assoc_cntr_ref::Ref::PeekL(
    this Ref const& ref, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void assoc_cntr_ref::Ref::PeekR(
    this Ref const& ref, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void assoc_cntr_ref::Ref::Derefer(
    this Ref const& ref, void* pos_cursor, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void assoc_cntr_ref::Ref::Find(
    this Ref const& ref, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Find, Find_Fn, key, key_hasher, key_elem_cmptr, lazy_copy_elem,
               dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr void* assoc_cntr_ref::Ref::Insert(
    this Ref& ref, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Writer&& writer,
    void* dst_cursor) {
    CallMethod(Insert, Insert_Fn, key, key_hasher, key_elem_cmptr, writer,
               dst_cursor);
}

constexpr void assoc_cntr_ref::Ref::PopL(this Ref& ref, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

constexpr void assoc_cntr_ref::Ref::PopR(this Ref& ref, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

constexpr void assoc_cntr_ref::Ref::Erase(this Ref& ref, void* pos_cursor) {
    CallMethod(Erase, Erase, pos_cursor);
}

constexpr void assoc_cntr_ref::Ref::EraseAll(this Ref& ref) {
    CallMethod(EraseAll, EraseAll);
}

constexpr void assoc_cntr_ref::Ref::CopyCursor(this Ref const& ref,
                                               void* src_cursor,
                                               void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

constexpr bool assoc_cntr_ref::Ref::AreEqualCursor(this Ref const& ref,
                                                   void const* cursor_a,
                                                   void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

constexpr comparison::Ordering assoc_cntr_ref::Ref::CompareCursor(
    this Ref const& ref, void const* cursor_a, void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

constexpr size_t assoc_cntr_ref::Ref::GetCursorDist(this Ref const& ref,
                                                    void const* cursor_a,
                                                    void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

constexpr size_t assoc_cntr_ref::Ref::GetCursorIdx(this Ref const& ref,
                                                   void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

constexpr void assoc_cntr_ref::Ref::CursorStepL(this Ref const& ref,
                                                void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

constexpr void assoc_cntr_ref::Ref::CursorStepR(this Ref const& ref,
                                                void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

constexpr void assoc_cntr_ref::Ref::CursorAdvanceL(this Ref const& ref,
                                                   void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

constexpr void assoc_cntr_ref::Ref::CursorAdvanceR(this Ref const& ref,
                                                   void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

constexpr void assoc_cntr_ref::Ref::Check(this Ref const& ref) {
    ZETA_Core_DebugAssert(0 < ref.elem_size);
    ZETA_Core_DebugAssert(ref.vtable != nullptr);
    ZETA_Core_DebugAssert(ref.cntr != nullptr);

    assoc_cntr::capability::Flag enabled_capability_flag{
        ref.dynamic_enabled_capability_flag
    };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(capability, method)                         \
    ZETA_Core_DebugAssert(                                      \
        !TestCapability(enabled_capability_flag, capability) || \
        ref.vtable->method != nullptr);

    CheckMethod(GetElemCnt, GetElemCnt);
    CheckMethod(GetMaxElemCnt, GetMaxElemCnt);

    CheckMethod(GetLBCursor, GetLBCursor);
    CheckMethod(GetRBCursor, GetRBCursor);
    CheckMethod(PeekL, PeekL);
    CheckMethod(PeekR, PeekR);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Find, Find_Fn);

    CheckMethod(Insert, Insert_Fn);

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

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetDynamicEnabledCapabilityFlag(
    assoc_cntr_ref::Ref& ref) {
    return ref.dynamic_enabled_capability_flag;
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>::GetDynamicDisabledCapabilityFlag(
    assoc_cntr_ref::Ref& ref) {
    return ref.dynamic_disabled_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Ref const>::GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Ref const>::GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::non_const_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::
    CntrTraits<assoc_cntr_ref::Ref const>::GetDynamicEnabledCapabilityFlag(
        assoc_cntr_ref::Ref const& ref) {
    return ref.dynamic_enabled_capability_flag &
           assoc_cntr::capability::const_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::
    CntrTraits<assoc_cntr_ref::Ref const>::GetDynamicDisabledCapabilityFlag(
        assoc_cntr_ref::Ref const& ref) {
    return ref.dynamic_disabled_capability_flag |
           assoc_cntr::capability::non_const_capability_flag;
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
