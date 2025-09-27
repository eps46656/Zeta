#pragma once

#include <zeta/core/multi_level_table.hpp>

namespace zeta::core {

inline constexpr bool MultiLevelTable::TestActiveMap_(
    unsigned long long active_map, unsigned idx) {
    return (active_map & (1ULL << (idx))) != 0;
}

inline void* MultiLevelTable::AllocateNavNode_(size_t branch_num,
                                               Allocator nav_node_allocator) {
    auto nav_node{ static_cast<NavNode*>(
        Allocator::SafeAllocate(&nav_node_allocator, alignof(NavNode),
                                offsetof(NavNode, ptrs[branch_num]))) };

    nav_node->active_map = 0;

    return &nav_node->active_map;
}

inline void MultiLevelTable::DeallocateNavNode_(Allocator nav_node_allocator,
                                                void* node) {
    Allocator::Deallocate(&nav_node_allocator,
                          ZETA_Core_MemberToStruct(NavNode, active_map, node));
}

}  // namespace zeta::core
