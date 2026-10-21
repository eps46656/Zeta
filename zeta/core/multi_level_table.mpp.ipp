// IWYU pragma: private

#if ZETA_Core_Editor
#pragma push_macro("DataNodeEn")
#endif

#if !defined(DataNodeEn)
#error "DataNodeEn is not defined."
#endif

#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/debug_utils/mem_recording.ipp>
#include <zeta/core/debug_utils/recording_allocator.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/multi_level_table.mpp.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

#if DataNodeEn

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                      \
    integral::IsUnsignedIntegral ActiveMap, meta::IsObj Data, \
        allocator::IsAllocatorLike NavNodeAllocatorLike,      \
        allocator::IsAllocatorLike DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    ActiveMap, Data, NavNodeAllocatorLike, DataNodeAllocatorLike

#else

#pragma push_macro("Namespace")
#define Namespace multi_level_ptr_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                    \
    integral::IsUnsignedIntegral ActiveMap, \
        allocator::IsAllocatorLike NavNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ActiveMap, NavNodeAllocatorLike

#endif

namespace zeta::core {

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap>
constexpr bool TestActiveMap_(ActiveMap active_map, BranchNum idx) {
    return (active_map & (static_cast<ActiveMap>(1)
                          << static_cast<unsigned long long>(idx))) != 0;
}

}  // namespace Namespace::detail

namespace Namespace::detail {

template <seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider>
constexpr BranchNum PullBranchIdx_(
    SrcBranchIdxesProvider& src_branch_idxes_provider, BranchNum branch_num) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        !seq_endpoint::provider::IsEnd(src_branch_idxes_provider));

    BranchNum branch_idx;

    seq_endpoint::provider::Transfer(
        src_branch_idxes_provider,
        lifecycle::LeftObjView<BranchNum>{ &branch_idx }, sizeof(BranchNum), 1);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(integral_math::Compare(
        comparison::OpTags::Less{}, branch_idx, branch_num));

    return branch_idx;
}

}  // namespace Namespace::detail

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap>
constexpr size_t GetNavNodeSize_(BranchNum branch_num,
                                 meta::TypeWrapper<ActiveMap>) {
    return __builtin_offsetof(ZETA_Core_Identity(NavNode<ActiveMap>),
                              ptrs[branch_num]);
}

}  // namespace Namespace::detail

template <integral::IsUnsignedIntegral ActiveMap>
constexpr Namespace::NavNode<ActiveMap>::NavNode(
    [[maybe_unused]] BranchNum branch_num)
    :
#if ZETA_Core_DebugEnable
      obj_hook{
          this,
          detail::GetNavNodeSize_(branch_num, meta::TypeWrapper<ActiveMap>{}),
          {},
      },
#endif
      active_map{ 0 } {
}

namespace Namespace::detail {

template <allocator::IsAllocator NavNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr NavNode<ActiveMap>* CreateNavNode_(NavNodeAllocator& nav_node_alctr,
                                             BranchNum branch_num,
                                             meta::TypeWrapper<ActiveMap>) {
    return new (allocator::SafeAllocate(
        nav_node_alctr, alignof(NavNode<ActiveMap>),
        (GetNavNodeSize_)(branch_num, meta::TypeWrapper<ActiveMap>{})))
        NavNode<ActiveMap>{ branch_num };
}

}  // namespace Namespace::detail

namespace Namespace::detail {

template <allocator::IsAllocator NavNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap>
constexpr void DestroyNavNode_(NavNodeAllocator& nav_node_alctr,
                               NavNode<ActiveMap>* node) {
    node->~NavNode<ActiveMap>();
    allocator::Deallocate(nav_node_alctr, node);
}

}  // namespace Namespace::detail

#if DataNodeEn

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap, meta::IsObj Data>
constexpr size_t GetDataNodeSize_(BranchNum branch_num,
                                  meta::TypeWrapper<ActiveMap>,
                                  meta::TypeWrapper<Data>) {
    return __builtin_offsetof(ZETA_Core_Identity(DataNode<ActiveMap, Data>),
                              data[sizeof(Data) * branch_num]);
}

}  // namespace Namespace::detail

