#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core::assoc_cntr_ref {

struct Ref {
    size_t cursor_size;

    size_t elem_size;
    size_t max_elem_cnt;

    assoc_cntr::capability::Flag dynamic_enabled_capability_flag;
    assoc_cntr::capability::Flag dynamic_disabled_capability_flag;

    assoc_cntr::VTable const* vtable;

    void* cntr;

    constexpr Ref() = default;

    constexpr Ref(Ref const&) = default;

    constexpr Ref(Ref&&) = default;

    template <assoc_cntr::IsAssocCntr Cntr>
    constexpr Ref(Cntr& cntr);

    constexpr Ref& operator=(Ref const&) = default;

    constexpr Ref& operator=(Ref&&) = default;

    template <assoc_cntr::IsAssocCntr Cntr>
    constexpr void Init(this Ref& ref, Cntr& cntr);

    constexpr void* GetReferedInstPtr(this Ref const& ref);

    constexpr size_t GetCursorSize(this Ref const&);

    constexpr size_t GetElemSize(this Ref const& ref);

    constexpr size_t GetSride(this Ref const& ref);

    constexpr size_t GetOffset(this Ref const& ref);

    constexpr size_t GetElemCnt(this Ref const& ref);

    constexpr size_t GetMaxElemCnt(this Ref const& ref);

    constexpr void GetLBCursor(this Ref const& ref, void* dst_cursor);

    constexpr void GetRBCursor(this Ref const& ref, void* dst_cursor);

    constexpr void PeekL(this Ref const& ref, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Ref const& ref, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Ref const& ref, void* pos_cursor,
                           bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this Ref const& ref, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        void* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Writer>
    constexpr void* Insert(this Ref& ref, void const* key,
                           KeyHasher const& key_hasher,
                           KeyElemComparator const& key_elem_cmptr,
                           Writer&& writer, void* dst_cursor);

    constexpr void PopL(this Ref& ref, size_t cnt);

    constexpr void PopR(this Ref& ref, size_t cnt);

    constexpr void Erase(this Ref& ref, void* pos_cursor);

    constexpr void EraseAll(this Ref& ref);

    constexpr void CopyCursor(this Ref const& ref, void* src_cursor,
                              void* dst_cursor);

    constexpr bool AreEqualCursor(this Ref const& ref, void const* cursor_a,
                                  void const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Ref const& ref,
                                                 void const* cursor_a,
                                                 void const* cursor_b);

    constexpr size_t GetCursorDist(this Ref const& ref, void const* cursor_a,
                                   void const* cursor_b);

    constexpr size_t GetCursorIdx(this Ref const& ref, void const* cursor);

    constexpr void CursorStepL(this Ref const& ref, void* cursor);

    constexpr void CursorStepR(this Ref const& ref, void* cursor);

    constexpr void CursorAdvanceL(this Ref const& ref, void* cursor,
                                  size_t step);

    constexpr void CursorAdvanceR(this Ref const& ref, void* cursor,
                                  size_t step);

    constexpr void Check(this Ref const& ref);
};

}  // namespace zeta::core::assoc_cntr_ref

namespace zeta::core {

template <>
struct lifecycle::Traits<assoc_cntr_ref::Ref>
    : public lifecycle::MemberFuncTraitsAdapter<assoc_cntr_ref::Ref> {};

template <>
struct assoc_cntr::CntrTraits<assoc_cntr_ref::Ref>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<assoc_cntr_ref::Ref,
                                                     void> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(assoc_cntr_ref::Ref& ref);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(assoc_cntr_ref::Ref& ref);
};

template <>
struct assoc_cntr::CntrTraits<assoc_cntr_ref::Ref const>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<assoc_cntr_ref::Ref const,
                                                     void> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(assoc_cntr_ref::Ref const& ref);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(assoc_cntr_ref::Ref const& ref);
};

}  // namespace zeta::core
