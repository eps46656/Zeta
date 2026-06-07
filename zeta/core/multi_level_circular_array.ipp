#pragma once

#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/compare_utils.ipp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/multi_level_circular_array.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename BranchNumTag, typename NodeAllocatorLike, typename SegAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList BranchNumTag, NodeAllocatorLike, SegAllocatorLike

namespace zeta::core {

namespace multi_level_circular_array::detail {

template <multi_level_ptr_table::BranchNum BranchNum>
size_t GetMaxElemCnt_(size_t seg_elem_capacity) {
    constexpr size_t max_seg_cnt{ TableMeta<BranchNum>::max_seg_cnt };

    return seg_elem_capacity <= ZETA_Core_max_capacity / max_seg_cnt
               ? seg_elem_capacity * max_seg_cnt
               : ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t elem_offset{ cntr.elem_offset };

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
void CheckCursor_(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(&cntr == cursor->cntr);

    Cursor re_cursor;
    (Access)(cntr, cursor->idx, true, &re_cursor, nullptr);

    ZETA_Core_DebugAssert(cursor->cntr == re_cursor.cntr);
    ZETA_Core_DebugAssert(cursor->idx == re_cursor.idx);
    ZETA_Core_DebugAssert(cursor->n == re_cursor.n);
    ZETA_Core_DebugAssert(cursor->seg_elem_slot_idx ==
                          re_cursor.seg_elem_slot_idx);
    ZETA_Core_DebugAssert(cursor->elem == re_cursor.elem);

    ZETA_Core_DebugAssert(*cursor == re_cursor);
}

inline Seg* NToSeg_(Node* n) { return ZETA_Core_MemberToStruct(Seg, n, n); }

inline Seg const* NToSeg_(Node const* n) {
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

inline size_t GetSegSize_(size_t elem_stride, size_t seg_elem_slot_cnt) {
    return offsetof(Seg, data[elem_stride * seg_elem_slot_cnt]);
}

template <typename SegAllocator>
Seg* AllocateSeg_(size_t seg_size, SegAllocator& seg_alctr) {
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
void* Access_(Cntr<CntrTplArgList> const& cntr, size_t idx, bool lazy_copy_elem,
              Cursor* dst_cursor, void* dst_elem) {
    ZETA_Core_StaticAssert(
        meta::IsAnyOf<Type, AccessType_::FromL, AccessType_::FromR,
                      AccessType_::AutoWithHint, AccessType_::AutoWithoutHint>);

    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };

    detail::CheckCntr_(cntr);

    if constexpr (meta::IsAnyOf<Type, AccessType_::AutoWithHint>) {
        ZETA_Core_DebugAssert(dst_cursor != nullptr);
        (CheckCursor_)(cntr, dst_cursor);
    }

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    multi_level_ptr_table::BranchNum const* rots{ cntr.rots };

    unsigned level{ cntr.level };

    void* root{ cntr.root };

    Node* head_n{ cntr.head_n };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, elem_cnt));

    if (idx == static_cast<size_t>(-1)) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = static_cast<size_t>(-1);
            dst_cursor->n = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return nullptr;
    }

    if (idx == elem_cnt) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = elem_cnt;
            dst_cursor->n = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return nullptr;
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

    if constexpr (meta::IsAnyOf<Type, AccessType_::FromL>) {
        goto ACCESS_FROM_L;
    }

    if constexpr (meta::IsAnyOf<Type, AccessType_::FromR>) {
        goto ACCESS_FROM_R;
    }

