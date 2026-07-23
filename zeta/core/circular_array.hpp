#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::circular_array {

struct Cntr;

struct Cursor {
    Cntr const* cntr;
    size_t idx;
    void* elem;
};

struct Cntr {
    void* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;
    size_t slot_cnt;
    size_t rot;

    static void* ReferElem(void* data, size_t elem_stride, size_t elem_capacity,
                           size_t slot_offset, size_t idx);

    static size_t GetLongestContSucr(size_t elem_cnt, size_t elem_capacity,
                                     size_t slot_offset, size_t idx);

    static size_t GetLongestContPred(size_t elem_cnt, size_t elem_capacity,
                                     size_t slot_offset, size_t idx);

    template <seq_cntr::IsSeqCntr SrcSeqCntr>
    void AssignFromSeqCntr(this Cntr& cntr, size_t dst_beg,
                           SrcSeqCntr const& src_seq_cntr,
                           void* src_seq_cntr_cursor, size_t cnt);

    void AssignFromCircularArray(this Cntr& cntr, size_t dst_beg,
                                 Cntr const& src_cntr, size_t src_beg,
                                 size_t cnt);

    void Init(this Cntr const& cntr);

    void Deinit(this Cntr& cntr);

    void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    size_t GetElemSize(this Cntr const& cntr);

    size_t GetElemStride(this Cntr const& cntr);

    size_t GetIdxOffset(this Cntr const& cntr);

    size_t GetElemCnt(this Cntr const& cntr);

    size_t GetMaxElemCnt(this Cntr const& cntr);

    void GetLBCursor(this Cntr const& cntr, void* dst_cursor);

    void GetRBCursor(this Cntr const& cntr, void* dst_cursor);

    void PeekL(this auto&& cntr, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void PeekR(this auto&& cntr, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void Refer(this auto&& cntr, size_t idx, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem);

    void Derefer(this auto&& cntr, void const* pos_cursor, bool lazy_copy_elem,
                 seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    template <seq_cntr::IsReader Reader>
    void Read(this Cntr const& cntr, void const* pos_cursor, size_t cnt,
              Reader&& reader, void* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    void Write(this Cntr& cntr, void const* pos_cursor, size_t cnt,
               Writer&& writer, void* dst_cursor);

    template <seq_cntr::IsReaderWriter ReaderWriter>
    void ReadWrite(this Cntr& cntr, void const* pos_cursor, size_t cnt,
                   ReaderWriter&& reader_writer, void* dst_cursor);

    template <seq_cntr::IsReader Reader>
    void IdxRead(this Cntr const& cntr, size_t idx, size_t cnt,
                 Reader&& reader_writer);

    template <seq_cntr::IsWriter Writer>
    void IdxWrite(this Cntr& cntr, size_t idx, size_t cnt,
                  Writer&& reader_writer);

    template <seq_cntr::IsReaderWriter ReaderWriter>
    void IdxReadWrite(this Cntr& cntr, size_t idx, size_t cnt,
                      ReaderWriter&& reader_writer);

    template <seq_cntr::IsWriter Writer>
    void PushL(this Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    void PushR(this Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    void Insert(this Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
                void* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    void IdxInsert(this Cntr& cntr, size_t idx, size_t cnt, Writer&& writer);

    template <seq_cntr::IsReader Reader>
    void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <seq_cntr::IsReader Reader>
    void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <seq_cntr::IsReader Reader>
    void Erase(this Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader);

    template <seq_cntr::IsReader Reader>
    void IdxErase(this Cntr& cntr, size_t idx, size_t cnt, Reader&& reader);

    void EraseAll(this Cntr& cntr);

    void CopyCursor(this Cntr const& cntr, void const* src_cursor,
                    void* dst_cursor);

    bool AreEqualCursor(this Cntr const& cntr, void const* cursor_a,
                        void const* cursor_b);

    int CompareCursor(this Cntr const& cntr, void const* cursor_a,
                      void const* cursor_b);

    size_t GetCursorDist(this Cntr const& cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t GetCursorIdx(this Cntr const& cntr, void const* cursor);

    void CursorStepL(this Cntr const& cntr, void* cursor);

    void CursorStepR(this Cntr const& cntr, void* cursor);

    void CursorAdvanceL(this Cntr const& cntr, void* cursor, size_t step);

    void CursorAdvanceR(this Cntr const& cntr, void* cursor, size_t step);
};

}  // namespace zeta::core::circular_array

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr const>
    : public DefaultCntrTraits<circular_array::Cntr const> {
    static constexpr seq_cntr::CapabilityFlag GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetDynamicEnabledCapabilityFlag(
        circular_array::Cntr const& cntr);

    static constexpr seq_cntr::CapabilityFlag GetDynamicDisabledCapabilityFlag(
        circular_array::Cntr const& cntr);
};

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr>
    : public seq_cntr::CntrTraits<circular_array::Cntr const> {
    static constexpr seq_cntr::CapabilityFlag GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::CapabilityFlag GetStaticDisabledCapabilityFlag();
};

}  // namespace zeta::core
