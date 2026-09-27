#pragma once

#include <zeta/core/debug_utils/sanity.hpp>
#include <zeta/core/poly_seq_endpoint.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::poly_seq_cntr {

struct Cntr {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    seq_cntr::capability::Flag dynamic_enabled_capability_flag;
    seq_cntr::capability::Flag dynamic_disabled_capability_flag;

    seq_cntr::VTable const* vtable;

    void* target_cntr;

    constexpr Cntr();

    constexpr Cntr(Cntr&) = default;

    constexpr Cntr(Cntr const&) = default;

    constexpr Cntr(Cntr&&) = default;

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr Cntr(TargetCntr& target_cntr);

    constexpr ~Cntr();

    constexpr Cntr& operator=(Cntr const&) = default;

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr void Set(this Cntr& self, TargetCntr& target_cntr);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        this Cntr& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicDisabledCapabilityFlag(
        this Cntr& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicDisabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<void> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<void> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    constexpr size_t GetCursorSize(this Cntr const&, seq_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetSride(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetOffset(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               void* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               void* dst_cursor);

    constexpr void PeekL(this Cntr const& self, seq_cntr::Tag,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Cntr const& self, seq_cntr::Tag,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Refer(this Cntr const& self, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Cntr const& self, seq_cntr::Tag,
                           void* pos_cursor, bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Cntr const& self, seq_cntr::Tag, void* pos_cursor,
                        size_t cnt, Reader&& reader, void* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Cntr& self, seq_cntr::Tag, void* pos_cursor,
                         size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Cntr& self, seq_cntr::Tag, void* pos_cursor,
                             size_t cnt, ReaderWriter&& reader_writer,
                             void* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Cntr& self, seq_cntr::Tag, void* pos_cursor,
                          size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void Erase(this Cntr& self, seq_cntr::Tag, void* pos_cursor,
                         size_t cnt, Reader&& reader);

    constexpr void EraseAll(this Cntr& self, seq_cntr::Tag);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              void* src_cursor, void* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  void* cursor_a, void* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag, void* cursor_a,
                                                 void* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   void* cursor_a, void* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  void* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               void* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               void* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  void* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  void* cursor, size_t step);

    constexpr void Check(this Cntr const& self);

    static constexpr void SanityCheck(
        void const* self, debug_utils::sanity::SanityCheckScope scope);
};

}  // namespace zeta::core::poly_seq_cntr
