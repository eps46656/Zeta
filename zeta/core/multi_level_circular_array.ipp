#pragma once

#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/mem_recording.ipp>
#include <zeta/core/debug_utils/recording_allocator.ipp>
#include <zeta/core/debug_utils/revision.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/multi_level_circular_array.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core/utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                               \
    meta::IsNonConstObj Elem,                                          \
        multi_level_circular_array::IsLayoutConfig auto layout_config, \
        allocator::IsAllocatorLike NodeAllocatorLike,                  \
        allocator::IsAllocatorLike SegAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList Elem, layout_config, NodeAllocatorLike, SegAllocatorLike

namespace zeta::core {

namespace multi_level_circular_array::detail {

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt>
constexpr DataSeg<Elem, seg_elem_slot_cnt>* NToDataSeg_(Node* n) {
    return ZETA_Core_MemberToStruct(
        ZETA_Core_Identity(DataSeg<Elem, seg_elem_slot_cnt>), n, n);
}

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt>
constexpr DataSeg<Elem, seg_elem_slot_cnt> const* NToDataSeg_(Node const* n) {
    return ZETA_Core_MemberToStruct(
        ZETA_Core_Identity(DataSeg<Elem, seg_elem_slot_cnt>), n, n);
}

}  // namespace multi_level_circular_array::detail

#if ZETA_Core_DebugEnable

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt>
constexpr multi_level_circular_array::DataSeg<Elem,
                                              seg_elem_slot_cnt>::DataSeg()
    : obj_hook{
          this,
          sizeof(Elem),
          {},
      } {}

#endif

namespace multi_level_circular_array::detail {

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt,
          allocator::IsAllocator SegAllocator>
constexpr DataSeg<Elem, seg_elem_slot_cnt>* CreateDataSeg_(
    meta::TypeWrapper<DataSeg<Elem, seg_elem_slot_cnt>>,
    SegAllocator& seg_alctr) {
    DataSeg<Elem, seg_elem_slot_cnt>* data_seg{
        new (allocator::SafeAllocate(seg_alctr,
                                     alignof(DataSeg<Elem, seg_elem_slot_cnt>),
                                     sizeof(DataSeg<Elem, seg_elem_slot_cnt>)))
            DataSeg<Elem, seg_elem_slot_cnt>{}
    };

    return data_seg;
}

}  // namespace multi_level_circular_array::detail

namespace multi_level_circular_array::detail {

template <meta::IsNonConstObj Elem, size_t seg_elem_slot_cnt,
          allocator::IsAllocator SegAllocator>
constexpr void DestroyDataSeg_(DataSeg<Elem, seg_elem_slot_cnt>* data_seg,
                               SegAllocator& seg_alctr) {
    data_seg->~DataSeg<Elem, seg_elem_slot_cnt>();
    allocator::Deallocate(seg_alctr, data_seg);
}

}  // namespace multi_level_circular_array::detail

namespace multi_level_circular_array::detail {

template <CntrTplParamList>
constexpr void CheckCntr_(Cntr<CntrTplArgList> const& self) {
    constexpr unsigned level_max{ layout_config.level_max };
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_elem_slot_cnts{ layout_config.acc_elem_slot_cnts };
    constexpr size_t max_elem_cnt{ layout_config.max_elem_cnt };

    size_t elem_cnt{ self.elem_cnt };
    size_t elem_offset{ self.tree_elem_offset };

    size_t level{ self.level };
    void* root{ self.root };

    Node* head_n{ self.head_n };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < level);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(level <= level_max);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= max_elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert((elem_cnt == 0) ==
                                            (root == nullptr));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_offset <
                                            acc_elem_slot_cnts[level - 1]);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <=
                                            acc_elem_slot_cnts[level]);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_offset + elem_cnt <=
                                            acc_elem_slot_cnts[level]);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(head_n != nullptr);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.rots[level_i] <
                                                branch_nums[level_i]);
    }
}

