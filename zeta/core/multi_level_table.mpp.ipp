// IWYU pragma: private

#if !defined(EnDataNode)
#error "EnDataNode is not defined."
#endif

#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_table.mpp.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/utils.hpp>

#if EnDataNode

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                               \
    typename ActiveMap, typename NavNodeAllocatorLike, \
        typename DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ActiveMap, NavNodeAllocatorLike, DataNodeAllocatorLike

#pragma push_macro("EnDataNodeTernary")
#define EnDataNodeTernary(x, y) x

#else

#pragma push_macro("Namespace")
#define Namespace multi_level_ptr_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename ActiveMap, typename NavNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ActiveMap, NavNodeAllocatorLike

#pragma push_macro("EnDataNodeTernary")
#define EnDataNodeTernary(x, y) y

#endif

namespace zeta::core {

namespace Namespace::detail {

template <CntrTplParamList>
void CheckCntr_  // NOLINT(misc-use-internal-linkage)
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

template <typename T>
struct TransferBranchIdxesCore_ {
    static auto& Src(unsigned, T& x) { return x; }
    static auto& Dst(unsigned, T& x) { return x; }
};

template <>
struct TransferBranchIdxesCore_<meta::NoneType> {
    static auto Dst(unsigned, meta::NoneType&) {
        struct {
            void operator()(BranchNum) {}
        } ret;

        return ret;
    }
};

template <typename T>
struct TransferBranchIdxesCore_<T*> {
    static auto Src(unsigned level, T* x) {
        struct {
            T* ptr;

            auto operator()() { return *(--this->ptr); }
        } ret{ .ptr = x + level };

        return ret;
    }

    static auto Dst(unsigned level, T* x) {
        struct {
            T* ptr;

            void operator()(BranchNum branch_idx) {
                *(--this->ptr) = static_cast<T>(branch_idx);
            }
        } ret{ .ptr = x + level };

        return ret;
    }
};

template <typename T>
struct TransferBranchIdxesCore_<T* const>
    : public TransferBranchIdxesCore_<T*> {};

template <typename T, size_t N>
struct TransferBranchIdxesCore_<T[N]> {
    static decltype(auto) Src(unsigned level, T (&x)[N]) {
        return TransferBranchIdxesCore_<T*>::Src(level, x);
    }

    static decltype(auto) Dst(unsigned level, T (&x)[N]) {
        return TransferBranchIdxesCore_<T*>::Dst(level, x);
    }
};

template <typename T>
struct TransferBranchIdxesCore_<T[]> {
    static decltype(auto) Src(unsigned level, T (&x)[]) {
        return TransferBranchIdxesCore_<T*>::Src(level, x);
    }

