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
};

constexpr seq_cntr::CapabilityFlag GetEnabledCapabilityFlag(Ref const&);

constexpr seq_cntr::CapabilityFlag GetDisabledCapabilityFlag(Ref&);

constexpr seq_cntr::CapabilityFlag GetDisabledCapabilityFlag(Ref const&);

seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(Ref&);

seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(Ref const&);

seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(Ref&);

seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(Ref const&);

size_t GetCursorSize(Ref const&);

size_t GetElemSize(Ref const& ref);

size_t GetSride(Ref const& ref);

size_t GetOffset(Ref const& ref);

size_t GetElemCnt(Ref const& ref);

size_t GetMaxElemCnt(Ref const& ref);

void GetLBCursor(Ref const& ref, void* dst_cursor);

void GetRBCursor(Ref const& ref, void* dst_cursor);

void PeekL(Ref const& ref, bool lazy_copy_elem,
           seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
           void* dst_elem);

void PeekR(Ref const& ref, bool lazy_copy_elem,
           seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
           void* dst_elem);

void Refer(Ref const& ref, size_t idx, bool lazy_copy_elem,
           seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
           void* dst_elem);

void Derefer(Ref const& ref, void* pos_cursor, bool lazy_copy_elem,
             seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

template <typename Reader>
void Read(Ref const& ref, void* pos_cursor, size_t cnt, Reader&& reader,
          void* dst_cursor);

template <typename Writer>
void Write(Ref& ref, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

template <typename ReaderWriter>
void ReadWrite(Ref& ref, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <typename Writer>
void PushL(Ref& ref, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename Writer>
void PushR(Ref& ref, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename Writer>
void Insert(Ref& ref, void* pos_cursor, size_t cnt, Writer&& writer,
            void* dst_cursor);

template <typename Reader>
void PopL(Ref& ref, size_t cnt, Reader&& reader);

template <typename Reader>
void PopR(Ref& ref, size_t cnt, Reader&& reader);

template <typename Reader>
void Erase(Ref& ref, void* pos_cursor, size_t cnt, Reader&& reader);

void EraseAll(Ref& ref);

void CopyCursor(Ref const& ref, void* src_cursor, void* dst_cursor);

bool AreEqualCursor(Ref const& ref, void* cursor_a, void* cursor_b);

int CompareCursor(Ref const& ref, void* cursor_a, void* cursor_b);

size_t GetCursorDist(Ref const& ref, void* cursor_a, void* cursor_b);

size_t GetCursorIdx(Ref const& ref, void* cursor);

void CursorStepL(Ref const& ref, void* cursor);

void CursorStepR(Ref const& ref, void* cursor);

void CursorAdvanceL(Ref const& ref, void* cursor, size_t step);

void CursorAdvanceR(Ref const& ref, void* cursor, size_t step);

void CheckRef(Ref& ref);

template <typename Cntr>
Ref MakeRef(Cntr& cntr);

}  // namespace zeta::core::seq_cntr_ref

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref const> {
    static void* GetReferedInstPtr(seq_cntr_ref::Ref const& ref);

    static constexpr seq_cntr::CapabilityFlag GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(
        seq_cntr_ref::Ref const& ref);

    static constexpr seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(
        seq_cntr_ref::Ref const& ref);

    static size_t GetCursorSize(seq_cntr_ref::Ref const& ref);

    static size_t GetElemSize(seq_cntr_ref::Ref const& ref);

    static size_t GetElemCnt(seq_cntr_ref::Ref const& ref);

    static size_t GetMaxElemCnt(seq_cntr_ref::Ref const& ref);

    static void GetLBCursor(seq_cntr_ref::Ref const& ref, void* dst_cursor);

    static void GetRBCursor(seq_cntr_ref::Ref const& ref, void* dst_cursor);

    static void PeekL(seq_cntr_ref::Ref const& ref, bool lazy_copy_elem,
                      seq_cntr::ElemPtrView* dst_elem_ptr_view,
                      void* dst_cursor, void* dst_elem);

    static void PeekR(seq_cntr_ref::Ref const& ref, bool lazy_copy_elem,
                      seq_cntr::ElemPtrView* dst_elem_ptr_view,
                      void* dst_cursor, void* dst_elem);

    static void Refer(seq_cntr_ref::Ref const& ref, size_t idx,
                      bool lazy_copy_elem,
                      seq_cntr::ElemPtrView* dst_elem_ptr_view,
                      void* dst_cursor, void* dst_elem);

    static void Derefer(seq_cntr_ref::Ref const& ref, void* pos_cursor,
                        bool lazy_copy_elem,
                        seq_cntr::ElemPtrView* dst_elem_ptr_view,
                        void* dst_elem);

    template <typename Reader>
    static void Read(seq_cntr_ref::Ref const& ref, void* pos_cursor, size_t cnt,
                     Reader&& reader, void* dst_cursor);

    static void CopyCursor(seq_cntr_ref::Ref const& ref, void* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(seq_cntr_ref::Ref const& ref, void* cursor_a,
                               void* cursor_b);

    static int CompareCursor(seq_cntr_ref::Ref const& ref, void* cursor_a,
                             void* cursor_b);

    static size_t GetCursorDist(seq_cntr_ref::Ref const& ref, void* cursor_a,
                                void* cursor_b);

    static size_t GetCursorIdx(seq_cntr_ref::Ref const& ref, void* cursor);

    static void CursorStepL(seq_cntr_ref::Ref const& ref, void* cursor);

    static void CursorStepR(seq_cntr_ref::Ref const& ref, void* cursor);

    static void CursorAdvanceL(seq_cntr_ref::Ref const& ref, void* cursor,
                               size_t step);

    static void CursorAdvanceR(seq_cntr_ref::Ref const& ref, void* cursor,
                               size_t step);
};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref>
    : public seq_cntr::CntrTraits<seq_cntr_ref::Ref const> {
    static constexpr seq_cntr::CapabilityFlag GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetStaticDisabledCapabilityFlag();

    template <typename Writer>
    static void Write(seq_cntr_ref::Ref& ref, void* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(seq_cntr_ref::Ref& ref, void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    static void PushL(seq_cntr_ref::Ref& ref, size_t cnt, Writer&& writer,
                      void* dst_cursor);

    template <typename Writer>
    static void PushR(seq_cntr_ref::Ref& ref, size_t cnt, Writer&& writer,
                      void* dst_cursor);

    template <typename Writer>
    static void Insert(seq_cntr_ref::Ref& ref, void* pos_cursor, size_t cnt,
                       Writer&& writer, void* dst_cursor);

    template <typename Reader>
    static void PopL(seq_cntr_ref::Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    static void PopR(seq_cntr_ref::Ref& ref, size_t cnt, Reader&& reader);

    template <typename Reader>
    static void Erase(seq_cntr_ref::Ref& ref, void* pos_cursor, size_t cnt,
                      Reader&& reader);

    static void EraseAll(seq_cntr_ref::Ref& ref);
};

}  // namespace zeta::core
