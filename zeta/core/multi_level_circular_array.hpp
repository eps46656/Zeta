#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/array.ipp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/seq_cntr.hpp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList(suffix)                                  \
    meta::IsValueWrapperT<multi_level_ptr_table::BranchNum>       \
        BranchNumTag##suffix, typename NodeAllocatorLike##suffix, \
        typename SegAllocatorLike##suffix

namespace zeta::core::multi_level_circular_array {

using Node = basic_llist_node::Node<void*,                          // LinkType
                                    meta::AutoValueWrapper<false>,  // LColorTag
                                    meta::AutoValueWrapper<false>   // RColorTag
                                    >;

struct Cursor {
    void const* cntr;
    size_t idx;
    Node* n;
    size_t seg_elem_slot_idx;
    void* elem;

    constexpr bool operator==(Cursor const&) const = default;
    constexpr bool operator!=(Cursor const&) const = default;
};

struct Seg {
    ZETA_Core_DebugStructPadding;

    Node n;

    unsigned char data[] __attribute__((aligned(max_align)));
};

template <multi_level_ptr_table::BranchNum BranchNum_>
struct TableMeta {
    static constexpr multi_level_ptr_table::BranchNum branch_num{ BranchNum_ };

    static constexpr array::Array<multi_level_ptr_table::BranchNum,
                                  multi_level_ptr_table::max_level>
        branch_nums{ []() {
            array::Array<multi_level_ptr_table::BranchNum,
                         multi_level_ptr_table::max_level>
                branch_nums;

            for (unsigned level_i{ 0 };
                 level_i < multi_level_ptr_table::max_level; ++level_i) {
                branch_nums.elems[level_i] = branch_num;
            }

            return branch_nums;
        }() };

    static constexpr array::Array<size_t, multi_level_ptr_table::max_level + 1>
        acc_branch_nums{ []() {
            array::Array<size_t, multi_level_ptr_table::max_level + 1>
                acc_branch_nums;

            acc_branch_nums[0] = 1;

            for (unsigned level_i{ 0 };
                 level_i < multi_level_ptr_table::max_level; ++level_i) {
                acc_branch_nums[level_i + 1] =
                    acc_branch_nums[level_i] * branch_num;
            }

            return acc_branch_nums;
        }() };

    static constexpr size_t max_seg_cnt{ []() {
        size_t ret{ 1 };

        for (unsigned level_i{ 0 }; level_i < multi_level_ptr_table::max_level;
             ++level_i) {
            if (ZETA_Core_max_capacity / ret < branch_num) {
                return ZETA_Core_max_capacity;
            }

            ret *= branch_num;
        }

        return ret;
    }() };

    using ActiveMap = integral_utils::UnsignedFastIntegral<branch_num>;
};

template <CntrTplParamList(_)>
struct Cntr {
    using BranchNumTag = BranchNumTag_;
    using NodeAllocatorLike = NodeAllocatorLike_;
    using SegAllocatorLike = SegAllocatorLike_;

    static constexpr size_t branch_num{
        meta::GetValueWrapperValue<BranchNumTag>
    };

    static constexpr array::Array<multi_level_ptr_table::BranchNum,
                                  multi_level_ptr_table::max_level>
        branch_nums{ TableMeta<branch_num>::branch_nums };

    static constexpr array::Array<size_t, multi_level_ptr_table::max_level + 1>
        acc_branch_nums{ TableMeta<branch_num>::acc_branch_nums };

    static constexpr size_t max_seg_cnt{ TableMeta<branch_num>::max_seg_cnt };

    using ActiveMap = TableMeta<branch_num>::ActiveMap;

    size_t elem_size;
    size_t elem_stride;
    size_t seg_elem_slot_cnt;
    size_t tree_elem_offset;
    size_t elem_cnt;

    multi_level_ptr_table::BranchNum rots[multi_level_ptr_table::max_level];

    unsigned level;
    void* root;

    Node* head_n;

    NodeAllocatorLike node_alctr_like;
    SegAllocatorLike seg_alctr_like;

    template <typename NodeAllocatorLikeInitArg,
              typename SegAllocatorLikeInitArg>
    constexpr Cntr(size_t elem_size, size_t elem_stride, size_t seg_slot_cnt,
                   NodeAllocatorLikeInitArg&& node_alctr_like_init_arg,
                   SegAllocatorLikeInitArg&& seg_alctr_like_init_arg);

    constexpr void Deinit(this Cntr& cntr);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr size_t GetCursorSize(seq_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetElemStride(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetSegElemCapacity(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto&& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto&& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Refer(this auto&& cntr, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void AccessWithHint(this auto&& cntr, size_t idx,
                                  bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto&& cntr, seq_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <seq_cntr::IsReader Reader>
    constexpr void Read(this Cntr const& cntr, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt, Reader& reader,
                        Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void Write(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, Writer& writer, Cursor* dst_cursor);

    template <seq_cntr::IsReaderWriter ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                             size_t cnt, ReaderWriter& reader_writer,
                             Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void PushL(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer& writer, Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void PushR(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer& writer, Cursor* dst_cursor);

    template <seq_cntr::IsWriter Writer>
    constexpr void Insert(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                          size_t cnt, Writer& writer, Cursor* dst_cursor);

    constexpr void PopL(this Cntr& cntr, seq_cntr::Tag, size_t cnt);

    constexpr void PopR(this Cntr& cntr, seq_cntr::Tag, size_t cnt);

    constexpr void Erase(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt);

    constexpr void EraseAll(this Cntr& cntr, seq_cntr::Tag);

    constexpr void CopyCursor(this Cntr const& cntr, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr int CompareCursor(this Cntr const& cntr, seq_cntr::Tag,
                                Cursor const* cursor_a, Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void Sanitize(this Cntr const& cntr,
                            mem_recorder::MemRecorder* dst_node,
                            mem_recorder::MemRecorder* dst_seg);
};

}  // namespace zeta::core::multi_level_circular_array