    static decltype(auto) Dst(unsigned level, T (&x)[]) {
        return TransferBranchIdxesCore_<T*>::Dst(level, x);
    }
};

template <typename T>
decltype(auto) GetSrcBranchIdxes_(unsigned level, T&& x) {
    return TransferBranchIdxesCore_<meta::RemoveRef<T>>::Src(
        level, meta::Forward<T>(x));
}

template <typename T>
decltype(auto) GetDstBranchIdxes_(unsigned level, T&& x) {
    return TransferBranchIdxesCore_<meta::RemoveRef<T>>::Dst(
        level, meta::Forward<T>(x));
}

template <typename BranchIdx, typename BranchNum>
void CheckBranchIdx_  // NOLINT(misc-use-internal-linkage)
    (BranchIdx const& idx, BranchNum branch_num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<BranchIdx>);
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(integral::MathCompare(idx, branch_num) < 0);
}

template <typename SrcBranchIdxes>
BranchNum FetchAndCheckBranchIdx_(
    SrcBranchIdxes&&
        src_branch_idxes,  // NOLINT(cppcoreguidelines-missing-std-forward)
    BranchNum branch_num) {
    auto branch_idx{ src_branch_idxes() };
    ZETA_Core_StaticAssert(integral::IsIntegral<decltype(branch_idx)>);
    ZETA_Core_DebugAssert(0 <= branch_idx);
    ZETA_Core_DebugAssert(integral::MathCompare(branch_idx, branch_num) < 0);

    return static_cast<BranchNum>(branch_idx);
}

template <typename ActiveMap>
constexpr bool TestActiveMap_  // NOLINT(misc-use-internal-linkage)
    (ActiveMap active_map, BranchNum idx) {
    return (active_map & (static_cast<ActiveMap>(1)
                          << static_cast<unsigned long long>(idx))) != 0;
}

template <typename ActiveMap>
size_t GetNavNodeSize_(BranchNum branch_num) {
    return __builtin_offsetof(ZETA_Core_Identity(NavNode<ActiveMap>),
                              ptrs[branch_num]);
}

template <typename ActiveMap,
          typename NavNodeAllocator>
NavNode<ActiveMap>* AllocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (BranchNum branch_num, NavNodeAllocator& nav_node_alctr) {
    auto* nav_node{ static_cast<NavNode<ActiveMap>*>(
        allocator::SafeAllocate(nav_node_alctr, alignof(NavNode<ActiveMap>),
                                (GetNavNodeSize_<ActiveMap>)(branch_num))) };

    nav_node->active_map = 0;

    return nav_node;
}

template <typename ActiveMap,
          typename NavNodeAllocator>
void DeallocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (NavNode<ActiveMap>* node, NavNodeAllocator& node_alctr) {
    allocator::Deallocate(node_alctr, node);
}

#if EnDataNode

template <typename ActiveMap>
size_t GetDataNodeSize_(size_t elem_stride, BranchNum branch_num) {
    return __builtin_offsetof(ZETA_Core_Identity(DataNode<ActiveMap>),
                              data[elem_stride * branch_num]);
}

template <typename ActiveMap, typename DataNodeAllocator>
DataNode<ActiveMap>* AllocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (size_t elem_stride, BranchNum branch_num,
     DataNodeAllocator& data_node_alctr) {
    auto* data_node{ static_cast<DataNode<ActiveMap>*>(allocator::SafeAllocate(
        data_node_alctr, alignof(DataNode<ActiveMap>),
        (GetDataNodeSize_<ActiveMap>)(elem_stride, branch_num))) };

    data_node->active_map = 0;

    return data_node;
}

template <typename ActiveMap, typename DataNodeAllocator>
void DeallocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (DataNode<ActiveMap>* node, DataNodeAllocator& data_node_alctr) {
    allocator::Deallocate(data_node_alctr, node);
}

#endif

}  // namespace Namespace::detail

template <CntrTplParamList, typename NavNodeAllocatorInitArg
#if EnDataNode
          ,
          typename DataNodeAllocatorInitArg
#endif
          >
void Namespace::Init(Cntr<CntrTplArgList>& cntr,
                     NavNodeAllocatorInitArg&& nav_node_alctr_init_arg,
#if EnDataNode
                     DataNodeAllocatorInitArg&& data_node_alctr_init_arg,
#endif
                     unsigned level, BranchNum const* branch_nums
#if EnDataNode
                     ,
                     size_t elem_stride
#endif
) {
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        BranchNum branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= integral::WidthOf<ActiveMap>);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }

    lifecycle::Init(cntr.nav_node_alctr, meta::Forward<NavNodeAllocatorInitArg>(
                                             nav_node_alctr_init_arg));
    allocator::CheckContract(cntr.nav_node_alctr);

#if EnDataNode
    lifecycle::Init(
        cntr.data_node_alctr,
        meta::Forward<DataNodeAllocatorInitArg>(data_node_alctr_init_arg));
    allocator::CheckContract(cntr.data_node_alctr);
#endif

    cntr.level = level;
    cntr.branch_nums = branch_nums;

#if EnDataNode
    ZETA_Core_DebugAssert(0 < elem_stride);
    cntr.elem_stride = elem_stride;
#endif

    cntr.size = 0;

    cntr.root = nullptr;
}

template <CntrTplParamList>
void Namespace::Deinit(Cntr<CntrTplArgList>& cntr) {
    EraseAll(cntr);
}

template <CntrTplParamList>
size_t Namespace::GetSize(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.size;
}

