#pragma once

#include <zeta/core/elem_stream_ref.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::seq_cntr_ref {

struct Cntr {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    seq_cntr::capability::Flag dynamic_enabled_capability_flag;
    seq_cntr::capability::Flag dynamic_disabled_capability_flag;

    seq_cntr::VTable const* vtable;

    void* target_cntr;

    constexpr Cntr() = default;

    constexpr Cntr(Cntr const&) = default;

    constexpr Cntr(Cntr&&) = default;

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr Cntr(TargetCntr& target_cntr);

    constexpr void Init(this Cntr& cntr, Cntr const& other_cntr);

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr void Init(this Cntr& cntr, TargetCntr& target_cntr);

    constexpr Cntr& operator=(Cntr const&) = default;

    constexpr void Set(this Cntr& cntr, Cntr const& other_cntr);

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr void Set(this Cntr& cntr, TargetCntr& target_cntr);

    constexpr void Deinit(this Cntr& cntr);

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
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Cntr const& cntr, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Refer(this Cntr const& cntr, size_t idx, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         void* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Cntr const& cntr, void* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Cntr const& cntr, void* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Cntr& cntr, void* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, void* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Cntr& cntr, size_t cnt, Writer&& writer,
                         void* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Cntr& cntr, size_t cnt, Writer&& writer,
                         void* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Cntr& cntr, void* pos_cursor, size_t cnt,
                          Writer&& writer, void* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
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

}  // namespace zeta::core::seq_cntr_ref

namespace zeta::core {

template <>
struct lifecycle::Traits<seq_cntr_ref::Cntr>
    : public lifecycle::MemberFuncTraitsAdapter<seq_cntr_ref::Cntr> {};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Cntr>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<seq_cntr_ref::Cntr, void> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Cntr& ref);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(seq_cntr_ref::Cntr& ref);
};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Cntr const>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<seq_cntr_ref::Cntr const,
                                                   void> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Cntr const& ref);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(seq_cntr_ref::Cntr const& ref);
};

}  // namespace zeta::core
