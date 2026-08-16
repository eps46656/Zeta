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
#define CallMethod_(method_ptr, cap_name, method, ...)                       \
    {                                                                        \
        ZETA_Core_DebugAssert(                                               \
            TestCapability(cntr.dynamic_enabled_capability_flag, cap_name)); \
                                                                             \
        auto method_ptr{ cntr.vtable->method };                              \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                        \
                                                                             \
        return method_ptr(cntr.target_cntr, __VA_ARGS__);                    \
    }                                                                        \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method, ...) \
    CallMethod_(ZETA_Core_TmpName, cap_name, method, __VA_ARGS__)

template <assoc_cntr::IsAssocCntr TargetCntr>
constexpr assoc_cntr_ref::Cntr::Cntr(TargetCntr& target_cntr) {
    this->Init(target_cntr);
}

template <assoc_cntr::IsAssocCntr TargetCntr>
constexpr void assoc_cntr_ref::Cntr::Init(this Cntr& cntr,
                                          TargetCntr& target_cntr) {
    cntr.cursor_size = assoc_cntr::GetCursorSize(target_cntr);
    cntr.elem_size = assoc_cntr::GetElemSize(target_cntr);
    cntr.max_elem_cnt = assoc_cntr::GetMaxElemCnt(target_cntr);
    cntr.dynamic_enabled_capability_flag =
        assoc_cntr::GetStaticEnabledCapabilityFlag<TargetCntr>() |
        assoc_cntr::GetDynamicEnabledCapabilityFlag(target_cntr);
    cntr.dynamic_disabled_capability_flag =
        assoc_cntr::GetStaticDisabledCapabilityFlag<TargetCntr>() |
        assoc_cntr::GetDynamicDisabledCapabilityFlag(target_cntr);
    cntr.vtable = &assoc_cntr::GetVTable<TargetCntr>();
    cntr.target_cntr =
        const_cast<void*>(static_cast<void const*>(&target_cntr));
}

constexpr void assoc_cntr_ref::Cntr::Init(this Cntr& cntr,
                                          Cntr const& other_cntr) {
    cntr = other_cntr;
}

constexpr void* assoc_cntr_ref::Cntr::GetReferedInstPtr(this Cntr const& cntr) {
    return cntr.target_cntr;
}

constexpr size_t assoc_cntr_ref::Cntr::GetCursorSize(this Cntr const& cntr) {
    return cntr.cursor_size;
}

constexpr size_t assoc_cntr_ref::Cntr::GetElemSize(this Cntr const& cntr) {
    return cntr.elem_size;
}

constexpr size_t assoc_cntr_ref::Cntr::GetElemCnt(this Cntr const& cntr) {
    CallMethod(GetElemCnt, get_elem_cnt);
}

constexpr size_t assoc_cntr_ref::Cntr::GetMaxElemCnt(this Cntr const& cntr) {
    CallMethod(GetMaxElemCnt, get_max_elem_cnt);
}

constexpr void assoc_cntr_ref::Cntr::GetLBCursor(this Cntr const& cntr,
                                                 void* dst_cursor) {
    CallMethod(GetLBCursor, get_lb_cursor, dst_cursor);
}

constexpr void assoc_cntr_ref::Cntr::GetRBCursor(this Cntr const& cntr,
                                                 void* dst_cursor) {
    CallMethod(GetRBCursor, get_rb_cursor, dst_cursor);
}

