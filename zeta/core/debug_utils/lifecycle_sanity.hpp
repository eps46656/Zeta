#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <zeta/core/type_identity.hpp>

namespace zeta::core::debug_utils::lifecycle_sanity {

struct RegionInfo {
    size_t size;
    char const* parent;
    size_t child_cnt;
};

extern std::map<char const*, RegionInfo> obj_tree;

constexpr bool Exists(void const* beg, size_t size);

constexpr bool Contains(void const* beg, size_t size);

struct LifeHook {
    size_t size;

    constexpr LifeHook(size_t size);

    constexpr LifeHook(LifeHook const& other_life_hook);

    constexpr ~LifeHook();

    constexpr LifeHook& operator=(LifeHook const& other_life_hook);
};

}  // namespace zeta::core::debug_utils::lifecycle_sanity
