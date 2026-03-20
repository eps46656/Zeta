#if !defined(EnData)
#error "EnData is not defined."
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
#include <zeta/core/utils.hpp>

#if EnData

#pragma push_macro("NameSpace")
#define NameSpace multi_level_data_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename NavNodeAllocatorLike, typename DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList NavNodeAllocatorLike, DataNodeAllocatorLike

#else

#pragma push_macro("NameSpace")
#define NameSpace multi_level_ptr_table

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename NavNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList NavNodeAllocatorLike

#endif

namespace zeta::core {

namespace NameSpace::detail {

template <CntrTplParamList>
void Check_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    unsigned level{ cntr->level };
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    unsigned short const* branch_nums{ cntr->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        unsigned branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }
}

template <CntrTplParamList>
void CheckIdxes_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, size_t const* idxes) {
    Check_(cntr);

    ZETA_Core_DebugAssert(idxes != nullptr);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}

constexpr bool TestActiveMap_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long active_map, unsigned idx) {
    return (active_map & (1ULL << (idx))) != 0;
}

template <typename NavNodeAllocator>
void* AllocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (size_t branch_num, NavNodeAllocator* nav_node_alctr) {
    auto* nav_node{ static_cast<NavNode*>(
        allocator::SafeAllocate(nav_node_alctr, alignof(NavNode),
                                offsetof(NavNode, ptrs[branch_num]))) };

    nav_node->active_map = 0;

    return &nav_node->active_map;
}

template <typename NavNodeAllocator>
void DeallocateNavNode_  // NOLINT(misc-use-internal-linkage)
    (NavNodeAllocator* node_alctr, void* node) {
    allocator::Deallocate(node_alctr,
                          ZETA_Core_MemberToStruct(NavNode, active_map, node));
}

#if EnData

inline size_t CalcDataNodeSize_  // NOLINT(misc-use-internal-linkage)
    (size_t stride, size_t branch_num) {
    return integral_math::AlignUp(
        stride * branch_num + sizeof(unsigned long long),
        alignof(unsigned long long));
}

template <typename DataNodeAllocator>
void* AllocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (size_t stride, size_t branch_num, DataNodeAllocator* data_node_alctr) {
    size_t data_node_size{ CalcDataNodeSize_(stride, branch_num) };

    void* data_node{ static_cast<char*>(allocator::SafeAllocate(
                         data_node_alctr, alignof(unsigned long long),
                         data_node_size)) +
                     (data_node_size - sizeof(unsigned long long)) };

    *static_cast<unsigned long long*>(data_node) = 0;

    return data_node;
}

template <typename DataNodeAllocator>
void DeAllocateDataNode_  // NOLINT(misc-use-internal-linkage)
    (size_t stride, size_t branch_num, DataNodeAllocator* data_node_alctr,
     void* node) {
    allocator::Deallocate(
        data_node_alctr,
        static_cast<char*>(node) - (CalcDataNodeSize_(stride, branch_num) -
                                    sizeof(unsigned long long)));
}

#endif

}  // namespace NameSpace::detail

template <CntrTplParamList, typename NavNodeAllocatorInitArg
#if EnData
          ,
          typename DataNodeAllocatorInitArg
#endif
          >
void NameSpace::Init(Cntr<CntrTplArgList>* cntr, unsigned level,
                     unsigned short const* branch_nums
#if EnData
                     ,
                     size_t stride
#endif
                     ,
                     NavNodeAllocatorInitArg&& nav_node_alctr_init_arg
#if EnData
                     ,
                     DataNodeAllocatorInitArg&& data_node_alctr_init_arg
#endif
) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        unsigned branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }

#if EnData
    ZETA_Core_DebugAssert(0 < cntr->stride);
#endif

    cntr->level = level;
    cntr->branch_nums = branch_nums;

#if EnData
    cntr->stride = stride;
#endif

    cntr->size = 0;

    cntr->root = nullptr;

    lifecycle::Init(
        cntr->nav_node_alctr,
        meta::Forward<NavNodeAllocatorInitArg>(nav_node_alctr_init_arg));
    allocator::CheckContract(cntr->nav_node_alctr);

#if EnData
    lifecycle::Init(
        cntr->data_node_alctr,
        meta::Forward<DataNodeAllocatorInitArg>(data_node_alctr_init_arg));
    allocator::CheckContract(cntr->data_node_alctr);
#endif
}

template <CntrTplParamList>
void NameSpace::Deinit(Cntr<CntrTplArgList>* cntr) {
    EraseAll(cntr);
}

template <CntrTplParamList>
size_t NameSpace::GetSize(Cntr<CntrTplArgList>* cntr) {
    detail::Check_(cntr);

    return cntr->size;
}

