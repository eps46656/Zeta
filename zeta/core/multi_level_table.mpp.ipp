// IWYU pragma: private

#if !defined(EnDataNode)
#error "EnDataNode is not defined."
#endif

#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_table.mpp.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/utils.hpp>

#if EnDataNode

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                                   \
    integral::IsUnsignedIntegral ActiveMap, typename NavNodeAllocatorLike, \
        typename DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ActiveMap, NavNodeAllocatorLike, DataNodeAllocatorLike

#pragma push_macro("EnDataNodeTernary")
#define EnDataNodeTernary(x, y) x

#else

#pragma push_macro("Namespace")
#define Namespace multi_level_ptr_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    integral::IsUnsignedIntegral ActiveMap, typename NavNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ActiveMap, NavNodeAllocatorLike

#pragma push_macro("EnDataNodeTernary")
#define EnDataNodeTernary(x, y) y

#endif

namespace zeta::core {

namespace Namespace::detail {

template <CntrTplParamList>
constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr) {
    unsigned level{ cntr.level };
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    BranchNum const* branch_nums{ cntr.branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }
}

template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
constexpr BranchNum PullBranchIdx_(
    SrcBranchIdxesProvider& src_branch_idxes_provider, BranchNum branch_num) {
    ZETA_Core_DebugAssert(
        !elem_stream::provider::IsEnd(src_branch_idxes_provider));

    BranchNum branch_idx;

    elem_stream::provider::Transfer(src_branch_idxes_provider, &branch_idx,
                                    sizeof(BranchNum), sizeof(BranchNum), 1);

    ZETA_Core_Debug_PrintVar(branch_idx);
    ZETA_Core_Debug_PrintVar(branch_num);

    ZETA_Core_DebugAssert(integral_math::Compare(comparison::OpTag::Less{},
                                                 branch_idx, branch_num));

    return branch_idx;
}

template <integral::IsUnsignedIntegral ActiveMap>
constexpr bool TestActiveMap_  // NOLINT(misc-use-internal-linkage)
    (ActiveMap active_map, BranchNum idx) {
    return (active_map & (static_cast<ActiveMap>(1)
                          << static_cast<unsigned long long>(idx))) != 0;
}

template <integral::IsUnsignedIntegral ActiveMap>
constexpr size_t GetNavNodeSize_(BranchNum branch_num) {
    return __builtin_offsetof(ZETA_Core_Identity(NavNode<ActiveMap>),
                              ptrs[branch_num]);
}

template <allocator::IsAllocator NavNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr NavNode<ActiveMap>*
    AllocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (NavNodeAllocator& nav_node_alctr, BranchNum branch_num,
     meta::TypeWrapper<ActiveMap>) {
    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(
        allocator::SafeAllocate(nav_node_alctr, alignof(NavNode<ActiveMap>),
                                (GetNavNodeSize_<ActiveMap>)(branch_num))) };

    nav_node->active_map = 0;

    return nav_node;
}

template <allocator::IsAllocator NavNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr void DeallocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (NavNodeAllocator& node_alctr, NavNode<ActiveMap>* node) {
    allocator::Deallocate(node_alctr, node);
}

#if EnDataNode

template <integral::IsUnsignedIntegral ActiveMap>
constexpr size_t GetDataNodeSize_(size_t elem_stride, BranchNum branch_num,
                                  meta::TypeWrapper<ActiveMap>) {
    return __builtin_offsetof(ZETA_Core_Identity(DataNode<ActiveMap>),
                              data[elem_stride * branch_num]);
}

template <allocator::IsAllocator DataNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr DataNode<ActiveMap>*
    AllocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (DataNodeAllocator& data_node_alctr, size_t elem_stride,
     BranchNum branch_num, meta::TypeWrapper<ActiveMap>) {
    auto* data_node{ static_cast<DataNode<ActiveMap>*>(allocator::SafeAllocate(
        data_node_alctr, alignof(DataNode<ActiveMap>),
        (GetDataNodeSize_)(elem_stride, branch_num,
                           meta::TypeWrapper<ActiveMap>{}))) };

    data_node->active_map = 0;

    return data_node;
}