template <CntrTplParamList>
size_t Namespace::GetCapacity(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    size_t ret{ 1 };

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

template <CntrTplParamList, typename BranchIdxesSource>
void* Namespace::Access(
    Cntr<CntrTplArgList>& cntr,
    BranchIdxesSource&&
        src_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    void* node{ cntr.root };

    if (node == nullptr) { return nullptr; }

    for (unsigned level_i{ level - 1 }; 0 < level_i; --level_i) {
        auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

        BranchNum cur_branch_idx{ detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i]) };

        if (!detail::TestActiveMap_(nav_node->active_map, cur_branch_idx)) {
            return nullptr;
        }

        node = nav_node->ptrs[cur_branch_idx];
    }

    BranchNum last_branch_idx{ detail::FetchAndCheckBranchIdx_(
        src_branch_idxes, branch_nums[0]) };

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

template <CntrTplParamList, typename DstBranchIdxes>
void* Namespace::FindFirst(
    Cntr<CntrTplArgList>& cntr,
    DstBranchIdxes&&
        dst_branch_idxes  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    struct {
        constexpr size_t operator()() { return 0; }
    } src_branch_idxes;

    return (FindNextIncl)(cntr, src_branch_idxes, dst_branch_idxes);
}

template <CntrTplParamList, typename DstBranchIdxes>
void* Namespace::FindLast(
    Cntr<CntrTplArgList>& cntr,
    DstBranchIdxes&&
        dst_branch_idxes  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    struct {
        BranchNum const* branch_nums;

        size_t operator()() { return *(--this->branch_nums) - 1; }
    } src_branch_idxes{ .branch_nums = cntr.branch_nums + cntr.level };

    return (FindPrevIncl)(cntr, src_branch_idxes, dst_branch_idxes);
}

template <CntrTplParamList, typename SrcBranchIdxes, typename DstBranchIdxes>
void* Namespace::FindPrevIncl(
    Cntr<CntrTplArgList>& cntr,
    SrcBranchIdxes&&
        src_branch_idxes_,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxes&&
        dst_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    decltype(auto) dst_branch_idxes{ detail::GetDstBranchIdxes_(
        level, dst_branch_idxes_) };

    void* root{ cntr.root };

    if (root == nullptr) {
        for (unsigned level_j{ level }; 0 < level_j;) {
            dst_branch_idxes(branch_nums[--level_j] - 1);
        }

        return nullptr;
    }

    BranchNum branch_idxes[max_level];
    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i]) };

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

            for (unsigned level_j{ level }; 0 < level_j;) {
                dst_branch_idxes(branch_idxes[--level_j]);
            }

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
            for (unsigned level_j{ level }; 0 < level_j;) {
                dst_branch_idxes(branch_nums[--level_j] - 1);
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

    for (unsigned level_j{ level }; 0 < level_j;) {
        dst_branch_idxes(branch_idxes[--level_j]);
    }

#if EnDataNode
    return static_cast<DataNode<ActiveMap>*>(nodes[0])->data +
           elem_stride * branch_idxes[0];
#else
    return static_cast<NavNode<ActiveMap>*>(nodes[0])->ptrs + branch_idxes[0];
#endif
}

template <CntrTplParamList, typename SrcBranchIdxes, typename DstBranchIdxes>
void* Namespace::FindPrevExcl(
    Cntr<CntrTplArgList>& cntr,
    SrcBranchIdxes&&
        src_branch_idxes_,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxes&&
        dst_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    decltype(auto) dst_branch_idxes{ detail::GetDstBranchIdxes_(
        level, dst_branch_idxes_) };

    BranchNum branch_idxes[max_level];

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        branch_idxes[level_i - 1] = detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i - 1]);
    }

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (0 < branch_idxes[level_i]) {
            --branch_idxes[level_i];
            goto L1;
        }

        branch_idxes[level_i] = branch_nums[level_i] - 1;
    }

    for (unsigned level_j{ level }; 0 < level_j;) {
        dst_branch_idxes(branch_nums[--level_j] - 1);
    }

    return nullptr;

L1:;

    return (FindPrevIncl)(cntr, branch_idxes, dst_branch_idxes);
}