template <integral::IsUnsignedIntegral ActiveMap, meta::IsObj Data>
constexpr Namespace::DataNode<ActiveMap, Data>::DataNode(
    [[maybe_unused]] BranchNum branch_num)
    :
#if ZETA_Core_DebugEnable
      obj_hook{
          this,
          detail::GetDataNodeSize_(branch_num, meta::TypeWrapper<ActiveMap>{},
                                   meta::TypeWrapper<Data>{}),
          {},
      },
#endif
      active_map{ 0 } {
}

namespace Namespace::detail {

template <allocator::IsAllocator DataNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap, meta::IsObj Data>
constexpr DataNode<ActiveMap, Data>* CreateDataNode_(
    DataNodeAllocator& data_node_alctr, BranchNum branch_num,
    meta::TypeWrapper<ActiveMap>, meta::TypeWrapper<Data>) {
    return new (allocator::SafeAllocate(
        data_node_alctr, alignof(DataNode<ActiveMap, Data>),
        (GetDataNodeSize_)(branch_num, meta::TypeWrapper<ActiveMap>{},
                           meta::TypeWrapper<Data>{})))
        DataNode<ActiveMap, Data>{ branch_num };
}

template <allocator::IsAllocator DataNodeAllocator,
          integral::IsUnsignedIntegral ActiveMap, meta::IsObj Data>
constexpr void DestroyDataNode_(DataNodeAllocator& data_node_alctr,
                                DataNode<ActiveMap, Data>* node) {
    node->~DataNode<ActiveMap, Data>();
    allocator::Deallocate(data_node_alctr, node);
}

}  // namespace Namespace::detail

#endif

namespace Namespace::detail {

template <CntrTplParamList>
constexpr void CheckCntr_(Cntr<CntrTplArgList> const& self) {
    unsigned level{ self.level };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < level);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(level <= level_max);

    BranchNum const* branch_nums{ self.branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num_min <= branch_num);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <=
                                                integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <= branch_num_max);
    }
}

}  // namespace Namespace::detail

template <CntrTplParamList>
template <typename NavNodeAllocatorConstructArg
#if DataNodeEn
          ,
          typename DataNodeAllocatorConstructArg
#endif
          >
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    lifecycle::DirectConstructTag, unsigned level, BranchNum const* branch_nums,
    size_t elem_cnt, void* root,
    NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if DataNodeEn
    ,
    DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    )
    :
#if ZETA_Core_DebugEnable
      obj_hook{
          this,
          sizeof(*this),
          {
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(
                  nav_node_alctr_like),
#if DataNodeEn
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(
                  data_node_alctr_like),
#endif
          },
      },
#endif
      level{ level },
      branch_nums{ branch_nums },
      elem_cnt{ elem_cnt },
      root{ root },
      nav_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          NavNodeAllocatorLike, NavNodeAllocatorConstructArg,
          nav_node_alctr_construct_arg) }
#if DataNodeEn
      ,
      data_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          DataNodeAllocatorLike, DataNodeAllocatorConstructArg,
          data_node_alctr_construct_arg) }
#endif
{
    detail::CheckCntr_(*this);
}

template <CntrTplParamList>
template <typename NavNodeAllocatorConstructArg
#if DataNodeEn
          ,
          typename DataNodeAllocatorConstructArg
#endif
          >
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    unsigned level, BranchNum const* branch_nums,
    NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if DataNodeEn
    ,
    DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    )
    :
#if ZETA_Core_DebugEnable
      obj_hook{
          this,
          sizeof(*this),
          {
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(
                  nav_node_alctr_like),
#if DataNodeEn
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(
                  data_node_alctr_like),
#endif
          },
      },
#endif
      nav_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          NavNodeAllocatorLike, NavNodeAllocatorConstructArg,
          nav_node_alctr_construct_arg) }
#if DataNodeEn
      ,
      data_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          DataNodeAllocatorLike, DataNodeAllocatorConstructArg,
          data_node_alctr_construct_arg) }
