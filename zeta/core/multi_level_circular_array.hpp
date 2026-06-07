#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/array.ipp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDefParamList")
#define CntrTplDefParamList                              \
    typename BranchNumTag_, typename NodeAllocatorLike_, \
        typename SegAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename BranchNumTag, typename NodeAllocatorLike, typename SegAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList BranchNumTag, NodeAllocatorLike, SegAllocatorLike

namespace zeta::core::multi_level_circular_array {

using Node = basic_llist_node::Node<void*,                     // LinkType
                                    value_wrapper::FalseType,  // LColorTag
                                    value_wrapper::FalseType   // RColorTag
                                    >;

struct Cursor {
    void const* cntr;
    size_t idx;
    Node* n;
    size_t seg_elem_slot_idx;
    void* elem;

    bool operator==(Cursor const&) const = default;
    bool operator!=(Cursor const&) const = default;
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

template <CntrTplDefParamList>
struct Cntr {
    using BranchNumTag = BranchNumTag_;
    using NodeAllocatorLike = NodeAllocatorLike_;
    using SegAllocatorLike = SegAllocatorLike_;

    ZETA_Core_StaticAssert(value_wrapper::IsStaticValueWrapper<
                           BranchNumTag, multi_level_ptr_table::BranchNum>);

    static constexpr size_t branch_num{ BranchNumTag::value };

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
    size_t elem_offset;
    size_t elem_cnt;

    multi_level_ptr_table::BranchNum rots[multi_level_ptr_table::max_level];

    unsigned level;
    void* root;

    Node* head_n;

    NodeAllocatorLike node_alctr;
    SegAllocatorLike seg_alctr;
};

template <CntrTplParamList, typename NodeAllocatorInitArg,
          typename SegAllocatorInitArg>
void Init(Cntr<CntrTplArgList>& cntr, size_t elem_width, size_t elem_stride,
          size_t seg_slot_cnt, NodeAllocatorInitArg&& node_alctr_init_arg,
          SegAllocatorInitArg&& seg_alctr_init_arg);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
constexpr size_t GetCursorSize(Cntr<CntrTplArgList> const&);

template <CntrTplParamList>
size_t GetElemSize(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetElemStride(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetSegElemCapacity(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Access(Cntr<CntrTplArgList> const& cntr, size_t idx, bool lazy_copy_elem,
             Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* AccessWithHint(Cntr<CntrTplArgList> const& cntr, size_t idx,
                     bool lazy_copy_elem, Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename Reader>
void Read(Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor,
          size_t cnt, Reader&& reader, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
static void Write(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
                  Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList, typename ReaderWriter>
void ReadWrite(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushL(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushR(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* Insert(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* src_cursor,
                Cursor* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplParamList>
int CompareCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor);

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const& cntr,
              mem_recorder::MemRecorder* dst_node,
              mem_recorder::MemRecorder* dst_seg);

}  // namespace zeta::core::multi_level_circular_array

namespace zeta::core {

template <CntrTplParamList>
struct lifecycle::Traits<multi_level_circular_array::Cntr<CntrTplArgList>> {
    template <typename... Args>
    static void Init(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                     Args&&... args);

    static void Deinit(multi_level_circular_array::Cntr<CntrTplArgList>& cntr);
};

template <CntrTplParamList>
struct seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const> {
    static void* GetReferedInstPtr(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static size_t GetCursorSize(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static size_t GetElemSize(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static size_t GetElemCnt(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static size_t GetMaxElemCnt(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr);

    static void GetLBCursor(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* dst_cursor);

    static void GetRBCursor(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* dst_cursor);

    static void* PeekL(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Access(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        size_t idx, bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* pos_cursor, bool lazy_copy_elem, void* dst_elem);

    template <typename Reader>
    static void Read(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* pos_cursor, size_t cnt, Reader&& reader, void* dst_cursor);

    static void CopyCursor(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* cursor_a, void const* cursor_b);

    static int CompareCursor(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* cursor);

    static void CursorStepL(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* cursor);

    static void CursorStepR(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* cursor);

    static void CursorAdvanceL(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* cursor, size_t step);

    static void CursorAdvanceR(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void* cursor, size_t step);
};

template <CntrTplParamList>
struct seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>
    : public seq_cntr::CntrTraits<
          multi_level_circular_array::Cntr<CntrTplArgList> const> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                      void* pos_cursor, size_t cnt, Writer&& writer,
                      void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(
        multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
        void* pos_cursor, size_t cnt, ReaderWriter&& reader_writer,
        void* dst_cursor);

    template <typename Writer>
    static void* PushL(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                       size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* PushR(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                       size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* Insert(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                        void* pos_cursor, size_t cnt, Writer&& writer,
                        void* dst_cursor);

    static void PopL(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                     size_t cnt);

    static void PopR(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                     size_t cnt);

    static void Erase(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
                      void* pos_cursor, size_t cnt);

    static void EraseAll(
        multi_level_circular_array::Cntr<CntrTplArgList>& cntr);
};

}  // namespace zeta::core
