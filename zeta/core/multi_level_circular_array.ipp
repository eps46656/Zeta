#pragma once

#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/llist.ipp>
#include <zeta/core/multi_level_circular_array.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                                  \
    meta::IsValueWrapperT<multi_level_ptr_table::BranchNum> BranchNumTag, \
        typename NodeAllocatorLike, typename SegAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList BranchNumTag, NodeAllocatorLike, SegAllocatorLike

namespace zeta::core {

namespace multi_level_circular_array::detail {

template <multi_level_ptr_table::BranchNum BranchNum>
constexpr size_t GetMaxElemCnt_(size_t seg_elem_capacity) {
    constexpr size_t max_seg_cnt{ TableMeta<BranchNum>::max_seg_cnt };

    return seg_elem_capacity <= ZETA_Core_max_capacity / max_seg_cnt
               ? seg_elem_capacity * max_seg_cnt
               : ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t elem_offset{ cntr.tree_elem_offset };

    size_t level{ cntr.level };
    void* root{ cntr.root };

    Node* head_n{ cntr.head_n };

    constexpr auto acc_branch_nums{
        TableMeta<Cntr<CntrTplArgList>::branch_num>::acc_branch_nums
    };

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(
        elem_cnt <=
        (GetMaxElemCnt_<Cntr<CntrTplArgList>::branch_num>)(seg_elem_slot_cnt));
    ZETA_Core_DebugAssert((elem_cnt == 0) == (root == nullptr));
    ZETA_Core_DebugAssert(elem_offset <
                          seg_elem_slot_cnt * acc_branch_nums[level - 1]);
    ZETA_Core_DebugAssert(elem_cnt <=
                          seg_elem_slot_cnt * acc_branch_nums[level]);
    ZETA_Core_DebugAssert(elem_offset + elem_cnt <=
                          seg_elem_slot_cnt * acc_branch_nums[level]);
    ZETA_Core_DebugAssert(head_n != nullptr);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(cntr.rots[level_i] < branch_num);
    }
}

template <CntrTplParamList>
constexpr void CheckCursor_(Cntr<CntrTplArgList> const& cntr,
                            Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    Cursor re_cursor;
    cntr.Refer(cursor->idx, true, nullptr, &re_cursor, nullptr);

    ZETA_Core_DebugAssert(*cursor == re_cursor);
}

constexpr Seg* NToSeg_(Node* n) { return ZETA_Core_MemberToStruct(Seg, n, n); }

constexpr Seg const* NToSeg_(Node const* n) {
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

constexpr size_t GetSegSize_(size_t elem_stride, size_t seg_elem_slot_cnt) {
    return offsetof(Seg, data[elem_stride * seg_elem_slot_cnt]);
}

template <typename SegAllocator>
constexpr Seg* AllocateSeg_(size_t seg_size, SegAllocator& seg_alctr) {
    Seg* seg{ static_cast<Seg*>(
        allocator::SafeAllocate(seg_alctr, alignof(Seg), seg_size)) };

    seg->n.Init();

    return seg;
}

struct SrcBranchIdxes_ {
    size_t seg_idx;
    multi_level_ptr_table::BranchNum const* rots;
    size_t branch_num;
    size_t acc_branch_num;

    size_t operator()() {
        size_t ret{ this->seg_idx / this->acc_branch_num + *this->rots };
        if (this->branch_num <= ret) { ret -= this->branch_num; }

        this->seg_idx %= this->acc_branch_num;

        --this->rots;
        this->acc_branch_num /= this->branch_num;

        return ret;
    }
};

struct AccessType_ {
    struct FromL {};
    struct FromR {};
    struct AutoWithHint {};
    struct AutoWithoutHint {};
};

template <typename Type, CntrTplParamList>
constexpr void Access_(Cntr<CntrTplArgList>& cntr, size_t idx,
                       bool lazy_copy_elem,
                       seq_cntr::ElemPtrView* dst_elem_ptr_view,
                       Cursor* dst_cursor, void* dst_elem) {
    ZETA_Core_StaticAssert(
        meta::IsAnySame<Type, AccessType_::FromL, AccessType_::FromR,
                        AccessType_::AutoWithHint,
                        AccessType_::AutoWithoutHint>);

    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };

    detail::CheckCntr_(cntr);

    if constexpr (meta::IsSame<Type, AccessType_::AutoWithHint>) {
        ZETA_Core_DebugAssert(dst_cursor != nullptr);
        (CheckCursor_)(cntr, dst_cursor);
    }

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.tree_elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    multi_level_ptr_table::BranchNum const* rots{ cntr.rots };

    unsigned level{ cntr.level };

    void* root{ cntr.root };

    Node* head_n{ cntr.head_n };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, elem_cnt));

    if (idx == static_cast<size_t>(-1) || idx == elem_cnt) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = idx;
            dst_cursor->n = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return;
    }

    size_t tree_seg_offset{ tree_elem_offset / seg_elem_slot_cnt };
    size_t seg_elem_offset{ tree_elem_offset % seg_elem_slot_cnt };

    size_t seg_cnt{ integral_math::CeilDiv(seg_elem_offset + elem_cnt,
                                           seg_elem_slot_cnt) };

    size_t elem_slot_idx{ seg_elem_offset + idx };
    size_t seg_idx{ elem_slot_idx / seg_elem_slot_cnt };
    size_t seg_elem_slot_idx{ elem_slot_idx % seg_elem_slot_cnt };

    size_t dist_from_l{ 1 + seg_idx };
    size_t dist_from_r{ seg_cnt - seg_idx };
    size_t dist_from_mlpt{ level };
    size_t dist_from_cursor;

    Node* n;

    size_t cursor_seg_idx;

    if constexpr (meta::IsSame<Type, AccessType_::FromL>) {
        goto ACCESS_FROM_L;
    }

    if constexpr (meta::IsSame<Type, AccessType_::FromR>) {
        goto ACCESS_FROM_R;
    }

    {
        if constexpr (meta::IsSame<Type, AccessType_::AutoWithHint>) {
            if (dst_cursor->idx == static_cast<size_t>(-1) ||
                dst_cursor->idx == elem_cnt) {
                dist_from_cursor = integral::RangeMaxOf<size_t>;
            } else {
                cursor_seg_idx =
                    (seg_elem_offset + dst_cursor->idx) / seg_elem_slot_cnt;

                dist_from_cursor = cursor_seg_idx <= seg_idx
                                       ? seg_idx - cursor_seg_idx
                                       : cursor_seg_idx - seg_idx;
            }
        }

        size_t best_dist;

        if constexpr (meta::IsSame<Type, AccessType_::AutoWithHint>) {
            best_dist = comparison_utils::BasicMin(
                dist_from_l, dist_from_r, dist_from_mlpt, dist_from_cursor);
        } else {
            best_dist = comparison_utils::BasicMin(dist_from_l, dist_from_r,
                                                   dist_from_mlpt);
        }

        if (dist_from_l == best_dist) { goto ACCESS_FROM_L; }
        if (dist_from_r == best_dist) { goto ACCESS_FROM_R; }

        if constexpr (meta::IsSame<Type, AccessType_::AutoWithHint>) {
            if (dist_from_cursor == best_dist) { goto ACCESS_FROM_CURSOR; }
        }

        goto ACCESS_FROM_MLPT;
    }

