// IWYU pragma: private

#if !defined(EnDataNode)
#error "EnDataNode is not defined."
#endif

#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/debug_utils/recording_allocator.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/multi_level_table.mpp.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

#if EnDataNode

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                                \
    integral::IsUnsignedIntegral ActiveMap, meta::IsContainerElem Data, \
        typename NavNodeAllocatorLike, typename DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    ActiveMap, Data, NavNodeAllocatorLike, DataNodeAllocatorLike

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
    (Cntr<CntrTplArgList> const& cntr) {
    unsigned level{ cntr.level };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < level);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(level <= max_level);

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_stride % alignof(Data) == 0);
#endif

    BranchNum const* branch_nums{ cntr.branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <=
                                                integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <= max_branch_num);
    }
}

template <meta::IsContainerElem Data,
          seq_endpoint::provider::IsProvider<Data> SrcBranchIdxesProvider>
constexpr BranchNum PullBranchIdx_(
    SrcBranchIdxesProvider& src_branch_idxes_provider, BranchNum branch_num) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        !seq_endpoint::provider::IsEnd(src_branch_idxes_provider));

    BranchNum branch_idx;

    seq_endpoint::provider::Transfer(src_branch_idxes_provider, &branch_idx,
                                     sizeof(BranchNum), sizeof(BranchNum), 1);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(integral_math::Compare(
        comparison::OpTags::Less{}, branch_idx, branch_num));

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
    (NavNodeAllocator& nav_node_alctr, NavNode<ActiveMap>* node) {
    allocator::Deallocate(nav_node_alctr, node);
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
template <typename NavNodeAllocatorConstructArg
#if EnDataNode
          ,
          typename DataNodeAllocatorConstructArg
#endif
          >
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    lifecycle::DirectConstructTag, unsigned level, BranchNum const* branch_nums,
#if EnDataNode
    size_t elem_stride,
#endif
    size_t elem_cnt, void* root,
    NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if EnDataNode
    ,
    DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    )
    : level{ level },
      branch_nums{ branch_nums },
#if EnDataNode
      elem_stride{ elem_stride },
#endif
      elem_cnt{ elem_cnt },
      root{ root },
      nav_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          NavNodeAllocatorLike, NavNodeAllocatorConstructArg,
          nav_node_alctr_construct_arg) }
#if EnDataNode
      ,
      data_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          DataNodeAllocatorLike, DataNodeAllocatorConstructArg,
          data_node_alctr_construct_arg) }
#endif
{
    detail::CheckCntr_(*this);

    zeta::core::debug_utils::sanity::RegisterSanityCheckFunc(this,
                                                             (SanityCheck));
}

template <CntrTplParamList>
template <typename NavNodeAllocatorConstructArg
#if EnDataNode
          ,
          typename DataNodeAllocatorConstructArg
#endif
          >
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    unsigned level, BranchNum const* branch_nums,
#if EnDataNode
    size_t elem_stride,
#endif
    NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if EnDataNode
    ,
    DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    )
    : nav_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          NavNodeAllocatorLike, NavNodeAllocatorConstructArg,
          nav_node_alctr_construct_arg) }
#if EnDataNode
      ,
      data_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          DataNodeAllocatorLike, DataNodeAllocatorConstructArg,
          data_node_alctr_construct_arg) }
#endif
{
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < level);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(level <= max_level);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <=
                                                integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <= max_branch_num);
    }

    this->level = level;
    this->branch_nums = branch_nums;

#if EnDataNode
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_stride % alignof(Data) == 0);

    this->elem_stride = elem_stride;
#endif

    this->elem_cnt = 0;

    this->root = nullptr;

    zeta::core::debug_utils::sanity::RegisterSanityCheckFunc(this,
                                                             (SanityCheck));
}