#endif
{
    if (level == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < level);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(level <= level_max);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num_min <= branch_num);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <=
                                                integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(branch_num <= branch_num_max);
    }

    this->level       = level;
    this->branch_nums = branch_nums;

    this->elem_cnt = 0;

    this->root = nullptr;
}

template <CntrTplParamList>
constexpr Namespace::Cntr<CntrTplArgList>::~Cntr() {
    this->Destruct();
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Destruct(this Cntr& self) {
    if (self.level == 0) { return; }

    self.EraseAll(
#if DataNodeEn
        seq_endpoint::acceptor::BasicAcceptor{}
#endif
    );
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Disown(this Cntr& self) {
    self.level = 0;
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetElemCnt(this Cntr& self) {
    detail::CheckCntr_(self);

    return self.elem_cnt;
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr& self) {
    detail::CheckCntr_(self);

    size_t ret{ 1 };

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

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
    this auto& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    void* node{ self.root };

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

    if (!detail::TestActiveMap_(static_cast<
#if DataNodeEn
                                    DataNode<ActiveMap, Data>*
#else
                                    NavNode<ActiveMap>*
#endif
                                    >(node)
                                    ->active_map,
                                last_branch_idx)) {
        return nullptr;
    }

#if DataNodeEn
    return static_cast<DataNode<ActiveMap, Data>*>(node)->data +
           sizeof(Data) * last_branch_idx;
#else
    return static_cast<NavNode<ActiveMap>*>(node)->ptrs + last_branch_idx;
#endif
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindFirst(
    this auto& self,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    constexpr BranchNum zero{ 0 };

    return self.FindNextIncl(
        lin_seq_endpoint::provider::Provider<
            lifecycle::ConstLeftObjView<BranchNum const>>{
            .data{ &zero },
            .elem_stride = 0,
            .elem_cnt    = integral::RangeMaxOf<size_t>,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindLast(
    this auto& self,
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    BranchNum branch_idxes[level_max];

    for (unsigned level_i{ 0 }; level_i < self.level; ++level_i) {
        branch_idxes[level_i] = self.branch_nums[level_i] - 1;
    }

    return self.FindPrevIncl(
        lin_seq_endpoint::provider::Provider<
            lifecycle::ConstLeftObjView<BranchNum const>>{
            .data{ branch_idxes + (self.level - 1) },
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            .elem_cnt    = integral::RangeMaxOf<size_t>,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindPrevIncl(
    this auto& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    void* root{ self.root };

    if (root == nullptr) {
        BranchNum branch_idxes[level_max];

        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            branch_idxes[level_i] = branch_nums[level_i] - 1;
        }

        seq_endpoint::acceptor::Transfer(
            dst_branch_idxes_acceptor,
            lifecycle::LeftObjView<BranchNum>{ branch_idxes + (level - 1) },
            -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

        return nullptr;
    }

    BranchNum branch_idxes[level_max];
    void* nodes[level_max];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        branch_idxes[level_i] = cur_branch_idx;

        nodes[level_i] = node;

        if (
#if DataNodeEn
            0 < level_i &&
#endif
            !detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            break;
        }

        if (level_i == 0) {
#if DataNodeEn
            if (!detail::TestActiveMap_(
                    static_cast<DataNode<ActiveMap, Data>*>(node)->active_map,
                    cur_branch_idx)) {
                break;
            }
#endif

            seq_endpoint::acceptor::Transfer(
                dst_branch_idxes_acceptor,
                lifecycle::ConstLeftObjView<BranchNum const>{ branch_idxes +
                                                              (level - 1) },
                -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if DataNodeEn
            return static_cast<DataNode<ActiveMap, Data>*>(node)->data +
                   sizeof(Data) * cur_branch_idx;
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
                BranchNum branch_idxes[level_max];

                for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
                    branch_idxes[level_i] = branch_nums[level_i] - 1;
                }

                seq_endpoint::acceptor::Transfer(
                    dst_branch_idxes_acceptor,
                    lifecycle::LeftObjView<BranchNum>{ branch_idxes +
                                                       (level - 1) },
                    -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);
            }

            return nullptr;
        }

        node = nodes[level_i];

        if (branch_idxes[level_i] == 0) { continue; }

        unsigned long long found_idx{ integral_bit::FindPrevBit(
#if DataNodeEn
            level_i == 0
                ? static_cast<DataNode<ActiveMap, Data>*>(node)->active_map
                :
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
#if DataNodeEn
                level_i == 0
                    ? static_cast<DataNode<ActiveMap, Data>*>(node)->active_map
                    :
#endif
                    static_cast<NavNode<ActiveMap>*>(node)->active_map,
                branch_nums[level_i] - 1));
    }

    seq_endpoint::acceptor::Transfer(
        dst_branch_idxes_acceptor,
        lifecycle::LeftObjView<BranchNum>{ branch_idxes + (level - 1) },
        -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if DataNodeEn
    return static_cast<DataNode<ActiveMap, Data>*>(nodes[0])->data +
           sizeof(Data) * branch_idxes[0];
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
    this auto& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    BranchNum branch_idxes[level_max];

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

    seq_endpoint::acceptor::Transfer(
        dst_branch_idxes_acceptor,
        lifecycle::ConstLeftObjView<BranchNum const>{ branch_idxes +
                                                      (level - 1) },
        -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

    return nullptr;

L1:;

    return self.FindPrevIncl(
        lin_seq_endpoint::provider::Provider<
            lifecycle::ConstLeftObjView<BranchNum const>>{
            .data{ branch_idxes + (level - 1) },
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            .elem_cnt    = level,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider,
          seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum>
              DstBranchIdxesAcceptor>
constexpr auto Namespace::Cntr<CntrTplArgList>::FindNextIncl(
    this auto& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    void* root{ self.root };

    if (root == nullptr) {
        constexpr BranchNum zero{ 0 };

        seq_endpoint::acceptor::Transfer(
            dst_branch_idxes_acceptor,
            lifecycle::ConstLeftObjView<BranchNum const>{ &zero }, 0, level);

        return nullptr;
    }

    BranchNum branch_idxes[level_max];
    void* nodes[level_max];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::PullBranchIdx_(
            src_branch_idxes_provider, branch_nums[level_i]) };

        branch_idxes[level_i] = cur_branch_idx;

        nodes[level_i] = node;

        if (
#if DataNodeEn
            0 < level_i &&
#endif
            !detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            break;
        }

        if (level_i == 0) {
#if DataNodeEn
            if (!detail::TestActiveMap_(
                    static_cast<DataNode<ActiveMap, Data>*>(node)->active_map,
                    cur_branch_idx)) {
                break;
            }
#endif

            seq_endpoint::acceptor::Transfer(
                dst_branch_idxes_acceptor,
                lifecycle::LeftObjView<BranchNum>{ branch_idxes + (level - 1) },
                -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if DataNodeEn
            return static_cast<DataNode<ActiveMap, Data>*>(node)->data +
                   sizeof(Data) * cur_branch_idx;
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

            seq_endpoint::acceptor::Transfer(
                dst_branch_idxes_acceptor,
                lifecycle::ConstLeftObjView<BranchNum const>{ &zero }, 0,
                level);

            return nullptr;
        }

        node = nodes[level_i];

        if (branch_idxes[level_i] == branch_nums[level_i] - 1) { continue; }

        unsigned long long found_idx{ integral_bit::FindNextBit(
#if DataNodeEn
            level_i == 0
                ? static_cast<DataNode<ActiveMap, Data>*>(node)->active_map
                :
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
#if DataNodeEn
                level_i == 0
                    ? static_cast<DataNode<ActiveMap, Data>*>(node)->active_map
                    :
#endif
                    static_cast<NavNode<ActiveMap>*>(node)->active_map,
                0));
    }

    seq_endpoint::acceptor::Transfer(
        dst_branch_idxes_acceptor,
        lifecycle::ConstLeftObjView<BranchNum const>{ branch_idxes +
                                                      (level - 1) },
        -static_cast<ptrdiff_t>(sizeof(BranchNum)), level);

#if DataNodeEn
    return static_cast<DataNode<ActiveMap, Data>*>(nodes[0])->data +
           sizeof(Data) * branch_idxes[0];
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
    this auto& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxesAcceptor&&
        dst_branch_idxes_acceptor  // NOLINT(cppcoreguidelines-missing-std-forward
    ) -> meta::MakeConstIf<void, meta::IsConst<decltype(self)>>* {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    BranchNum branch_idxes[level_max];

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

        seq_endpoint::acceptor::Transfer(
            dst_branch_idxes_acceptor,
            lifecycle::ConstLeftObjView<BranchNum const>{ &zero }, 0, level);
    }

    return nullptr;

L1:;

    return self.FindNextIncl(
        lin_seq_endpoint::provider::Provider<
            lifecycle::ConstLeftObjView<BranchNum const>>{
            .data{ branch_idxes + (level - 1) },
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(BranchNum)),
            .elem_cnt    = level,
        },
        dst_branch_idxes_acceptor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider
#if DataNodeEn
          ,
          seq_endpoint::provider::IsProvider<Data> DataProvider
#endif
          >
constexpr pair::Pair<void*, bool> Namespace::Cntr<CntrTplArgList>::Insert(
    this Cntr& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
#if DataNodeEn
    ,
    DataProvider&& data_provider
#endif
) {
    ZETA_Core_DebugUtils_Diag_LogCurPos();

    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    auto& nav_node_alctr{ meta::GetInstRef(self.nav_node_alctr_like) };

#if DataNodeEn
    auto& data_node_alctr{ meta::GetInstRef(self.data_node_alctr_like) };
#endif

    if (self.root == nullptr) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();

        self.root =
#if DataNodeEn
            level == 1 ? static_cast<void*>(detail::CreateDataNode_(
                             data_node_alctr, branch_nums[0],
                             meta::TypeWrapper<ActiveMap>{},
                             meta::TypeWrapper<Data>{}))
                       :
#endif
                       static_cast<void*>(detail::CreateNavNode_(
                           nav_node_alctr, branch_nums[level - 1],
                           meta::TypeWrapper<ActiveMap>{}));
    }

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    unsigned level_i{ level - 1 };
    void* node{ self.root };
    bool exist_node{ true };

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    for (; 0 < level_i; --level_i) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();

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
#if DataNodeEn
            level_i == 1 ? static_cast<void*>(detail::CreateDataNode_(
                               data_node_alctr, branch_nums[0],
                               meta::TypeWrapper<ActiveMap>{},
                               meta::TypeWrapper<Data>{}))
                         :
#endif
                         static_cast<void*>(detail::CreateNavNode_(
                             nav_node_alctr, branch_nums[level_i - 1],
                             meta::TypeWrapper<ActiveMap>{}))
        };

        ZETA_Core_DebugUtils_Diag_LogCurPos();

        static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx] = nxt_node;

        node = nxt_node;
    }

    BranchNum last_branch_idx{ detail::PullBranchIdx_(src_branch_idxes_provider,
                                                      branch_nums[0]) };

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    void* addr{
#if DataNodeEn
        static_cast<DataNode<ActiveMap, Data>*>(node)->data +
        sizeof(Data) * last_branch_idx
#else
        static_cast<NavNode<ActiveMap>*>(node)->ptrs + last_branch_idx
#endif
    };

    bool newly_inserted{ !detail::TestActiveMap_(static_cast<
#if DataNodeEn
                                                     DataNode<ActiveMap, Data>*
#else
                                                     NavNode<ActiveMap>*
#endif
                                                     >(node)
                                                     ->active_map,
                                                 last_branch_idx) };

    if (newly_inserted) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();

        ++self.elem_cnt;

        static_cast<
#if DataNodeEn
            DataNode<ActiveMap, Data>*
#else
            NavNode<ActiveMap>*
#endif
            >(node)
            ->active_map += static_cast<ActiveMap>(1) << last_branch_idx;
    }

#if DataNodeEn
    if (newly_inserted) {
        seq_endpoint::provider::Transfer(
            data_provider,
            lifecycle::ConstructingMemToObjView<Data>{
                static_cast<Data*>(addr) },
            sizeof(Data), 1);
    } else {
        seq_endpoint::provider::Transfer(
            data_provider,
            lifecycle::LeftObjView<Data>{ static_cast<Data*>(addr) },
            sizeof(Data), 1);
    }
#endif

    return { .first = addr, .second = newly_inserted };
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<Namespace::BranchNum>
              SrcBranchIdxesProvider
#if DataNodeEn
          ,
          seq_endpoint::acceptor::IsAcceptor<Data> DataAcceptor
#endif
          >
constexpr bool Namespace::Cntr<CntrTplArgList>::Erase(
    this Cntr& self,
    SrcBranchIdxesProvider&&
        src_branch_idxes_provider  // NOLINT(cppcoreguidelines-missing-std-forward)
#if DataNodeEn
    ,
    DataAcceptor&& data_acceptor
#endif
) {
    detail::CheckCntr_(self);

    unsigned level{ self.level };

    BranchNum const* branch_nums{ self.branch_nums };

    void* node{ self.root };

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    if (node == nullptr) {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
        return false;
    }

    auto& nav_node_alctr{ meta::GetInstRef(self.nav_node_alctr_like) };

#if DataNodeEn
    auto& data_node_alctr{ meta::GetInstRef(self.data_node_alctr_like) };
#endif

    void* nodes[level_max];
    size_t branch_idxes[level_max];

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

    if (!detail::TestActiveMap_(static_cast<
#if DataNodeEn
                                    DataNode<ActiveMap, Data>*
#else
                                    NavNode<ActiveMap>*
#endif
                                    >(node)
                                    ->active_map,
                                last_branch_idx)) {
        return false;
    }

#if DataNodeEn
    seq_endpoint::acceptor::Transfer(
        data_acceptor,
        lifecycle::DestructingObjToMemView<Data>{ reinterpret_cast<Data*>(
            static_cast<DataNode<ActiveMap, Data>*>(node)->data +
            sizeof(Data) * last_branch_idx) },
        sizeof(Data), 1);
#endif

    --self.elem_cnt;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugUtils_Diag_LogVar(level_i);

        node = nodes[level_i];

#if DataNodeEn
        if (level_i == 0) {
            ZETA_Core_DebugUtils_Diag_LogCurPos();

            auto* data_node{ static_cast<DataNode<ActiveMap, Data>*>(node) };

            data_node->active_map -= static_cast<ActiveMap>(1)
                                     << branch_idxes[level_i];

            if (data_node->active_map != 0) { return true; }

            detail::DestroyDataNode_(data_node_alctr, data_node);
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

            detail::DestroyNavNode_(nav_node_alctr, nav_node);
        }
    }

    ZETA_Core_DebugUtils_Diag_LogCurPos();

    self.root = nullptr;

    return true;
}

namespace Namespace::detail {

template <CntrTplParamList
#if DataNodeEn
          ,
          seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor
#endif
          >
constexpr void EraseAllRecursive_(Cntr<CntrTplArgList>& self, void* node,
                                  unsigned level_i
#if DataNodeEn
                                  ,
                                  DataAcceptor&& data_acceptor
#endif
) {
    auto& nav_node_alctr{ meta::GetInstRef(self.nav_node_alctr_like) };

#if DataNodeEn
    auto& data_node_alctr{ meta::GetInstRef(self.data_node_alctr_like) };
#endif

    if (level_i == 0) {
#if DataNodeEn
        DataNode<ActiveMap, Data>* data_node{
            static_cast<DataNode<ActiveMap, Data>*>(node)
        };

        for (BranchNum i{ 0 }; i < self.branch_nums[0]; ++i) {
            if (detail::TestActiveMap_(data_node->active_map, i)) {
                seq_endpoint::acceptor::Transfer(
                    data_acceptor,
                    lifecycle::DestructingObjToMemView<Data>{
                        reinterpret_cast<Data*>(data_node->data +
                                                sizeof(Data) * i) },
                    sizeof(Data), 1);
            }
        }

        (DestroyDataNode_)(data_node_alctr, data_node);
#else
        (DestroyNavNode_)(nav_node_alctr,
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
        (EraseAllRecursive_)(self, nav_node->ptrs[idx], level_i - 1
#if DataNodeEn
                             ,
                             data_acceptor
#endif
        );
    }

    (DestroyNavNode_)(nav_node_alctr, nav_node);
}

}  // namespace Namespace::detail

template <CntrTplParamList>
#if DataNodeEn
template <seq_endpoint::acceptor::IsAcceptor<Namespace::BranchNum> DataAcceptor>
#endif
constexpr void Namespace::Cntr<CntrTplArgList>::EraseAll(
    this Cntr& self
#if DataNodeEn
    ,
    DataAcceptor&& data_acceptor
#endif
) {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    void* root{ self.root };

    if (root == nullptr) { return; }

    detail::EraseAllRecursive_(self, root, level - 1
#if DataNodeEn
                               ,
                               data_acceptor
#endif
    );

    self.elem_cnt = 0;
    self.root     = nullptr;
}

#if ZETA_Core_DebugUtils_Sanity_Enable

namespace Namespace::detail {

template <integral::IsUnsignedIntegral ActiveMap
#if DataNodeEn
          ,
          meta::IsObj Data
#endif
          >
size_t SanityCheckRecursive_  // NOLINT(
                              // misc-no-recursion,
                              // misc-use-internal-linkage)
    (debug_utils::mem_recording::MemRecorder& nav_node_recorder,
#if DataNodeEn
     debug_utils::mem_recording::MemRecorder& data_node_recorder,
#endif
     unsigned level_i, BranchNum const* branch_nums, void* node) {
#if DataNodeEn
    if (level_i == 0) {
        data_node_recorder.Add(
            node, detail::GetDataNodeSize_(branch_nums[0],
                                           meta::TypeWrapper<ActiveMap>{},
                                           meta::TypeWrapper<Data>{}));

        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<DataNode<ActiveMap, Data>*>(node)->active_map)) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < ret);

        size_t k{ 0 };

        for (unsigned i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<DataNode<ActiveMap, Data>*>(node)->active_map,
                static_cast<BranchNum>(i));
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(k == ret);

        return ret;
    }
#endif

    nav_node_recorder.Add(
        node, detail::GetNavNodeSize_(branch_nums[level_i],
                                      meta::TypeWrapper<ActiveMap>{}));

#if !DataNodeEn
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

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        static_cast<NavNode<ActiveMap>*>(node)->active_map != 0);

    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

    size_t size{ 0 };

    for (unsigned long long idx{ 0 };
         (idx = integral_bit::FindNextBit(nav_node->active_map, idx)) <
         integral::WidthOf<ActiveMap>;
         ++idx) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(idx < branch_nums[level_i]);
        ZETA_Core_DebugUtils_Diag_PromiseAssert((
            TestActiveMap_)(nav_node->active_map, static_cast<BranchNum>(idx)));

        size += (SanityCheckRecursive_<ActiveMap
#if DataNodeEn
                                       ,
                                       Data
#endif
                                       >)(nav_node_recorder,
#if DataNodeEn
                                          data_node_recorder,
#endif
                                          level_i - 1, branch_nums,
                                          nav_node->ptrs[idx]);
    }

    return size;
}

}  // namespace Namespace::detail

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::SanityCheck(
    this Cntr& self, debug_utils::mem_recording::MemRecorder& nav_node_recorder
#if DataNodeEn
    ,
    debug_utils::mem_recording::MemRecorder& data_node_recorder
#endif
) {
    detail::CheckCntr_(self);

    unsigned level{ self.level };
    BranchNum const* branch_nums{ self.branch_nums };

    void* root{ self.root };

    size_t size{ root == nullptr ? 0
                                 : detail::SanityCheckRecursive_<ActiveMap
#if DataNodeEn
                                                                 ,
                                                                 Data
#endif
                                                                 >(
                                       nav_node_recorder,
#if DataNodeEn
                                       data_node_recorder,
#endif
                                       level - 1, branch_nums, root) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(size == self.elem_cnt);
}

#endif

}  // namespace zeta::core

#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("Namespace")

#if ZETA_Core_Editor
#pragma pop_macro("DataNodeEn")
#endif