template <CntrTplParamList>
constexpr void CheckCursor_(Cntr<CntrTplArgList> const& self,
                            Cursor const* cursor) {
    (CheckCntr_)(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    Cursor re_cursor;
    self.Refer(seq_cntr::Tag{}, cursor->idx, true, nullptr, &re_cursor,
               nullptr);

    if (*cursor != re_cursor) {
        ZETA_Core_DebugUtils_Diag_LogVar(self.elem_cnt);

        ZETA_Core_DebugUtils_Diag_LogVar(&self);
        ZETA_Core_DebugUtils_Diag_LogVar(cursor->idx);
        ZETA_Core_DebugUtils_Diag_LogVar(re_cursor.idx);
        ZETA_Core_DebugUtils_Diag_LogVar(cursor->n);
        ZETA_Core_DebugUtils_Diag_LogVar(re_cursor.n);
        ZETA_Core_DebugUtils_Diag_LogVar(cursor->seg_elem_slot_idx);
        ZETA_Core_DebugUtils_Diag_LogVar(re_cursor.seg_elem_slot_idx);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(*cursor == re_cursor);
}

struct SrcBranchIdxesProvider_ {
    size_t seg_idx;
    multi_level_ptr_table::BranchNum const* rots;
    size_t const* branch_nums;
    size_t const* acc_branch_nums;

    static constexpr bool GetElemSize(seq_endpoint::provider::Tag) {
        return sizeof(multi_level_ptr_table::BranchNum);
    }

    static constexpr bool IsEnd(seq_endpoint::provider::Tag) { return false; }

    constexpr size_t Transfer(this SrcBranchIdxesProvider_& self,
                              seq_endpoint::provider::Tag,
                              multi_level_ptr_table::BranchNum* dst,
                              ptrdiff_t elem_stride, size_t elem_cnt) {
        size_t transfer_elem_cnt{ 0 };

        for (; transfer_elem_cnt < elem_cnt; ++transfer_elem_cnt) {
            size_t rot{ *self.rots };
            size_t branch_num{ *self.branch_nums };
            size_t acc_branch_num{ *self.acc_branch_nums };

            size_t ret{ self.seg_idx / acc_branch_num + rot };

            if (branch_num <= ret) { ret -= branch_num; }

            self.seg_idx %= acc_branch_num;

            --self.rots;
            --self.branch_nums;
            --self.acc_branch_nums;

            *(static_cast<multi_level_ptr_table::BranchNum*>(dst)) =
                static_cast<multi_level_ptr_table::BranchNum>(ret);

            dst = utils::PtrInc(dst, elem_stride);
        }

        return transfer_elem_cnt;
    }
};

enum struct ReferStrategy_ : unsigned char {
    FromL           = 0b0001,
    FromR           = 0b0010,
    AutoWithHint    = 0b0100,
    AutoWithoutHint = 0b1100,
};

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          ReferStrategy_ strategy, lifecycle::IsDstStorageView DstElemView>
constexpr void Refer_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
    meta::ValueWrapper<ReferStrategy_, strategy>, size_t idx,
    bool lazy_copy_elem, lifecycle::DynamicStorageView<Elem>* dst_elem_view,
    Cursor* dst_cursor, DstElemView dst_elem) {
    static_assert(strategy == ReferStrategy_::FromL ||
                  strategy == ReferStrategy_::FromR ||
                  strategy == ReferStrategy_::AutoWithHint ||
                  strategy == ReferStrategy_::AutoWithoutHint);

    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_branch_nums{ layout_config.acc_branch_nums };

    auto& self{ self_view->GetRef() };

    (CheckCntr_)(self);

    if constexpr (strategy == ReferStrategy_::AutoWithHint) {
        (CheckCursor_)(self, dst_cursor);
    }

    size_t tree_elem_offset{ self.tree_elem_offset };
    size_t elem_cnt{ self.elem_cnt };

    multi_level_ptr_table::BranchNum const* rots{ self.rots };

    unsigned level{ self.level };
    void* root{ self.root };

    Node* head_n{ self.head_n };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, elem_cnt));

    if (idx == static_cast<size_t>(-1) || idx == elem_cnt) {
        if (dst_elem_view != nullptr) {
            dst_elem_view->semantics =
                lifecycle::DynamicStorageView<Elem>::SemanticsEnum::LeftObj;
            dst_elem_view->ptr = nullptr;
        }

        if (dst_cursor != nullptr) {
            dst_cursor->idx               = idx;
            dst_cursor->n                 = head_n;
            dst_cursor->seg_elem_slot_idx = 0;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
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

    if constexpr (strategy == ReferStrategy_::FromL) { goto ACCESS_FROM_L; }

    if constexpr (strategy == ReferStrategy_::FromR) { goto ACCESS_FROM_R; }

    if constexpr (strategy == ReferStrategy_::AutoWithHint ||
                  strategy == ReferStrategy_::AutoWithoutHint) {
        if constexpr (strategy == ReferStrategy_::AutoWithHint) {
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

        if constexpr (strategy == ReferStrategy_::AutoWithHint) {
            best_dist = comparison_utils::BasicMin(
                dist_from_l, dist_from_r, dist_from_mlpt, dist_from_cursor);
        } else {
            best_dist = comparison_utils::BasicMin(dist_from_l, dist_from_r,
                                                   dist_from_mlpt);
        }

        if (dist_from_l == best_dist) { goto ACCESS_FROM_L; }
        if (dist_from_r == best_dist) { goto ACCESS_FROM_R; }

        if constexpr (strategy == ReferStrategy_::AutoWithHint) {
            if (dist_from_cursor == best_dist) { goto ACCESS_FROM_CURSOR; }
        }

        goto ACCESS_FROM_MLPT;
    }

ACCESS_FROM_L:
    if constexpr (strategy == ReferStrategy_::FromL ||
                  strategy == ReferStrategy_::AutoWithHint ||
                  strategy == ReferStrategy_::AutoWithoutHint) {
        n = head_n;
        for (size_t i{ 0 }; i < dist_from_l; ++i) { n = llist::GetR(n); }
        goto END;
    } else {
        static_assert(false);
    }

ACCESS_FROM_R:
    if constexpr (strategy == ReferStrategy_::FromR ||
                  strategy == ReferStrategy_::AutoWithHint ||
                  strategy == ReferStrategy_::AutoWithoutHint) {
        n = head_n;
        for (size_t i{ 0 }; i < dist_from_r; ++i) { n = llist::GetL(n); }
        goto END;
    } else {
        static_assert(false);
    }

ACCESS_FROM_CURSOR:
    if constexpr (strategy == ReferStrategy_::AutoWithoutHint) {
        if constexpr (strategy == ReferStrategy_::AutoWithHint) {
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
    } else {
        static_assert(false);
    }

ACCESS_FROM_MLPT:
    if constexpr (strategy == ReferStrategy_::AutoWithHint ||
                  strategy == ReferStrategy_::AutoWithoutHint) {
        multi_level_ptr_table::Cntr<typename Cntr<CntrTplArgList>::ActiveMap,
                                    meta::RemoveRef<decltype(node_alctr)>>
            mlpt{
                lifecycle::DirectConstructTag{},
                level,
                branch_nums.elems,
                elem_cnt,
                root,
                node_alctr,
            };

        n = static_cast<Node*>(
            *static_cast<void**>(mlpt.Access(SrcBranchIdxesProvider_{
                .seg_idx         = tree_seg_offset + seg_idx,
                .rots            = rots + (level - 1),
                .branch_nums     = branch_nums.elems + (level - 1),
                .acc_branch_nums = acc_branch_nums + (level - 1),
            })));

        mlpt.Disown();

        goto END;
    } else {
        static_assert(false);
    }

END:
    {
        lifecycle::StorageView<cntr_semantics, Elem> elem_view{
            (NToDataSeg_<Elem, seg_elem_slot_cnt>)(n)->elems + seg_elem_slot_idx
        };

        if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

        if (dst_cursor != nullptr) {
            dst_cursor->idx               = idx;
            dst_cursor->n                 = n;
            dst_cursor->seg_elem_slot_idx = seg_elem_slot_idx;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        if (elem_view.ptr != nullptr && !lazy_copy_elem &&
            dst_elem != nullptr) {
            lifecycle::StorageTransfer(*dst_elem, elem_view);
        }
    }
}

enum struct Direction : unsigned char {
    L,
    R,
};

template <Direction direction, size_t dst_seg_elem_slot_cnt,
          size_t src_seg_elem_slot_cnt,
          lifecycle::IsDstStorageView DstStorageView,
          lifecycle::IsSrcStorageView SrcStorageView>
constexpr void DataTransfer_(meta::TypeWrapper<DstStorageView>,
                             meta::TypeWrapper<SrcStorageView>,
                             meta::ValueWrapper<Direction, direction>,
                             meta::ValueWrapper<size_t, dst_seg_elem_slot_cnt>,
                             meta::ValueWrapper<size_t, src_seg_elem_slot_cnt>,
                             Cursor& dst_cursor, Cursor& src_cursor,
                             size_t cnt) {
    static_assert(direction == Direction::L || direction == Direction::R);

    using DstElem = typename DstStorageView::Obj;
    using SrcElem = typename SrcStorageView::Obj;

    for (;;) {
        if constexpr (direction == Direction::R) {
            if (dst_cursor.seg_elem_slot_idx == dst_seg_elem_slot_cnt) {
                dst_cursor.n                 = llist::GetR(dst_cursor.n);
                dst_cursor.seg_elem_slot_idx = 0;
            }

            if (src_cursor.seg_elem_slot_idx == src_seg_elem_slot_cnt) {
                src_cursor.n                 = llist::GetR(src_cursor.n);
                src_cursor.seg_elem_slot_idx = 0;
            }
        }

        if (cnt == 0) { break; }

        if constexpr (direction == Direction::L) {
            if (dst_cursor.seg_elem_slot_idx == 0) {
                dst_cursor.n                 = llist::GetL(dst_cursor.n);
                dst_cursor.seg_elem_slot_idx = dst_seg_elem_slot_cnt;
            }

            if (src_cursor.seg_elem_slot_idx == 0) {
                src_cursor.n                 = llist::GetL(src_cursor.n);
                src_cursor.seg_elem_slot_idx = src_seg_elem_slot_cnt;
            }
        } else if constexpr (direction == Direction::R) {
        } else {
            static_assert(false);
        }

        size_t cur_cnt;
        DstElem* dst_elem;
        SrcElem* src_elem;

        if constexpr (direction == Direction::L) {
            cur_cnt =
                comparison_utils::BasicMin(dst_cursor.seg_elem_slot_idx,
                                           src_cursor.seg_elem_slot_idx, cnt);

            dst_elem =
                (NToDataSeg_<DstElem, dst_seg_elem_slot_cnt>)(dst_cursor.n)
                    ->data +
                (dst_cursor.seg_elem_slot_idx - cur_cnt);

            src_elem =
                (NToDataSeg_<SrcElem, src_seg_elem_slot_cnt>)(src_cursor.n)
                    ->data +
                (src_cursor.seg_elem_slot_idx - cur_cnt);

            for (size_t i{ cur_cnt }; 0 < i;
                 --i, dst_elem = utils::PtrDec(dst_elem, sizeof(DstElem)),
                      src_elem = utils::PtrDec(src_elem, sizeof(SrcElem))) {
                lifecycle::StorageTransfer(DstStorageView{ dst_elem },
                                           SrcStorageView{ src_elem });
            }
        } else if (direction == Direction::R) {
            cur_cnt = comparison_utils::BasicMin(
                dst_seg_elem_slot_cnt - dst_cursor.seg_elem_slot_idx,
                src_seg_elem_slot_cnt - src_cursor.seg_elem_slot_idx, cnt);

            dst_elem =
                (NToDataSeg_<DstElem, dst_seg_elem_slot_cnt>)(dst_cursor.n)
                    ->data +
                dst_cursor.seg_elem_slot_idx;

            src_elem =
                (NToDataSeg_<SrcElem, src_seg_elem_slot_cnt>)(src_cursor.n)
                    ->data +
                src_cursor.seg_elem_slot_idx;

            for (size_t i{ cur_cnt }; 0 < i;
                 --i, dst_elem = utils::PtrInc(dst_elem, sizeof(DstElem)),
                      src_elem = utils::PtrInc(src_elem, sizeof(SrcElem))) {
                lifecycle::StorageTransfer(DstStorageView{ dst_elem },
                                           SrcStorageView{ src_elem });
            }
        } else {
            static_assert(false);
        }

        if constexpr (direction == Direction::L) {
            dst_cursor.idx -= cur_cnt;
            dst_cursor.seg_elem_slot_idx -= cur_cnt;

            src_cursor.idx -= cur_cnt;
            src_cursor.seg_elem_slot_idx -= cur_cnt;
        } else if constexpr (direction == Direction::R) {
            dst_cursor.idx += cur_cnt;
            dst_cursor.seg_elem_slot_idx += cur_cnt;

            src_cursor.idx += cur_cnt;
            src_cursor.seg_elem_slot_idx += cur_cnt;
        } else {
            static_assert(false);
        }

        cnt -= cur_cnt;
    }
}

template <
    meta::IsAnySame<seq_endpoint::acceptor::Tag, seq_endpoint::provider::Tag>
        EndpointTag,
    lifecycle::IsStorageView StorageView, size_t seg_elem_slot_cnt,
    typename Endpoint>
constexpr void EndpointTransfer_(EndpointTag, meta::TypeWrapper<StorageView>,
                                 meta::ValueWrapper<size_t, seg_elem_slot_cnt>,
                                 Cursor& cursor, size_t cnt,
                                 Endpoint&& endpoint) {
    using Elem = typename StorageView::Obj;

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, seg_elem_slot_cnt - cursor.seg_elem_slot_idx) };

        if constexpr (meta::IsSame<EndpointTag, seq_endpoint::acceptor::Tag>) {
            seq_endpoint::acceptor::Transfer(
                endpoint,
                StorageView{
                    (NToDataSeg_<Elem, seg_elem_slot_cnt>)(cursor.n)->data +
                    cursor.seg_elem_slot_idx },
                sizeof(Elem), cur_cnt);
        } else if constexpr (meta::IsSame<EndpointTag,
                                          seq_endpoint::provider::Tag>) {
            seq_endpoint::provider::Transfer(
                endpoint,
                StorageView{
                    (NToDataSeg_<Elem, seg_elem_slot_cnt>)(cursor.n)->data +
                    cursor.seg_elem_slot_idx },
                sizeof(Elem), cur_cnt);
        } else {
            static_assert(false);
        }

        cursor.seg_elem_slot_idx += cur_cnt;

        if (cursor.seg_elem_slot_idx == seg_elem_slot_cnt) {
            cursor.seg_elem_slot_idx = 0;
            cursor.n                 = llist::GetR(cursor.n);
        }

        cnt -= cur_cnt;
    }
}

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          seq_endpoint::provider::IsProvider<Elem> Provider>
void InsertElems_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
    unsigned level, void* mlpt_node_or_n, size_t elem_slot_idx, size_t cnt,
    Node* tail, Provider&& provider) {
    using ActiveMap = typename decltype(layout_config)::ActiveMap;
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_branch_nums{ layout_config.acc_branch_nums };
    constexpr auto acc_elem_slot_cnts{ layout_config.acc_elem_slot_cnts };

    auto& self{ self_view->GetRef() };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };
    auto& seg_alctr{ meta::GetInstRef(self.seg_alctr_like) };

    if (level == 0) {
        seq_endpoint::provider::Transfer(
            provider,
            lifecycle::DestructingObjToMemView<Elem>{
                (NToDataSeg_<Elem, seg_elem_slot_cnt>)(mlpt_node_or_n)->data +
                elem_slot_idx },
            sizeof(Elem), cnt);

        return;
    }

    constexpr size_t sub_tree_elem_slot_cnt{ acc_elem_slot_cnts[level - 1] };

    multi_level_ptr_table::BranchNum rot{ self.rots[level - 1] };

    size_t sub_tree_slot_idx{ elem_slot_idx / sub_tree_elem_slot_cnt };
    size_t sub_elem_slot_idx{ elem_slot_idx % sub_tree_elem_slot_cnt };

    multi_level_ptr_table::NavNode<ActiveMap>* mlpt_node{
        static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(mlpt_node_or_n)
    };

    while (0 < cnt) {
        size_t mlpt_slot_idx{ rot + sub_tree_slot_idx };

        if (branch_nums[level - 1] <= mlpt_slot_idx) {
            mlpt_slot_idx -= branch_nums[level - 1];
        }

        size_t cur_cnt{ comparison_utils::BasicMin(
            sub_tree_elem_slot_cnt - sub_elem_slot_idx, cnt) };

        void* sub_n;

        if ((mlpt_node->active_map &
             (static_cast<ActiveMap>(1) << mlpt_slot_idx)) == 0) {
            if (level == 0) {
                sub_n = 0;
                // TODO
            } else {
                sub_n = multi_level_ptr_table::detail::CreateNavNode_(
                    node_alctr, branch_nums[level - 1],
                    meta::TypeWrapper<ActiveMap>{});
            }

            mlpt_node->active_map += static_cast<ActiveMap>(1) << mlpt_slot_idx;
            mlpt_node->ptrs[mlpt_slot_idx] = sub_n;
        } else {
            sub_n = static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(
                mlpt_node->ptrs[mlpt_slot_idx]);
        }

        (InsertElems_)(self_view, level - 1, sub_n, sub_elem_slot_idx, cur_cnt,
                       tail);

        ++sub_tree_slot_idx;
        sub_elem_slot_idx = 0;
        cnt -= cur_cnt;
    }

    return;
}

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          Direction direction,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void Push_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
    meta::ValueWrapper<Direction, direction>, size_t cnt, Provider&& provider) {
    static_assert(direction == Direction::L || direction == Direction::R);

    using ActiveMap = typename decltype(layout_config)::AtiveMap;
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_branch_nums{ layout_config.acc_branch_nums };
    constexpr auto acc_elem_slot_cnts{ layout_config.acc_elem_slot_cnts };
    constexpr size_t max_elem_cnt{ layout_config.max_elem_cnt };

    auto& self{ self_view->GetRef() };

    (CheckCntr_)(self);

    size_t tree_elem_offset{ self.tree_elem_offset };
    size_t elem_cnt{ self.elem_cnt };

    multi_level_ptr_table::BranchNum* rots{ self.rots };

    unsigned level{ self.level };
    void* root{ self.root };

    Node* head_n{ self.head_n };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanInsert(0, cnt, elem_cnt, max_elem_cnt));

    if (cnt == 0) { return; }

    if (root == nullptr) {
        multi_level_ptr_table::NavNode<ActiveMap>* new_root{
            multi_level_ptr_table::detail::CreateNavNode_(
                node_alctr, branch_nums[0], meta::TypeWrapper<ActiveMap>{})
        };

        new_root->active_map = 0;

        root = new_root;

        rots[0] = 0;
    }

    if constexpr (direction == Direction::L) {
        if (elem_cnt == 0) {
            tree_elem_offset = seg_elem_slot_cnt * acc_branch_nums[level - 1] *
                               branch_nums[level - 1];
        } else {
            size_t cur_sub_tree_cnt{ integral_bit::PopCount(
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root)
                    ->active_map) };

            size_t rot_dec{ branch_nums[level - 1] - cur_sub_tree_cnt };

            tree_elem_offset += acc_elem_slot_cnts[level - 1] * rot_dec;

            rots[level - 1] = ({
                size_t k{ rots[level - 1] + branch_nums[level - 1] - rot_dec };
                static_cast<multi_level_ptr_table::BranchNum>(
                    k < branch_nums[level - 1] ? k
                                               : k - branch_nums[level - 1]);
            });
        }
    } else if constexpr (direction == Direction::R) {
    } else {
        static_assert(false);
    }

    if constexpr (direction == Direction::L) {
        tree_elem_offset =
            acc_elem_slot_cnts[level] - tree_elem_offset - elem_cnt;
    } else if constexpr (direction == Direction::R) {
    } else {
        static_assert(false);
    }

    while (acc_elem_slot_cnts[level] < tree_elem_offset + elem_cnt + cnt) {
        multi_level_ptr_table::NavNode<ActiveMap>* new_root{
            multi_level_ptr_table::detail::CreateNavNode_(
                node_alctr, branch_nums[level], meta::TypeWrapper<ActiveMap>{})
        };

        if constexpr (direction == Direction::L) {
            new_root->active_map = static_cast<ActiveMap>(1)
                                   << (branch_nums[level] - 1);
            new_root->ptrs[branch_nums[level] - 1] = root;
        } else if constexpr (direction == Direction::R) {
            new_root->active_map = 1;
            new_root->ptrs[0]    = root;
        } else {
            static_assert(false);
        }

        ++level;
        root = new_root;

        rots[level - 1] = 0;
    }

    if constexpr (direction == Direction::L) {
        tree_elem_offset =
            acc_elem_slot_cnts[level] - elem_cnt - cnt - tree_elem_offset;

        if (0 < cnt) {
            (InsertElems_)(
                self, level,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root),
                tree_elem_offset, cnt, llist::GetR(head_n), provider);
        }
    } else if constexpr (direction == Direction::R) {
        if (0 < cnt) {
            (InsertElems_)(
                self, level,
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root),
                tree_elem_offset + elem_cnt, cnt, head_n, provider);
        }
    } else {
        static_assert(false);
    }

    if constexpr (direction == Direction::L) {
        rots[level - 1] = ({
            size_t k{ rots[level - 1] +
                      tree_elem_offset / acc_elem_slot_cnts[level - 1] };
            static_cast<multi_level_ptr_table::BranchNum>(
                k < branch_nums[level - 1] ? k : k - branch_nums[level - 1]);
        });

        tree_elem_offset %= acc_elem_slot_cnts[level - 1];
    } else if (direction == Direction::R) {
    } else {
        static_assert(false);
    }

    self.elem_cnt += cnt;

    if constexpr (direction == Direction::L) {
        self.tree_elem_offset = tree_elem_offset;
    } else if (direction == Direction::R) {
    } else {
        static_assert(false);
    }

    self.level = level;
    self.root  = root;
}

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr bool EraseElems_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>>& self_view,
    unsigned level, void* mlpt_node_or_n, size_t elem_slot_idx, size_t cnt,
    bool l_is_empty, bool r_is_empty, Acceptor&& acceptor) {
    using ActiveMap = typename decltype(layout_config)::ActiveMap;
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_branch_nums{ layout_config.acc_branch_nums };
    constexpr auto acc_elem_slot_cnts{ layout_config.acc_elem_slot_cnts };

    auto& self{ self_view->GetRef() };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };
    auto& seg_alctr{ meta::GetInstRef(self.seg_alctr_like) };

    if (level == 0) {
        DataSeg<Elem, seg_elem_slot_cnt>* data_seg{ (
            NToDataSeg_<Elem, seg_elem_slot_cnt>)(mlpt_node_or_n) };

        size_t beg{ elem_slot_idx };
        size_t end{ beg + cnt };

        seq_endpoint::acceptor::Transfer(
            acceptor,
            lifecycle::DestructingObjToMemView<Elem>{ data_seg->elems + beg },
            sizeof(Elem), cnt);

        l_is_empty |= beg == 0;
        r_is_empty |= end == seg_elem_slot_cnt;

        bool is_empty{ l_is_empty && r_is_empty };

        if (is_empty) { (DestroyDataSeg_)(data_seg, seg_alctr); }

        return is_empty;
    }

    constexpr size_t sub_tree_elem_slot_cnt{ acc_elem_slot_cnts[level - 1] };

    multi_level_ptr_table::BranchNum rot{ self.rots[level - 1] };

    size_t sub_tree_slot_idx{ elem_slot_idx / sub_tree_elem_slot_cnt };
    size_t sub_elem_slot_idx{ elem_slot_idx % sub_tree_elem_slot_cnt };

    multi_level_ptr_table::NavNode<ActiveMap>* mlpt_node{
        static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(mlpt_node_or_n)
    };

    while (0 < cnt) {
        size_t mlpt_slot_idx{ rot + sub_tree_slot_idx };

        if (branch_nums[level - 1] <= mlpt_slot_idx) {
            mlpt_slot_idx -= branch_nums[level - 1];
        }

        size_t cur_cnt{ comparison_utils::BasicMin(
            sub_tree_elem_slot_cnt - sub_elem_slot_idx, cnt) };

        bool sub_is_empty{ (
            EraseElems_)(self, level - 1,
                         static_cast<
                             multi_level_ptr_table::NavNode<ActiveMap>*>(
                             mlpt_node->ptrs[mlpt_slot_idx]),
                         sub_elem_slot_idx, cur_cnt, l_is_empty, r_is_empty,
                         acceptor) };

        if (sub_is_empty) {
            mlpt_node->active_map -= static_cast<ActiveMap>(1) << mlpt_slot_idx;
        }

        ++sub_tree_slot_idx;
        sub_elem_slot_idx = 0;

        cnt -= cur_cnt;
    }

    bool is_empty{ mlpt_node->active_map == 0 };

    if (is_empty) {
        multi_level_ptr_table::detail::DestroyNavNode_(node_alctr, mlpt_node);
    }

    return is_empty;
}

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          Direction direction,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void Pop_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
    meta::ValueWrapper<Direction, direction>, size_t cnt, Acceptor&& acceptor) {
    static_assert(direction == Direction::L || direction == Direction::R);

    using ActiveMap = typename decltype(layout_config)::ActiveMap;
    constexpr auto branch_nums{ layout_config.branch_nums };
    constexpr auto acc_branch_nums{ layout_config.acc_branch_nums };
    constexpr auto acc_elem_slot_cnts{ layout_config.acc_elem_slot_cnts };

    auto& self{ self_view->GetRef() };

    (CheckCntr_)(self);

    size_t elem_cnt{ self.elem_cnt };
    size_t tree_elem_offset{ self.tree_elem_offset };

    multi_level_ptr_table::BranchNum* rots{ self.rots };

    unsigned level{ self.level };
    void* root{ self.root };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(0, cnt, elem_cnt));

    if (cnt == 0) { return; }

    if constexpr (direction == Direction::L) {
        (EraseElems_)(
            self, level - 1,
            static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root),
            tree_elem_offset, cnt, true, elem_cnt - cnt == 0, acceptor);

        tree_elem_offset += cnt;
    } else if constexpr (direction == Direction::R) {
        (EraseElems_)(
            self, level - 1,
            static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root),
            tree_elem_offset + elem_cnt - cnt, cnt, elem_cnt - cnt == 0, true,
            acceptor);
    } else {
        static_assert(false);
    }

    if (elem_cnt - cnt == 0) {
        level = 1;
        root  = nullptr;

        rots[0]          = 0;
        tree_elem_offset = 0;
    } else {
        while (1 < level) {
            auto* old_root{
                static_cast<multi_level_ptr_table::NavNode<ActiveMap>*>(root)
            };

            if (1 < integral_bit::PopCount(old_root->active_map)) { break; }

            void* new_root{
                old_root->ptrs[integral_bit::CTZ(old_root->active_map)]
            };

            multi_level_ptr_table::detail::DestroyNavNode_(node_alctr,
                                                           old_root);

            --level;
            root = new_root;
        }

        tree_elem_offset %= acc_elem_slot_cnts[level];

        rots[level - 1] = ({
            size_t k{ rots[level - 1] +
                      tree_elem_offset / acc_elem_slot_cnts[level - 1] };
            static_cast<multi_level_ptr_table::BranchNum>(
                k < branch_nums[level - 1] ? k : k - branch_nums[level - 1]);
        });

        tree_elem_offset %= acc_branch_nums[level - 1];
    }

    self.elem_cnt -= cnt;
    self.tree_elem_offset = tree_elem_offset;

    self.level = level;
    self.root  = root;
}

}  // namespace multi_level_circular_array::detail

