#if !defined(EnData)
#error "EnData is not defined."
#endif

#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/multi_level_table.mpp.hpp>

#if EnData

#define Cntr MultiLevelDataTable
#define TplList NavNodeAllocator, DataNodeAllocator
#define TypenameTplList typename NavNodeAllocator, typename DataNodeAllocator

#else

#define Cntr MultiLevelPtrTable
#define TplList NavNodeAllocator
#define TypenameTplList typename NavNodeAllocator

#endif

namespace zeta::core {

template <TypenameTplList>
constexpr bool Cntr<TplList>::TestActiveMap_(unsigned long long active_map,
                                             unsigned idx) {
    return (active_map & (1ULL << (idx))) != 0;
}

template <TypenameTplList>
void* Cntr<TplList>::AllocateNavNode_(size_t branch_num,
                                      NavNodeAllocator* nav_node_allocator) {
    auto nav_node{ static_cast<NavNode*>(
        allocator::SafeAllocate(nav_node_allocator, alignof(NavNode),
                                offsetof(NavNode, ptrs[branch_num]))) };

    nav_node->active_map = 0;

    return &nav_node->active_map;
}

template <TypenameTplList>
void Cntr<TplList>::DeallocateNavNode_(NavNodeAllocator* nav_node_allocator,
                                       void* node) {
    NavNodeAllocator::Deallocate(
        nav_node_allocator,
        ZETA_Core_MemberToStruct(NavNode, active_map, node));
}

// -----------------------------------------------------------------------------

#if EnData

template <TypenameTplList>
size_t Cntr<TplList>::CalcDataNodeSize_(size_t stride, size_t branch_num) {
    return UIntAlignUp(stride * branch_num + sizeof(unsigned long long),
                       alignof(unsigned long long));
}

template <TypenameTplList>
void* Cntr<TplList>::AllocateDataNode_(size_t stride, size_t branch_num,
                                       DataNodeAllocator* data_node_allocator) {
    size_t data_node_size{ CalcDataNodeSize_(stride, branch_num) };

    void* data_node{ static_cast<char*>(allocator::SafeAllocate(
                         data_node_allocator, alignof(unsigned long long),
                         data_node_size)) +
                     (data_node_size - sizeof(unsigned long long)) };

    *static_cast<unsigned long long*>(data_node) = 0;

    return data_node;
}

template <TypenameTplList>
void Cntr<TplList>::DeallocateDataNode_(size_t stride, size_t branch_num,
                                        DataNodeAllocator* data_node_allocator,
                                        void* node) {
    DataNodeAllocator::Deallocate(
        data_node_allocator,
        static_cast<char*>(node) - (CalcDataNodeSize_(stride, branch_num) -
                                    sizeof(unsigned long long)));
}

#endif

// -----------------------------------------------------------------------------

template <TypenameTplList>
void Cntr<TplList>::Init(void* cntr_) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
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

#if EnData
    ZETA_Core_DebugAssert(0 < cntr->stride);
#endif

    cntr->size = 0;

    cntr->root = nullptr;

    ZETA_Core_DebugAssert(cntr->nav_node_allocator.vtable != nullptr);
    ZETA_Core_DebugAssert(cntr->nav_node_allocator.align % alignof(NavNode) ==
                          0);

#if EnData
    ZETA_Core_DebugAssert(cntr->data_node_allocator.vtable != nullptr);
    ZETA_Core_DebugAssert(
        cntr->data_node_allocator.align % alignof(unsigned long long) == 0);
#endif
}

template <TypenameTplList>
void Cntr<TplList>::Deinit(void* cntr) {
    EraseAll(cntr);
}

template <TypenameTplList>
size_t Cntr<TplList>::GetSize(void* cntr_) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

    return cntr->size;
}

template <TypenameTplList>
size_t Cntr<TplList>::GetCapacity(void* cntr_) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

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

