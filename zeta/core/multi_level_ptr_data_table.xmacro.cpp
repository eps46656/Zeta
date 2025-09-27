#include <zeta/core/allocator.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/multi_level_ptr_data_table.xmacro.hpp>
#include <zeta/core/multi_level_table.ipp>
#include <zeta/core/utils.ipp>

#pragma push_macro("Cntr")

#if DATA
#define Cntr MultiLevelDataTable
#else
#define Cntr MultiLevelPtrTable
#endif

namespace zeta::core {

#if DATA

constexpr size_t Cntr::CalcDataNodeSize_(size_t stride, size_t branch_num) {
    return UIntAlignUp(stride * branch_num + sizeof(unsigned long long),
                       alignof(unsigned long long));
}

void* Cntr::AllocateDatNode_(size_t stride, size_t branch_num,
                             Allocator dat_node_allocator) {
    size_t data_node_size{ CalcDataNodeSize_(stride, branch_num) };

    void* dat_node{ static_cast<char*>(Allocator::SafeAllocate(
                        &dat_node_allocator, alignof(unsigned long long),
                        data_node_size)) +
                    (data_node_size - sizeof(unsigned long long)) };

    *static_cast<unsigned long long*>(dat_node) = 0;

    return dat_node;
}

void Cntr::DeallocateDatNode_(size_t stride, size_t branch_num,
                              Allocator dat_node_allocator, void* node) {
    Allocator::Deallocate(
        &dat_node_allocator,
        static_cast<char*>(node) - (CalcDataNodeSize_(stride, branch_num) -
                                    sizeof(unsigned long long)));
}

#endif

// -----------------------------------------------------------------------------

void Cntr::Init(void* mlt_) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    ZETA_Core_DebugAssert(mlt != nullptr);

    unsigned level{ mlt->level };
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    unsigned short const* branch_nums{ mlt->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        unsigned branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }

#if DATA
    ZETA_Core_DebugAssert(0 < mlt->stride);
#endif

    mlt->size = 0;

    mlt->root = nullptr;

    ZETA_Core_DebugAssert(mlt->nav_node_allocator.vtable != nullptr);
    ZETA_Core_DebugAssert(mlt->nav_node_allocator.align % alignof(NavNode) ==
                          0);

#if DATA
    ZETA_Core_DebugAssert(mlt->dat_node_allocator.vtable != nullptr);
    ZETA_Core_DebugAssert(
        mlt->dat_node_allocator.align % alignof(unsigned long long) == 0);
#endif
}

void Cntr::Deinit(void* mlt) { EraseAll(mlt); }

size_t Cntr::GetSize(void* mlt_) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    return mlt->size;
}

size_t Cntr::GetCapacity(void* mlt_) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    size_t ret{ 1 };

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

void* Cntr::Access(void* mlt_, size_t* idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    CheckIdxes(mlt, idxes);

    unsigned level{ mlt->level };

#if DATA
    size_t stride{ mlt->stride };
#endif

    void* node{ mlt->root };

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

#if DATA
    return static_cast<char*>(node) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, mlt->branch_nums[0]) + stride * last_idx;
#else
    return static_cast<void*>(static_cast<NavNode*>(node)->ptrs + last_idx);
#endif
}