constexpr multi_level_circular_array::Cursor::Cursor(
    seq_cntr::CursorLimit const& src_cursor) {
    *this = src_cursor;
}

constexpr multi_level_circular_array::Cursor::operator seq_cntr::CursorLimit(
    this Cursor const& self) {
    seq_cntr::CursorLimit ret;

    utils::MemCopy(&ret, &self, sizeof(Cursor));

    return ret;
}

constexpr multi_level_circular_array::Cursor&
multi_level_circular_array::Cursor::operator=(
    seq_cntr::CursorLimit const& src_cursor) {
    utils::MemCopy(this, &src_cursor, sizeof(Cursor));
    return *this;
}

template <CntrTplParamList>
template <typename NodeAllocatorLikeConstructArg,
          typename SegAllocatorLikeConstructArg>
constexpr multi_level_circular_array::Cntr<CntrTplArgList>::Cntr(
    size_t seg_elem_slot_cnt,
    NodeAllocatorLikeConstructArg&& node_alctr_like_construct_arg,
    SegAllocatorLikeConstructArg&& seg_alctr_like_construct_arg)
    : node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          NodeAllocatorLike, NodeAllocatorLikeConstructArg,
          node_alctr_like_construct_arg) },
      seg_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          SegAllocatorLike, SegAllocatorLikeConstructArg,
          seg_alctr_like_construct_arg) } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg_elem_slot_cnt);

    this->seg_elem_slot_cnt = seg_elem_slot_cnt;
    this->tree_elem_offset  = 0;
    this->elem_cnt          = 0;

    this->rots[0] = 0;

    this->level = 1;
    this->root  = nullptr;

    this->head_n =
        new (allocator::SafeAllocate(meta::GetInstRef(this->node_alctr_like),
                                     alignof(Node), sizeof(Node))) Node;
}