    {
        if constexpr (meta::IsAnyOf<Type, AccessType_::AutoWithHint>) {
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

        if constexpr (meta::IsAnyOf<Type, AccessType_::AutoWithHint>) {
            best_dist = compare_utils::BasicMin(
                dist_from_l, dist_from_r, dist_from_mlpt, dist_from_cursor);
        } else {
            best_dist = compare_utils::BasicMin(dist_from_l, dist_from_r,
                                                dist_from_mlpt);
        }

        if (dist_from_l == best_dist) { goto ACCESS_FROM_L; }
        if (dist_from_r == best_dist) { goto ACCESS_FROM_R; }

        if constexpr (meta::IsAnyOf<Type, AccessType_::AutoWithHint>) {
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
    if constexpr (meta::IsAnyOf<Type, AccessType_::AutoWithHint>) {
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

    n = static_cast<Node*>(*static_cast<void**>(multi_level_ptr_table::Access(
        mlpt,
        SrcBranchIdxes_{
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

    return elem;
}
}

template <bool EnWrite, CntrTplParamList, typename ReaderWriter>
void ReadWrite_(Cntr<CntrTplArgList>& cntr, Cursor const* pos_cursor,
                size_t cnt, ReaderWriter&& reader_writer, Cursor* dst_cursor) {
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
        size_t cur_cnt{ compare_utils::BasicMin(
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

    ZETA_Core_DebugAssert(0 < cnt);

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };

    multi_level_ptr_table::BranchNum rot{ cntr.rots[level_i] };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr) };

    if (level_i == 0) {
        size_t seg_size{ detail::GetSegSize_(elem_stride, seg_elem_slot_cnt) };

        while (0 < cnt) {
            // SANITIZE
            ZETA_Core_DebugAssert(seg_slot_idx < branch_num);

            size_t mlpt_slot_idx{ rot + seg_slot_idx };
            if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

            // SANITIZE
            ZETA_Core_DebugAssert(
                (mlpt_node->active_map &
                 (static_cast<ActiveMap>(1) << mlpt_slot_idx)) == 0);

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

        size_t cur_cnt{ compare_utils::BasicMin(
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
bool EraseSegs_(
    Cntr<CntrTplArgList>& cntr, unsigned level_i,
    multi_level_ptr_table::NavNode<typename Cntr<CntrTplArgList>::ActiveMap>*
        mlpt_node,
    size_t seg_slot_idx, size_t cnt) {
    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    ZETA_Core_DebugAssert(0 < cnt);

    multi_level_ptr_table::BranchNum rot{ cntr.rots[level_i] };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr) };

    if (level_i == 0) {
        while (0 < cnt) {
            // SANITIZE
            ZETA_Core_DebugAssert(seg_slot_idx < branch_num);

            size_t mlpt_slot_idx{ rot + seg_slot_idx };
            if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

            // SANITIZE
            ZETA_Core_DebugAssert(
                (mlpt_node->active_map &
                 (static_cast<ActiveMap>(1) << mlpt_slot_idx)) != 0);

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
        // SANITIZE
        ZETA_Core_DebugAssert(sub_tree_slot_idx < branch_num);

        size_t mlpt_slot_idx{ rot + sub_tree_slot_idx };
        if (branch_num <= mlpt_slot_idx) { mlpt_slot_idx -= branch_num; }

        // SANITIZE
        ZETA_Core_DebugAssert((mlpt_node->active_map &
                               (static_cast<ActiveMap>(1) << mlpt_slot_idx)) !=
                              0);

        size_t cur_cnt{ compare_utils::BasicMin(
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

template <int LR, CntrTplParamList, typename Writer>
void Push_(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer&& writer,
           Cursor* dst_cursor) {
    (CheckCntr_)(cntr);

    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t max_elem_cnt{ (GetMaxElemCnt_<branch_num>)(seg_elem_slot_cnt) };

    multi_level_ptr_table::BranchNum* rots{ cntr.rots };

    unsigned level{ cntr.level };

    Node* head_n{ cntr.head_n };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };

    ZETA_Core_DebugAssert(
        seq_cntr::IsInsertable(0, cnt, elem_cnt, max_elem_cnt));

    size_t cur_elem_cnt{ elem_cnt };
    size_t nxt_elem_cnt{ cur_elem_cnt + cnt };

    size_t cur_seg_elem_offset{ tree_elem_offset % seg_elem_slot_cnt };

    size_t nxt_seg_elem_offset;

    if constexpr (LR == 0) {
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
        if constexpr (LR == 0) {
            (Access_<detail::AccessType_::FromL>)(cntr, 0, true, nullptr,
                                                  dst_cursor);
        } else {
            (Access_<detail::AccessType_::FromR>)(cntr, elem_cnt, true, nullptr,
                                                  dst_cursor);
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

    if constexpr (LR == 0) {
        if (cur_elem_cnt == 0) {
            tree_l_seg_offset = acc_branch_nums[mlpt.level - 1] * branch_num;
        } else {
            size_t cur_sub_tree_cnt{ integral_bit::PopCount(
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                    mlpt.root)
                    ->active_map) };

            // SANITIZE
            ZETA_Core_DebugAssert(0 < cur_sub_tree_cnt);

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

    void* old_first_n;
    void* new_first_n;

    // SANITIZE
    if constexpr (LR == 1) {
        if (elem_cnt != 0) {
            void* mp{ multi_level_ptr_table::Access(
                mlpt,
                detail::SrcBranchIdxes_{
                    .seg_idx = tree_l_seg_offset + 0,
                    .rots = rots + (mlpt.level - 1),
                    .branch_num = branch_num,
                    .acc_branch_num =
                        TableMeta<branch_num>::acc_branch_nums[mlpt.level - 1],
                }) };

            ZETA_Core_DebugAssert(mp != nullptr);

            old_first_n = *static_cast<void**>(mp);
        }
    }

    while (LR == 0
               ? acc_branch_nums[mlpt.level] < tree_r_seg_offset + nxt_seg_cnt
               : acc_branch_nums[mlpt.level] <
                     tree_l_seg_offset + nxt_seg_cnt) {
        multi_level_ptr_table::NavNode<ActiveMap>* new_root{
            multi_level_ptr_table::detail::AllocateNavNode_<ActiveMap>(
                branch_num, node_alctr)
        };

        if constexpr (LR == 0) {
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

    // SANITIZE
    if constexpr (LR == 1) {
        if (elem_cnt != 0) {
            void* mp{ multi_level_ptr_table::Access(
                mlpt,
                detail::SrcBranchIdxes_{
                    .seg_idx = tree_l_seg_offset + 0,
                    .rots = rots + (mlpt.level - 1),
                    .branch_num = branch_num,
                    .acc_branch_num =
                        TableMeta<branch_num>::acc_branch_nums[mlpt.level - 1],
                }) };

            ZETA_Core_DebugAssert(mp != nullptr);

            new_first_n = *static_cast<void**>(mp);

            ZETA_Core_DebugAssert(old_first_n == new_first_n);
        }
    }

    Node* old_last_n{ elem_cnt == 0 ? head_n : llist::GetL(head_n) };
    size_t old_last_seg_elem_slot_idx{ elem_cnt == 0
                                           ? seg_elem_slot_cnt - 1
                                           : (tree_elem_offset + elem_cnt - 1) %
                                                 seg_elem_slot_cnt };

    if constexpr (LR == 0) {
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

    if constexpr (LR == 0) {
        rots[mlpt.level - 1] = ({
            size_t k{ rots[mlpt.level - 1] +
                      tree_l_seg_offset / acc_branch_nums[mlpt.level - 1] };
            static_cast<multi_level_ptr_table::BranchNum>(
                k < branch_num ? k : k - branch_num);
        });

        tree_l_seg_offset %= acc_branch_nums[mlpt.level - 1];
    }

    cntr.elem_cnt = nxt_elem_cnt;

    if constexpr (LR == 0) {
        cntr.elem_offset =
            seg_elem_slot_cnt * tree_l_seg_offset + nxt_seg_elem_offset;
    }

    cntr.level = mlpt.level;
    cntr.root = mlpt.root;

    dst_cursor->cntr = &cntr;

    if constexpr (LR == 0) {
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

    // SANITIZE
    (Sanitize)(cntr, nullptr, nullptr);

    // SANITIZE
    (CheckCursor_)(cntr, dst_cursor);

    (ReadWrite_<true>)(cntr, dst_cursor, cnt, writer, nullptr);
}

template <int LR, CntrTplParamList>
void Pop_(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    (CheckCntr_)(cntr);

    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t tree_elem_offset{ cntr.elem_offset };

    multi_level_ptr_table::BranchNum* rots{ cntr.rots };

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };

    ZETA_Core_DebugAssert(seq_cntr::IsErasable(0, cnt, elem_cnt));

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

            // SANITIZE
            ZETA_Core_DebugAssert(old_root->active_map != 0);

            if (1 < integral_bit::PopCount(old_root->active_map)) { break; }

            void* new_root{
                old_root->ptrs[integral_bit::CTZ(old_root->active_map)]
            };

            ZETA_Core_DebugAssert(new_root != nullptr);

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
    cntr.elem_offset =
        seg_elem_slot_cnt * tree_l_seg_offset + nxt_seg_elem_offset;

    cntr.level = mlpt.level;
    cntr.root = mlpt.root;
}

template <int LR, int CopyMove>
pair::Pair<pair::Pair<Node*, size_t>, pair::Pair<Node const*, size_t>> Assign_(
    size_t elem_size, size_t dst_elem_stride, size_t src_elem_stride,
    size_t dst_seg_elem_slot_cnt, size_t src_seg_elem_slot_cnt, Node* dst_n,
    Node const* src_n, size_t dst_seg_elem_slot_idx,
    size_t src_seg_elem_slot_idx, size_t cnt) {
    ZETA_Core_StaticAssert(LR == 0 || LR == 1);
    ZETA_Core_StaticAssert(CopyMove == 0 || CopyMove == 1);

    for (;;) {
        if constexpr (LR == 0) {
            if (dst_seg_elem_slot_idx == 0) {
                dst_n = llist::GetL(dst_n);
                dst_seg_elem_slot_idx = dst_seg_elem_slot_cnt;
            }

            if (src_seg_elem_slot_idx == 0) {
                src_n = llist::GetL(src_n);
                src_seg_elem_slot_idx = src_seg_elem_slot_cnt;
            }
        } else {
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

        size_t cur_cnt;
        void* dst_elem;
        void const* src_elem;

        if constexpr (LR == 0) {
            cur_cnt = compare_utils::BasicMin(
                cnt, compare_utils::BasicMin(src_seg_elem_slot_idx,
                                             dst_seg_elem_slot_idx));

            dst_elem = (NToSeg_)(dst_n)->data +
                       dst_elem_stride * (dst_seg_elem_slot_idx - cur_cnt);

            src_elem = (NToSeg_)(src_n)->data +
                       src_elem_stride * (src_seg_elem_slot_idx - cur_cnt);

        } else {
            cur_cnt = compare_utils::BasicMin(
                cnt, compare_utils::BasicMin(
                         src_seg_elem_slot_cnt - src_seg_elem_slot_idx,
                         dst_seg_elem_slot_cnt - dst_seg_elem_slot_idx));

            dst_elem = (NToSeg_)(dst_n)->data +
                       dst_elem_stride * dst_seg_elem_slot_idx;

            src_elem = (NToSeg_)(src_n)->data +
                       src_elem_stride * src_seg_elem_slot_idx;
        }

        if constexpr (CopyMove == 0) {
            utils::ElemCopy(dst_elem, src_elem, elem_size, dst_elem_stride,
                            src_elem_stride, cur_cnt);
        } else {
            utils::ElemMove(dst_elem, src_elem, elem_size, dst_elem_stride,
                            src_elem_stride, cur_cnt);
        }

        if constexpr (LR == 0) {
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

template <CntrTplParamList, typename NodeAllocatorInitArg,
          typename SegAllocatorInitArg>
void multi_level_circular_array::Init(
    Cntr<CntrTplArgList>& cntr, size_t elem_size, size_t elem_stride,
    size_t seg_elem_slot_cnt, NodeAllocatorInitArg&& node_alctr_init_arg,
    SegAllocatorInitArg&& seg_alctr_init_arg) {
    lifecycle::Init(cntr.node_alctr,
                    meta::Forward<NodeAllocatorInitArg>(node_alctr_init_arg));
    allocator::CheckContract(cntr.node_alctr);

    lifecycle::Init(cntr.seg_alctr,
                    meta::Forward<NodeAllocatorInitArg>(seg_alctr_init_arg));
    allocator::CheckContract(cntr.seg_alctr);

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);

    cntr.elem_size = elem_size;
    cntr.elem_stride = elem_stride;
    cntr.seg_elem_slot_cnt = seg_elem_slot_cnt;
    cntr.elem_offset = 0;
    cntr.elem_cnt = 0;

    cntr.rots[0] = 0;

    cntr.level = 1;
    cntr.root = nullptr;

    cntr.head_n = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(cntr.node_alctr), alignof(Node), sizeof(Node)));

    cntr.head_n->Init();
}

template <CntrTplParamList>
void multi_level_circular_array::Deinit(Cntr<CntrTplArgList>& cntr) {
    (EraseAll)(cntr);
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::GetCursorSize(
    Cntr<CntrTplArgList> const&) {
    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetElemSize(
    Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);
    return cntr.elem_size;
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetElemStride(
    Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);
    return cntr.elem_stride;
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetSegElemCapacity(
    Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);
    return cntr.seg_elem_slot_cnt;
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetElemCnt(
    Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);
    return cntr.elem_cnt;
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetMaxElemCnt(
    Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);
    // TODO
    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void multi_level_circular_array::GetLBCursor(Cntr<CntrTplArgList> const& cntr,
                                             Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->n = cntr.head_n;
    dst_cursor->seg_elem_slot_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void multi_level_circular_array::GetRBCursor(Cntr<CntrTplArgList> const& cntr,
                                             Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = cntr.elem_cnt;
    dst_cursor->n = cntr.head_n;
    dst_cursor->seg_elem_slot_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void* multi_level_circular_array::PeekL(Cntr<CntrTplArgList> const& cntr,
                                        bool lazy_copy_elem, Cursor* dst_cursor,
                                        void* dst_elem) {
    detail::CheckCntr_(cntr);

    return detail::Access_<detail::AccessType_::FromL>(cntr, 0, lazy_copy_elem,
                                                       dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* multi_level_circular_array::PeekR(Cntr<CntrTplArgList> const& cntr,
                                        bool lazy_copy_elem, Cursor* dst_cursor,
                                        void* dst_elem) {
    detail::CheckCntr_(cntr);

    size_t elem_cnt{ cntr.elem_cnt };

    return detail::Access_<detail::AccessType_::FromR>(
        cntr, elem_cnt - 1, lazy_copy_elem, dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* multi_level_circular_array::Access(Cntr<CntrTplArgList> const& cntr,
                                         size_t idx, bool lazy_copy_elem,
                                         Cursor* dst_cursor, void* dst_elem) {
    return detail::Access_<detail::AccessType_::AutoWithoutHint>(
        cntr, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* multi_level_circular_array::AccessWithHint(
    Cntr<CntrTplArgList> const& cntr, size_t idx, bool lazy_copy_elem,
    Cursor* dst_cursor, void* dst_elem) {
    return detail::Access_<detail::AccessType_::AutoWithHint>(
        cntr, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* multi_level_circular_array::Derefer(Cntr<CntrTplArgList> const& cntr,
                                          Cursor const* pos_cursor,
                                          bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    size_t elem_size{ cntr.elem_size };

    void* elem{ pos_cursor->elem };

    if (!lazy_copy_elem && elem != nullptr && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }

    return elem;
}

template <CntrTplParamList, typename Reader>
void multi_level_circular_array::Read(Cntr<CntrTplArgList> const& cntr,
                                      Cursor const* pos_cursor, size_t cnt,
                                      Reader&& reader, Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr<CntrTplArgList>&>(cntr),
                              pos_cursor, cnt, reader, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void multi_level_circular_array::Write(Cntr<CntrTplArgList>& cntr,
                                       Cursor* pos_cursor, size_t cnt,
                                       Writer&& writer, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList, typename ReaderWriter>
void multi_level_circular_array::ReadWrite(Cntr<CntrTplArgList>& cntr,
                                           Cursor* pos_cursor, size_t cnt,
                                           ReaderWriter&& reader_writer,
                                           Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* multi_level_circular_array::PushL(Cntr<CntrTplArgList>& cntr, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    if (dst_cursor == nullptr) {
        dst_cursor = static_cast<Cursor*>(__builtin_alloca(sizeof(Cursor)));
    }

    detail::Push_<0>(cntr, cnt, writer, dst_cursor);

    return dst_cursor->elem;
}

template <CntrTplParamList, typename Writer>
void* multi_level_circular_array::PushR(Cntr<CntrTplArgList>& cntr, size_t cnt,
                                        Writer&& writer, Cursor* dst_cursor) {
    if (dst_cursor == nullptr) {
        dst_cursor = static_cast<Cursor*>(__builtin_alloca(sizeof(Cursor)));
    }

    detail::Push_<1>(cntr, cnt, writer, dst_cursor);

    return dst_cursor->elem;
}

template <CntrTplParamList, typename Writer>
void* multi_level_circular_array::Insert(Cntr<CntrTplArgList>& cntr,
                                         Cursor* pos_cursor, size_t cnt,
                                         Writer&& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    if (cnt == 0) {
        if (dst_cursor != nullptr) { *dst_cursor = *pos_cursor; }
        return pos_cursor->elem;
    }

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t tree_elem_offset{ cntr.elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    size_t idx{ pos_cursor->idx };

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx };

    unsigned long long random_seed{ utils::GetRandom() };

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case 0: {
        if (dst_cursor == nullptr) {
            dst_cursor = static_cast<Cursor*>(__builtin_alloca(sizeof(Cursor)));
        }

        Node* old_l_n{ llist::GetR(cntr.head_n) };
        size_t old_l_seg_elem_slot_idx{ cntr.elem_offset % seg_elem_slot_cnt };

        detail::Push_<0>(cntr, cnt, seq_cntr::EmptyWriter{}, dst_cursor);

        if (cnt < seg_elem_slot_cnt) {
            auto p{ detail::Assign_<0, 1>(elem_size, elem_stride, elem_stride,
                                          seg_elem_slot_cnt, seg_elem_slot_cnt,
                                          dst_cursor->n, old_l_n,
                                          dst_cursor->seg_elem_slot_idx,
                                          old_l_seg_elem_slot_idx, l_cnt) };

            dst_cursor->n = p.first.first;
            dst_cursor->seg_elem_slot_idx = p.first.second;
        } else {
            auto p{ detail::Assign_<0, 0>(elem_size, elem_stride, elem_stride,
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

        detail::ReadWrite_<true>(cntr, dst_cursor, cnt, writer, nullptr);
    }
    case 1: {
        if (dst_cursor == nullptr) {
            dst_cursor = static_cast<Cursor*>(__builtin_alloca(sizeof(Cursor)));
        }

        detail::Push_<1>(cntr, cnt, seq_cntr::EmptyWriter{}, dst_cursor);

        Node* r_n{ llist::GetL(cntr.head_n) };

        size_t r_seg_elem_slot_idx{ (tree_elem_offset + elem_cnt + cnt - 1) %
                                    seg_elem_slot_cnt };

        if (cnt < seg_elem_slot_cnt) {
            auto p{ detail::Assign_<1, 1>(
                elem_size, elem_stride, elem_stride, seg_elem_slot_cnt,
                seg_elem_slot_cnt, r_n, dst_cursor->n, r_seg_elem_slot_idx,
                dst_cursor->seg_elem_slot_idx, r_cnt) };

            dst_cursor->n = const_cast<Node*>(p.second.first);
            dst_cursor->seg_elem_slot_idx = p.second.second;
        } else {
            auto p{ detail::Assign_<1, 0>(
                elem_size, elem_stride, elem_stride, seg_elem_slot_cnt,
                seg_elem_slot_cnt, r_n, dst_cursor->n, r_seg_elem_slot_idx,
                dst_cursor->seg_elem_slot_idx, r_cnt) };

            dst_cursor->n = const_cast<Node*>(p.second.first);
            dst_cursor->seg_elem_slot_idx = p.second.second;
        }

        dst_cursor->idx -= r_cnt;
        dst_cursor->elem = detail::NToSeg_(dst_cursor->n)->data +
                           elem_stride * dst_cursor->seg_elem_slot_idx;

        detail::ReadWrite_<true>(cntr, dst_cursor, cnt, writer, nullptr);
    }
    default: __builtin_unreachable();
    }
}

template <CntrTplParamList>
void multi_level_circular_array::PopL(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::Pop_<0>(cntr, cnt);
}

template <CntrTplParamList>
void multi_level_circular_array::PopR(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::Pop_<1>(cntr, cnt);
}

template <CntrTplParamList>
void multi_level_circular_array::Erase(Cntr<CntrTplArgList>& cntr,
                                       Cursor* pos_cursor, size_t cnt) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, pos_cursor);

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(seq_cntr::IsErasable(idx, cnt, elem_cnt));

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx };

    if (r_cnt == 0) {
        (PopR)(cntr, cnt);
        (GetRBCursor)(cntr, pos_cursor);
        return;
    }

    if (l_cnt == 0) {
        (PopL)(cntr, cnt);
        (PeekL)(cntr, true, pos_cursor, nullptr);
        return;
    }

    unsigned long long random_seed{ utils::GetRandom() };

    Cursor end_cursor{ *pos_cursor };
    (AccessWithHint)(cntr, idx + cnt, true, &end_cursor, nullptr);

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case 0: {
        if (cnt < seg_elem_slot_cnt) {
            detail::Assign_<0, 1>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  pos_cursor->n, end_cursor.n,
                                  pos_cursor->seg_elem_slot_idx,
                                  end_cursor.seg_elem_slot_idx, l_cnt);
        } else {
            detail::Assign_<0, 0>(elem_size, elem_stride, elem_stride,
                                  seg_elem_slot_cnt, seg_elem_slot_cnt,
                                  pos_cursor->n, end_cursor.n,
                                  pos_cursor->seg_elem_slot_idx,
                                  end_cursor.seg_elem_slot_idx, l_cnt);
        }

        detail::Pop_<0>(cntr, cnt);

        *pos_cursor = end_cursor;

        return;
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

        return;
    }
    default: __builtin_unreachable();
    }
}

template <CntrTplParamList>
void multi_level_circular_array::EraseAll(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    (PopR)(cntr, cntr.elem_cnt);
}

template <CntrTplParamList>
void multi_level_circular_array::CopyCursor(Cntr<CntrTplArgList> const& cntr,
                                            Cursor const* src_cursor,
                                            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool multi_level_circular_array::AreEqualCursor(
    Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

template <CntrTplParamList>
int multi_level_circular_array::CompareCursor(Cntr<CntrTplArgList> const& cntr,
                                              Cursor const* cursor_a,
                                              Cursor const* cursor_b) {
    return compare::BasicCompare(compare::compare_type::ThreeWay{},
                                 (GetCursorIdx)(cntr, cursor_a) + 1,
                                 (GetCursorIdx)(cntr, cursor_b) + 1);
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetCursorDist(
    Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_b) - (GetCursorIdx)(cntr, cursor_a);
}

template <CntrTplParamList>
size_t multi_level_circular_array::GetCursorIdx(
    Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

template <CntrTplParamList>
void multi_level_circular_array::CursorStepL(Cntr<CntrTplArgList> const& cntr,
                                             Cursor* cursor) {
    (CursorAdvanceL)(cntr, cursor, 1);
}

template <CntrTplParamList>
void multi_level_circular_array::CursorStepR(Cntr<CntrTplArgList> const& cntr,
                                             Cursor* cursor) {
    (CursorAdvanceR)(cntr, cursor, 1);
}

template <CntrTplParamList>
void multi_level_circular_array::CursorAdvanceL(
    Cntr<CntrTplArgList> const& cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(
        seq_cntr::IsAdvancableL(cursor->idx, step, cntr.elem_cnt));

    detail::Access_<detail::AccessType_::AutoWithHint>(cntr, cursor->idx - step,
                                                       true, cursor, nullptr);
}

template <CntrTplParamList>
void multi_level_circular_array::CursorAdvanceR(
    Cntr<CntrTplArgList> const& cntr, Cursor* cursor, size_t step) {
    detail::CheckCntr_(cntr);
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugAssert(
        seq_cntr::IsAdvancableR(cursor->idx, step, cntr.elem_cnt));

    detail::Access_<detail::AccessType_::AutoWithHint>(cntr, cursor->idx + step,
                                                       true, cursor, nullptr);
}

namespace multi_level_circular_array::detail {

template <CntrTplParamList>
pair::Pair<Node*, Node*> SanitizeMLPTNode_(
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

        size_t cur_cnt{ compare_utils::BasicMin(
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
void multi_level_circular_array::Sanitize(Cntr<CntrTplArgList> const& cntr,
                                          mem_recorder::MemRecorder* dst_node,
                                          mem_recorder::MemRecorder* dst_seg) {
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };

    detail::CheckCntr_(cntr);

    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_offset{ cntr.elem_offset };
    size_t elem_cnt{ cntr.elem_cnt };

    Node* head_n{ cntr.head_n };

    size_t tree_seg_offset{ elem_offset / seg_elem_slot_cnt };
    size_t seg_elem_offset{ elem_offset % seg_elem_slot_cnt };

    size_t seg_cnt{ integral_math::CeilDiv(seg_elem_offset + elem_cnt,
                                           seg_elem_slot_cnt) };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr) };

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
            void* mp{ multi_level_ptr_table::Access(
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
            void* mp{ multi_level_ptr_table::Access(
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
template <typename... Args>
void lifecycle::Traits<multi_level_circular_array::Cntr<CntrTplArgList>>::Init(
    multi_level_circular_array::Cntr<CntrTplArgList>& cntr, Args&&... args) {
    multi_level_circular_array::Init(cntr, meta::Forward<Args>(args)...);
}

template <CntrTplParamList>
void lifecycle::Traits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    Deinit(multi_level_circular_array::Cntr<CntrTplArgList>& cntr) {
    multi_level_circular_array::Deinit(cntr);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetReferedInstPtr(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return const_cast<multi_level_circular_array::Cntr<CntrTplArgList>*>(&cntr);
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
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
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetStaticEnabledAbilityFlag() {
    return seq_cntr::CntrTraits<multi_level_circular_array::Cntr<
               CntrTplArgList>>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledAbilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledAbilityFlag(
        multi_level_circular_array::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetCursorSize(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return multi_level_circular_array::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetElemSize(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return multi_level_circular_array::GetElemSize(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetElemCnt(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return multi_level_circular_array::GetElemCnt(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetMaxElemCnt(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr) {
    return multi_level_circular_array::GetMaxElemCnt(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetLBCursor(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                void* dst_cursor) {
    multi_level_circular_array::GetLBCursor(
        cntr, static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetRBCursor(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                void* dst_cursor) {
    multi_level_circular_array::GetRBCursor(
        cntr, static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* seq_cntr::
    CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::PeekL(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return multi_level_circular_array::PeekL(
        cntr, lazy_copy_elem,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::
    CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::PeekR(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return multi_level_circular_array::PeekR(
        cntr, lazy_copy_elem,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::
    CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::Access(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        size_t idx, bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return multi_level_circular_array::Access(
        cntr, idx, lazy_copy_elem,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::
    CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::Derefer(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* pos_cursor, bool lazy_copy_elem, void* dst_elem) {
    return multi_level_circular_array::Derefer(
        cntr,
        static_cast<multi_level_circular_array::Cursor const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename Reader>
void seq_cntr::
    CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::Read(
        multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
        void const* pos_cursor, size_t cnt,
        Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
        void* dst_cursor) {
    multi_level_circular_array::Read(
        cntr,
        static_cast<multi_level_circular_array::Cursor const*>(pos_cursor), cnt,
        reader, static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    Write(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
          void* pos_cursor, size_t cnt,
          Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
          void* dst_cursor) {
    multi_level_circular_array::Write(
        cntr, static_cast<multi_level_circular_array::Cursor*>(pos_cursor), cnt,
        writer, static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename ReaderWriter>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    ReadWrite(
        multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
        void* pos_cursor, size_t cnt,
        ReaderWriter&&
            reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
        void* dst_cursor) {
    multi_level_circular_array::ReadWrite(
        cntr, static_cast<multi_level_circular_array::Cursor*>(pos_cursor), cnt,
        reader_writer,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    PushL(multi_level_circular_array::Cntr<CntrTplArgList>& cntr, size_t cnt,
          Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
          void* dst_cursor) {
    return multi_level_circular_array::PushL(
        cntr, cnt, writer,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    PushR(multi_level_circular_array::Cntr<CntrTplArgList>& cntr, size_t cnt,
          Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
          void* dst_cursor) {
    return multi_level_circular_array::PushR(
        cntr, cnt, writer,
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    Insert(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
           void* pos_cursor, size_t cnt,
           Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
           void* dst_cursor) {
    return multi_level_circular_array::Insert(
        cntr, static_cast<multi_level_circular_array::Cursor*>(pos_cursor), cnt,
        writer, static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    PopL(multi_level_circular_array::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    multi_level_circular_array::PopL(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    PopR(multi_level_circular_array::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    multi_level_circular_array::PopR(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    Erase(multi_level_circular_array::Cntr<CntrTplArgList>& cntr,
          void* pos_cursor, size_t cnt) {
    multi_level_circular_array::Erase(
        cntr, static_cast<multi_level_circular_array::Cursor*>(pos_cursor),
        cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList>>::
    EraseAll(multi_level_circular_array::Cntr<CntrTplArgList>& cntr) {
    multi_level_circular_array::EraseAll(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CopyCursor(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
               void const* src_cursor, void* dst_cursor) {
    multi_level_circular_array::CopyCursor(
        cntr,
        static_cast<multi_level_circular_array::Cursor const*>(src_cursor),
        static_cast<multi_level_circular_array::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    AreEqualCursor(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                   void const* cursor_a, void const* cursor_b) {
    return multi_level_circular_array::AreEqualCursor(
        cntr, static_cast<multi_level_circular_array::Cursor const*>(cursor_a),
        static_cast<multi_level_circular_array::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
int seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CompareCursor(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                  void const* cursor_a, void const* cursor_b) {
    return multi_level_circular_array::CompareCursor(
        cntr, static_cast<multi_level_circular_array::Cursor const*>(cursor_a),
        static_cast<multi_level_circular_array::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetCursorDist(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                  void const* cursor_a, void const* cursor_b) {
    return multi_level_circular_array::GetCursorDist(
        cntr, static_cast<multi_level_circular_array::Cursor const*>(cursor_a),
        static_cast<multi_level_circular_array::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<multi_level_circular_array::Cntr<CntrTplArgList> const>::
    GetCursorIdx(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                 void const* cursor) {
    return multi_level_circular_array::GetCursorIdx(
        cntr, static_cast<multi_level_circular_array::Cursor const*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CursorStepL(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    multi_level_circular_array::CursorStepL(
        cntr, static_cast<multi_level_circular_array::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CursorStepR(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    multi_level_circular_array::CursorStepR(
        cntr, static_cast<multi_level_circular_array::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CursorAdvanceL(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                   void* cursor, size_t step) {
    multi_level_circular_array::CursorAdvanceL(
        cntr, static_cast<multi_level_circular_array::Cursor*>(cursor), step);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<
    multi_level_circular_array::Cntr<CntrTplArgList> const>::
    CursorAdvanceR(multi_level_circular_array::Cntr<CntrTplArgList> const& cntr,
                   void* cursor, size_t step) {
    multi_level_circular_array::CursorAdvanceR(
        cntr, static_cast<multi_level_circular_array::Cursor*>(cursor), step);
}

}  // namespace zeta::core

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")