template <CntrTplParamList, typename SrcBranchIdxes, typename DstBranchIdxes>
void* Namespace::FindNextIncl(
    Cntr<CntrTplArgList>& cntr,
    SrcBranchIdxes&&
        src_branch_idxes_,  // NOLINT(cppcoreguidelines-missing-std-forward)
    DstBranchIdxes&&
        dst_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    decltype(auto) dst_branch_idxes{ detail::GetDstBranchIdxes_(
        level, dst_branch_idxes_) };

    void* root{ cntr.root };

    if (root == nullptr) {
        for (unsigned level_j{ level }; 0 < level_j; --level_j) {
            dst_branch_idxes(0);
        }

        return nullptr;
    }

    BranchNum branch_idxes[max_level];
    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        BranchNum cur_branch_idx{ detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i]) };

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

            for (unsigned level_j{ level }; 0 < level_j;) {
                dst_branch_idxes(branch_idxes[--level_j]);
            }

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
            for (unsigned level_j{ level }; 0 < level_j; --level_j) {
                dst_branch_idxes(0);
            }

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

    for (unsigned level_j{ level }; 0 < level_j;) {
        dst_branch_idxes(branch_idxes[--level_j]);
    }

#if EnDataNode
    return static_cast<DataNode<ActiveMap>*>(nodes[0])->data +
           elem_stride * branch_idxes[0];
#else
    return static_cast<NavNode<ActiveMap>*>(nodes[0])->ptrs + branch_idxes[0];
#endif
}

template <CntrTplParamList, typename SrcBranchIdxes, typename DstBranchIdxes>
void* Namespace::FindNextExcl(
    Cntr<CntrTplArgList>& cntr,
    SrcBranchIdxes&&
        src_branch_idxes_,  // NOLINT(cppcoreguidelines-missing-std-forward
    DstBranchIdxes&&
        dst_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    decltype(auto) dst_branch_idxes{ detail::GetDstBranchIdxes_(
        level, dst_branch_idxes_) };

    BranchNum branch_idxes[max_level];

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        branch_idxes[level_i - 1] = detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i - 1]);
    }

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (branch_idxes[level_i] < branch_nums[level_i] - 1) {
            ++branch_idxes[level_i];
            goto L1;
        }

        branch_idxes[level_i] = 0;
    }

    for (unsigned level_j{ level }; 0 < level_j; --level_j) {
        dst_branch_idxes(0);
    }

    return nullptr;

L1:;

    return (FindNextIncl)(cntr, branch_idxes, dst_branch_idxes);
}

template <CntrTplParamList, typename BranchIdxesSource>
pair::Pair<void*, bool> Namespace::Insert(
    Cntr<CntrTplArgList>& cntr,
    BranchIdxesSource&&
        src_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr) };
#endif

    unsigned level{ cntr.level };
    BranchNum const* branch_nums{ cntr.branch_nums };

#if EnDataNode
    size_t elem_stride{ cntr.elem_stride };
#endif

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    if (cntr.root == nullptr) {
        cntr.root =
#if EnDataNode
            level == 1
                ? static_cast<void*>(detail::AllocateDataNode_<ActiveMap>(
                      elem_stride, branch_nums[0], data_node_alctr))
                :
#endif
                static_cast<void*>(detail::AllocateNavNode_<ActiveMap>(
                    branch_nums[level - 1], nav_node_alctr));
    }

    unsigned level_i{ level - 1 };
    void* node{ cntr.root };
    bool exist_node{ true };

    for (; 0 < level_i; --level_i) {
        BranchNum cur_branch_idx{ detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i]) };

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
            level_i == 1
                ? static_cast<void*>(detail::AllocateDataNode_<ActiveMap>(
                      elem_stride, branch_nums[0], data_node_alctr))
                :
#endif
                static_cast<void*>(detail::AllocateNavNode_<ActiveMap>(
                    branch_nums[level_i - 1], nav_node_alctr))
        };

        static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx] = nxt_node;

        node = nxt_node;
    }

    BranchNum last_branch_idx{ detail::FetchAndCheckBranchIdx_(
        src_branch_idxes, branch_nums[0]) };

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
        ++cntr.size;

        static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
            (node)->active_map += static_cast<ActiveMap>(1) << last_branch_idx;
    }

    return { .first = addr, .second = newly_inserted };
}

