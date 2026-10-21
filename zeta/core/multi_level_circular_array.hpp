#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/array.ipp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils/revision.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/seq_cntr.hpp>

ZETA_Core_ClangdPreambleBarrier;

namespace zeta::core::multi_level_circular_array {

using Node = basic_llist_node::Node<  //
    void*,                            // LinkType
    false,                            // LColorTag
    false                             // RColorTag
    >;

struct Cursor {
    size_t idx;
    Node* n;
    size_t seg_elem_slot_idx;

#if ZETA_Core_DebugEnable
    mutable debug_utils::revision::Stamp stamp;
#endif

    constexpr Cursor() = default;

    constexpr Cursor(Cursor const& src_cursor) = default;

    constexpr Cursor(seq_cntr::CursorLimit const& src_cursor);

    constexpr operator seq_cntr::CursorLimit(this Cursor const& self);

    constexpr Cursor& operator=(Cursor const& src_cursor) = default;

    constexpr Cursor& operator=(seq_cntr::CursorLimit const& src_cursor);

    constexpr bool operator==(Cursor const&) const = default;
    constexpr bool operator!=(Cursor const&) const = default;
};

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt>
struct DataSeg {
    ZETA_Core_DebugStructPadding;

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    Node n;

    union {
        Elem elems[seg_elem_slot_cnt];
    };

    constexpr DataSeg();
};

/*
template <unsigned max_level_>
struct LayoutConfig {
    static constexpr unsigned max_level{ max_level_ };

    static_assert(0 < max_level);

    size_t seg_elem_slot_cnt;
    array::Array<multi_level_ptr_table::BranchNum, max_level> branch_nums;

    array::Array<size_t, max_level + 1> acc_branch_num;
    array::Array<size_t, max_level + 1> acc_elem_cnt;

    size_t max_seg_cnt;
    size_t max_elem_cnt;

    constexpr LayoutConfig(
        size_t seg_elem_slot_cnt,
        array::Array<multi_level_ptr_table::BranchNum, max_level> branch_nums);
};
*/

using BranchNum = multi_level_ptr_table::BranchNum;

constexpr unsigned level_max_min{ 1 };
constexpr unsigned level_max_max{ multi_level_ptr_table::level_max };

constexpr size_t seg_elem_slot_cnt_min{ 1 };
constexpr size_t seg_elem_slot_cnt_max{ ZETA_Core_max_capacity };

constexpr BranchNum branch_num_min{ 4 };
constexpr BranchNum branch_num_max{ multi_level_ptr_table::branch_num_max };

static_assert(4 <= branch_num_min);
static_assert(multi_level_ptr_table::branch_num_min <= branch_num_min);

template <
    unsigned level_max_, integral::IsUnsignedIntegral ActiveMap_,
    size_t seg_elem_slot_cnt_,
    array::Array<multi_level_ptr_table::BranchNum, level_max_> branch_nums_>
struct LayoutConfig {
    static constexpr unsigned level_max{ level_max_ };

    static_assert(level_max_min <= level_max);
    static_assert(level_max <= level_max_max);

    using ActiveMap = ActiveMap_;

    static_assert(seg_elem_slot_cnt_min <= seg_elem_slot_cnt_);
    static_assert(seg_elem_slot_cnt_ <= seg_elem_slot_cnt_max);

    static constexpr size_t seg_elem_slot_cnt{ seg_elem_slot_cnt_ };

    static constexpr array::Array<multi_level_ptr_table::BranchNum, level_max>
        branch_nums{ branch_nums_ };