template <allocator::IsAllocator DataNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr void DeallocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (DataNodeAllocator& data_node_alctr, DataNode<ActiveMap>* node) {
    allocator::Deallocate(data_node_alctr, node);
}

#endif

}  // namespace Namespace::detail

template <CntrTplParamList>
template <typename NavNodeAllocatorInitArg
#if EnDataNode
          ,
          typename DataNodeAllocatorInitArg
#endif
          >
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    NavNodeAllocatorInitArg&& nav_node_alctr_init_arg,
#if EnDataNode
    DataNodeAllocatorInitArg&& data_node_alctr_init_arg,
#endif
    unsigned level, BranchNum const* branch_nums
#if EnDataNode
    ,
    size_t elem_stride
#endif
    )
    : nav_node_alctr_like{ ZETA_Core_Lifecycle_UnpackInitArg(
          NavNodeAllocatorLike, NavNodeAllocatorInitArg,
          nav_node_alctr_init_arg) }
#if EnDataNode
      ,
      data_node_alctr_like{ ZETA_Core_Lifecycle_UnpackInitArg(
          DataNodeAllocatorLike, DataNodeAllocatorInitArg,
          data_node_alctr_init_arg) }
#endif
{
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }

    this->level = level;
    this->branch_nums = branch_nums;

#if EnDataNode
    ZETA_Core_DebugAssert(0 < elem_stride);
    this->elem_stride = elem_stride;
#endif

    this->elem_cnt = 0;

    this->root = nullptr;
}