template <TypenameTplList>
void* Cntr<TplList>::Access(void* cntr_, size_t* idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(CheckIdxes(cntr, idxes));

    unsigned level{ cntr->level };

#if EnData
    unsigned short const* branch_nums{ cntr->branch_nums };
    size_t stride{ cntr->stride };
#endif

    void* node{ cntr->root };

    if (node == nullptr) { return nullptr; }

    for (unsigned level_i{ level - 1 }; 0 < level_i; --level_i) {
        size_t cur_idx{ idxes[level_i] };

        if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
                            static_cast<unsigned>(cur_idx))) {
            return nullptr;
        }

        node = static_cast<NavNode*>(node)->ptrs[cur_idx];
    }

    size_t last_idx{ idxes[0] };

    if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
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

template <TypenameTplList>
void* Cntr<TplList>::FindFirst(void* cntr_, size_t* dst_idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

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

template <TypenameTplList>
void* Cntr<TplList>::FindLast(void* cntr_, size_t* dst_idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

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

template <TypenameTplList>
void* Cntr<TplList>::FindPrev(void* cntr_, size_t* idxes, bool included) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(CheckIdxes(cntr, idxes));

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

        if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
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

        int found_idx{ FindPrevOne(*static_cast<unsigned long long*>(node),
                                   static_cast<int>(idxes[level_i])) };

        if (0 <= found_idx) {
            idxes[level_i] = static_cast<size_t>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = static_cast<size_t>(FindPrevOne(
            *static_cast<unsigned long long*>(node), branch_nums[level_i]));
    }

#if EnData
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

template <TypenameTplList>
void* Cntr<TplList>::FindNext(void* cntr_, size_t* idxes, bool included) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(CheckIdxes(cntr, idxes));

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

        if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
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

        int found_idx{ FindNextOne(*static_cast<unsigned long long*>(node),
                                   static_cast<int>(idxes[level_i])) };

        if (0 <= found_idx) {
            idxes[level_i] = static_cast<size_t>(found_idx);
            break;
        }
    }

    while (0 < level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = static_cast<size_t>(
            FindNextOne(*static_cast<unsigned long long*>(node), -1));
    }

#if EnData
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

template <TypenameTplList>
Pair<void*, bool> Cntr<TplList>::Insert(void* cntr_, size_t* idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(CheckIdxes(cntr, idxes));

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
#endif

    NavNodeAllocator* nav_node_allocator{ &cntr->nav_node_allocator };

#if EnData
    DataNodeAllocator* data_node_allocator{ &cntr->data_node_allocator };
#endif

    if (cntr->root == nullptr) {
#if EnData
        if (level == 1) {
            cntr->root =
                AllocateDataNode_(stride, branch_nums[0], data_node_allocator);
        } else
#endif
        {
            cntr->root =
                AllocateNavNode_(branch_nums[level - 1], nav_node_allocator);
        }
    }

    unsigned level_i{ level - 1 };
    void* node{ cntr->root };

    for (;
         0 < level_i && TestActiveMap_(*static_cast<unsigned long long*>(node),
                                       static_cast<unsigned>(idxes[level_i]));
         --level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        *static_cast<unsigned long long*>(node) += 1ULL << idxes[level_i];

        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]] =
#if EnData
            level_i == 1
                ? AllocateDataNode_(stride, branch_nums[0], data_node_allocator)
                :
#endif
                AllocateNavNode_(branch_nums[level_i - 1], nav_node_allocator);
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

    bool newly_inserted{ !TestActiveMap_(
        *static_cast<unsigned long long*>(node),
        static_cast<unsigned>(last_idx)) };

    if (newly_inserted) {
        ++cntr->size;
        *static_cast<unsigned long long*>(node) += (1ULL << last_idx);
    }

    return { .first = addr, .second = newly_inserted };
}