template <CntrTplParamList>
size_t NameSpace::GetCapacity(Cntr<CntrTplArgList>* cntr) {
    detail::Check_(cntr);

    size_t ret{ 1 };

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

template <CntrTplParamList>
void* NameSpace::Access(Cntr<CntrTplArgList>* cntr, size_t* idxes) {
    detail::CheckIdxes_(cntr, idxes);

    unsigned level{ cntr->level };

#if EnData
    unsigned short const* branch_nums{ cntr->branch_nums };
    size_t stride{ cntr->stride };
#endif

    void* node{ cntr->root };

    if (node == nullptr) { return nullptr; }

    for (unsigned level_i{ level - 1 }; 0 < level_i; --level_i) {
        size_t cur_idx{ idxes[level_i] };

        if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                    static_cast<unsigned>(cur_idx))) {
            return nullptr;
        }

        node = static_cast<NavNode*>(node)->ptrs[cur_idx];
    }

    size_t last_idx{ idxes[0] };

    if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                static_cast<unsigned>(last_idx))) {
        return nullptr;
    }

#if EnData
    return static_cast<char*>(node) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * last_idx;
#else
    return static_cast<void*>(static_cast<NavNode*>(node)->ptrs + last_idx);
#endif
}

template <CntrTplParamList>
void* NameSpace::FindFirst(Cntr<CntrTplArgList>* cntr, size_t* dst_idxes) {
    detail::Check_(cntr);

    unsigned level{ cntr->level };

    size_t idxes[max_level];

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = 0;
    }

    void* ret{ FindNext(cntr, idxes, true) };

    if (dst_idxes != nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

template <CntrTplParamList>
void* NameSpace::FindLast(Cntr<CntrTplArgList>* cntr, size_t* dst_idxes) {
    detail::Check_(cntr);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

    size_t idxes[max_level];

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    void* ret{ FindPrev(cntr, idxes, true) };

    if (dst_idxes != nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

template <CntrTplParamList>
void* NameSpace::FindPrev(Cntr<CntrTplArgList>* cntr, size_t* idxes,
                          bool included) {
    detail::CheckIdxes_(cntr, idxes);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
    size_t data_node_size{ CalcDataNodeSize_(stride, branch_nums[0]) };
#endif

    if (!included) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            if (0 < idxes[level_i]) {
                --idxes[level_i];
                goto L1;
            }

            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return nullptr;
    }

L1:

    void* root{ cntr->root };

    if (root == nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return nullptr;
    }

    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        nodes[level_i] = node;

        size_t cur_idx{ idxes[level_i] };

        if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                    static_cast<unsigned>(cur_idx))) {
            break;
        }

#if EnData
        if (level_i == 0) {
            return static_cast<char*>(node) + sizeof(unsigned long long) -
                   data_node_size + stride * cur_idx;
        }
#endif

        void* addr{ static_cast<void*>(static_cast<NavNode*>(node)->ptrs +
                                       cur_idx) };

#if !EnData
        if (level_i == 0) { return addr; }
#endif

        node = *static_cast<void**>(addr);
    }

    for (;; ++level_i) {
        if (level_i == level) {
            for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
                idxes[level_i] = branch_nums[level_i] - 1;
            }

            return nullptr;
        }

        node = nodes[level_i];

        long long found_idx{ integral_bit::FindPrevBit(
            *static_cast<unsigned long long*>(node),
            static_cast<long long>(idxes[level_i]) - 1) };

        if (0 <= found_idx) {
            idxes[level_i] = static_cast<size_t>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = static_cast<size_t>(integral_bit::FindPrevBit(
            *static_cast<unsigned long long*>(node), branch_nums[level_i] - 1));
    }

#if EnData
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

template <CntrTplParamList>
void* NameSpace::FindNext(Cntr<CntrTplArgList>* cntr, size_t* idxes,
                          bool included) {
    detail::CheckIdxes_(cntr, idxes);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
    size_t data_node_size{ CalcDataNodeSize_(stride, branch_nums[0]) };
#endif

    if (!included) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            if (idxes[level_i] < branch_nums[level_i] - 1) {
                ++idxes[level_i];
                goto L1;
            }

            idxes[level_i] = 0;
        }

        return nullptr;
    }

L1:

    void* root{ cntr->root };

    if (root == nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            idxes[level_i] = 0;
        }

        return nullptr;
    }

    void* nodes[max_level];

    unsigned level_i{ level - 1 };
    void* node{ root };

    for (;; --level_i) {
        nodes[level_i] = node;

        size_t cur_idx{ idxes[level_i] };

        if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                    static_cast<unsigned>(cur_idx))) {
            break;
        }