template <CntrTplParamList>
constexpr Namespace::Cntr<CntrTplArgList>::~Cntr() {
    this->EraseAll();
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetElemCnt(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_cnt;
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    size_t ret{ 1 };

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (__builtin_mul_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
constexpr auto Namespace::Cntr<CntrTplArgList>::Access(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* node{ cntr.root };

    if (node == nullptr) { return nullptr; }

    for (unsigned level_i{ level - 1 }; 0 < level_i; --level_i) {
        auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        if (!detail::TestActiveMap_(nav_node->active_map, cur_branch_idx)) {
            return nullptr;
        }

        node = nav_node->ptrs[cur_branch_idx];
    }

    BranchNum last_branch_idx{ detail::PullBranchIdx_(src_branch_idxes_provider,
                                                      branch_nums[0]) };

    if (!detail::TestActiveMap_(
            static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
                (node)->active_map,
            last_branch_idx)) {
        return nullptr;
    }

#if EnDataNode
    return static_cast<DataNode<ActiveMap>*>(node)->data +
           elem_stride * last_branch_idx;
#else
    return static_cast<NavNode<ActiveMap>*>(node)->ptrs + last_branch_idx;
#endif
}

template <CntrTplParamList>
template <elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindFirst(
    this auto& cntr,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    constexpr BranchNum zero{ 0 };

    return cntr.FindNextIncl(
        lin_seq_elem_stream::Provider{
            .data = &zero,
            .elem_size = sizeof(BranchNum),
            .elem_stride = 0,
            .elem_cnt = integral::RangeMaxOf<size_t>,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindLast(
    this auto& cntr,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    struct SrcBranchIdxesProvider {
        BranchNum const* branch_nums;
        size_t elem_cnt;

        static constexpr bool GetElemSize(elem_stream::provider::Tag) {
            return sizeof(BranchNum);
        }

        static constexpr bool IsEnd(elem_stream::provider::Tag) {
            return false;
        }

        constexpr size_t Transfer(this SrcBranchIdxesProvider& self,
                                  elem_stream::provider::Tag, void* dst_,
                                  size_t elem_size, ptrdiff_t elem_stride,
                                  size_t elem_cnt) {
            ZETA_Core_DebugAssert(elem_size == sizeof(BranchNum));
            ZETA_Core_DebugAssert(elem_stride == sizeof(BranchNum));
            ZETA_Core_DebugAssert(elem_cnt == self.elem_cnt);

            BranchNum* dst{ static_cast<BranchNum*>(dst_) };

            for (size_t i{ 0 }; i < elem_cnt; ++i) {
                *(dst++) = *(--self.branch_nums) - 1;
            }

            return elem_cnt;
        }
    } src_branch_idxes_provider{
        .branch_nums = cntr.branch_nums + cntr.level,
        .elem_cnt = cntr.level,
    };

    return cntr.FindPrevIncl(src_branch_idxes_provider,
                             dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
          elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindPrevIncl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* root{ cntr.root };

    if (root == nullptr) {
        BranchNum branch_idxes[max_level];

        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            branch_idxes[level_i] = branch_nums[level_i] - 1;
        }

        elem_stream::acceptor::Transfer(
            dst_branch_idxes_acceptor, branch_idxes + (level - 1),
            sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            level);

        return nullptr;
    }

    BranchNum branch_idxes[max_level];
    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        branch_idxes[level_i] = cur_branch_idx;

        nodes[level_i] = node;

        if (
#if EnDataNode
            0 < level_i &&
#endif
            !detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            break;
        }

        if (level_i == 0) {
#if EnDataNode
            if (!detail::TestActiveMap_(
                    static_cast<DataNode<ActiveMap>*>(node)->active_map,
                    cur_branch_idx)) {
                break;
            }
#endif

            elem_stream::provider::Transfer(
                src_branch_idxes_provider, branch_idxes + (level - 1),
                sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)),
                level);

#if EnDataNode
            return static_cast<DataNode<ActiveMap>*>(node)->data +
                   elem_stride * cur_branch_idx;
#else
            return static_cast<NavNode<ActiveMap>*>(node)->ptrs +
                   cur_branch_idx;
#endif
        }

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];
    }

    for (;; ++level_i) {
        if (level_i == level) {
            {
                BranchNum branch_idxes[max_level];

                for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
                    branch_idxes[level_i] = branch_nums[level_i] - 1;
                }

                elem_stream::acceptor::Transfer(
                    dst_branch_idxes_acceptor, branch_idxes + (level - 1),
                    sizeof(BranchNum),
                    -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);
            }

            return nullptr;
        }

        node = nodes[level_i];

        if (branch_idxes[level_i] == 0) { continue; }

        unsigned long long found_idx{ integral_bit::FindPrevBit(
#if EnDataNode
            level_i == 0 ? static_cast<DataNode<ActiveMap>*>(node)->active_map :
#endif
                         static_cast<NavNode<ActiveMap>*>(node)->active_map,
            branch_idxes[level_i] - 1) };

        if (found_idx != integral::RangeMaxOf<unsigned long long>) {
            branch_idxes[level_i] = static_cast<BranchNum>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        BranchNum cur_branch_idx{ branch_idxes[level_i] };

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];

        --level_i;

        nodes[level_i] = node;

        branch_idxes[level_i] =
            static_cast<BranchNum>(integral_bit::FindPrevBit(
#if EnDataNode
                level_i == 0
                    ? static_cast<DataNode<ActiveMap>*>(node)->active_map
                    :
#endif
                    static_cast<NavNode<ActiveMap>*>(node)->active_map,
                branch_nums[level_i] - 1));
    }

    elem_stream::acceptor::Transfer(
        dst_branch_idxes_acceptor, branch_idxes + (level - 1),
        sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if EnDataNode
    return static_cast<DataNode<ActiveMap>*>(nodes[0])->data +
           elem_stride * branch_idxes[0];
#else
    return static_cast<NavNode<ActiveMap>*>(nodes[0])->ptrs + branch_idxes[0];
#endif
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
          elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindPrevExcl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    BranchNum branch_idxes[max_level];

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        branch_idxes[level_i - 1] = detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i - 1]);
    }

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (0 < branch_idxes[level_i]) {
            --branch_idxes[level_i];
            goto L1;
        }

        branch_idxes[level_i] = branch_nums[level_i] - 1;
    }

    elem_stream::provider::Transfer(
        src_branch_idxes_provider, branch_idxes + (level - 1),
        sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

    return nullptr;

L1:;

    return cntr.FindPrevIncl(branch_idxes, dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
          elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindNextIncl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* root{ cntr.root };

    if (root == nullptr) {
        constexpr BranchNum zero{ 0 };

        elem_stream::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
                                        sizeof(BranchNum), 0, level);

        return nullptr;
    }

    BranchNum branch_idxes[max_level];
    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        branch_idxes[level_i] = cur_branch_idx;

        nodes[level_i] = node;

        if (
#if EnDataNode
            0 < level_i &&
#endif
            !detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            break;
        }

        if (level_i == 0) {
#if EnDataNode
            if (!detail::TestActiveMap_(
                    static_cast<DataNode<ActiveMap>*>(node)->active_map,
                    cur_branch_idx)) {
                break;
            }
#endif

            elem_stream::acceptor::Transfer(
                dst_branch_idxes_acceptor, branch_idxes + (level - 1),
                sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)),
                level);

