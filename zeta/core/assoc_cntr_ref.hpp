#pragma once

#include <zeta/core/assoc_cntr.hpp>

namespace zeta::core::assoc_cntr_ref {

struct Cntr {
    size_t cursor_size;

    size_t elem_size;
    size_t max_elem_cnt;

    assoc_cntr::capability::Flag dynamic_enabled_capability_flag;
    assoc_cntr::capability::Flag dynamic_disabled_capability_flag;

    assoc_cntr::VTable const* vtable;

    void* target_cntr;

    constexpr Cntr() = default;

    constexpr Cntr(Cntr const&) = default;

    constexpr Cntr(Cntr&&) = default;

    template <assoc_cntr::IsAssocCntr TargetCntr>
    constexpr Cntr(TargetCntr& target_cntr);

    constexpr Cntr& operator=(Cntr const&) = default;

    constexpr Cntr& operator=(Cntr&&) = default;

    template <assoc_cntr::IsAssocCntr TargetCntr>
    constexpr void Init(this Cntr& cntr, TargetCntr& target_cntr);

    constexpr void Init(this Cntr& cntr, Cntr const& other_cntr);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const&);

    constexpr size_t GetElemSize(this Cntr const& cntr);

    constexpr size_t GetSride(this Cntr const& cntr);

    constexpr size_t GetOffset(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr);

    constexpr void GetLBCursor(this Cntr const& cntr, void* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, void* dst_cursor);

    constexpr void PeekL(this Cntr const& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Cntr const& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Cntr const& cntr, void* pos_cursor,
                           bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    constexpr void Find(this Cntr const& cntr, void const* elem,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        void* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this Cntr const& cntr, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        void* dst_cursor, void* dst_elem);

    template <assoc_cntr::IsWriter Writer>
    constexpr void* Insert(this Cntr& cntr, void const* elem, Writer&& writer,
                           void* dst_cursor);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Writer>
    constexpr void* Insert(this Cntr& cntr, void const* key,
                           KeyHasher const& key_hasher,
                           KeyElemComparator const& key_elem_cmptr,
                           Writer&& writer, void* dst_cursor);

    template <assoc_cntr::IsReader Reader>
    constexpr void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <assoc_cntr::IsReader Reader>
    constexpr void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <assoc_cntr::IsReader Reader>
    constexpr void Erase(this Cntr& cntr, void* pos_cursor, size_t cnt,
                         Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr);

    constexpr void CopyCursor(this Cntr const& cntr, void* src_cursor,
                              void* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, void* cursor_a,
                                  void* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& cntr,
                                                 void* cursor_a,
                                                 void* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr, void* cursor_a,
                                   void* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, void* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, void* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, void* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, void* cursor,
                                  size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, void* cursor,
                                  size_t step);

    constexpr void Check(this Cntr const& cntr);
};

}  // namespace zeta::core::assoc_cntr_ref
