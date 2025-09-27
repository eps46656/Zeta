#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>

namespace zeta::core {

struct MultiLevelTable {
    static constexpr unsigned max_level{ 12 };

    static constexpr unsigned min_branch_num{ 2 };
    static constexpr unsigned max_branch_num{ ZETA_Core_ullong_width };

    struct NavNode {
        unsigned long long active_map;
        void* ptrs[];
    };

    ZETA_Core_StaticAssert(offsetof(NavNode, active_map) == 0);

    // -------------------------------------------------------------------------

    static constexpr bool TestActiveMap_(unsigned long long active_map,
                                         unsigned idx);

    static void* AllocateNavNode_(size_t branch_num,
                                  Allocator nav_node_allocator);

    static void DeallocateNavNode_(Allocator nav_node_allocator, void* node);
};

// -----------------------------------------------------------------------------

}  // namespace zeta::core