ACCESS_FROM_L: {
    n = head_n;
    for (size_t i{ 0 }; i < dist_from_l; ++i) { n = llist::GetR(n); }
    goto END;
}

ACCESS_FROM_R: {
    n = head_n;
    for (size_t i{ 0 }; i < dist_from_r; ++i) { n = llist::GetL(n); }
    goto END;
}

ACCESS_FROM_CURSOR: {
    if constexpr (meta::IsSame<Type, AccessType_::AutoWithHint>) {
        n = dst_cursor->n;

        if (cursor_seg_idx <= seg_idx) {
            for (size_t i{ 0 }; i < dist_from_cursor; ++i) {
                n = llist::GetR(n);
            }
        } else {
            for (size_t i{ 0 }; i < dist_from_cursor; ++i) {
                n = llist::GetL(n);
            }
        }

        goto END;
    }
}

ACCESS_FROM_MLPT: {
    multi_level_ptr_table::Cntr<typename Cntr<CntrTplArgList>::ActiveMap,
                                decltype(node_alctr)>
        mlpt{
            .level = level,
            .branch_nums = Cntr<CntrTplArgList>::branch_nums.elems,
            .size = seg_cnt,
            .root = root,
            .nav_node_alctr = node_alctr,
        };

    n = static_cast<Node*>(*static_cast<void**>(mlpt.Refer(SrcBranchIdxes_{
        .seg_idx = tree_seg_offset + seg_idx,
        .rots = rots + (level - 1),
        .branch_num = branch_num,
        .acc_branch_num = TableMeta<branch_num>::acc_branch_nums[level - 1],
    })));

    goto END;
}

END: {
    Seg* seg{ detail::NToSeg_(n) };
    void* elem{ seg->data + elem_stride * seg_elem_slot_idx };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;
        dst_elem_ptr_view->aliasability =
            seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = idx;
        dst_cursor->n = n;
        dst_cursor->seg_elem_slot_idx = seg_elem_slot_idx;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}
}

template <bool EnWrite, CntrTplParamList, seq_cntr::IsReaderWriter ReaderWriter>
constexpr void ReadWrite_(Cntr<CntrTplArgList>& cntr, Cursor const* pos_cursor,
                          size_t cnt, ReaderWriter& reader_writer,
                          Cursor* dst_cursor) {
    (CheckCntr_)(cntr);
    (CheckCursor_)(cntr, pos_cursor);

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };

    Node* head_n{ cntr.head_n };

    Node* n{ pos_cursor->n };
    size_t seg_elem_slot_idx{ pos_cursor->seg_elem_slot_idx };

    size_t end_idx{ pos_cursor->idx + cnt };

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, seg_elem_slot_cnt - seg_elem_slot_idx) };

        reader_writer(
            static_cast<meta::Conditional<EnWrite, void*, void const*>>(
                (NToSeg_)(n)->data + elem_stride * seg_elem_slot_idx),
            elem_stride, cur_cnt);

        seg_elem_slot_idx += cur_cnt;

        if (seg_elem_slot_idx == seg_elem_slot_cnt) {
            seg_elem_slot_idx = 0;
            n = llist::GetR(n);
        }

        cnt -= cur_cnt;
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = end_idx;

        if (end_idx == elem_cnt) {
            dst_cursor->n = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
            dst_cursor->elem = nullptr;
        } else {
            dst_cursor->n = n;
            dst_cursor->seg_elem_slot_idx = seg_elem_slot_idx;
            dst_cursor->elem =
                (NToSeg_)(n)->data + elem_stride * seg_elem_slot_idx;
        }
    }
}

template <CntrTplParamList>
void InsertSegs_(
    Cntr<CntrTplArgList>& cntr, unsigned level_i,
    multi_level_ptr_table::NavNode<typename Cntr<CntrTplArgList>::ActiveMap>*
        mlpt_node,
    size_t seg_slot_idx, size_t cnt, Node* tail) {
    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };

    multi_level_ptr_table::BranchNum rot{ cntr.rots[level_i] };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr_like) };

    if (level_i == 0) {
        size_t seg_size{ detail::GetSegSize_(elem_stride, seg_elem_slot_cnt) };

        while (0 < cnt) {
            size_t mlpt_slot_idx{ rot + seg_slot_idx };
            if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

            Seg* ins_seg{ detail::AllocateSeg_(seg_size, seg_alctr) };

            mlpt_node->active_map += static_cast<ActiveMap>(1) << mlpt_slot_idx;
            mlpt_node->ptrs[mlpt_slot_idx] = &ins_seg->n;

            llist::InsertL(tail, &ins_seg->n);

            ++seg_slot_idx;
            --cnt;
        }

        return;
    }

    size_t sub_tree_seg_slot_cnt{ acc_branch_nums[level_i] };

    size_t sub_tree_slot_idx{ seg_slot_idx / sub_tree_seg_slot_cnt };
    size_t sub_seg_slot_idx{ seg_slot_idx % sub_tree_seg_slot_cnt };

    while (0 < cnt) {
        size_t mlpt_slot_idx{ rot + sub_tree_slot_idx };
        if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

        size_t cur_cnt{ comparison_utils::BasicMin(
            sub_tree_seg_slot_cnt - sub_seg_slot_idx, cnt) };

        multi_level_ptr_table::NavNode<ActiveMap>* sub_mlpt_node;

        if ((mlpt_node->active_map &
             (static_cast<ActiveMap>(1) << mlpt_slot_idx)) == 0) {
            sub_mlpt_node =
                multi_level_ptr_table::detail::AllocateNavNode_<ActiveMap>(
                    branch_num, node_alctr);

            mlpt_node->active_map += static_cast<ActiveMap>(1) << mlpt_slot_idx;
            mlpt_node->ptrs[mlpt_slot_idx] = sub_mlpt_node;
        } else {
            sub_mlpt_node =
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt_node->ptrs[mlpt_slot_idx]);
        }

        (InsertSegs_)(cntr, level_i - 1, sub_mlpt_node, sub_seg_slot_idx,
                      cur_cnt, tail);

        ++sub_tree_slot_idx;
        sub_seg_slot_idx = 0;
        cnt -= cur_cnt;
    }

    return;
}