template <TypenameTplList>
bool Cntr<TplList>::Erase(void* cntr_, size_t* idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(CheckIdxes(cntr, idxes));

    unsigned level{ cntr->level };

#if EnData
    unsigned short const* branch_nums{ cntr->branch_nums };
    size_t stride{ cntr->stride };
#endif

    void* node{ cntr->root };

    if (node == nullptr) { return false; }

    NavNodeAllocator* nav_node_allocator{ &cntr->nav_node_allocator };

#if EnData
    DataNodeAllocator* data_node_allocator{ &cntr->data_node_allocator };
#endif

    void* nodes[max_level];

    for (unsigned level_i{ level - 1 };; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
                            static_cast<unsigned>(idxes[level_i]))) {
            return false;
        }

        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]];
    }

    if (!TestActiveMap_(*static_cast<unsigned long long*>(node),
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
            DeallocateDataNode_(stride, branch_nums[0], data_node_allocator,
                                node);
        } else
#endif
        {
            DeallocateNavNode_(nav_node_allocator, node);
        }
    }

    cntr->root = nullptr;

    return true;
}

template <TypenameTplList>
void Cntr<TplList>::EraseAll_(Cntr* cntr, void* node, unsigned level_i) {
#if EnData
    unsigned branch_num{ cntr->branch_nums[level_i] };
    size_t stride{ cntr->stride };
#endif

    NavNodeAllocator* nav_node_allocator{ &cntr->nav_node_allocator };

#if EnData
    DataNodeAllocator* data_node_allocator{ &cntr->data_node_allocator };
#endif

    if (level_i == 0) {
#if EnData
        DeallocateDataNode_(stride, branch_num, data_node_allocator, node);
#else
        DeallocateNavNode_(nav_node_allocator, node);
#endif

        return;
    }

    for (int idx{ -1 };
         (idx = FindNextOne(*static_cast<unsigned long long*>(node), idx)) !=
         -1;) {
        EraseAll_(cntr, static_cast<NavNode*>(node)->ptrs[idx], level_i - 1);
    }

    DeallocateNavNode_(nav_node_allocator, node);
}

template <TypenameTplList>
void Cntr<TplList>::EraseAll(void* cntr_) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

    unsigned level{ cntr->level };
    void* root{ cntr->root };

    if (root == nullptr) { return; }

    EraseAll_(cntr, root, level - 1);

    cntr->size = 0;
    cntr->root = nullptr;
}

template <TypenameTplList>
void Cntr<TplList>::Check(void* cntr_) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
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

template <TypenameTplList>
void Cntr<TplList>::CheckIdxes(void* cntr_, size_t const* idxes) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

    ZETA_Core_DebugAssert(idxes != nullptr);

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}
template <TypenameTplList>
size_t Cntr<TplList>::Sanitize_(MemRecorder* dst_nav_node,
#if EnData
                                MemRecorder* dst_data_node,
#endif
                                unsigned level_i,
                                unsigned short const* branch_nums,
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
            size_t data_node_size{ CalcDataNodeSize_(stride, branch_nums[0]) };

            MemRecorder::Record(
                dst_data_node,
                static_cast<char*>(node) -
                    (data_node_size - sizeof(unsigned long long)),
                data_node_size);
        }

        return cur_cnt;
    }
#endif

    if (dst_nav_node != nullptr) {
        MemRecorder::Record(dst_nav_node,
                            ZETA_Core_MemberToStruct(NavNode, active_map, node),
                            sizeof(NavNode));
    }

#if !EnData
    if (level_i == 0) { return cur_cnt; }
#endif

    size_t size{ 0 };

    for (int idx{ -1 };
         (idx = FindNextOne(*static_cast<unsigned long long*>(node), idx)) !=
         -1;) {
        ZETA_Core_DebugAssert(
            TestActiveMap_(*static_cast<unsigned long long*>(node),
                           static_cast<unsigned>(idx)));

        size += Sanitize_(dst_nav_node,
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

template <TypenameTplList>
void Cntr<TplList>::Sanitize(void* cntr_, MemRecorder* dst_nav_node
#if EnData
                             ,
                             MemRecorder* dst_data_node
#endif
) {
    auto* cntr{ static_cast<Cntr*>(cntr_) };
    ZETA_Core_WhenEnableDebug(Check(cntr));

    unsigned level{ cntr->level };
    unsigned short const* branch_nums{ cntr->branch_nums };

#if EnData
    size_t stride{ cntr->stride };
#endif

    void* root{ cntr->root };

    size_t size{ root == nullptr ? 0
                                 : Sanitize_(dst_nav_node,
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