#if EnDataNode
            return static_cast<DataNode<ActiveMap>*>(node)->data +
                   elem_stride * cur_branch_idx;
#else
            return static_cast<NavNode<ActiveMap>*>(node)->ptrs +
                   cur_branch_idx;
#endif
        }

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];
    }

    for (;; ++level_i) {
        if (level_i == level) {
            constexpr BranchNum zero{ 0 };

            elem_stream::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
                                            sizeof(BranchNum), 0, level);

            return nullptr;
        }

        node = nodes[level_i];

        if (branch_idxes[level_i] == branch_nums[level_i] - 1) { continue; }

        unsigned long long found_idx{ integral_bit::FindNextBit(
#if EnDataNode
            level_i == 0 ? static_cast<DataNode<ActiveMap>*>(node)->active_map :
#endif
                         static_cast<NavNode<ActiveMap>*>(node)->active_map,
            branch_idxes[level_i] + 1) };

        if (found_idx != integral::RangeMaxOf<unsigned long long>) {
            branch_idxes[level_i] = static_cast<BranchNum>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        BranchNum cur_branch_idx{ branch_idxes[level_i] };

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];

        --level_i;

        nodes[level_i] = node;

        branch_idxes[level_i] =
            static_cast<BranchNum>(integral_bit::FindNextBit(
#if EnDataNode
                level_i == 0
                    ? static_cast<DataNode<ActiveMap>*>(node)->active_map
                    :
#endif
                    static_cast<NavNode<ActiveMap>*>(node)->active_map,
                0));
    }

    elem_stream::acceptor::Transfer(
        dst_branch_idxes_acceptor, branch_idxes + (level - 1),
        sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if EnDataNode
    return static_cast<DataNode<ActiveMap>*>(nodes[0])->data +
           elem_stride * branch_idxes[0];
#else
    return static_cast<NavNode<ActiveMap>*>(nodes[0])->ptrs + branch_idxes[0];
#endif
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
          elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindNextExcl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*> {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    BranchNum branch_idxes[max_level];

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        branch_idxes[level_i - 1] = detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i - 1]);
    }

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (branch_idxes[level_i] < branch_nums[level_i] - 1) {
            ++branch_idxes[level_i];
            goto L1;
        }

        branch_idxes[level_i] = 0;
    }

    {
        constexpr BranchNum zero{ 0 };

        elem_stream::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
                                        sizeof(BranchNum), 0, level);
    }

    return nullptr;