template <CntrTplParamList>
template <meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList>&,
                          multi_level_circular_array::Cntr<CntrTplArgList>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag multi_level_circular_array::
    Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetElemCnt    = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer   = true,
        .Derefer = true,

        .Read      = true,
        .Write     = true,
        .ReadWrite = true,

        .PushL  = true,
        .PushR  = true,
        .Insert = true,

        .PopL     = true,
        .PopR     = true,
        .Erase    = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor  = true,
        .GetCursorDist  = true,
        .GetCursorIdx   = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
template <
    meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList> const&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&&>
        TargetCntr>
constexpr seq_cntr::capability::Flag multi_level_circular_array::
    Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr&>{}) &
           seq_cntr::capability::const_capability_flag;
}

template <CntrTplParamList>
template <meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList>&,
                          multi_level_circular_array::Cntr<CntrTplArgList>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag multi_level_circular_array::
    Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
template <
    meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList> const&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&&>
        TargetCntr>
constexpr seq_cntr::capability::Flag multi_level_circular_array::
    Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag multi_level_circular_array::Cntr<
    CntrTplArgList>::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag multi_level_circular_array::Cntr<
    CntrTplArgList>::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr void*
multi_level_circular_array::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return const_cast<void*>(static_cast<void const*>(&self));
}