template <CntrTplParamList>
constexpr bool EraseSegs_(
    Cntr<CntrTplArgList>& cntr, unsigned level_i,
    multi_level_ptr_table::NavNode<typename Cntr<CntrTplArgList>::ActiveMap>*
        mlpt_node,
    size_t seg_slot_idx, size_t cnt) {
    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    multi_level_ptr_table::BranchNum rot{ cntr.rots[level_i] };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr_like) };

    if (level_i == 0) {
        while (0 < cnt) {
            size_t mlpt_slot_idx{ rot + seg_slot_idx };
            if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

            Seg* era_seg{ ZETA_Core_MemberToStruct(
                Seg, n, mlpt_node->ptrs[mlpt_slot_idx]) };

            mlpt_node->active_map -= static_cast<ActiveMap>(1) << mlpt_slot_idx;

            llist::Extract(&era_seg->n);
            allocator::Deallocate(seg_alctr, era_seg);

            ++seg_slot_idx;
            --cnt;
        }

        bool is_empty{ mlpt_node->active_map == 0 };

        if (is_empty) {
            multi_level_ptr_table::detail::DeallocateNavNode_<ActiveMap>(
                mlpt_node, node_alctr);
        }

        return is_empty;
    }

    size_t sub_tree_seg_slot_cnt{ acc_branch_nums[level_i] };

    size_t sub_tree_slot_idx{ seg_slot_idx / sub_tree_seg_slot_cnt };
    size_t sub_seg_slot_idx{ seg_slot_idx % sub_tree_seg_slot_cnt };

    while (0 < cnt) {
        size_t mlpt_slot_idx{ rot + sub_tree_slot_idx };
        if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

        size_t cur_cnt{ comparison_utils::BasicMin(
            sub_tree_seg_slot_cnt - sub_seg_slot_idx, cnt) };

        bool sub_is_empty{ (
            EraseSegs_)(cntr, level_i - 1,
                        static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                            mlpt_node->ptrs[mlpt_slot_idx]),
                        sub_seg_slot_idx, cur_cnt) };

        if (sub_is_empty) {
            mlpt_node->active_map -= static_cast<ActiveMap>(1) << mlpt_slot_idx;
        }

        ++sub_tree_slot_idx;
        sub_seg_slot_idx = 0;

        cnt -= cur_cnt;
    }

    bool is_empty{ mlpt_node->active_map == 0 };

    if (is_empty) {
        multi_level_ptr_table::detail::DeallocateNavNode_<ActiveMap>(
            mlpt_node, node_alctr);
    }

    return is_empty;
}

template <int D, CntrTplParamList, typename Writer>
constexpr void Push_(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer& writer,
                     Cursor* dst_cursor) {
    (CheckCntr_)(cntr);

    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.tree_elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t max_elem_cnt{ (GetMaxElemCnt_<branch_num>)(seg_elem_slot_cnt) };

    multi_level_ptr_table::BranchNum* rots{ cntr.rots };

    unsigned level{ cntr.level };

    Node* head_n{ cntr.head_n };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanInsert(0, cnt, elem_cnt, max_elem_cnt));

    size_t cur_elem_cnt{ elem_cnt };
    size_t nxt_elem_cnt{ cur_elem_cnt + cnt };

    size_t cur_seg_elem_offset{ tree_elem_offset % seg_elem_slot_cnt };

    size_t nxt_seg_elem_offset;

    if constexpr (D == 0) {
        size_t k{ cnt % seg_elem_slot_cnt };

        nxt_seg_elem_offset = cur_seg_elem_offset < k
                                  ? cur_seg_elem_offset + seg_elem_slot_cnt - k
                                  : cur_seg_elem_offset - k;
    } else {
        nxt_seg_elem_offset = cur_seg_elem_offset;
    }

    size_t cur_seg_cnt{ integral_math::CeilDiv(
        cur_seg_elem_offset + cur_elem_cnt, seg_elem_slot_cnt) };

    size_t nxt_seg_cnt{ integral_math::CeilDiv(
        nxt_seg_elem_offset + nxt_elem_cnt, seg_elem_slot_cnt) };

    if (cnt == 0) {
        if constexpr (D == 0) {
            (Access_<detail::AccessType_::FromL>)(cntr, 0, true, nullptr,
                                                  dst_cursor, nullptr);
        } else {
            (Access_<detail::AccessType_::FromR>)(cntr, elem_cnt, true, nullptr,
                                                  dst_cursor, nullptr);
        }

        return;
    }

    multi_level_ptr_table::Cntr<ActiveMap, decltype(node_alctr)> mlpt{
        .level = level,
        .branch_nums = Cntr<CntrTplArgList>::branch_nums.elems,
        .size = cur_seg_cnt,
        .root = cntr.root,
        .nav_node_alctr = node_alctr,
    };

    if (mlpt.root == nullptr) {
        multi_level_ptr_table::NavNode<ActiveMap>* new_root{
            multi_level_ptr_table::detail::AllocateNavNode_<ActiveMap>(
                branch_num, node_alctr)
        };

        new_root->active_map = 0;

        mlpt.root = new_root;

        rots[0] = 0;
    }

    size_t tree_l_seg_offset{ tree_elem_offset / seg_elem_slot_cnt };

    if constexpr (D == 0) {
        if (cur_elem_cnt == 0) {
            tree_l_seg_offset = acc_branch_nums[mlpt.level - 1] * branch_num;
        } else {
            size_t cur_sub_tree_cnt{ integral_bit::PopCount(
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root)
                    ->active_map) };

            size_t rot{ branch_num - cur_sub_tree_cnt };

            tree_l_seg_offset += acc_branch_nums[mlpt.level - 1] * rot;

            rots[mlpt.level - 1] = ({
                size_t k{ rots[mlpt.level - 1] + branch_num - rot };
                static_cast<multi_level_ptr_table::BranchNum>(
                    k < branch_num ? k : k - branch_num);
            });
        }
    }

    size_t tree_r_seg_offset{ acc_branch_nums[mlpt.level] - tree_l_seg_offset -
                              cur_seg_cnt };

    while (D == 0
               ? acc_branch_nums[mlpt.level] < tree_r_seg_offset + nxt_seg_cnt
               : acc_branch_nums[mlpt.level] <
                     tree_l_seg_offset + nxt_seg_cnt) {
        multi_level_ptr_table::NavNode<ActiveMap>* new_root{
            multi_level_ptr_table::detail::AllocateNavNode_<ActiveMap>(
                branch_num, node_alctr)
        };

        if constexpr (D == 0) {
            new_root->active_map = static_cast<ActiveMap>(1)
                                   << (branch_num - 1);
            new_root->ptrs[branch_num - 1] = mlpt.root;
        } else {
            new_root->active_map = 1;
            new_root->ptrs[0] = mlpt.root;
        }

        ++mlpt.level;
        mlpt.root = new_root;

        rots[mlpt.level - 1] = 0;
    }

    Node* old_last_n{ elem_cnt == 0 ? head_n : llist::GetL(head_n) };
    size_t old_last_seg_elem_slot_idx{ elem_cnt == 0
                                           ? seg_elem_slot_cnt - 1
                                           : (tree_elem_offset + elem_cnt - 1) %
                                                 seg_elem_slot_cnt };

    if constexpr (D == 0) {
        tree_l_seg_offset =
            acc_branch_nums[mlpt.level] - nxt_seg_cnt - tree_r_seg_offset;

        if (0 < nxt_seg_cnt - cur_seg_cnt) {
            (InsertSegs_)(
                cntr, mlpt.level - 1,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root),
                tree_l_seg_offset, nxt_seg_cnt - cur_seg_cnt,
                llist::GetR(head_n));
        }
    } else {
        tree_r_seg_offset =
            acc_branch_nums[mlpt.level] - tree_l_seg_offset - nxt_seg_cnt;

        if (0 < nxt_seg_cnt - cur_seg_cnt) {
            (InsertSegs_)(
                cntr, mlpt.level - 1,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root),
                tree_l_seg_offset + cur_seg_cnt, nxt_seg_cnt - cur_seg_cnt,
                head_n);
        }
    }

    mlpt.size = nxt_seg_cnt;

    if constexpr (D == 0) {
        rots[mlpt.level - 1] = ({
            size_t k{ rots[mlpt.level - 1] +
                      tree_l_seg_offset / acc_branch_nums[mlpt.level - 1] };
            static_cast<multi_level_ptr_table::BranchNum>(
                k < branch_num ? k : k - branch_num);
        });

        tree_l_seg_offset %= acc_branch_nums[mlpt.level - 1];
    }

    cntr.elem_cnt = nxt_elem_cnt;

    if constexpr (D == 0) {
        cntr.tree_elem_offset =
            seg_elem_slot_cnt * tree_l_seg_offset + nxt_seg_elem_offset;
    }

    cntr.level = mlpt.level;
    cntr.root = mlpt.root;

    dst_cursor->cntr = &cntr;

    if constexpr (D == 0) {
        dst_cursor->idx = 0;
        dst_cursor->n = llist::GetR(head_n);
        dst_cursor->seg_elem_slot_idx = nxt_seg_elem_offset;
    } else {
        Node* new_first_n;
        size_t new_first_seg_elem_slot_idx;

        if (old_last_seg_elem_slot_idx + 1 == seg_elem_slot_cnt) {
            new_first_n = llist::GetR(old_last_n);
            new_first_seg_elem_slot_idx = 0;
        } else {
            new_first_n = old_last_n;
            new_first_seg_elem_slot_idx = old_last_seg_elem_slot_idx + 1;
        }

        dst_cursor->idx = elem_cnt;
        dst_cursor->n = new_first_n;
        dst_cursor->seg_elem_slot_idx = new_first_seg_elem_slot_idx;
    }

    dst_cursor->elem = (NToSeg_)(dst_cursor->n)->data +
                       elem_stride * dst_cursor->seg_elem_slot_idx;

    (ReadWrite_<true>)(cntr, dst_cursor, cnt, writer, nullptr);
}