constexpr void assoc_cntr_ref::Cntr::PeekL(
    this Cntr const& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekL, peek_l, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void assoc_cntr_ref::Cntr::PeekR(
    this Cntr const& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekR, peek_r, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void assoc_cntr_ref::Cntr::Derefer(
    this Cntr const& cntr, void* pos_cursor, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(Derefer, derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

constexpr void assoc_cntr_ref::Cntr::Find(
    this Cntr const& cntr, void const* elem, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Find, find.with_elem.fn, elem, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void assoc_cntr_ref::Cntr::Find(
    this Cntr const& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Find, find.with_key.fn, key, key_hasher, key_elem_cmptr,
               lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <assoc_cntr::IsWriter Writer>
constexpr void* assoc_cntr_ref::Cntr::Insert(this Cntr& cntr, void const* elem,
                                             Writer&& writer,
                                             void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, assoc_cntr::EmptyWriter>) {
        CallMethod(Insert, insert.with_elem.empty, elem, writer, dst_cursor);
    } else {
        CallMethod(Insert, insert.with_elem.fn, elem, writer, dst_cursor);
    }
}

template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr void* assoc_cntr_ref::Cntr::Insert(
    this Cntr& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Writer&& writer,
    void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, assoc_cntr::EmptyWriter>) {
        CallMethod(Insert, insert.with_key.empty, key, key_hasher,
                   key_elem_cmptr, writer, dst_cursor);
    } else {
        CallMethod(Insert, insert.with_key.fn, key, key_hasher, key_elem_cmptr,
                   writer, dst_cursor);
    }
}

template <assoc_cntr::IsReader Reader>
constexpr void assoc_cntr_ref::Cntr::PopL(this Cntr& cntr, size_t cnt,
                                          Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, assoc_cntr::EmptyReader>) {
        CallMethod(PopL, pop_l.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, assoc_cntr::LinSeqReader>) {
        CallMethod(PopL, pop_l.lin_seq, cnt, reader);
    } else {
        CallMethod(PopL, pop_l.fn, cnt, reader);
    }
}

template <assoc_cntr::IsReader Reader>
constexpr void assoc_cntr_ref::Cntr::PopR(this Cntr& cntr, size_t cnt,
                                          Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, assoc_cntr::EmptyReader>) {
        CallMethod(PopR, pop_l.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, assoc_cntr::LinSeqReader>) {
        CallMethod(PopR, pop_l.lin_seq, cnt, reader);
    } else {
        CallMethod(PopR, pop_l.fn, cnt, reader);
    }
}

template <assoc_cntr::IsReader Reader>
constexpr void assoc_cntr_ref::Cntr::Erase(this Cntr& cntr, void* pos_cursor,
                                           size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, assoc_cntr::EmptyReader>) {
        CallMethod(Erase, erase.empty, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, assoc_cntr::LinSeqReader>) {
        CallMethod(Erase, erase.lin_seq, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, erase.fn, pos_cursor, cnt, reader);
    }
}

constexpr void assoc_cntr_ref::Cntr::EraseAll(this Cntr& cntr) {
    CallMethod(EraseAll, erase_all);
}

constexpr void assoc_cntr_ref::Cntr::CopyCursor(this Cntr const& cntr,
                                                void* src_cursor,
                                                void* dst_cursor) {
    CallMethod(CopyCursor, copy_cursor, src_cursor, dst_cursor);
}

constexpr bool assoc_cntr_ref::Cntr::AreEqualCursor(this Cntr const& cntr,
                                                    void* cursor_a,
                                                    void* cursor_b) {
    CallMethod(AreEqualCursor, are_equal_cursor, cursor_a, cursor_b);
}

constexpr comparison::Ordering assoc_cntr_ref::Cntr::CompareCursor(
    this Cntr const& cntr, void* cursor_a, void* cursor_b) {
    CallMethod(CompareCursor, compare_cursor, cursor_a, cursor_b);
}

constexpr size_t assoc_cntr_ref::Cntr::GetCursorDist(this Cntr const& cntr,
                                                     void* cursor_a,
                                                     void* cursor_b) {
    CallMethod(GetCursorDist, get_cursor_dist, cursor_a, cursor_b);
}

constexpr size_t assoc_cntr_ref::Cntr::GetCursorIdx(this Cntr const& cntr,
                                                    void* cursor) {
    CallMethod(GetCursorIdx, get_cursor_idx, cursor);
}

constexpr void assoc_cntr_ref::Cntr::CursorStepL(this Cntr const& cntr,
                                                 void* cursor) {
    CallMethod(CursorStepL, cursor_step_l, cursor);
}

constexpr void assoc_cntr_ref::Cntr::CursorStepR(this Cntr const& cntr,
                                                 void* cursor) {
    CallMethod(CursorStepR, cursor_step_r, cursor);
}

constexpr void assoc_cntr_ref::Cntr::CursorAdvanceL(this Cntr const& cntr,
                                                    void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, cursor_advance_l, cursor, step);
}

constexpr void assoc_cntr_ref::Cntr::CursorAdvanceR(this Cntr const& cntr,
                                                    void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, cursor_advance_r, cursor, step);
}

#pragma pop_macro("CallMethod")

constexpr void assoc_cntr_ref::Cntr::Check(this Cntr const& cntr) {
    ZETA_Core_DebugAssert(0 < cntr.elem_size);
    ZETA_Core_DebugAssert(cntr.vtable != nullptr);
    ZETA_Core_DebugAssert(cntr.target_cntr != nullptr);

    assoc_cntr::capability::Flag enabled_capability_flag{
        cntr.dynamic_enabled_capability_flag
    };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(capability, method)                         \
    ZETA_Core_DebugAssert(                                      \
        !TestCapability(enabled_capability_flag, capability) || \
        cntr.vtable->method != nullptr);

    CheckMethod(GetElemCnt, get_elem_cnt);
    CheckMethod(GetMaxElemCnt, get_max_elem_cnt);

    CheckMethod(GetLBCursor, get_lb_cursor);
    CheckMethod(GetRBCursor, get_rb_cursor);
    CheckMethod(PeekL, peek_l);
    CheckMethod(PeekR, peek_r);
    CheckMethod(Derefer, derefer);

    CheckMethod(Find, find.with_elem.fn);
    CheckMethod(Find, find.with_key.fn);

    CheckMethod(Insert, insert.with_elem.empty);
    CheckMethod(Insert, insert.with_elem.fn);
    CheckMethod(Insert, insert.with_key.empty);
    CheckMethod(Insert, insert.with_key.fn);

    CheckMethod(PopL, pop_l.empty);
    CheckMethod(PopL, pop_l.lin_seq);
    CheckMethod(PopL, pop_l.fn);

    CheckMethod(PopR, pop_r.empty);
    CheckMethod(PopR, pop_r.lin_seq);
    CheckMethod(PopR, pop_r.fn);

    CheckMethod(Erase, erase.empty);
    CheckMethod(Erase, erase.lin_seq);
    CheckMethod(Erase, erase.fn);

    CheckMethod(EraseAll, erase_all);

    CheckMethod(CopyCursor, copy_cursor);
    CheckMethod(AreEqualCursor, are_equal_cursor);
    CheckMethod(CompareCursor, compare_cursor);
    CheckMethod(GetCursorDist, get_cursor_dist);
    CheckMethod(GetCursorIdx, get_cursor_idx);
    CheckMethod(CursorStepL, cursor_step_l);
    CheckMethod(CursorStepR, cursor_step_r);
    CheckMethod(CursorAdvanceL, cursor_advance_l);
    CheckMethod(CursorAdvanceR, cursor_advance_r);

#pragma pop_macro("CheckMethod")
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Cntr>::GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Cntr>::GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Cntr>::GetDynamicEnabledCapabilityFlag(
    assoc_cntr_ref::Cntr& cntr) {
    return cntr.dynamic_enabled_capability_flag;
}

constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<assoc_cntr_ref::Cntr>::GetDynamicDisabledCapabilityFlag(
    assoc_cntr_ref::Cntr& cntr) {
    return cntr.dynamic_disabled_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Cntr const>::GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::empty_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    assoc_cntr_ref::Cntr const>::GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::non_const_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::
    CntrTraits<assoc_cntr_ref::Cntr const>::GetDynamicEnabledCapabilityFlag(
        assoc_cntr_ref::Cntr const& cntr) {
    return cntr.dynamic_enabled_capability_flag &
           assoc_cntr::capability::const_capability_flag;
}

constexpr assoc_cntr::capability::Flag assoc_cntr::
    CntrTraits<assoc_cntr_ref::Cntr const>::GetDynamicDisabledCapabilityFlag(
        assoc_cntr_ref::Cntr const& cntr) {
    return cntr.dynamic_disabled_capability_flag |
           assoc_cntr::capability::non_const_capability_flag;
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