void* Cntr::FindFirst(void* mlt_, size_t* dst_idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    unsigned level{ mlt->level };

    size_t idxes[max_level];

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = 0;
    }

    void* ret{ FindNext(mlt, idxes, true) };

    if (dst_idxes != nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void* Cntr::FindLast(void* mlt_, size_t* dst_idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

    size_t idxes[max_level];

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    void* ret{ FindPrev(mlt, idxes, true) };

    if (dst_idxes != nullptr) {
        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void* Cntr::FindPrev(void* mlt_, size_t* idxes, bool included) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    CheckIdxes(mlt, idxes);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

#if DATA
    size_t stride{ mlt->stride };
#endif

#if DATA
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

    void* root{ mlt->root };

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

#if DATA
        if (level_i == 0) {
            return static_cast<char*>(node) + sizeof(unsigned long long) -
                   data_node_size + stride * cur_idx;
        }
#endif

        void* addr{ static_cast<void*>(static_cast<NavNode*>(node)->ptrs +
                                       cur_idx) };

#if !DATA
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

#if DATA
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

void* Cntr::FindNext(void* mlt_, size_t* idxes, bool included) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    CheckIdxes(mlt, idxes);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

#if DATA
    size_t stride{ mlt->stride };
#endif

#if DATA
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

    void* root{ mlt->root };

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

#if DATA
        if (level_i == 0) {
            return static_cast<char*>(node) + sizeof(unsigned long long) -
                   data_node_size + stride * cur_idx;
        }
#endif

        void* addr{ static_cast<void*>(static_cast<NavNode*>(node)->ptrs +
                                       cur_idx) };

#if !DATA
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

#if DATA
    return static_cast<char*>(nodes[0]) + sizeof(unsigned long long) -
           CalcDataNodeSize_(stride, branch_nums[0]) + stride * idxes[0];
#else
    return static_cast<void*>(static_cast<NavNode*>(nodes[0])->ptrs + idxes[0]);
#endif
}

Pair<void*, bool> Cntr::Insert(void* mlt_, size_t* idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    CheckIdxes(mlt, idxes);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

#if DATA
    size_t stride{ mlt->stride };
#endif

    if (mlt->root == nullptr) {
#if DATA
        if (level == 1) {
            mlt->root = AllocateDatNode_(stride, branch_nums[level - 1],
                                         mlt->dat_node_allocator);
        } else
#endif
        {
            mlt->root = AllocateNavNode_(branch_nums[level - 1],
                                         mlt->nav_node_allocator);
        }
    }

    unsigned level_i{ level - 1 };
    void* node{ mlt->root };

    for (;
         0 < level_i && TestActiveMap_(*static_cast<unsigned long long*>(node),
                                       static_cast<unsigned>(idxes[level_i]));
         --level_i) {
        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        *static_cast<unsigned long long*>(node) += 1ULL << idxes[level_i];

        void* nxt_node{
#if DATA
            level_i == 1 ? AllocateDatNode_(stride, branch_nums[0],
                                            mlt->dat_node_allocator)
                         :
#endif
                         AllocateNavNode_(branch_nums[level_i - 1],
                                          mlt->nav_node_allocator)
        };

        node = static_cast<NavNode*>(node)->ptrs[idxes[level_i]] = nxt_node;
    }

    size_t last_idx{ idxes[0] };

#if DATA
    void* addr{ static_cast<char*>(node) + sizeof(unsigned long long) -
                CalcDataNodeSize_(stride, branch_nums[0]) + stride * last_idx };
#else
    void* addr{ static_cast<void*>(static_cast<NavNode*>(node)->ptrs +
                                   last_idx) };
#endif

    bool inserted{ !TestActiveMap_(*static_cast<unsigned long long*>(node),
                                   static_cast<unsigned>(last_idx)) };

    if (inserted) {
        ++mlt->size;
        *static_cast<unsigned long long*>(node) += (1ULL << last_idx);
    }

    return { .first = addr, .second = inserted };
}

bool Cntr::Erase(void* mlt_, size_t* idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    CheckIdxes(mlt, idxes);

    unsigned level{ mlt->level };

    void* node{ mlt->root };

    if (node == nullptr) { return false; }

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

    --mlt->size;

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        node = nodes[level_i];

        *static_cast<unsigned long long*>(node) -= 1ULL << idxes[level_i];

        if (*static_cast<unsigned long long*>(node) != 0) { return true; }

#if DATA
        if (level_i == 0) {
            DeallocateDatNode_(mlt->stride, mlt->branch_nums[0],
                               mlt->dat_node_allocator, node);
        } else
#endif
        {
            DeallocateNavNode_(mlt->nav_node_allocator, node);
        }
    }

    mlt->root = nullptr;

    return true;
}

void Cntr::EraseAll(void* mlt_) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

#if DATA
    size_t stride{ mlt->stride };
#endif

    void* root{ mlt->root };

    if (root == nullptr) { return; }

    Allocator nav_node_allocator{ mlt->nav_node_allocator };
    Allocator dat_node_allocator{ mlt->dat_node_allocator };

    struct {
        unsigned level_i;
        void* node;
    } buffer[max_level * (max_branch_num - 1) + 1];  // to avoid warning

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = { .level_i = level - 1, .node = root };

    while (0 < buffer_i) {
        auto [level_i, node]{ buffer[--buffer_i] };

        unsigned branch_num{ branch_nums[level] };

        if (level_i == 0) {
#if DATA
            DeallocateDatNode_(stride, branch_num, dat_node_allocator, node);
#else
            DeallocateNavNode_(nav_node_allocator, node);
#endif

            continue;
        }

        for (unsigned idx{ 0 }; idx < branch_num; ++idx) {
            if (TestActiveMap_(*static_cast<unsigned long long*>(node), idx)) {
                buffer[buffer_i++] = {
                    .level_i = level_i - 1,
                    .node = static_cast<NavNode*>(node)->ptrs[idx]
                };
            }
        }

        DeallocateNavNode_(nav_node_allocator, node);
    }

    mlt->root = nullptr;
}