#if EnData
        if (level_i == 0) {
            return static_cast<char*>(node) + sizeof(unsigned long long) -
                   data_node_size + stride * cur_idx;
        }
#endif

        void* addr{ static_cast<void*>(static_cast<NavNode*>(node)->ptrs +
                                       cur_idx) };

#if !EnData
        if (level_i == 0) { return addr; }
#endif

        node = *static_cast<void**>(addr);
    }

    for (;; ++level_i) {
        if (level_i == level) {
            for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
                idxes[level_i] = 0;
            }

            return nullptr;
        }

        node = nodes[level_i];

        long long found_idx{ integral_bit::FindNextBit(
            *static_cast<unsigned long long*>(node),
            static_cast<long long>(idxes[level_i]) + 1) };

        if (found_idx <
            static_cast<long long>(integral::WidthOf<unsigned long long>)) {
            idxes[level_i] = static_cast<size_t>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = static_cast<size_t>(integral_bit::FindNextBit(
            *static_cast<unsigned long long*>(node), 0));
    }

#if EnData
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

template <CntrTplParamList>
utils::Pair<void*, bool> NameSpace::Insert(Cntr<CntrTplArgList>* cntr,
                                           size_t* idxes) {
    detail::CheckIdxes_(cntr, idxes);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
#endif

    auto* nav_node_alctr{ utils::GetInstPtr(cntr->nav_node_alctr) };

#if EnData
    auto* data_node_alctr{ utils::GetInstPtr(cntr->data_node_alctr) };
#endif

    if (cntr->root == nullptr) {
#if EnData
        if (level == 1) {
            cntr->root = detail::AllocateDataNode_(stride, branch_nums[0],
                                                   data_node_alctr);
        } else
#endif
        {
            cntr->root = detail::AllocateNavNode_(branch_nums[level - 1],
                                                  nav_node_alctr);
        }
    }

    unsigned level_i{ level - 1 };
    void* node{ cntr->root };

    for (; 0 < level_i &&
           detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                  static_cast<unsigned>(idxes[level_i]));
         --level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        *static_cast<unsigned long long*>(node) += 1ULL << idxes[level_i];

        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]] =
#if EnData
            level_i == 1 ? detail::AllocateDataNode_(stride, branch_nums[0],
                                                     data_node_alctr)
                         :
#endif
                         detail::AllocateNavNode_(branch_nums[level_i - 1],
                                                  nav_node_alctr);
    }

    size_t last_idx{ idxes[0] };

    void* addr{
#if EnData
        static_cast<char*>(node) + sizeof(unsigned long long) -
        CalcDataNodeSize_(stride, branch_nums[0]) + stride * last_idx
#else
        static_cast<void*>(static_cast<NavNode*>(node)->ptrs + last_idx)
#endif
    };

    bool newly_inserted{ !detail::TestActiveMap_(
        *static_cast<unsigned long long*>(node),
        static_cast<unsigned>(last_idx)) };

    if (newly_inserted) {
        ++cntr->size;
        *static_cast<unsigned long long*>(node) += (1ULL << last_idx);
    }

    return { .first = addr, .second = newly_inserted };
}

template <CntrTplParamList>
bool NameSpace::Erase(Cntr<CntrTplArgList>* cntr, size_t* idxes) {
    detail::CheckIdxes_(cntr, idxes);

    unsigned level{ cntr->level };

#if EnData
    unsigned short const* branch_nums{ cntr->branch_nums };
    size_t stride{ cntr->stride };
#endif

    void* node{ cntr->root };

    if (node == nullptr) { return false; }

    auto* nav_node_alctr{ utils::GetInstPtr(cntr->nav_node_alctr) };

#if EnData
    auto* data_node_alctr{ utils::GetInstPtr(cntr->data_node_alctr) };
#endif

    void* nodes[max_level];

    for (unsigned level_i{ level - 1 };; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                    static_cast<unsigned>(idxes[level_i]))) {
            return false;
        }

        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]];
    }

    if (!detail::TestActiveMap_(*static_cast<unsigned long long*>(node),
                                static_cast<unsigned>(idxes[0]))) {
        return false;
    }

    --cntr->size;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        node = nodes[level_i];

        *static_cast<unsigned long long*>(node) -= 1ULL << idxes[level_i];

        if (*static_cast<unsigned long long*>(node) != 0) { return true; }

#if EnData
        if (level_i == 0) {
            detail::DeAllocateDataNode_(stride, branch_nums[0], data_node_alctr,
                                        node);
        } else