template <CntrTplParamList>
template <
    meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList>&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&,
                    multi_level_circular_array::Cntr<CntrTplArgList>&&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&&>
        TargetCntr>
constexpr meta::TypeWrapper<Elem>
multi_level_circular_array::Cntr<CntrTplArgList>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <CntrTplParamList>
template <
    meta::IsAnySame<multi_level_circular_array::Cntr<CntrTplArgList>&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&,
                    multi_level_circular_array::Cntr<CntrTplArgList>&&,
                    multi_level_circular_array::Cntr<CntrTplArgList> const&&>
        TargetCntr>
constexpr meta::TypeWrapper<multi_level_circular_array::Cursor>
multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorType(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetSegElemCapacity(
    this Cntr const& self) {
    detail::CheckCntr_(self);
    return self.seg_elem_slot_cnt;
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);
    return self.elem_cnt;
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);
    // TODO
    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::GetLBCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    dst_cursor->idx               = static_cast<size_t>(-1);
    dst_cursor->n                 = self.head_n;
    dst_cursor->seg_elem_slot_idx = 0;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    dst_cursor->idx               = self.elem_cnt;
    dst_cursor->n                 = self.head_n;
    dst_cursor->seg_elem_slot_idx = 0;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <CntrTplParamList>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PeekL(
    this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    detail::Refer_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReferStrategy_::FromL>{}, 0,
        lazy_copy_elem, dst_elem_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PeekR(
    this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    detail::Refer_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReferStrategy_::FromL>{}, elem_cnt - 1,
        lazy_copy_elem, dst_elem_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Refer(
    this auto&& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, self.elem_cnt));

    detail::Refer_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReferStrategy_::AutoWithoutHint>{}, idx,
        lazy_copy_elem, dst_elem_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::ReferWithHint(
    this auto&& self, size_t idx, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem>* dst_elem_view, Cursor* dst_cursor,
    DstStorageView dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, self.elem_cnt));

    detail::Refer_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReferStrategy_::AutoWithHint>{}, idx,
        lazy_copy_elem, dst_elem_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Derefer(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, lifecycle::DynamicStorageView<Elem>* dst_elem_view,
    DstStorageView dst_elem) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };

    detail::CheckCursor_(self, pos_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_view != nullptr ||
                                            dst_elem.ptr != nullptr);

    size_t elem_cnt{ self.elem_cnt };

    size_t idx{ pos_cursor->idx };

    lifecycle::ForwardStorageView<decltype(self), Elem> elem_view{
        idx == static_cast<size_t>(-1) || idx == elem_cnt
            ? nullptr
            : detail::NToDataSeg_<Elem, seg_elem_slot_cnt>(pos_cursor->n)
                      ->elems +
                  pos_cursor->seg_elem_slot_idx
    };

    if (dst_elem_view != nullptr) { *dst_elem_view = elem_view; }

    if (!lazy_copy_elem && elem_view.ptr != nullptr &&
        dst_elem.ptr != nullptr) {
        lifecycle::StorageTransfer(dst_elem, elem_view);
    }
}