template <CntrTplParamList>
constexpr Namespace::Cntr<CntrTplArgList>::~Cntr() {
    this->Destruct();
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Destruct(this Cntr& cntr) {
    cntr.EraseAll();

    zeta::core::debug_utils::sanity::UnregisterSanityCheckFunc(&cntr);
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::DisownDestruct(
    this Cntr& cntr) {
    zeta::core::debug_utils::sanity::UnregisterSanityCheckFunc(&cntr);
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
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider>
constexpr auto Namespace::Cntr<CntrTplArgList>::Access(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
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
template <seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindFirst(
    this auto& cntr,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
    detail::CheckCntr_(cntr);

    constexpr BranchNum zero{ 0 };

    return cntr.FindNextIncl(
        lin_seq_endpoint::provider::Provider<BranchNum>{
            .data_transfer_semantics = lifecycle::DataTransferSemantics::Copy,
            .data = &zero,
            .elem_stride = 0,
            .elem_cnt = integral::RangeMaxOf<size_t>,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindLast(
    this auto& cntr,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
    detail::CheckCntr_(cntr);

    struct SrcBranchIdxesProvider {
        BranchNum const* branch_nums;
        size_t elem_cnt;

        static constexpr bool GetElemSize(seq_endpoint::provider::Tag) {
            return sizeof(BranchNum);
        }

        static constexpr bool IsEnd(seq_endpoint::provider::Tag) {
            return false;
        }

        constexpr size_t Transfer(this SrcBranchIdxesProvider& self,
                                  seq_endpoint::provider::Tag, void* dst_,
                                  size_t elem_size, ptrdiff_t elem_stride,
                                  size_t elem_cnt) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_size ==
                                                    sizeof(BranchNum));
            ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_stride ==
                                                    sizeof(BranchNum));
            ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt == self.elem_cnt);

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
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindPrevIncl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
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

        seq_endpoint::acceptor::Transfer(
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

            seq_endpoint::provider::Transfer(
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

                seq_endpoint::acceptor::Transfer(
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

    seq_endpoint::acceptor::Transfer(
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
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindPrevExcl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
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

    seq_endpoint::provider::Transfer(
        src_branch_idxes_provider, branch_idxes + (level - 1),
        sizeof(BranchNum), -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

    return nullptr;

L1:;

    return cntr.FindPrevIncl(
        lin_seq_endpoint::provider::Provider<BranchNum>{
            .data_transfer_semantics = lifecycle::DataTransferSemantics::Copy,
            .data = branch_idxes + (level - 1),
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            .elem_cnt = level,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindNextIncl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* root{ cntr.root };

    if (root == nullptr) {
        constexpr BranchNum zero{ 0 };

        seq_endpoint::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
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

            seq_endpoint::acceptor::Transfer(
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

            seq_endpoint::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
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

    seq_endpoint::acceptor::Transfer(
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
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindNextExcl(
    this auto& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>* {
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

        seq_endpoint::acceptor::Transfer(dst_branch_idxes_acceptor, &zero,
                                         sizeof(BranchNum), 0, level);
    }

    return nullptr;

L1:;

    return cntr.FindNextIncl(
        lin_seq_endpoint::provider::Provider<BranchNum>{
            .data_transfer_semantics = lifecycle::DataTransferSemantics::Copy,
            .data = branch_idxes + (level - 1),
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            .elem_cnt = level,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider
#if EnDataNode
          ,
          seq_endpoint::provider::IsProvider<Data> DataProvider
#endif
          >
constexpr pair::Pair<void*, bool> Namespace::Cntr<CntrTplArgList>::Insert(
    this Cntr& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
#if EnDataNode
    ,
    DataProvider&& data_provider
#endif
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
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

#if EnDataNode
    seq_endpoint::provider::Transfer(data_provider,
                                     newly_inserted
                                         ? lifecycle::DataLifeState::Mem
                                         : lifecycle::DataLifeState::Obj,
                                     addr, elem_stride, 1);
#endif

    return { .first = addr, .second = newly_inserted };
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider
#if EnDataNode
          ,
          seq_endpoint::acceptor::IsAcceptor<Data> DataAcceptor
#endif
          >
constexpr bool Namespace::Cntr<CntrTplArgList>::Erase(
    this Cntr& cntr,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
#if EnDataNode
    ,
    DataAcceptor&& data_acceptor
#endif
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };

    BranchNum const* branch_nums{ cntr.branch_nums };

    void* node{ cntr.root };

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    if (node == nullptr) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        return false;
    }

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    void* nodes[max_level];
    size_t branch_idxes[max_level];

    for (unsigned level_i{ level - 1 };; --level_i) {
        ZETA_Core_DebugUtils_Diag_LogVar(level_i);

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

#if EnDataNode
    seq_endpoint::acceptor::Transfer(
        data_acceptor, lifecycle::DataLifeState::Reloc,
        static_cast<DataNode<ActiveMap>*>(node)->data +
            cntr.elem_stride * last_branch_idx,
        cntr.elem_stride, 1);
#endif

    --cntr.elem_cnt;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugUtils_Diag_LogVar(level_i);

        node = nodes[level_i];

#if EnDataNode
        if (level_i == 0) {
            ZETA_Core_DebugUtils_Diag_LogCurPos();

            auto* data_node{ static_cast<DataNode<ActiveMap>*>(node) };

            data_node->active_map -= static_cast<ActiveMap>(1)
                                     << branch_idxes[level_i];

            if (data_node->active_map != 0) { return true; }

            detail::DeallocateDataNode_(data_node_alctr, data_node);
        } else
#endif
        {
            ZETA_Core_DebugUtils_Diag_LogCurPos();

            auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

            nav_node->active_map -= static_cast<ActiveMap>(1)
                                    << branch_idxes[level_i];

            if (nav_node->active_map != 0) {
                ZETA_Core_DebugUtils_Diag_LogCurPos();
                return true;
            }

            detail::DeallocateNavNode_(nav_node_alctr, nav_node);
        }
    }

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    cntr.root = nullptr;

    return true;
}

namespace Namespace::detail {

template <CntrTplParamList>
#if EnDataNode
template <
    seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor
#endif
    constexpr void EraseAllRecursive_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, void* node, unsigned level_i
#if EnDataNode
     ,
     DataAcceptor&& data_acceptor
#endif
    ) {
    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    if (level_i == 0) {
#if EnDataNode
        DataNode<ActiveMap>* data_node{ static_cast<DataNode<ActiveMap>*>(
            node) };

        for (BranchNum i{ 0 }; i < cntr.branch_nums[0]; ++i) {
            if (detail::TestActiveMap_(data_node->active_map, i)) {
                seq_endpoint::acceptor::Transfer(
                    data_acceptor, lifecycle::DataTransferSemantics::Reloc,
                    data_node->data + cntr.elem_stride * i, cntr.elem_stride,
                    1);
            }
        }

        (DeallocateDataNode_)(data_node_alctr, data_node);
#else
        (DeallocateNavNode_)(nav_node_alctr,
                             static_cast<NavNode<ActiveMap>*>(node));
#endif

        return;
    }

    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

    for (unsigned long long idx{ 0 };
         (idx =
              integral_bit::FindNextBit(*static_cast<ActiveMap*>(node), idx)) <
         static_cast<long long>(integral::WidthOf<ActiveMap>);
         ++idx) {
        (EraseAllRecursive_)(cntr, nav_node->ptrs[idx], level_i - 1
#if EnDataNode
                             ,
                             data_acceptor
#endif
        );
    }

    (DeallocateNavNode_)(nav_node_alctr, nav_node);
}

}  // namespace Namespace::detail

template <CntrTplParamList>
#if EnDataNode
template <seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor
#endif
          constexpr void Namespace::Cntr<CntrTplArgList>::EraseAll(
              this Cntr& cntr
#if EnDataNode
              ,
              DataAcceptor&& data_acceptor
#endif
          ) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    if (root == nullptr) { return; }

    detail::EraseAllRecursive_(cntr, root,
                               level - 1
#if EnDataNode
                                   data_acceptor
#endif
    );

    cntr.elem_cnt = 0;
    cntr.root = nullptr;
}

#if ZETA_Core_DebugUtils_Sanity_Enable

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap>
size_t SanityCheckRecursive_  // NOLINT(
                              // misc-no-recursion,
                              // misc-use-internal-linkage)
    (debug_utils::memory::MemRecorder& scanned_nav_node_recorder,
#if EnDataNode
     debug_utils::memory::MemRecorder& scanned_data_node_recorder,
#endif
     unsigned level_i, BranchNum const* branch_nums,
#if EnDataNode
     size_t elem_stride,
#endif
     void* node) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(*static_cast<ActiveMap*>(node) !=
                                            0);

#if EnDataNode
    if (level_i == 0) {
        scanned_data_node_recorder.Add(
            node, detail::GetDataNodeSize_(elem_stride, branch_nums[0],
                                           meta::TypeWrapper<ActiveMap>{}));

        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<DataNode<ActiveMap>*>(node)->active_map)) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < ret);

        size_t k{ 0 };

        for (unsigned i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<DataNode<ActiveMap>*>(node)->active_map,
                static_cast<BranchNum>(i));
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(k == ret);

        return ret;
    }
#endif

    scanned_nav_node_recorder.Add(
        node, detail::GetNavNodeSize_<ActiveMap>(branch_nums[level_i]));

#if !EnDataNode
    if (level_i == 0) {
        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<NavNode<ActiveMap>*>(node)->active_map)) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < ret);

        size_t k{ 0 };

        for (BranchNum i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map, i);
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(k == ret);

        return ret;
    }
#endif

    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

    size_t size{ 0 };

    for (unsigned long long idx{ 0 };
         (idx = integral_bit::FindNextBit(nav_node->active_map, idx)) <
         integral::WidthOf<ActiveMap>;
         ++idx) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(idx < branch_nums[level_i]);
        ZETA_Core_DebugUtils_Diag_PromiseAssert((
            TestActiveMap_)(nav_node->active_map, static_cast<BranchNum>(idx)));

        size += (SanityCheckRecursive_<ActiveMap>)(scanned_nav_node_recorder,
#if EnDataNode
                                                   scanned_data_node_recorder,
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
constexpr void Namespace::Cntr<CntrTplArgList>::SanityCheck(
    void const* cntr_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr_ != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope == debug_utils::sanity::SanityCheckScope::Basic ||
        scope == debug_utils::sanity::SanityCheckScope::Complete);

    Cntr const& cntr{ *static_cast<Cntr const*>(cntr_) };

    detail::CheckCntr_(cntr);

    debug_utils::sanity::ExpandFinishedSanityCheckScope(
        cntr_, debug_utils::sanity::SanityCheckScope::Basic);

    if (scope == debug_utils::sanity::SanityCheckScope::Basic) { return; }

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    void* root{ cntr.root };

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr_like) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr_like) };
#endif

    if (!meta::IsPointer<NavNodeAllocatorLike> &&
        !meta::IsRef<NavNodeAllocatorLike>) {
        debug_utils::sanity::SanityCheck(&nav_node_alctr, scope);
    }

#if EnDataNode
    if (!meta::IsPointer<DataNodeAllocatorLike> &&
        !meta::IsRef<DataNodeAllocatorLike>) {
        debug_utils::sanity::SanityCheck(&data_node_alctr, scope);
    }
#endif

    debug_utils::memory::MemRecorder scanned_nav_node_recorder;

#if EnDataNode
    debug_utils::memory::MemRecorder scanned_data_node_recorder;
#endif

    size_t size{ root == nullptr ? 0
                                 : detail::SanityCheckRecursive_<ActiveMap>(
                                       scanned_nav_node_recorder,
#if EnDataNode
                                       scanned_data_node_recorder,
#endif
                                       level - 1, branch_nums,
#if EnDataNode
                                       elem_stride,
#endif
                                       root) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(size == cntr.elem_cnt);

    if constexpr (debug_utils::recording_allocator::IsRecordingAllocator<
                      meta::RemoveCVRef<decltype(nav_node_alctr)>>) {
        scanned_nav_node_recorder.InChargeOf(nav_node_alctr.GetMemRecorder());
    } else {
        ZETA_Core_DebugUtils_Logging_ImmLogMsg(
            "\033[38;5;208m"
            "WARNING" ZETA_Core_DebugUtils_Logging_ValColorCode
            ": nav_node_alctr is not a "                        //
            ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode  //
            "zeta::core::debug_utils::recording_allocator::RecordingAllocator"  //
            ZETA_Core_DebugUtils_Logging_ValColorCode  //
            ", skip matching memory.");
    }

#if EnDataNode
    if constexpr (debug_utils::recording_allocator::IsRecordingAllocator<
                      meta::RemoveCVRef<decltype(data_node_alctr)>>) {
        scanned_data_node_recorder.InChargeOf(data_node_alctr.GetMemRecorder());
    } else {
        ZETA_Core_DebugUtils_Logging_ImmLogMsg(
            "\033[38;5;208m"
            "WARNING" ZETA_Core_DebugUtils_Logging_ValColorCode
            ": data_node_alctr is not a "                       //
            ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode  //
            "zeta::core::debug_utils::recording_allocator::RecordingAllocator"  //
            ZETA_Core_DebugUtils_Logging_ValColorCode  //
            ", skip matching memory.");
    }
#endif
}

#endif

}  // namespace zeta::core

#pragma pop_macro("EnDataNodeTernary")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("Namespace")