    static constexpr array::Array<size_t, level_max + 1> acc_branch_nums{ []() {
        array::Array<size_t, level_max + 1> ret;

        ret[0] = 1;

        for (unsigned level_i{ 1 }; level_i <= level_max; ++level_i) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num_min <=
                                                    branch_nums[level_i - 1]);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                branch_nums[level_i - 1] <= integral::WidthOf<ActiveMap>);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_nums[level_i - 1] <=
                                                    branch_num_max);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                ret[level_i - 1] <=
                ZETA_Core_max_capacity / branch_nums[level_i - 1]);

            ret[level_i] = ret[level_i - 1] * branch_nums[level_i - 1];
        }

        return ret;
    }() };

    static constexpr array::Array<size_t, level_max + 1> acc_elem_cnt{ []() {
        array::Array<size_t, level_max + 1> ret;

        for (unsigned level_i{ 0 }; level_i <= level_max; ++level_i) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(acc_branch_nums[level_i] <=
                                                    ZETA_Core_max_capacity /
                                                        seg_elem_slot_cnt);

            ret[level_i] = seg_elem_slot_cnt * acc_branch_nums[level_i];
        }

        return ret;
    }() };

    static constexpr size_t max_elem_cnt{ acc_elem_cnt[level_max] /
                                          branch_nums[level_max - 1] *
                                          (branch_nums[level_max - 1] - 2) };
};

namespace detail {

template <typename T>
struct IsLayoutConfig_ {
    static constexpr bool value{ false };
};

template <unsigned max_level, integral::IsUnsignedIntegral ActiveMap,
          size_t seg_elem_slot_cnt_,
          array::Array<multi_level_ptr_table::BranchNum, max_level> branch_nums>
struct IsLayoutConfig_<
    LayoutConfig<max_level, ActiveMap, seg_elem_slot_cnt_, branch_nums>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsLayoutConfig = detail::IsLayoutConfig_<T>::value;

template <meta::IsNonConstObj Elem_, IsLayoutConfig auto layout_config_,
          allocator::IsAllocatorLike NodeAllocatorLike_,
          allocator::IsAllocatorLike SegAllocatorLike_>
struct Cntr {
    using Elem = Elem_;

    static constexpr unsigned max_level{ max_level };

    static constexpr auto layout_config{ layout_config_ };

    using NodeAllocatorLike = NodeAllocatorLike_;
    using SegAllocatorLike  = SegAllocatorLike_;

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    size_t seg_elem_slot_cnt;
    size_t tree_elem_offset;
    size_t elem_cnt;

    multi_level_ptr_table::BranchNum rots[multi_level_ptr_table::level_max];

    unsigned level;
    void* root;

    Node* head_n;

    NodeAllocatorLike node_alctr_like;
    SegAllocatorLike seg_alctr_like;

#if ZETA_Core_DebugEnable
    debug_utils::revision::Stamp stamp;
#endif

    template <typename NodeAllocatorLikeConstructArg,
              typename SegAllocatorLikeConstructArg>
    constexpr Cntr(
        size_t seg_slot_cnt,
        NodeAllocatorLikeConstructArg&& node_alctr_like_construct_arg,
        SegAllocatorLikeConstructArg&& seg_alctr_like_construct_arg);

    constexpr ~Cntr();

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr size_t GetSegElemCapacity(this Cntr const& self);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void PeekL(this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void PeekR(this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void Refer(this auto&& self, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void ReferWithHint(
        this auto&& self, size_t idx, bool lazy_copy_elem,
        lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
        DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void Derefer(this auto&& self, seq_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                           DstStorageView dst_elem);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this auto&& self, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this meta::IsNonConstWithoutRef auto&& self,
                             seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                             Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this meta::IsNonConstWithoutRef auto&& self,
                          seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                          Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this meta::IsNonConstWithoutRef auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this meta::IsNonConstWithoutRef auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this meta::IsNonConstWithoutRef auto&& self,
                            seq_cntr::Tag, Acceptor&& acceptor);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void SanityCheck(
        this Cntr const& self,
        debug_utils::mem_recording::MemRecorder& scanned_head_n_recorder,
        debug_utils::mem_recording::MemRecorder& scanned_nav_node_recorder,
        debug_utils::mem_recording::MemRecorder& scanned_data_seg_recorder);
};

}  // namespace zeta::core::multi_level_circular_array