namespace multi_level_circular_array::detail {

enum struct ReadWriteOpType_ : unsigned char {
    Read,
    Write,
    ReadWrite,
};

template <ReadWriteOpType_ op_type, lifecycle::StorageSemantics cntr_semantics,
          CntrTplParamList, seq_endpoint::IsEndpoint<Elem> Endpoint>
constexpr void ReadWrite_(
    lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
    Cursor const* pos_cursor, size_t cnt, Endpoint&& endpoint,
    Cursor* dst_cursor) {
    static_assert(op_type == ReadWriteOpType_::Read ||
                  op_type == ReadWriteOpType_::Write ||
                  op_type == ReadWriteOpType_::ReadWrite);

    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };

    auto& self{ self_view->GetRef() };

    (CheckCursor_)(self, pos_cursor);

    Cursor work_cursor{ *pos_cursor };

    if constexpr (op_type == ReadWriteOpType_::Read ||
                  op_type == ReadWriteOpType_::ReadWrite) {
        (EndpointTransfer_)(
            seq_endpoint::acceptor::Tag{},
            meta::TypeWrapper<lifecycle::StorageView<cntr_semantics, Elem>>{},
            seg_elem_slot_cnt, work_cursor, cnt, endpoint);
    } else if constexpr (op_type == ReadWriteOpType_::Write) {
        (EndpointTransfer_)(
            seq_endpoint::provider::Tag{},
            meta::TypeWrapper<lifecycle::StorageView<cntr_semantics, Elem>>{},
            seg_elem_slot_cnt, work_cursor, cnt, endpoint);
    } else {
        static_assert(false);
    }

    if constexpr ((op_type == ReadWriteOpType_::Write ||
                   op_type == ReadWriteOpType_::ReadWrite) &&
                  (cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                   cntr_semantics == lifecycle::StorageSemantics::RightObj)) {
#if ZETA_Core_DebugEnable
        self.stamp        = debug_utils::revision::GetStamp();
        pos_cursor->stamp = self.stamp;
        work_cursor.stamp = self.stamp;
#endif
    }

    if (dst_cursor != nullptr) { *dst_cursor = work_cursor; }
}

}  // namespace multi_level_circular_array::detail

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Read(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::ReadWrite_<false>(const_cast<Cntr&>(self), pos_cursor, cnt,
                              acceptor, dst_cursor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Write(
    this meta::IsNonConstWithoutRef auto&& cntr, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& writer, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::ReadWrite(
    this meta::IsNonConstWithoutRef auto&& cntr, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, acceptor, dst_cursor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PushL(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Provider&& writer, Cursor* dst_cursor) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr size_t max_elem_cnt{ layout_config.max_elem_cnt };

    detail::CheckCntr_(self);

    size_t tree_elem_offset{ self.tree_elem_offset };
    size_t elem_cnt{ self.elem_cnt };

    Node* head_n{ self.head_n };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPushL(cnt, elem_cnt, max_elem_cnt));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    detail::Push_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::Direction::L>{}, cnt, writer);

    if (dst_cursor != nullptr) {
        dst_cursor->idx               = 0;
        dst_cursor->n                 = llist::GetR(head_n);
        dst_cursor->seg_elem_slot_idx = tree_elem_offset % seg_elem_slot_cnt;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PushR(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Provider&& writer, Cursor* dst_cursor) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr size_t max_elem_cnt{ layout_config.max_elem_cnt };

    detail::CheckCntr_(self);

    size_t tree_elem_offset{ self.tree_elem_offset };
    size_t elem_cnt{ self.elem_cnt };

    Node* head_n{ self.head_n };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPushR(cnt, elem_cnt, max_elem_cnt));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    Cursor cursor{
        .idx = elem_cnt,
        .n   = llist::GetR(head_n),
        .seg_elem_slot_idx =
            (tree_elem_offset + elem_cnt - 1) % seg_elem_slot_cnt + 1,
#if ZETA_Core_DebugEnable
        .stamp = self.stamp,
#endif
    };

    detail::Push_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::Direction::R>{}, cnt, writer,
        dst_cursor);

    if (cursor.seg_elem_slot_idx == seg_elem_slot_cnt) {
        cursor.n                 = llist::GetR(cursor.n);
        cursor.seg_elem_slot_idx = 0;
    }

    if (dst_cursor != nullptr) { *dst_cursor = cursor; }
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Insert(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& provider, Cursor* dst_cursor) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };

    detail::CheckCursor_(self, pos_cursor);

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

    if (cnt == 0) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        if (dst_cursor != nullptr) {
            ZETA_Core_DebugUtils_Diag_LogCurPos();
            *dst_cursor = *pos_cursor;
        }
        return;
    }

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    size_t elem_cnt{ self.elem_cnt };

    Node* head_n{ self.head_n };

    size_t idx{ pos_cursor->idx };

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx };

#if ZETA_Core_DebugEnable
    size_t stamp{ self.stamp = debug_utils::revision::GetStamp() };
