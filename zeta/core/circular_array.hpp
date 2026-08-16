#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::circular_array {

struct Cntr;

struct Cursor {
    Cntr const* cntr;
    size_t idx;
    void* elem;
};

constexpr void* ReferElem(void* data, size_t elem_stride, size_t elem_capacity,
                          size_t slot_offset, size_t idx);

constexpr size_t GetLongestContSucr(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

constexpr size_t GetLongestContPred(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

struct Cntr {
    void* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;
    size_t slot_cnt;
    size_t rot;

    constexpr void Init(this Cntr const& cntr);

    constexpr void Deinit(this Cntr& cntr);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    constexpr size_t GetElemSize(this Cntr const& cntr);

    constexpr size_t GetElemStride(this Cntr const& cntr);

    constexpr size_t GetIdxOffset(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr);

    constexpr void GetLBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void PeekL(this auto&& cntr, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto&& cntr, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Refer(this auto&& cntr, size_t idx, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto&& cntr, Cursor const* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <seq_cntr::IsReader Reader>
    constexpr void Read(this Cntr const& cntr, Cursor const* pos_cursor,
                        size_t cnt, Reader&& reader, Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void Write(this Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <seq_cntr::IsReaderWriter ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, Cursor const* pos_cursor,
                             size_t cnt, ReaderWriter&& reader_writer,
                             Cursor* dst_cursor);

    template <seq_cntr::IsReader Reader>
    constexpr void IdxRead(this Cntr const& cntr, size_t idx, size_t cnt,
                           Reader&& reader_writer);

    template <seq_cntr::IsWriter Writer>
    constexpr void IdxWrite(this Cntr& cntr, size_t idx, size_t cnt,
                            Writer&& reader_writer);

    template <seq_cntr::IsReaderWriter ReaderWriter>
    constexpr void IdxReadWrite(this Cntr& cntr, size_t idx, size_t cnt,
                                ReaderWriter&& reader_writer);

    template <seq_cntr::IsWriter Writer>
    constexpr void PushL(this Cntr& cntr, size_t cnt, Writer&& writer,
                         Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void PushR(this Cntr& cntr, size_t cnt, Writer&& writer,
                         Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void Insert(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                          Writer&& writer, Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void IdxInsert(this Cntr& cntr, size_t idx, size_t cnt,
                             Writer&& writer);

    template <seq_cntr::IsReader Reader>
    constexpr void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <seq_cntr::IsReader Reader>
    constexpr void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <seq_cntr::IsReader Reader>
    constexpr void Erase(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                         Reader&& reader);

    template <seq_cntr::IsReader Reader>
    constexpr void IdxErase(this Cntr& cntr, size_t idx, size_t cnt,
                            Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr);

    constexpr void CopyCursor(this Cntr const& cntr, void const* src_cursor,
                              Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& cntr,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, Cursor* cursor,
                                  size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, Cursor* cursor,
                                  size_t step);

    template <seq_cntr::IsSeqCntr SrcSeqCntr>
    constexpr void AssignFromSeqCntr(this Cntr& cntr, size_t dst_beg,
                                     SrcSeqCntr const& src_seq_cntr,
                                     void* src_seq_cntr_cursor, size_t cnt);

    constexpr void AssignFromCircularArray(this Cntr& cntr, size_t dst_beg,
                                           Cntr const& src_cntr, size_t src_beg,
                                           size_t cnt);
};

}  // namespace zeta::core::circular_array

namespace zeta::core {

template <>
struct lifecycle::Traits<circular_array::Cntr>
    : public lifecycle::MemberFuncTraitsAdapter<circular_array::Cntr> {};

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr>
    : public MemberFuncCntrTraitsAdapter<circular_array::Cntr,
                                         circular_array::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        circular_array::Cntr&);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(circular_array::Cntr&);
};

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr const>
    : public MemberFuncCntrTraitsAdapter<circular_array::Cntr const,
                                         circular_array::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        circular_array::Cntr const& cntr);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(circular_array::Cntr const& cntr);
};

}  // namespace zeta::core