L1:;

    return cntr.FindNextIncl(
        lin_seq_elem_stream::Provider{
            .data = branch_idxes,
            .elem_size = sizeof(BranchNum),
            .elem_stride = sizeof(BranchNum),
            .elem_cnt = level,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
constexpr pair::Pair<void*, bool> Namespace::Cntr<CntrTplArgList>::Insert(
    this Cntr& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    if (cntr.root == nullptr) {
        cntr.root =
#if EnDataNode
            level == 1 ? static_cast<void*>(detail::AllocateDataNode_(
                             data_node_alctr, elem_stride, branch_nums[0],
                             meta::TypeWrapper<ActiveMap>{}))
                       :
#endif
                       static_cast<void*>(detail::AllocateNavNode_(
                           nav_node_alctr, branch_nums[level - 1],
                           meta::TypeWrapper<ActiveMap>{}));
    }

    unsigned level_i{ level - 1 };
    void* node{ cntr.root };
    bool exist_node{ true };

    for (; 0 < level_i; --level_i) {
        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
        if ((exist_node =
                 exist_node &&
                 detail::TestActiveMap_(
                     static_cast<NavNode<ActiveMap>*>(node)->active_map,
                     cur_branch_idx))) {
            node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];
            continue;
        }

        static_cast<NavNode<ActiveMap>*>(node)->active_map +=
            static_cast<ActiveMap>(1) << cur_branch_idx;

        void* nxt_node{
#if EnDataNode
            level_i == 1 ? static_cast<void*>(detail::AllocateDataNode_(
                               data_node_alctr, elem_stride, branch_nums[0],
                               meta::TypeWrapper<ActiveMap>{}))
                         :
#endif
                         static_cast<void*>(detail::AllocateNavNode_(
                             nav_node_alctr, branch_nums[level_i - 1],
                             meta::TypeWrapper<ActiveMap>{}))
        };

        static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx] = nxt_node;

        node = nxt_node;
    }

    BranchNum last_branch_idx{ detail::PullBranchIdx_(src_branch_idxes_provider,
                                                      branch_nums[0]) };

    auto addr{
#if EnDataNode
        static_cast<DataNode<ActiveMap>*>(node)->data +
        elem_stride * last_branch_idx
#else
        static_cast<NavNode<ActiveMap>*>(node)->ptrs + last_branch_idx
#endif
    };

    bool newly_inserted{ !detail::TestActiveMap_(
        static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
            (node)->active_map,
        last_branch_idx) };

    if (newly_inserted) {
        ++cntr.elem_cnt;

        static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
            (node)->active_map += static_cast<ActiveMap>(1) << last_branch_idx;
    }

    return { .first = addr, .second = newly_inserted };
}

template <CntrTplParamList>
template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
constexpr bool Namespace::Cntr<CntrTplArgList>::Erase(
    this Cntr& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };

    BranchNum const* branch_nums{ cntr.branch_nums };

    void* node{ cntr.root };

    if (node == nullptr) { return false; }

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    void* nodes[max_level];
    size_t branch_idxes[max_level];

    for (unsigned level_i{ level - 1 };; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };
        branch_idxes[level_i] = cur_branch_idx;

        if (!detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            return false;
        }

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];
    }

    BranchNum last_branch_idx{ detail::PullBranchIdx_(src_branch_idxes_provider,
                                                      branch_nums[0]) };
    branch_idxes[0] = last_branch_idx;

    if (!detail::TestActiveMap_(
            static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
                (node)->active_map,
            last_branch_idx)) {
        return false;
    }

    --cntr.elem_cnt;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        node = nodes[level_i];

#if EnDataNode
        if (level_i == 0) {
            auto* data_node{ static_cast<DataNode<ActiveMap>*>(node) };

            data_node->active_map -= static_cast<ActiveMap>(1)
                                     << branch_idxes[level_i];

            if (data_node->active_map != 0) { return true; }

            detail::DeallocateDataNode_(data_node_alctr, data_node);
        } else
#endif
        {
            auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

            nav_node->active_map -= static_cast<ActiveMap>(1)
                                    << branch_idxes[level_i];

            if (nav_node->active_map != 0) { return true; }

            detail::DeallocateNavNode_(nav_node_alctr, nav_node);
        }
    }

    cntr.root = nullptr;

    return true;
}