template <int LR, CntrTplParamList>
constexpr void Pop_(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    (CheckCntr_)(cntr);

    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t tree_elem_offset{ cntr.tree_elem_offset };

    multi_level_ptr_table::BranchNum* rots{ cntr.rots };

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanErase(0, cnt, elem_cnt));

    if (cnt == 0) { return; }

    size_t cur_elem_cnt{ elem_cnt };
    size_t nxt_elem_cnt{ cur_elem_cnt - cnt };

    size_t cur_seg_elem_offset{ tree_elem_offset % seg_elem_slot_cnt };

    size_t nxt_seg_elem_offset;

    if constexpr (LR == 0) {
        nxt_seg_elem_offset =
            nxt_elem_cnt == 0 ? 0
                              : (cur_seg_elem_offset + cnt) % seg_elem_slot_cnt;
    } else {
        nxt_seg_elem_offset = nxt_elem_cnt == 0 ? 0 : cur_seg_elem_offset;
    }

    size_t cur_seg_cnt{ integral_math::CeilDiv(
        cur_seg_elem_offset + cur_elem_cnt, seg_elem_slot_cnt) };

    size_t nxt_seg_cnt{ integral_math::CeilDiv(
        nxt_seg_elem_offset + nxt_elem_cnt, seg_elem_slot_cnt) };

    multi_level_ptr_table::Cntr<ActiveMap, decltype(node_alctr)> mlpt{
        .level = level,
        .branch_nums = Cntr<CntrTplArgList>::branch_nums.elems,
        .size = cur_seg_cnt,
        .root = root,
        .nav_node_alctr = node_alctr,
    };

    size_t tree_l_seg_offset{ tree_elem_offset / seg_elem_slot_cnt };

    if constexpr (LR == 0) {
        if (0 < cur_seg_cnt - nxt_seg_cnt) {
            (EraseSegs_)(
                cntr, mlpt.level - 1,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root),
                tree_l_seg_offset, cur_seg_cnt - nxt_seg_cnt);
        }

        tree_l_seg_offset += cur_seg_cnt - nxt_seg_cnt;
    } else {
        if (0 < cur_seg_cnt - nxt_seg_cnt) {
            (EraseSegs_)(
                cntr, mlpt.level - 1,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root),
                tree_l_seg_offset + nxt_seg_cnt, cur_seg_cnt - nxt_seg_cnt);
        }
    }

    mlpt.size = nxt_seg_cnt;

    if (nxt_elem_cnt == 0) {
        mlpt.level = 1;
        mlpt.root = nullptr;

        rots[0] = 0;
        tree_l_seg_offset = 0;
    } else {
        while (1 < mlpt.level) {
            auto* old_root{
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root)
            };

            if (1 < integral_bit::PopCount(old_root->active_map)) { break; }

            void* new_root{
                old_root->ptrs[integral_bit::CTZ(old_root->active_map)]
            };

            multi_level_ptr_table::detail::DeallocateNavNode_(old_root,
                                                              node_alctr);

            --mlpt.level;
            mlpt.root = new_root;
        }

        tree_l_seg_offset %= acc_branch_nums[mlpt.level];

        rots[mlpt.level - 1] = ({
            size_t k{ rots[mlpt.level - 1] +
                      tree_l_seg_offset / acc_branch_nums[mlpt.level - 1] };
            static_cast<multi_level_ptr_table::BranchNum>(
                k < branch_num ? k : k - branch_num);
        });

        tree_l_seg_offset %= acc_branch_nums[mlpt.level - 1];
    }

    cntr.elem_cnt = nxt_elem_cnt;
    cntr.tree_elem_offset =
        seg_elem_slot_cnt * tree_l_seg_offset + nxt_seg_elem_offset;

    cntr.level = mlpt.level;
    cntr.root = mlpt.root;
}