template <CntrTplParamList, typename BramchIdxSource>
bool Namespace::Erase(
    Cntr<CntrTplArgList>& cntr,
    BramchIdxSource&&
        src_branch_idxes_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };

    BranchNum const* branch_nums{ cntr.branch_nums };

    decltype(auto) src_branch_idxes{ detail::GetSrcBranchIdxes_(
        level, src_branch_idxes_) };

    void* node{ cntr.root };

    if (node == nullptr) { return false; }

    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr) };
#endif

    void* nodes[max_level];
    size_t branch_idxes[max_level];

    for (unsigned level_i{ level - 1 };; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        BranchNum cur_branch_idx{ detail::FetchAndCheckBranchIdx_(
            src_branch_idxes, branch_nums[level_i]) };
        branch_idxes[level_i] = cur_branch_idx;

        if (!detail::TestActiveMap_(
                static_cast<NavNode<ActiveMap>*>(node)->active_map,
                cur_branch_idx)) {
            return false;
        }

        node = static_cast<NavNode<ActiveMap>*>(node)->ptrs[cur_branch_idx];
    }

    BranchNum last_branch_idx{ detail::FetchAndCheckBranchIdx_(
        src_branch_idxes, branch_nums[0]) };
    branch_idxes[0] = last_branch_idx;

    if (!detail::TestActiveMap_(
            static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
                (node)->active_map,
            last_branch_idx)) {
        return false;
    }

    --cntr.size;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        node = nodes[level_i];

#if EnDataNode
        if (level_i == 0) {
            auto* data_node{ static_cast<DataNode<ActiveMap>*>(node) };

            data_node->active_map -= static_cast<ActiveMap>(1)
                                     << branch_idxes[level_i];

            if (data_node->active_map != 0) { return true; }

            detail::DeallocateDataNode_(data_node, data_node_alctr);
        } else
#endif
        {
            auto* nav_node{ static_cast<NavNode<ActiveMap>*>(node) };

            nav_node->active_map -= static_cast<ActiveMap>(1)
                                    << branch_idxes[level_i];

            if (nav_node->active_map != 0) { return true; }

            detail::DeallocateNavNode_(nav_node, nav_node_alctr);
        }
    }

    cntr.root = nullptr;

    return true;
}

namespace Namespace::detail {

template <CntrTplParamList>
void EraseAllRecursive_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, void* node, unsigned level_i) {
    auto& nav_node_alctr{ meta::GetInstRef(cntr.nav_node_alctr) };

#if EnDataNode
    auto& data_node_alctr{ meta::GetInstRef(cntr.data_node_alctr) };
#endif

    if (level_i == 0) {
        (EnDataNodeTernary(DeallocateDataNode_, DeallocateNavNode_))(
            static_cast<EnDataNodeTernary(DataNode, NavNode) < ActiveMap>* >
                (node),
            EnDataNodeTernary(data_node_alctr, nav_node_alctr));

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

    (DeallocateNavNode_)(nav_node, nav_node_alctr);
}

}  // namespace Namespace::detail

template <CntrTplParamList>
void Namespace::EraseAll(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    unsigned level{ cntr.level };
    void* root{ cntr.root };

    if (root == nullptr) { return; }

    detail::EraseAllRecursive_(cntr, root, level - 1);

    cntr.size = 0;
    cntr.root = nullptr;
}

namespace Namespace::detail {

template <typename ActiveMap>
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
            detail::GetDataNodeSize_<ActiveMap>(elem_stride, branch_nums[0]));

        size_t ret{ static_cast<size_t>(integral_bit::PopCount(
            static_cast<DataNode<ActiveMap>*>(node)->active_map)) };

        ZETA_Core_DebugAssert(0 < ret);

        size_t k{ 0 };

        for (unsigned i{ 0 }; i < branch_nums[0]; ++i) {
            k += detail::TestActiveMap_(
                static_cast<DataNode<ActiveMap>*>(node)->active_map, i);
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
void Namespace::Sanitize(Cntr<CntrTplArgList>& cntr,
                         mem_recorder::MemRecorder* dst_nav_node
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

    ZETA_Core_DebugAssert(size == cntr.size);

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