namespace Namespace::detail {

template <CntrTplParamList>
constexpr void EraseAllRecursive_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, void* node, unsigned level_i) {
    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    if (level_i == 0) {
        (EnDataNodeTernary(DeallocateDataNode_, DeallocateNavNode_))(
            EnDataNodeTernary(data_node_alctr, nav_node_alctr),
            static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
                (node));

        return;
    }

    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

    for (unsigned long long idx{ 0 };
         (idx =
              integral_bit::FindNextBit(*static_cast<ActiveMap*>(node), idx)) <
         static_cast<long long>(integral::WidthOf<ActiveMap>);
         ++idx) {
        (EraseAllRecursive_)(cntr, nav_node->ptrs[idx], level_i - 1);
    }

    (DeallocateNavNode_)(nav_node_alctr, nav_node);
}

}  // namespace Namespace::detail

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::EraseAll(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    if (root == nullptr) { return; }

    detail::EraseAllRecursive_(cntr, root, level - 1);

    cntr.elem_cnt = 0;
    cntr.root = nullptr;
}

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap>
size_t SanitizeRecursive_  // NOLINT(
                           // misc-no-recursion,
                           // misc-use-internal-linkage)
    (mem_recorder::MemRecorder& dst_nav_node,
#if EnDataNode
     mem_recorder::MemRecorder& dst_data_node,
#endif
     unsigned level_i, BranchNum const* branch_nums,
#if EnDataNode
     size_t elem_stride,
#endif
     void* node) {
    ZETA_Core_DebugAssert(*static_cast<ActiveMap*>(node) != 0);

#if EnDataNode
    if (level_i == 0) {
        mem_recorder::Record(
            dst_data_node, node,
            detail::GetDataNodeSize_(elem_stride, branch_nums[0],
                                     meta::TypeWrapper<ActiveMap>{}));

        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<DataNode<ActiveMap>*>(node)->active_map)) };

        ZETA_Core_DebugAssert(0 < ret);

        size_t k{ 0 };

        for (unsigned i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<DataNode<ActiveMap>*>(node)->active_map,
                static_cast<BranchNum>(i));
        }

        ZETA_Core_DebugAssert(k == ret);

        return ret;
    }
#endif

    mem_recorder::Record(
        dst_nav_node, node,
        detail::GetNavNodeSize_<ActiveMap>(branch_nums[level_i]));

#if !EnDataNode
    if (level_i == 0) {
        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<NavNode<ActiveMap>*>(node)->active_map)) };

        ZETA_Core_DebugAssert(0 < ret);

        size_t k{ 0 };

        for (BranchNum i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map, i);
        }

        ZETA_Core_DebugAssert(k == ret);

        return ret;
    }
#endif

    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

    size_t size{ 0 };

    for (unsigned long long idx{ 0 };
         (idx = integral_bit::FindNextBit(nav_node->active_map, idx)) <
         integral::WidthOf<ActiveMap>;
         ++idx) {
        ZETA_Core_DebugAssert(idx < branch_nums[level_i]);
        ZETA_Core_DebugAssert((TestActiveMap_)(nav_node->active_map,
                                               static_cast<BranchNum>(idx)));

        size += SanitizeRecursive_<ActiveMap>(dst_nav_node,
#if EnDataNode
                                              dst_data_node,
#endif
                                              level_i - 1, branch_nums,
#if EnDataNode
                                              elem_stride,
#endif
                                              nav_node->ptrs[idx]);
    }

    return size;
}

}  // namespace Namespace::detail

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Sanitize(
    this Cntr& cntr, mem_recorder::MemRecorder* dst_nav_node
#if EnDataNode
    ,
    mem_recorder::MemRecorder* dst_data_node
#endif
) {
#if !ZETA_Core_EnableDebug
    ZETA_Core_Unused(cntr);
    ZETA_Core_Unused(dst_nav_node);
    ZETA_Core_Unused(dst_data_node);
#else
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* root{ cntr.root };

    mem_recorder::MemRecorder* origin_dst_nav_node{ dst_nav_node };

#if EnDataNode
    mem_recorder::MemRecorder* origin_dst_data_node{ dst_data_node };
#endif

    if (dst_nav_node == nullptr) { dst_nav_node = mem_recorder::Create(); }

#if EnDataNode
    if (dst_data_node == nullptr) { dst_data_node = mem_recorder::Create(); }
#endif

    size_t size{ root == nullptr ? 0
                                 : detail::SanitizeRecursive_<ActiveMap>(
                                       *dst_nav_node,
#if EnDataNode
                                       *dst_data_node,
#endif
                                       level - 1, branch_nums,
#if EnDataNode
                                       elem_stride,
#endif
                                       root) };

    ZETA_Core_DebugAssert(size == cntr.elem_cnt);

    if (origin_dst_nav_node != dst_nav_node) {
        mem_recorder::Destroy(dst_nav_node);
        dst_nav_node = origin_dst_nav_node;
    }

#if EnDataNode
    if (origin_dst_data_node != dst_data_node) {
        mem_recorder::Destroy(dst_data_node);
        dst_data_node = origin_dst_data_node;
    }
#endif
#endif
}

}  // namespace zeta::core

#pragma pop_macro("EnDataNodeTernary")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("Namespace")