#endif

    if (r_cnt == 0) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        self.PushR(seq_cntr::Tag{}, cnt, provider, pos_cursor);

        if (dst_cursor != nullptr) {
            ZETA_Core_DebugUtils_Diag_LogCurPos();

            dst_cursor->idx               = elem_cnt + cnt;
            dst_cursor->n                 = head_n;
            dst_cursor->seg_elem_slot_idx = 0;
        }

        return;
    }

    if (l_cnt == 0) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        Node* pos_n{ pos_cursor->n };
        size_t pos_seg_elem_slot_idx{ pos_cursor->seg_elem_slot_idx };

        self.PushL(seq_cntr::Tag{}, cnt, provider, pos_cursor);

        if (dst_cursor != nullptr) {
            ZETA_Core_DebugUtils_Diag_LogCurPos();

            dst_cursor->idx               = cnt;
            dst_cursor->n                 = pos_n;
            dst_cursor->seg_elem_slot_idx = pos_seg_elem_slot_idx;
        }

        return;
    }

    Cursor dst_work_cursor;
    Cursor src_work_cursor;

    dst_work_cursor.stamp = stamp;
    src_work_cursor.stamp = stamp;

    unsigned long long random_seed{ utils::GetRandom() };

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case utils::Choose2Result::Case0: {
        ZETA_Core_DebugUtils_Diag_LogCurPos();

        src_work_cursor.idx = cnt;
        src_work_cursor.n   = llist::GetR(head_n);
        src_work_cursor.seg_elem_slot_idx =
            self.tree_elem_offset % seg_elem_slot_cnt;

        detail::Push_(self, meta::AutoValueWrapper<detail::Direction::L>{}, cnt,
                      seq_endpoint::provider::EmptyProvider{});

        src_work_cursor.idx = 0;
        src_work_cursor.n   = llist::GetR(head_n);
        src_work_cursor.seg_elem_slot_idx =
            self.tree_elem_offset % seg_elem_slot_cnt;

        if (l_cnt < cnt) {
            detail::DataTransfer_<
                detail::Direction::R,
                lifecycle::StorageSemantics::ConstructingMemToObj,
                lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, l_cnt);

            *pos_cursor = dst_work_cursor;

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::ConstructingMemToObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt - l_cnt, provider);

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, l_cnt, provider);

            if (dst_cursor != nullptr) { *dst_cursor = dst_work_cursor; }
        } else {
            detail::DataTransfer_<
                detail::Direction::R,
                lifecycle::StorageSemantics::ConstructingMemToObj,
                lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, cnt);

            detail::DataTransfer_<detail::Direction::R,
                                  lifecycle::StorageSemantics::LeftObj,
                                  lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, l_cnt - cnt);

            *pos_cursor = dst_work_cursor;

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt, provider);

            if (dst_cursor != nullptr) { *dst_cursor = dst_work_cursor; }
        }

        break;
    }

    case utils::Choose2Result::Case1: {
        src_work_cursor.idx = elem_cnt;
        src_work_cursor.n   = llist::GetL(head_n);
        src_work_cursor.seg_elem_slot_idx =
            (self.tree_elem_offset + elem_cnt - 1) % seg_elem_slot_cnt + 1;

        detail::Push_(self, meta::AutoValueWrapper<detail::Direction::R>{}, cnt,
                      seq_endpoint::provider::EmptyProvider{});

        dst_work_cursor.idx = elem_cnt + cnt;
        dst_work_cursor.n   = llist::GetL(head_n);
        dst_work_cursor.seg_elem_slot_idx =
            (self.tree_elem_offset + elem_cnt + cnt - 1) % seg_elem_slot_cnt +
            1;

        if (r_cnt < cnt) {
            detail::DataTransfer_<
                detail::Direction::L,
                lifecycle::StorageSemantics::ConstructingMemToObj,
                lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, r_cnt);

            *pos_cursor = src_work_cursor;

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                seg_elem_slot_cnt, src_work_cursor, r_cnt, provider);

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::ConstructingMemToObjView<Elem>>{},
                seg_elem_slot_cnt, src_work_cursor, cnt - r_cnt, provider);

            if (dst_cursor != nullptr) { *dst_cursor = src_work_cursor; }
        } else {
            detail::DataTransfer_<
                detail::Direction::L,
                lifecycle::StorageSemantics::ConstructingMemToObj,
                lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, cnt);

            detail::DataTransfer_<detail::Direction::L,
                                  lifecycle::StorageSemantics::LeftObj,
                                  lifecycle::StorageSemantics::RightObj>(
                meta::TypeWrapper<Elem>{}, meta::TypeWrapper<Elem>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, r_cnt - cnt);

            *pos_cursor = src_work_cursor;

            detail::EndpointTransfer_(
                seq_endpoint::provider::Tag{},
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                seg_elem_slot_cnt, src_work_cursor, cnt, provider);

            if (dst_cursor != nullptr) { *dst_cursor = src_work_cursor; }
        }

        break;
    }

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PopL(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopR(cnt, elem_cnt));

    detail::Pop_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::Direction::L>{}, cnt, acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::PopR(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag, size_t cnt,
    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopR(cnt, elem_cnt));

    detail::Pop_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::Direction::R>{}, cnt, acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::Erase(
    this meta::IsNonConstWithoutRef auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };

    detail::CheckCursor_(self, pos_cursor);

    size_t elem_cnt{ self.elem_cnt };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(idx, cnt, elem_cnt));

    if (cnt == 0) { return; }

    size_t l_cnt{ idx };
    size_t r_cnt{ elem_cnt - idx - cnt };

    if (r_cnt == 0) {
        self.PopR(seq_cntr::Tag{}, cnt, acceptor);
        self.GetRBCursor(seq_cntr::Tag{}, pos_cursor);
        return;
    }

    if (l_cnt == 0) {
        self.PopL(seq_cntr::Tag{}, cnt, acceptor);
        self.PeekL(seq_cntr::Tag{}, true, nullptr, pos_cursor, nullptr);
        return;
    }

    unsigned long long random_seed{ utils::GetRandom() };

    Cursor dst_work_cursor;
    Cursor src_work_cursor;

    switch (utils::Choose2(l_cnt <= r_cnt, r_cnt <= l_cnt, &random_seed)) {
    case utils::Choose2Result::Case0: {
        dst_work_cursor = *pos_cursor;
        src_work_cursor = *pos_cursor;

        if (l_cnt < cnt) {
            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt - l_cnt, acceptor);

            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, l_cnt, acceptor);

            *pos_cursor = dst_work_cursor;

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::L>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, l_cnt);
        } else {
            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt, acceptor);

            *pos_cursor = dst_work_cursor;

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::L>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, l_cnt - cnt);

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::L>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, cnt);
        }

        detail::Pop_(
            lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
            meta::AutoValueWrapper<detail::Direction::L>{}, cnt,
            seq_endpoint::acceptor::EmptyAcceptor{});

        break;
    }

    case utils::Choose2Result::Case1: {
        if (r_cnt < cnt) {
            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, r_cnt, acceptor);

            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt - r_cnt, acceptor);

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::R>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, r_cnt);
        } else {
            detail::EndpointTransfer_(
                seq_endpoint::acceptor::Tag{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                seg_elem_slot_cnt, dst_work_cursor, cnt, acceptor);

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::RightObjView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::R>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, r_cnt - cnt);

            detail::DataTransfer_(
                meta::TypeWrapper<lifecycle::LeftObjView<Elem>>{},
                meta::TypeWrapper<lifecycle::DestructingObjToMemView<Elem>>{},
                meta::AutoValueWrapper<detail::Direction::R>{},
                seg_elem_slot_cnt, seg_elem_slot_cnt, dst_work_cursor,
                src_work_cursor, cnt);
        }

        detail::Pop_(
            lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
            meta::AutoValueWrapper<detail::Direction::R>{}, cnt,
            seq_endpoint::acceptor::EmptyAcceptor{});

        break;
    }

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::EraseAll(
    this meta::IsNonConstWithoutRef auto&& cntr, seq_cntr::Tag,
    Acceptor&& acceptor) {
    detail::CheckCntr_(cntr);

    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t elem_cnt{ cntr.elem_cnt };
    size_t tree_elem_offset{ cntr.tree_elem_offset };

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    auto& node_alctr{ meta::GetInstRef(cntr.node_alctr_like) };

    size_t tree_l_seg_offset{ tree_elem_offset / seg_elem_slot_cnt };

    detail::EraseElems_(cntr, level, root, tree_l_seg_offset, elem_cnt, true,
                        true, acceptor);

    cntr.elem_cnt         = 0;
    cntr.tree_elem_offset = 0;
    cntr.rots[0]          = 0;
    cntr.level            = 1;
    cntr.root             = nullptr;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* src_cursor,
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool multi_level_circular_array::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

template <CntrTplParamList>
constexpr comparison::Ordering
multi_level_circular_array::Cntr<CntrTplArgList>::CompareCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        cntr.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        cntr.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <CntrTplParamList>
constexpr size_t
multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorDist(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return cntr.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           cntr.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <CntrTplParamList>
constexpr size_t multi_level_circular_array::Cntr<CntrTplArgList>::GetCursorIdx(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* cursor) {
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* cursor) {
    cntr.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* cursor) {
    cntr.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorAdvanceL(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* cursor, size_t step) {
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanAdvanceL(cursor->idx, step, cntr.elem_cnt));

    detail::Refer_<detail::ReferStrategy_::AutoWithHint>(
        const_cast<Cntr&>(cntr), cursor->idx - step, true, nullptr, cursor,
        nullptr);
}

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::CursorAdvanceR(
    this Cntr const& cntr, seq_cntr::Tag, Cursor* cursor, size_t step) {
    detail::CheckCursor_(cntr, cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanAdvanceR(cursor->idx, step, cntr.elem_cnt));

    detail::Refer_<detail::ReferStrategy_::AutoWithHint>(
        const_cast<Cntr&>(cntr), cursor->idx + step, true, nullptr, cursor,
        nullptr);
}

namespace multi_level_circular_array::detail {

template <CntrTplParamList>
constexpr pair::Pair<Node*, Node*> SanityCheckRecursive_(
    Cntr<CntrTplArgList> const& cntr, size_t level_i, void* n, size_t seg_idx,
    size_t cnt) {
    using ActiveMap = typename Cntr<CntrTplArgList>::ActiveMap;
    constexpr size_t branch_num{ Cntr<CntrTplArgList>::branch_num };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    size_t tree_seg_slot_cnt{ acc_branch_nums[level_i + 1] };
    size_t sub_tree_seg_slot_cnt{ acc_branch_nums[level_i] };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_idx < tree_seg_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= tree_seg_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_idx + cnt <= tree_seg_slot_cnt);

    size_t sub_tree_idx_beg{ seg_idx / sub_tree_seg_slot_cnt };
    size_t sub_tree_idx_end{ (seg_idx + cnt - 1) / sub_tree_seg_slot_cnt };

    for (size_t sub_tree_idx{ 0 }; sub_tree_idx < branch_num; ++sub_tree_idx) {
        size_t sub_tree_slot_idx{ cntr.rots[level_i] + sub_tree_idx };

        if (branch_num <= sub_tree_slot_idx) {
            sub_tree_slot_idx -= branch_num;
        }

        if (sub_tree_idx < sub_tree_idx_beg ||
            sub_tree_idx_end < sub_tree_idx) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                (mlpt_node->active_map &
                 (static_cast<ActiveMap>(1) << sub_tree_slot_idx)) == 0);
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
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
                ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetR(last_n) ==
                                                        n);
                ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetL(n) ==
                                                        last_n);
            }

            last_n = n;
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            integral_bit::PopCount(mlpt_node->active_map) == cnt);

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
            SanityCheckMLPTNode_)(cntr, level_i - 1,
                                  static_cast<multi_level_ptr_table::NavNode<
                                      ActiveMap>*>(
                                      mlpt_node->ptrs[sub_tree_slot_idx]),
                                  seg_idx % sub_tree_seg_slot_cnt, cur_cnt) };

        if (first_n == nullptr) {
            first_n = cur_first_n;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetR(last_n) ==
                                                    cur_first_n);
            ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetL(cur_first_n) ==
                                                    last_n);
        }

        last_n = cur_last_n;

        seg_idx += cur_cnt;
        cnt -= cur_cnt;
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == 0);

    return { first_n, last_n };
}

}  // namespace multi_level_circular_array::detail