void Cntr::Check(void* mlt_) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    ZETA_Core_DebugAssert(mlt != nullptr);

    unsigned level{ mlt->level };
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    unsigned short const* branch_nums{ mlt->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        unsigned branch_num{ branch_nums[level_i] };

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }
}

void Cntr::CheckIdxes(void* mlt_, size_t const* idxes) {
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    ZETA_Core_DebugAssert(idxes != nullptr);

    unsigned level{ mlt->level };
    unsigned short const* branch_nums{ mlt->branch_nums };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}

#if DATA
size_t Cntr::Sanitize_(MemRecorder* dst_nav_node, MemRecorder* dst_dat_node,
                       unsigned level_i, unsigned short const* branch_nums,
                       size_t stride, void* node)
#else
size_t Cntr::Sanitize_(MemRecorder* dst_nav_node, unsigned level_i,
                       unsigned short const* branch_nums, void* node)
#endif
{
    ZETA_Core_DebugAssert(*static_cast<unsigned long long*>(node) != 0);

#if DATA
    if (level_i == 0) {
        if (dst_dat_node != nullptr) {
            size_t data_node_size{ CalcDataNodeSize_(stride, branch_nums[0]) };

            MemRecorder::Record(
                dst_dat_node,
                static_cast<char*>(node) -
                    (data_node_size - sizeof(unsigned long long)),
                data_node_size);
        }

        return static_cast<size_t>(
            __builtin_popcountll(*static_cast<unsigned long long*>(node)));
    }
#endif

    if (dst_nav_node != nullptr) {
        MemRecorder::Record(dst_nav_node,
                            ZETA_Core_MemberToStruct(NavNode, active_map, node),
                            sizeof(NavNode));
    }

#if !DATA
    if (level_i == 0) {
        return static_cast<size_t>(
            __builtin_popcountll(*static_cast<unsigned long long*>(node)));
    }
#endif

    size_t size{ 0 };

    for (int idx{ -1 };
         (idx = FindNextOne(*static_cast<unsigned long long*>(node), idx)) !=
         -1;) {
        ZETA_Core_DebugAssert(
            TestActiveMap_(*static_cast<unsigned long long*>(node),
                           static_cast<unsigned>(idx)));

#if DATA
        size += Sanitize_(dst_nav_node, dst_dat_node, level_i - 1, branch_nums,
                          stride, static_cast<NavNode*>(node)->ptrs[idx]);
#else
        size += Sanitize_(dst_nav_node, level_i - 1, branch_nums,
                          static_cast<NavNode*>(node)->ptrs[idx]);
#endif
    }

    return size;
}

#if DATA
void Cntr::Sanitize(void* mlt_, MemRecorder* dst_nav_node,
                    MemRecorder* dst_dat_node)
#else
void Cntr::Sanitize(void* mlt_, MemRecorder* dst_nav_node)
#endif
{
    auto* mlt{ static_cast<Cntr*>(mlt_) };
    Check(mlt);

    size_t size{ mlt->root == nullptr
                     ? 0
                     :
#if DATA
                     Sanitize_(dst_nav_node, dst_dat_node, mlt->level - 1,
                               mlt->branch_nums, mlt->stride, mlt->root)
#else
                     Sanitize_(dst_nav_node, mlt->level - 1, mlt->branch_nums,
                               mlt->root)
#endif
    };

    ZETA_Core_DebugAssert(size == mlt->size);
}

}  // namespace zeta::core

#pragma pop_macro("Cntr")