template <int D, int CopyOrMove>
constexpr pair::Pair<pair::Pair<Node*, size_t>, pair::Pair<Node const*, size_t>>
Assign_(size_t elem_size, size_t dst_elem_stride, size_t src_elem_stride,
        size_t dst_seg_elem_slot_cnt, size_t src_seg_elem_slot_cnt, Node* dst_n,
        Node const* src_n, size_t dst_seg_elem_slot_idx,
        size_t src_seg_elem_slot_idx, size_t cnt) {
    ZETA_Core_StaticAssert(D == 0 || D == 1);
    ZETA_Core_StaticAssert(CopyOrMove == 0 || CopyOrMove == 1);

    for (;;) {
        if constexpr (D == 1) {
            if (dst_seg_elem_slot_idx == dst_seg_elem_slot_cnt) {
                dst_n = llist::GetR(dst_n);
                dst_seg_elem_slot_idx = 0;
            }

            if (src_seg_elem_slot_idx == src_seg_elem_slot_cnt) {
                src_n = llist::GetR(src_n);
                src_seg_elem_slot_idx = 0;
            }
        }

        if (cnt == 0) { break; }

        if constexpr (D == 0) {
            if (dst_seg_elem_slot_idx == 0) {
                dst_n = llist::GetL(dst_n);
                dst_seg_elem_slot_idx = dst_seg_elem_slot_cnt;
            }

            if (src_seg_elem_slot_idx == 0) {
                src_n = llist::GetL(src_n);
                src_seg_elem_slot_idx = src_seg_elem_slot_cnt;
            }
        }

        size_t cur_cnt;
        void* dst_elem;
        void const* src_elem;

        if constexpr (D == 0) {
            cur_cnt = comparison_utils::BasicMin(src_seg_elem_slot_idx,
                                                 dst_seg_elem_slot_idx, cnt);

            dst_elem = (NToSeg_)(dst_n)->data +
                       dst_elem_stride * (dst_seg_elem_slot_idx - cur_cnt);

            src_elem = (NToSeg_)(src_n)->data +
                       src_elem_stride * (src_seg_elem_slot_idx - cur_cnt);
        } else {
            cur_cnt = comparison_utils::BasicMin(
                src_seg_elem_slot_cnt - src_seg_elem_slot_idx,
                dst_seg_elem_slot_cnt - dst_seg_elem_slot_idx, cnt);

            dst_elem = (NToSeg_)(dst_n)->data +
                       dst_elem_stride * dst_seg_elem_slot_idx;

            src_elem = (NToSeg_)(src_n)->data +
                       src_elem_stride * src_seg_elem_slot_idx;
        }

        if constexpr (CopyOrMove == 0) {
            utils::ElemCopy(dst_elem, src_elem, elem_size, dst_elem_stride,
                            src_elem_stride, cur_cnt);
        } else {
            utils::ElemMove(dst_elem, src_elem, elem_size, dst_elem_stride,
                            src_elem_stride, cur_cnt);
        }

        if constexpr (D == 0) {
            dst_seg_elem_slot_idx -= cur_cnt;
            src_seg_elem_slot_idx -= cur_cnt;
        } else {
            dst_seg_elem_slot_idx += cur_cnt;
            src_seg_elem_slot_idx += cur_cnt;
        }

        cnt -= cur_cnt;
    }

    return {
        { dst_n, dst_seg_elem_slot_idx },
        { src_n, src_seg_elem_slot_idx },
    };
}

}  // namespace multi_level_circular_array::detail