template <CntrTplParamList>
constexpr void multi_level_circular_array::Cntr<CntrTplArgList>::SanityCheck(
    this Cntr const& self,
    debug_utils::mem_recording::MemRecorder& scanned_head_n_recorder,
    debug_utils::mem_recording::MemRecorder& scanned_nav_node_recorder,
    debug_utils::mem_recording::MemRecorder& scanned_data_seg_recorder) {
    constexpr size_t seg_elem_slot_cnt{ layout_config.seg_elem_slot_cnt };
    constexpr auto branch_nums{ Cntr<CntrTplArgList>::branch_nums };
    constexpr auto acc_branch_nums{ Cntr<CntrTplArgList>::acc_branch_nums };

    detail::CheckCntr_(self);

    size_t elem_offset{ self.tree_elem_offset };
    size_t elem_cnt{ self.elem_cnt };

    unsigned level{ self.level };
    void* root{ self.root };

    Node* head_n{ self.head_n };

    size_t tree_seg_offset{ elem_offset / seg_elem_slot_cnt };
    size_t seg_elem_offset{ elem_offset % seg_elem_slot_cnt };

    size_t seg_cnt{ integral_math::CeilDiv(seg_elem_offset + elem_cnt,
                                           seg_elem_slot_cnt) };

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    auto& seg_alctr{ meta::GetInstRef(self.seg_alctr_like) };

    constexpr bool node_alctr_is_recording_alctr{
        debug_utils::recording_allocator::IsRecordingAllocator<
            decltype(node_alctr)>
    };

    constexpr bool seg_alctr_is_recording_alctr{
        debug_utils::recording_allocator::IsRecordingAllocator<
            decltype(seg_alctr)>
    };

    multi_level_ptr_table::Cntr<typename Cntr<CntrTplArgList>::ActiveMap,
                                meta::RemoveRef<decltype(node_alctr)>>
        mlpt{
            lifecycle::DirectConstructTag{},
            level,
            branch_nums.elems,
            elem_cnt,
            root,
            node_alctr,
        };

    scanned_head_n_recorder.Add(head_n, sizeof(Node));

    if constexpr (node_alctr_is_recording_alctr) {
        scanned_head_n_recorder.InChargeOf(
            debug_utils::recording_allocator::TryMakeSubGroupAllocator(
                node_alctr, "head_n")
                .GetMemRecorderClient());
    } else {
        // send a warning
    }

    {
        Node* n{ llist::GetR(head_n) };

        for (size_t seg_idx{ 0 }; seg_idx != seg_cnt; ++seg_idx) {
            void* mp{ mlpt.Access(detail::SrcBranchIdxesProvider_{
                .seg_idx         = tree_seg_offset + seg_idx,
                .rots            = self.rots + (mlpt.level - 1),
                .branch_nums     = branch_nums.elems + (mlpt.level - 1),
                .acc_branch_nums = acc_branch_nums.elems + (mlpt.level - 1),
            }) };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(mp != nullptr);

            void* m{ *static_cast<void**>(mp) };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(n == m);

            scanned_data_seg_recorder.Add(
                detail::NToDataSeg_<Elem, seg_elem_slot_cnt>(n),
                sizeof(DataSeg<Elem, seg_elem_slot_cnt>));

            n = llist::GetR(n);
        }
    }

    {
        Node* n{ llist::GetL(head_n) };

        for (size_t seg_idx{ seg_cnt - 1 }; seg_idx != static_cast<size_t>(-1);
             --seg_idx) {
            void* mp{ mlpt.Access(detail::SrcBranchIdxesProvider_{
                .seg_idx         = tree_seg_offset + seg_idx,
                .rots            = self.rots + (mlpt.level - 1),
                .branch_nums     = branch_nums.elems + (mlpt.level - 1),
                .acc_branch_nums = acc_branch_nums.elems + (mlpt.level - 1),
            }) };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(mp != nullptr);

            void* m{ *static_cast<void**>(mp) };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(n == m);

            n = llist::GetL(n);
        }
    }

    if (seg_cnt == 0) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetL(head_n) == head_n);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetR(head_n) == head_n);
    } else {
        auto [first_n, last_n]{ detail::SanityCheckRecursive_(
            self, mlpt.level, mlpt.root, tree_seg_offset, seg_cnt) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetR(head_n) == first_n);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetL(first_n) == head_n);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetR(last_n) == head_n);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(llist::GetL(head_n) == last_n);
    }

    if constexpr (seg_alctr_is_recording_alctr) {
        scanned_data_seg_recorder.InChargeOf(seg_alctr);
    } else {
        // send warning
    }
}

}  // namespace zeta::core

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")
