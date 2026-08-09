#pragma once

#include <zeta/core/fn_elem_stream.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::seq_cntr_ref {

struct Ref {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    seq_cntr::capability::Flag dynamic_enabled_capability_flag;
    seq_cntr::capability::Flag dynamic_disabled_capability_flag;

    seq_cntr::VTable const* vtable;

    void* cntr;

    constexpr Ref() = default;

    constexpr Ref(Ref const&) = default;

    constexpr Ref(Ref&&) = default;

    template <seq_cntr::IsSeqCntr Cntr>
    constexpr Ref(Cntr& cntr);

    constexpr Ref& operator=(Ref const&) = default;

    constexpr Ref& operator=(Ref&&) = default;

    template <seq_cntr::IsSeqCntr Cntr>
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
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Ref const& ref, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Refer(this Ref const& ref, size_t idx, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Ref const& ref, void* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Ref const& ref, void* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Ref& ref, void* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Ref& ref, void* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Ref& ref, size_t cnt, Writer&& writer,
                         void* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Ref& ref, size_t cnt, Writer&& writer,
                         void* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Ref& ref, void* pos_cursor, size_t cnt,
                          Writer&& writer, void* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    constexpr void Erase(this Ref& ref, void* pos_cursor, size_t cnt,
                         Reader&& reader);

    constexpr void EraseAll(this Ref& ref);

    constexpr void CopyCursor(this Ref const& ref, void* src_cursor,
                              void* dst_cursor);

    constexpr bool AreEqualCursor(this Ref const& ref, void* cursor_a,
                                  void* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Ref const& ref,
                                                 void* cursor_a,
                                                 void* cursor_b);

    constexpr size_t GetCursorDist(this Ref const& ref, void* cursor_a,
                                   void* cursor_b);

    constexpr size_t GetCursorIdx(this Ref const& ref, void* cursor);

    constexpr void CursorStepL(this Ref const& ref, void* cursor);

    constexpr void CursorStepR(this Ref const& ref, void* cursor);

    constexpr void CursorAdvanceL(this Ref const& ref, void* cursor,
                                  size_t step);

    constexpr void CursorAdvanceR(this Ref const& ref, void* cursor,
                                  size_t step);

    constexpr void Check(this Ref const& ref);
};

}  // namespace zeta::core::seq_cntr_ref

namespace zeta::core {

template <>
struct lifecycle::Traits<seq_cntr_ref::Ref>
    : public lifecycle::MemberFuncTraitsAdapter<seq_cntr_ref::Ref> {};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<seq_cntr_ref::Ref, void> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Ref& ref);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(seq_cntr_ref::Ref& ref);
};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref const>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<seq_cntr_ref::Ref const,
                                                   void> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Ref const& ref);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(seq_cntr_ref::Ref const& ref);
};

}  // namespace zeta::core