template <CntrTplParamList>
template <typename NodeAllocatorLikeInitArg, typename SegAllocatorLikeInitArg>
constexpr multi_level_circular_array::Cntr<CntrTplArgList>::Cntr(
    size_t elem_size, size_t elem_stride, size_t seg_elem_slot_cnt,
    NodeAllocatorLikeInitArg&& node_alctr_like_init_arg,
    SegAllocatorLikeInitArg&& seg_alctr_like_init_arg)
    : node_alctr_like{ ZETA_Core_Lifecycle_UnpackInitArg(
          NodeAllocatorLike, NodeAllocatorLikeInitArg,
          node_alctr_like_init_arg) },
      seg_alctr_like{ ZETA_Core_Lifecycle_UnpackInitArg(
          SegAllocatorLike, SegAllocatorLikeInitArg,
          seg_alctr_like_init_arg) } {
    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);

    this->elem_size = elem_size;
    this->elem_stride = elem_stride;
    this->seg_elem_slot_cnt = seg_elem_slot_cnt;
    this->tree_elem_offset = 0;
    this->elem_cnt = 0;

    this->rots[0] = 0;

    this->level = 1;
    this->root = nullptr;

    this->head_n = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(this->node_alctr_like), alignof(Node), sizeof(Node)));

    this->head_n->Init();
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Deinit(
    this Cntr<CntrTplArgList>& cntr) {
    cntr.EraseAll();
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorSize(seq_cntr::Tag) {
    return sizeof(Cursor);
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& cntr, seq_cntr::Tag) {
    detail::CheckCntr_(cntr);
    return cntr.elem_size;
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetElemStride(
    this Cntr const& cntr, seq_cntr::Tag) {
    detail::CheckCntr_(cntr);
    return cntr.elem_stride;
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetSegElemCapacity(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);
    return cntr.seg_elem_slot_cnt;
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& cntr, seq_cntr::Tag) {
    detail::CheckCntr_(cntr);
    return cntr.elem_cnt;
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& cntr, seq_cntr::Tag) {
    detail::CheckCntr_(cntr);
    // TODO
    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::GetLBCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->n = cntr.head_n;
    dst_cursor->seg_elem_slot_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = cntr.elem_cnt;
    dst_cursor->n = cntr.head_n;
    dst_cursor->seg_elem_slot_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PeekL(
    this auto&& cntr, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    detail::Access_<detail::AccessType_::FromL>(
        cntr, 0, lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);

    if constexpr (meta::IsConst<decltype(cntr)>) {
        if (dst_elem_ptr_view != nullptr &&
            dst_elem_ptr_view->aliasability ==
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        }
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PeekR(
    this auto&& cntr, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };

    detail::Access_<detail::AccessType_::FromR>(
        cntr, elem_cnt - 1, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
        dst_elem);

    if constexpr (meta::IsConst<decltype(cntr)>) {
        if (dst_elem_ptr_view != nullptr &&
            dst_elem_ptr_view->aliasability ==
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        }
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Refer(
    this auto&& cntr, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::Access_<detail::AccessType_::AutoWithoutHint>(
        cntr, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);

    if constexpr (meta::IsConst<decltype(cntr)>) {
        if (dst_elem_ptr_view != nullptr &&
            dst_elem_ptr_view->aliasability ==
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        }
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::AccessWithHint(
    this auto&& cntr, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::Access_<detail::AccessType_::AutoWithHint>(
        cntr, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);

    if constexpr (meta::IsConst<decltype(cntr)>) {
        if (dst_elem_ptr_view != nullptr &&
            dst_elem_ptr_view->aliasability ==
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite) {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        }
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Derefer(
    this auto&& cntr, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    void* dst_elem) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(dst_elem_ptr_view != nullptr || dst_elem != nullptr);

    size_t elem_size{ cntr.elem_size };

    void* elem{ pos_cursor->elem };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (!lazy_copy_elem && elem != nullptr && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

template <CntrTplParamList>
template <seq_cntr::IsReader Reader>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Read(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
    Reader& reader, Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr<CntrTplArgList>&>(cntr),
                              pos_cursor, cnt, reader, dst_cursor);
}

template <CntrTplParamList>
template <seq_cntr::IsWriter Writer>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Write(
    this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
    Writer& writer, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList>
template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::ReadWrite(
    this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
    ReaderWriter& reader_writer, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <CntrTplParamList>
template <seq_cntr::IsWriter Writer>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PushL(
    this Cntr& cntr, seq_cntr::Tag, size_t cnt, Writer& writer,
    Cursor* dst_cursor) {
    Cursor dst_cursor_fallback;
    if (dst_cursor == nullptr) { dst_cursor = &dst_cursor_fallback; }

    detail::Push_<0>(cntr, cnt, writer, dst_cursor);
}

template <CntrTplParamList>
template <seq_cntr::IsWriter Writer>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PushR(
    this Cntr<CntrTplArgList>& cntr, size_t cnt, Writer& writer,
    Cursor* dst_cursor) {
    Cursor dst_cursor_fallback;
    if (dst_cursor == nullptr) { dst_cursor = &dst_cursor_fallback; }

    detail::Push_<1>(cntr, cnt, writer, dst_cursor);
}

template <CntrTplParamList>
template <seq_cntr::IsWriter Writer>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Insert(
    this Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
    Writer& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    if (cnt == 0) {
        if (dst_cursor != nullptr) { *dst_cursor = *pos_cursor; }
        return;
    }

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.tree_elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    Node* head_n{ cntr.head_n };

    size_t idx{ pos_cursor->idx };

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx };

    if (r_cnt == 0) {
        cntr.PushR(cnt, writer, pos_cursor);

        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = elem_cnt + cnt;
            dst_cursor->n = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return;
    }

    if (l_cnt == 0) {
        Node* pos_n{ pos_cursor->n };
        size_t pos_seg_elem_slot_idx{ pos_cursor->seg_elem_slot_idx };
        void* pos_elem{ pos_cursor->elem };

        cntr.PushL(cnt, writer, pos_cursor);

        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = elem_cnt + cnt;
            dst_cursor->n = pos_n;
            dst_cursor->seg_elem_slot_idx = pos_seg_elem_slot_idx;
            dst_cursor->elem = pos_elem;
        }

        return;
    }

    Cursor dst_cursor_fallback;
    if (dst_cursor == nullptr) { dst_cursor = &dst_cursor_fallback; }

    unsigned long long random_seed{ utils::GetRandom() };

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case 0: {
        Node* old_l_n{ llist::GetR(head_n) };
        size_t old_l_seg_elem_slot_idx{ tree_elem_offset % seg_elem_slot_cnt };

        detail::Push_<0>(cntr, cnt, elem_stream::provider::EmptyProvider{},
                         dst_cursor);

        if (cnt < seg_elem_slot_cnt) {
            auto p{ detail::Assign_<1, 1>(elem_size, elem_stride, elem_stride,
                                          seg_elem_slot_cnt, seg_elem_slot_cnt,
                                          dst_cursor->n, old_l_n,
                                          dst_cursor->seg_elem_slot_idx,
                                          old_l_seg_elem_slot_idx, l_cnt) };

            dst_cursor->n = p.first.first;
            dst_cursor->seg_elem_slot_idx = p.first.second;
        } else {
            auto p{ detail::Assign_<1, 0>(elem_size, elem_stride, elem_stride,
                                          seg_elem_slot_cnt, seg_elem_slot_cnt,
                                          dst_cursor->n, old_l_n,
                                          dst_cursor->seg_elem_slot_idx,
                                          old_l_seg_elem_slot_idx, l_cnt) };

            dst_cursor->n = p.first.first;
            dst_cursor->seg_elem_slot_idx = p.first.second;
        }

        dst_cursor->idx += l_cnt;
        dst_cursor->elem = detail::NToSeg_(dst_cursor->n)->data +
                           elem_stride * dst_cursor->seg_elem_slot_idx;

        *pos_cursor = *dst_cursor;

        detail::ReadWrite_<true>(cntr, dst_cursor, cnt, writer, dst_cursor);

        break;
    }
    case 1: {
        detail::Push_<1>(cntr, cnt, elem_stream::provider::EmptyProvider{},
                         dst_cursor);

        cntr.Sanitize(nullptr, nullptr);

        Node* r_n{ llist::GetL(head_n) };

        size_t r_seg_elem_slot_idx{
            (tree_elem_offset + elem_cnt + cnt - 1) % seg_elem_slot_cnt + 1
        };

        if (cnt < seg_elem_slot_cnt) {
            auto p{ detail::Assign_<0, 1>(
                elem_size, elem_stride, elem_stride, seg_elem_slot_cnt,
                seg_elem_slot_cnt, r_n, dst_cursor->n, r_seg_elem_slot_idx,
                dst_cursor->seg_elem_slot_idx, r_cnt) };

            dst_cursor->n = const_cast<Node*>(p.second.first);
            dst_cursor->seg_elem_slot_idx = p.second.second;
        } else {
            auto p{ detail::Assign_<0, 0>(
                elem_size, elem_stride, elem_stride, seg_elem_slot_cnt,
                seg_elem_slot_cnt, r_n, dst_cursor->n, r_seg_elem_slot_idx,
                dst_cursor->seg_elem_slot_idx, r_cnt) };

            dst_cursor->n = const_cast<Node*>(p.second.first);
            dst_cursor->seg_elem_slot_idx = p.second.second;
        }

        dst_cursor->idx -= r_cnt;
        dst_cursor->elem = detail::NToSeg_(dst_cursor->n)->data +
                           elem_stride * dst_cursor->seg_elem_slot_idx;

        detail::ReadWrite_<true>(cntr, dst_cursor, cnt, writer, dst_cursor);

        break;
    }
    default: ZETA_Core_Unreachable();
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PopL(
    this Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::Pop_<0>(cntr, cnt);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PopR(
    this Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::Pop_<1>(cntr, cnt);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Erase(
    this Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(
        seq_cntr::check_operation::CanErase(idx, cnt, elem_cnt));

    if (cnt == 0) { return; }

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx - cnt };

    if (r_cnt == 0) {
        cntr.PopR(cnt);
        cntr.GetRBCursor(pos_cursor);
        return;
    }

    if (l_cnt == 0) {
        cntr.PopL(cnt);
        cntr.PeekL(true, nullptr, pos_cursor, nullptr);
        return;
    }

    unsigned long long random_seed{ utils::GetRandom() };

    Cursor end_cursor{ *pos_cursor };
    cntr.AccessWithHint(idx + cnt, true, nullptr, &end_cursor, nullptr);

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case 0: {
        if (cnt < seg_elem_slot_cnt) {
            detail::Assign_<0, 1>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  end_cursor.n, pos_cursor->n,
                                  end_cursor.seg_elem_slot_idx,
                                  pos_cursor->seg_elem_slot_idx, l_cnt);
        } else {
            detail::Assign_<0, 0>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  end_cursor.n, pos_cursor->n,
                                  end_cursor.seg_elem_slot_idx,
                                  pos_cursor->seg_elem_slot_idx, l_cnt);
        }

        detail::Pop_<0>(cntr, cnt);

        pos_cursor->n = end_cursor.n;
        pos_cursor->seg_elem_slot_idx = end_cursor.seg_elem_slot_idx;
        pos_cursor->elem = end_cursor.elem;

        break;
    }
    case 1: {
        if (cnt < seg_elem_slot_cnt) {
            detail::Assign_<1, 1>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  pos_cursor->n, end_cursor.n,
                                  pos_cursor->seg_elem_slot_idx,
                                  end_cursor.seg_elem_slot_idx, r_cnt);
        } else {
            detail::Assign_<1, 0>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  pos_cursor->n, end_cursor.n,
                                  pos_cursor->seg_elem_slot_idx,
                                  end_cursor.seg_elem_slot_idx, r_cnt);
        }

        detail::Pop_<1>(cntr, cnt);

        break;
    }
    default: ZETA_Core_Unreachable();
    }
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::EraseAll(
    this Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    cntr.PopR(cntr.elem_cnt);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr<CntrTplArgList> const& cntr, Cursor const* src_cursor,
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool multi_level_circular_array::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

template <CntrTplParamList>
constexpr int multi_level_circular_array::Cntr<CntrTplArgList>::CompareCursor(
    this Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(comparison::OpTag::Order{},
                                    cntr.GetCursorIdx(cursor_a) + 1,
                                    cntr.GetCursorIdx(cursor_b) + 1);
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorDist(
    this Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return cntr.GetCursorIdx(cursor_b) - cntr.GetCursorIdx(cursor_a);
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorIdx(
    this Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cursor);

    return cursor->idx;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr<CntrTplArgList> const& cntr, Cursor* cursor) {
    cntr.CursorAdvanceL(cursor, 1);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr<CntrTplArgList> const& cntr, Cursor* cursor) {
    cntr.CursorAdvanceR(cursor, 1);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorAdvanceL(
    this Cntr<CntrTplArgList> const& cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanAdvanceL(
        cursor->idx, step, cntr.elem_cnt));

    detail::Access_<detail::AccessType_::AutoWithHint>(
        cntr, cursor->idx - step, true, nullptr, cursor, nullptr);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorAdvanceR(
    this Cntr<CntrTplArgList> const& cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(seq_cntr::check_operation::CanAdvanceR(
        cursor->idx, step, cntr.elem_cnt));

    detail::Access_<detail::AccessType_::AutoWithHint>(
        cntr, cursor->idx + step, true, nullptr, cursor, nullptr);
}

namespace multi_level_circular_array::detail {

template <CntrTplParamList>
constexpr pair::Pair<Node*, Node*> SanitizeMLPTNode_(
    Cntr<CntrTplArgList> const& cntr, size_t level_i,
    multi_level_ptr_table::NavNode<typename Cntr<CntrTplArgList>::ActiveMap>*
        mlpt_node,
    size_t seg_idx, size_t cnt) {
    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t tree_seg_slot_cnt{ acc_branch_nums[level_i + 1] };
    size_t sub_tree_seg_slot_cnt{ acc_branch_nums[level_i] };

    ZETA_Core_DebugAssert(seg_idx < tree_seg_slot_cnt);
    ZETA_Core_DebugAssert(0 < cnt);
    ZETA_Core_DebugAssert(cnt <= tree_seg_slot_cnt);
    ZETA_Core_DebugAssert(seg_idx + cnt <= tree_seg_slot_cnt);

    size_t sub_tree_idx_beg{ seg_idx / sub_tree_seg_slot_cnt };
    size_t sub_tree_idx_end{ (seg_idx + cnt - 1) / sub_tree_seg_slot_cnt };

    for (size_t sub_tree_idx{ 0 }; sub_tree_idx < branch_num; ++sub_tree_idx) {
        size_t sub_tree_slot_idx{ cntr.rots[level_i] + sub_tree_idx };

        if (branch_num <= sub_tree_slot_idx) {
            sub_tree_slot_idx -= branch_num;
        }

        if (sub_tree_idx < sub_tree_idx_beg ||
            sub_tree_idx_end < sub_tree_idx) {
            ZETA_Core_DebugAssert(
                (mlpt_node->active_map &
                 (static_cast<ActiveMap>(1) << sub_tree_slot_idx)) == 0);
        } else {
            ZETA_Core_DebugAssert(
                (mlpt_node->active_map &
                 (static_cast<ActiveMap>(1) << sub_tree_slot_idx)) != 0);
        }
    }

    Node* first_n{ nullptr };
    Node* last_n{ nullptr };

    if (level_i == 0) {
        for (size_t sub_tree_idx{ sub_tree_idx_beg };
             sub_tree_idx <= sub_tree_idx_end; ++sub_tree_idx) {
            size_t sub_tree_slot_idx{ cntr.rots[level_i] + sub_tree_idx };

            if (branch_num <= sub_tree_slot_idx) {
                sub_tree_slot_idx -= branch_num;
            }

            Node* n{ static_cast<Node*>(mlpt_node->ptrs[sub_tree_slot_idx]) };

            if (first_n == nullptr) {
                first_n = n;
            } else {
                ZETA_Core_DebugAssert(llist::GetR(last_n) == n);
                ZETA_Core_DebugAssert(llist::GetL(n) == last_n);
            }

            last_n = n;
        }

        ZETA_Core_DebugAssert(integral_bit::PopCount(mlpt_node->active_map) ==
                              cnt);

        return { first_n, last_n };
    }

    for (size_t sub_tree_idx{ sub_tree_idx_beg };
         sub_tree_idx <= sub_tree_idx_end; ++sub_tree_idx) {
        size_t sub_tree_slot_idx{ cntr.rots[level_i] + sub_tree_idx };

        if (branch_num <= sub_tree_slot_idx) {
            sub_tree_slot_idx -= branch_num;
        }

        size_t cur_cnt{ comparison_utils::BasicMin(
            sub_tree_seg_slot_cnt - seg_idx % sub_tree_seg_slot_cnt, cnt) };

        auto [cur_first_n, cur_last_n]{ (
            SanitizeMLPTNode_)(cntr, level_i - 1,
                               static_cast<
                                   multi_level_ptr_table::NavNode<ActiveMap>*>(
                                   mlpt_node->ptrs[sub_tree_slot_idx]),
                               seg_idx % sub_tree_seg_slot_cnt, cur_cnt) };

        if (first_n == nullptr) {
            first_n = cur_first_n;
        } else {
            ZETA_Core_DebugAssert(llist::GetR(last_n) == cur_first_n);
            ZETA_Core_DebugAssert(llist::GetL(cur_first_n) == last_n);
        }

        last_n = cur_last_n;

        seg_idx += cur_cnt;
        cnt -= cur_cnt;
    }

    ZETA_Core_DebugAssert(cnt == 0);

    return { first_n, last_n };
}

}  // namespace multi_level_circular_array::detail

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Sanitize(
    this Cntr const& cntr, mem_recorder::MemRecorder* dst_node,
    mem_recorder::MemRecorder* dst_seg) {
    constexpr size_t branch_num{ Cntr::branch_num };

    detail::CheckCntr_(cntr);

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_offset{ cntr.tree_elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    Node* head_n{ cntr.head_n };

    size_t tree_seg_offset{ elem_offset / seg_elem_slot_cnt };
    size_t seg_elem_offset{ elem_offset % seg_elem_slot_cnt };

    size_t seg_cnt{ integral_math::CeilDiv(seg_elem_offset + elem_cnt,
                                           seg_elem_slot_cnt) };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };

    multi_level_ptr_table::Cntr<typename Cntr<CntrTplArgList>::ActiveMap,
                                decltype(node_alctr)>
        mlpt{
            .level = cntr.level,
            .branch_nums = Cntr<CntrTplArgList>::branch_nums.elems,
            .size = seg_cnt,
            .root = cntr.root,
            .nav_node_alctr = node_alctr,
        };

    mem_recorder::MemRecorder* origin_dst_node{ dst_node };
    mem_recorder::MemRecorder* origin_dst_seg{ dst_seg };

    if (dst_node == nullptr) { dst_node = mem_recorder::Create(); }
    if (dst_seg == nullptr) { dst_seg = mem_recorder::Create(); }

    multi_level_ptr_table::Sanitize(mlpt, dst_node);

    size_t seg_size{ detail::GetSegSize_(elem_stride, seg_elem_slot_cnt) };

    mem_recorder::Record(*dst_node, head_n, sizeof(Node));

    {
        Node* n{ llist::GetR(head_n) };

        for (size_t seg_idx{ 0 }; seg_idx != seg_cnt; ++seg_idx) {
            void* mp{ multi_level_ptr_table::Refer(
                mlpt,
                detail::SrcBranchIdxes_{
                    .seg_idx = tree_seg_offset + seg_idx,
                    .rots = cntr.rots + (mlpt.level - 1),
                    .branch_num = branch_num,
                    .acc_branch_num =
                        TableMeta<branch_num>::acc_branch_nums[cntr.level - 1],
                }) };

            ZETA_Core_DebugAssert(mp != nullptr);

            void* m{ *static_cast<void**>(mp) };

            ZETA_Core_DebugAssert(n == m);

            mem_recorder::Record(*dst_seg, ZETA_Core_MemberToStruct(Seg, n, n),
                                 seg_size);

            n = llist::GetR(n);
        }
    }

    {
        Node* n{ llist::GetL(head_n) };

        for (size_t seg_idx{ seg_cnt - 1 }; seg_idx != static_cast<size_t>(-1);
             --seg_idx) {
            void* mp{ multi_level_ptr_table::Refer(
                mlpt,
                detail::SrcBranchIdxes_{
                    .seg_idx = tree_seg_offset + seg_idx,
                    .rots = cntr.rots + (mlpt.level - 1),
                    .branch_num = branch_num,
                    .acc_branch_num =
                        TableMeta<branch_num>::acc_branch_nums[cntr.level - 1],
                }) };

            ZETA_Core_DebugAssert(mp != nullptr);

            void* m{ *static_cast<void**>(mp) };

            ZETA_Core_DebugAssert(n == m);

            n = llist::GetL(n);
        }
    }

    if (seg_cnt == 0) {
        ZETA_Core_DebugAssert(llist::GetL(head_n) == head_n);
        ZETA_Core_DebugAssert(llist::GetR(head_n) == head_n);
    } else {
        auto [first_n, last_n]{ detail::SanitizeMLPTNode_(
            cntr, mlpt.level - 1,
            static_cast<multi_level_ptr_table::NavNode<
                typename Cntr<CntrTplArgList>::ActiveMap>*>(mlpt.root),
            tree_seg_offset, seg_cnt) };

        ZETA_Core_DebugAssert(llist::GetR(head_n) == first_n);
        ZETA_Core_DebugAssert(llist::GetL(first_n) == head_n);

        ZETA_Core_DebugAssert(llist::GetR(last_n) == head_n);
        ZETA_Core_DebugAssert(llist::GetL(head_n) == last_n);
    }

    if (origin_dst_node != dst_node) {
        mem_recorder::Destroy(dst_node);
        dst_node = origin_dst_node;
    }

    if (origin_dst_seg != dst_seg) {
        mem_recorder::Destroy(dst_seg);
        dst_seg = origin_dst_seg;
    }
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetReferedInstPtr(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return const_cast<multi_level_circular_array::Cntr<CntrTplArgList>*>(&cntr);
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::FlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer = true,
        .Derefer = true,

        .Read = true,
        .Write = true,
        .ReadWrite = true,

        .PushL = true,
        .PushR = true,
        .Insert = true,

        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CntrTraits<multi_level_circular_array::Cntr<
               CntrTplArgList>>::GetStaticEnabledCapabilityFlag() &
           seq_cntr::const_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    GetStaticDisabledCapabilityFlag() {
    return seq_cntr::empty_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetStaticDisabledCapabilityFlag() {
    return seq_cntr::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledCapabilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledCapabilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_capability_flag;
}

}  // namespace zeta::core

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")
