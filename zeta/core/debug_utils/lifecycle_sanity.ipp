#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/lifecycle_sanity.hpp>

namespace zeta::core::debug_utils {

namespace lifecycle_sanity::detail {

inline constexpr char null_region[1]{ 0 };

enum class CheckRegionResult {
    Disjoint = 0,
    Contained = 1,
    Crossed = 2,
};

constexpr std::pair<CheckRegionResult, char const*> CheckRegion_(
    char const* beg, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    char const* end{ beg + size };

    auto iter{ obj_tree.upper_bound(beg) };

    if (iter != obj_tree.end()) {
        if (iter->first < end) {
            return { CheckRegionResult::Crossed, null_region };
        }
    }

    if (iter == obj_tree.begin()) {
        return { CheckRegionResult::Disjoint, null_region };
    }

    for (--iter;;) {
        char const* prv_region_beg{ iter->first };

        char const* prv_region_end{ prv_region_beg + iter->second.size };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(prv_region_beg != beg);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(prv_region_beg < beg);

        if (end <= prv_region_end) {
            return { CheckRegionResult::Contained, prv_region_beg };
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(prv_region_end <= beg);

        char const* prv_region_parent{ iter->second.parent };

        if (prv_region_parent == null_region) {
            return { CheckRegionResult::Disjoint, null_region };
        }

        iter = obj_tree.find(prv_region_parent);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != obj_tree.end());
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr void AddRegion_(char const* beg, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto [check_result, parent_region]{ CheckRegion_(beg, size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        check_result == CheckRegionResult::Disjoint ||
        check_result == CheckRegionResult::Contained);

    auto p{ obj_tree.insert({
        beg,
        {
            .size = size,
            .parent = parent_region,
            .child_cnt = 0,
        },
    }) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(p.second);

    if (parent_region != null_region) {
        auto parent_iter{ obj_tree.find(parent_region) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(parent_iter != obj_tree.end());

        ++parent_iter->second.child_cnt;
    }
}

constexpr void RemoveRegion_(char const* beg, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!obj_tree.empty());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ obj_tree.find(beg) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != obj_tree.end());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second.size == size);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter->second.child_cnt == 0);

    if (iter->second.parent != null_region) {
        auto parent_iter{ obj_tree.find(iter->second.parent) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(parent_iter != obj_tree.end());

        RegionInfo& parent_info{ parent_iter->second };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < parent_info.child_cnt);

        --parent_info.child_cnt;
    }

    obj_tree.erase(iter);
}

}  // namespace lifecycle_sanity::detail

inline std::map<char const*, lifecycle_sanity::RegionInfo>
    lifecycle_sanity::obj_tree;

constexpr bool lifecycle_sanity::Exists(void const* beg, size_t size) {
    if (beg == nullptr || size == 0) { return false; }

    auto iter{ obj_tree.find(static_cast<char const*>(beg)) };

    return iter != obj_tree.end() && iter->second.size == size;
}

constexpr bool lifecycle_sanity::Contains(void const* beg, size_t size) {
    if (beg == nullptr || size == 0) { return false; }

    auto [check_result, parent_region]{ detail::CheckRegion_(
        static_cast<char const*>(beg), size) };

    return check_result != detail::CheckRegionResult::Disjoint;
}

namespace lifecycle_sanity::detail {

constexpr void CheckLifeHook_(LifeHook const& life_hook) {
    (Exists)(reinterpret_cast<char const*>(&life_hook), life_hook.size);
}

}  // namespace lifecycle_sanity::detail

constexpr lifecycle_sanity::LifeHook::LifeHook(size_t size) : size{ size } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < this->size);

    detail::AddRegion_(reinterpret_cast<char const*>(this), this->size);
}

constexpr lifecycle_sanity::LifeHook::LifeHook(
    lifecycle_sanity::LifeHook const& other_life_hook) {
    detail::CheckLifeHook_(other_life_hook);

    this->size = other_life_hook.size;

    detail::AddRegion_(reinterpret_cast<char const*>(this), this->size);
}

constexpr lifecycle_sanity::LifeHook::~LifeHook() {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < this->size);

    detail::RemoveRegion_(reinterpret_cast<char const*>(this), this->size);
}

constexpr lifecycle_sanity::LifeHook& lifecycle_sanity::LifeHook::operator=(
    lifecycle_sanity::LifeHook const&) {
    detail::CheckLifeHook_(*this);

    return *this;
}

}  // namespace zeta::core::debug_utils