#endif
        {
            detail::DeallocateNavNode_(nav_node_alctr, node);
        }
    }

    cntr->root = nullptr;

    return true;
}

namespace NameSpace::detail {

template <CntrTplParamList>
void EraseAllRecursive_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, void* node, unsigned level_i) {
#if EnData
    unsigned branch_num{ cntr->branch_nums[level_i] };
    size_t stride{ cntr->stride };
#endif

    auto* nav_node_alctr{ utils::GetInstPtr(cntr->nav_node_alctr) };

#if EnData
    auto* data_node_alctr{ utils::GetInstPtr(cntr->data_node_alctr) };
#endif

    if (level_i == 0) {
#if EnData
        DeAllocateDataNode_(stride, branch_num, data_node_alctr, node);
#else
        DeallocateNavNode_(nav_node_alctr, node);
#endif

        return;
    }

    for (long long idx{ -1 };
         (idx = integral_bit::FindNextBit(
              *static_cast<unsigned long long*>(node), idx + 1)) <
         static_cast<long long>(integral::WidthOf<unsigned long long>);) {
        (EraseAllRecursive_)(cntr, static_cast<NavNode*>(node)->ptrs[idx],
                             level_i - 1);
    }

    (DeallocateNavNode_)(nav_node_alctr, node);
}

}  // namespace NameSpace::detail

template <CntrTplParamList>
void NameSpace::EraseAll(Cntr<CntrTplArgList>* cntr) {
    detail::Check_(cntr);

    unsigned level{ cntr->level };
    void* root{ cntr->root };

    if (root == nullptr) { return; }

    detail::EraseAllRecursive_(cntr, root, level - 1);

    cntr->size = 0;
    cntr->root = nullptr;
}

namespace NameSpace::detail {

inline size_t SanitizeRecursive_  // NOLINT(
                                  // misc-no-recursion,
                                  // misc-use-internal-linkage)
    (mem_recorder::MemRecorder* dst_nav_node,
#if EnData
     mem_recorder::MemRecorder* dst_data_node,
#endif
     unsigned level_i, unsigned short const* branch_nums,
#if EnData
     size_t stride,
#endif
     void* node) {
    ZETA_Core_DebugAssert(*static_cast<unsigned long long*>(node) != 0);

    size_t cur_cnt{ static_cast<size_t>(
        __builtin_popcountll(*static_cast<unsigned long long*>(node))) };

    ZETA_Core_DebugAssert(0 < cur_cnt);

#if EnData
    if (level_i == 0) {
        if (dst_data_node != nullptr) {
            size_t data_node_size{ (CalcDataNodeSize_)(stride,
                                                       branch_nums[0]) };

            mem_recorder::Record(
                dst_data_node,
                static_cast<char*>(node) -
                    (data_node_size - sizeof(unsigned long long)),
                data_node_size);
        }

        return cur_cnt;
    }
#endif

    if (dst_nav_node != nullptr) {
        mem_recorder::Record(
            dst_nav_node, ZETA_Core_MemberToStruct(NavNode, active_map, node),
            sizeof(NavNode));
    }

#if !EnData
    if (level_i == 0) { return cur_cnt; }
#endif

    size_t size{ 0 };

    for (long long idx{ -1 };
         (idx = integral_bit::FindNextBit(
              *static_cast<unsigned long long*>(node), idx + 1)) <
         static_cast<long long>(integral::WidthOf<unsigned long long>);) {
        ZETA_Core_DebugAssert(
            TestActiveMap_(*static_cast<unsigned long long*>(node),
                           static_cast<unsigned>(idx)));

        size += SanitizeRecursive_(dst_nav_node,
#if EnData
                                   dst_data_node,
#endif
                                   level_i - 1, branch_nums,
#if EnData
                                   stride,
#endif
                                   static_cast<NavNode*>(node)->ptrs[idx]);
    }

    return size;
}

}  // namespace NameSpace::detail

template <CntrTplParamList>
void NameSpace::Sanitize(Cntr<CntrTplArgList>* cntr,
                         mem_recorder::MemRecorder* dst_nav_node
#if EnData
                         ,
                         mem_recorder::MemRecorder* dst_data_node
#endif
) {

    detail::Check_(cntr);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
#endif

    void* root{ cntr->root };

    size_t size{ root == nullptr
                     ? 0
                     : detail::SanitizeRecursive_(dst_nav_node,
#if EnData
                                                  dst_data_node,
#endif
                                                  level - 1, branch_nums,
#if EnData
                                                  stride,
#endif
                                                  root) };

    ZETA_Core_DebugAssert(size == cntr->size);
}

}  // namespace zeta::core

#pragma pop_macro("NameSpace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
