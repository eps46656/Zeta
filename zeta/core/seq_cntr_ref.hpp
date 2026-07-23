#pragma once

#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::seq_cntr_ref {

struct Ref {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    seq_cntr::CapabilityFlag dynamic_enabled_capability_flag;
    seq_cntr::CapabilityFlag dynamic_disabled_capability_flag;

    seq_cntr::VTable const* vtable;

    void* cntr;

    Ref() = default;

    Ref(Ref const&) = default;

    Ref(Ref&&) = default;

    template <seq_cntr::IsSeqCntr Cntr>
    Ref(Cntr& cntr);

    void* GetReferedInstPtr(this Ref const& ref);

    size_t GetCursorSize(this Ref const&);

    size_t GetElemSize(this Ref const& ref);

    size_t GetSride(this Ref const& ref);

    size_t GetOffset(this Ref const& ref);

    size_t GetElemCnt(this Ref const& ref);

    size_t GetMaxElemCnt(this Ref const& ref);

    void GetLBCursor(this Ref const& ref, void* dst_cursor);

    void GetRBCursor(this Ref const& ref, void* dst_cursor);

    void PeekL(this Ref const& ref, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void PeekR(this Ref const& ref, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void Refer(this Ref const& ref, size_t idx, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void Derefer(this Ref const& ref, void* pos_cursor, bool lazy_copy_elem,
                 seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    template <typename Reader>
    void Read(this Ref const& ref, void* pos_cursor, size_t cnt,
              Reader&& reader, void* dst_cursor);

    template <typename Writer>
    void Write(this Ref& ref, void* pos_cursor, size_t cnt, Writer&& writer,
               void* dst_cursor);

    template <typename ReaderWriter>
    void ReadWrite(this Ref& ref, void* pos_cursor, size_t cnt,
                   ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    void PushL(this Ref& ref, size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename Writer>
    void PushR(this Ref& ref, size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename Writer>
    void Insert(this Ref& ref, void* pos_cursor, size_t cnt, Writer&& writer,
                void* dst_cursor);

    template <typename Reader>
    void PopL(this Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    void PopR(this Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    void Erase(this Ref& ref, void* pos_cursor, size_t cnt, Reader&& reader);

    void EraseAll(this Ref& ref);

    void CopyCursor(this Ref const& ref, void* src_cursor, void* dst_cursor);

    bool AreEqualCursor(this Ref const& ref, void* cursor_a, void* cursor_b);

    int CompareCursor(this Ref const& ref, void* cursor_a, void* cursor_b);

    size_t GetCursorDist(this Ref const& ref, void* cursor_a, void* cursor_b);

    size_t GetCursorIdx(this Ref const& ref, void* cursor);

    void CursorStepL(this Ref const& ref, void* cursor);

    void CursorStepR(this Ref const& ref, void* cursor);

    void CursorAdvanceL(this Ref const& ref, void* cursor, size_t step);

    void CursorAdvanceR(this Ref const& ref, void* cursor, size_t step);

    void Check(this Ref const& ref);
};

}  // namespace zeta::core::seq_cntr_ref

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref const>
    : public seq_cntr::DefaultCntrTraits<seq_cntr_ref::Ref> {
    static constexpr seq_cntr::CapabilityFlag GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Ref const& ref);

    static constexpr seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(
        seq_cntr_ref::Ref const& ref);
};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref>
    : public seq_cntr::CntrTraits<seq_cntr_ref::Ref const> {
    static constexpr seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Ref& ref);

    static constexpr seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(
        seq_cntr_ref::Ref& ref);
};

}  // namespace zeta::core
