#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/identity_graph.ipp>
#include <zeta/core/debug_utils/lifecycle_sanity.hpp>

namespace zeta::core::debug_utils {

#pragma push_macro("FindAt_")
#define FindAt_(cntr_tmp, iter_tmp, cntr, find_key)                          \
    ({                                                                       \
        auto& cntr_tmp{ (cntr) };                                            \
        auto iter_tmp{ cntr_tmp.find(find_key) };                            \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(iter_tmp != cntr_tmp.end()); \
        iter_tmp;                                                            \
    })

#pragma push_macro("FindAt")
#define FindAt(cntr, find_key) \
    FindAt_(ZETA_Core_TmpName, ZETA_Core_TmpName, cntr, find_key)

namespace lifecycle_sanity::detail {

template <typename A, typename B>
constexpr bool PtrLess_(A* ptr_a, B* ptr_b) {
    return reinterpret_cast<uintptr_t>(ptr_a) <
           reinterpret_cast<uintptr_t>(ptr_b);
}

template <typename A, typename B>
constexpr bool PtrLessEqual_(A* ptr_a, B* ptr_b) {
    return reinterpret_cast<uintptr_t>(ptr_a) <=
           reinterpret_cast<uintptr_t>(ptr_b);
}

}  // namespace lifecycle_sanity::detail

constexpr bool lifecycle_sanity::RegionCompare::operator()(
    RegionKey const& lhs, RegionKey const& rhs) const {
    if (detail::PtrLess_(lhs.beg, rhs.beg)) { return true; }
    if (detail::PtrLess_(rhs.beg, lhs.beg)) { return false; }

    if (rhs.size < lhs.size) { return true; }
    if (lhs.size < rhs.size) { return false; }

    return lhs.id < rhs.id;
}

constexpr bool lifecycle_sanity::RegionCompare::operator()(
    RegionInfo const* lhs, RegionInfo const* rhs) const {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(lhs != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rhs != nullptr);

    return operator()(*lhs, *rhs);
}

constexpr lifecycle_sanity::RegionInfo::operator RegionKey(
    this RegionInfo const& self) {
    return {
        .id{ self.id },
        .beg{ self.beg },
        .size{ self.size },
    };
}

constexpr lifecycle_sanity::LifecycleServer::RegionTableIterator
lifecycle_sanity::LifecycleServer::FindRegion_(this LifecycleServer const& self,
                                               void const* beg_, size_t size) {
    char const* beg{ static_cast<char const*>(beg_) };

    if (beg == nullptr && size == 0) { return self.region_table_.end(); }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(beg != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ self.region_table_.upper_bound(RegionKey{
        .id{ identity_graph::max_id },
        .beg{ beg },
        .size{ size },
    }) };

    if (iter == self.region_table_.begin()) { return self.region_table_.end(); }

    --iter;

    if (iter->beg == beg && iter->size == size) { return iter; }

    return self.region_table_.end();
}

constexpr lifecycle_sanity::RegionInfo const*
lifecycle_sanity::LifecycleServer::FindRegion(this LifecycleServer const& self,
                                              identity_graph::Id region_id) {
    auto jter{ self.region_id_to_region_table_iter_.find(region_id) };

    if (jter == self.region_id_to_region_table_iter_.end()) { return nullptr; }

    return &*jter->second;
}

constexpr lifecycle_sanity::RegionInfo const*
lifecycle_sanity::LifecycleServer::FindRegion(this LifecycleServer const& self,
                                              void const* beg, size_t size) {
    auto iter{ self.FindRegion_(beg, size) };
    return iter == self.region_table_.end() ? nullptr : &*iter;
}

constexpr std::pair<lifecycle_sanity::RegionRelation,
                    lifecycle_sanity::RegionInfo const*>
lifecycle_sanity::LifecycleServer::FindDeepestContainingRegion(
    this LifecycleServer const& self, void const* beg_, size_t size) {
    char const* beg{ static_cast<char const*>(beg_) };
    char const* end{ beg + size };

    auto iter{ self.region_table_.upper_bound(RegionKey{
        .id{ 0 },
        .beg{ beg },
        .size{ 0 },
    }) };

    if (iter != self.region_table_.end() && detail::PtrLess_(iter->beg, end)) {
        return { RegionRelation::Crossed, nullptr };
    }

    if (iter == self.region_table_.begin()) {
        return { RegionRelation::Disjoint, nullptr };
    }

    auto check_relation{ [beg, end](RegionInfo const* prv_region) {
        char const* prv_region_beg{ static_cast<char const*>(prv_region->beg) };
        char const* prv_region_end{ prv_region_beg + prv_region->size };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            detail::PtrLessEqual_(prv_region_beg, beg));

        if (detail::PtrLessEqual_(prv_region_end, beg)) {
            return RegionRelation::Disjoint;
        }

        if (detail::PtrLessEqual_(end, prv_region_end)) {
            return RegionRelation::Contained;
        }

        return RegionRelation::Crossed;
    } };

    RegionInfo const* cur_region{ &*(--iter) };

    switch (check_relation(cur_region)) {
    case RegionRelation::Disjoint: break;

    case RegionRelation::Contained:
        return { RegionRelation::Contained, cur_region };

    case RegionRelation::Crossed: return { RegionRelation::Crossed, nullptr };

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    for (size_t parent_rank{ RegionInfo::max_parent_rank - 1 };;) {
        RegionInfo const* parent_region{ cur_region->parents[parent_rank] };

        if (parent_region == nullptr) {
            if (parent_rank == 0) {
                return { RegionRelation::Disjoint, nullptr };
            }

            --parent_rank;
            continue;
        }

        switch (check_relation(parent_region)) {
        case RegionRelation::Disjoint:
            if (0 < parent_rank &&
                parent_rank < RegionInfo::max_parent_rank - 1) {
                --parent_rank;
            }

            cur_region = parent_region;

            break;

        case RegionRelation::Contained:
            if (parent_rank == 0) {
                return { RegionRelation::Contained, parent_region };
            }

            --parent_rank;

            break;

        case RegionRelation::Crossed:
            return { RegionRelation::Crossed, nullptr };

        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr lifecycle_sanity::RegionInfo const*
lifecycle_sanity::LifecycleServer::InsertRegion_(this LifecycleServer& self,
                                                 void const* beg, size_t size,
                                                 RegionLifeState life_state,
                                                 RegionInfo const* parent) {
    std::array<RegionInfo const*, RegionInfo::max_parent_rank> parents;

    parents[0] = parent;

    for (size_t parent_rank{ 1 }; parent_rank < RegionInfo::max_parent_rank;
         ++parent_rank) {
        parents[parent_rank] =
            parents[parent_rank - 1] == nullptr
                ? nullptr
                : parents[parent_rank - 1]->parents[parent_rank - 1];
    }

    auto p{ self.region_table_.insert(RegionInfo{
        .id{ identity_graph::GetId("") },
        .beg{ beg },
        .size{ size },
        .life_state{ life_state },
        .depth{ parent == nullptr ? 0 : parent->depth + 1 },
        .parents{ parents },
        .children{},
    }) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(p.second);

    RegionTableIterator iter{ p.first };
    RegionInfo* region{ const_cast<RegionInfo*>(&*iter) };

    {
        bool b{ self.region_ptr_to_region_table_iter_.insert({ region, iter })
                    .second };
        ZETA_Core_DebugUtils_Diag_PromiseAssert(b);
    }

    {
        bool b{ self.region_id_to_region_table_iter_
                    .insert({ region->id, iter })
                    .second };
        ZETA_Core_DebugUtils_Diag_PromiseAssert(b);
    }

    if (parent != nullptr) {
        bool b{
            const_cast<RegionInfo*>(parent)->children.insert(region).second
        };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(b);
    }

    return region;
}

constexpr lifecycle_sanity::RegionInfo const*
lifecycle_sanity::LifecycleServer::DeclareMem(this LifecycleServer& self,
                                              void const* beg_, size_t size) {
    char const* beg{ static_cast<char const*>(beg_) };

    if (beg == nullptr && size == 0) { return nullptr; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(beg != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto [relation, parent]{ self.FindDeepestContainingRegion(beg, size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        relation == RegionRelation::Disjoint ||
        relation == RegionRelation::Contained);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        parent == nullptr || parent->life_state == RegionLifeState::Mem);

    return self.InsertRegion_(beg, size, RegionLifeState::Mem, parent);
}

constexpr lifecycle_sanity::RegionInfo const*
lifecycle_sanity::LifecycleServer::ConstructObj(
    this LifecycleServer& self, void const* beg_, size_t size,
    std::vector<std::pair<void const*, size_t>> const& sub_mem_regions) {
    char const* beg{ static_cast<char const*>(beg_) };

    auto [relation, parent]{ self.FindDeepestContainingRegion(beg, size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        relation == RegionRelation::Disjoint ||
        relation == RegionRelation::Contained);

    char const* end{ beg + size };

    {
        char const* last_end{ beg };

        for (auto [sub_mem_region_beg, sub_mem_region_size] : sub_mem_regions) {
            if (sub_mem_region_beg == nullptr && sub_mem_region_size == 0) {
                continue;
            }

            ZETA_Core_DebugUtils_Diag_PromiseAssert(sub_mem_region_beg !=
                                                    nullptr);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < sub_mem_region_size);

            char const* sub_mem_region_end{ static_cast<char const*>(
                                                sub_mem_region_beg) +
                                            sub_mem_region_size };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                detail::PtrLessEqual_(last_end, sub_mem_region_beg));
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                detail::PtrLessEqual_(sub_mem_region_end, end));

            last_end = sub_mem_region_end;
        }
    }

    RegionInfo const* obj_region{ self.InsertRegion_(
        beg, size, RegionLifeState::Obj, parent) };

    for (auto [sub_mem_region_beg, sub_mem_region_size] : sub_mem_regions) {
        if (sub_mem_region_beg != nullptr) {
            self.InsertRegion_(sub_mem_region_beg, sub_mem_region_size,
                               RegionLifeState::Mem, obj_region);
        }
    }

    return obj_region;
}

constexpr void lifecycle_sanity::LifecycleServer::RemoveRegion_(
    this LifecycleServer& self, RegionTableIterator iter) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.region_table_.end());

    RegionInfo const* region{ &*iter };

    switch (region->life_state) {
    case RegionLifeState::Mem:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            region->parents[0] == nullptr ||
            region->parents[0]->life_state == RegionLifeState::Mem);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(region->children.empty());

        break;
    case RegionLifeState::Obj:
        for (RegionInfo const* region : region->children) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(region->children.empty());
        }

        break;

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    auto erase_region{ [&self](RegionTableIterator iter) {
        RegionInfo const* region{ &*iter };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(region->children.empty());

        RegionInfo const* parent{ region->parents[0] };

        if (parent != nullptr) {
            size_t cnt{ const_cast<RegionInfo*>(parent)->children.erase(
                region) };
            ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == 1);
        }

        {
            size_t cnt{ self.region_id_to_region_table_iter_.erase(
                region->id) };
            ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == 1);
        }

        {
            size_t cnt{ self.region_ptr_to_region_table_iter_.erase(region) };
            ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == 1);
        }

        identity_graph::RemoveId(region->id);

        self.region_table_.erase(iter);
    } };

    if (region->life_state == RegionLifeState::Obj) {
        while (!region->children.empty()) {
            erase_region(FindAt(self.region_ptr_to_region_table_iter_,
                                *region->children.begin())
                             ->second);
        }
    }

    erase_region(iter);
}

constexpr void lifecycle_sanity::LifecycleServer::UndeclareMem(
    this LifecycleServer& self, void const* beg, size_t size) {
    auto iter{ self.FindRegion_(beg, size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.region_table_.end());

    RegionInfo const* region{ &*iter };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(region->life_state ==
                                            RegionLifeState::Mem);

    self.RemoveRegion_(iter);
}

constexpr void lifecycle_sanity::LifecycleServer::UndeclareMem(
    this LifecycleServer& self, identity_graph::Id region_id) {
    auto iter{
        FindAt(self.region_id_to_region_table_iter_, region_id)->second
    };

    RegionInfo const* region{ &*iter };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(region->life_state ==
                                            RegionLifeState::Mem);

    self.RemoveRegion_(iter);
}

constexpr void lifecycle_sanity::LifecycleServer::DestructObj(
    this LifecycleServer& self, identity_graph::Id region_id) {
    auto iter{
        FindAt(self.region_id_to_region_table_iter_, region_id)->second
    };

    RegionInfo const* region{ &*iter };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(region->life_state ==
                                            RegionLifeState::Obj);

    self.RemoveRegion_(iter);
}

namespace lifecycle_sanity::detail {

struct ObjHookInfo_ {
    RegionInfo const* region;
};

inline std::unordered_map<ObjHook const*, ObjHookInfo_> obj_hook_table_;

}  // namespace lifecycle_sanity::detail

constexpr lifecycle_sanity::ObjHook::ObjHook(
    void const* beg, size_t size,
    std::vector<std::pair<void const*, size_t>> const& sub_mem_regions) {
    this->AssertIsNotConstructed();

    RegionInfo const* region{ region_server.ConstructObj(beg, size,
                                                         sub_mem_regions) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(region != nullptr);

    {
        bool b{ detail::obj_hook_table_
                    .insert({
                        this,
                        {
                            .region{ region },
                        },
                    })
                    .second };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(b);
    }
}

constexpr lifecycle_sanity::ObjHook::~ObjHook() {
    this->AssertIsConstructed();

    auto iter{ FindAt(detail::obj_hook_table_, this) };

    detail::ObjHookInfo_ const& obj_hook_info{ iter->second };

    RegionInfo const* region{ obj_hook_info.region };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(region != nullptr);

    region_server.DestructObj(region->id);

    detail::obj_hook_table_.erase(iter);
}

constexpr void lifecycle_sanity::ObjHook::AssertIsConstructed(
    this ObjHook const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        detail::obj_hook_table_.count(&self) == 1);
}

constexpr void lifecycle_sanity::ObjHook::AssertIsNotConstructed(
    this ObjHook const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        detail::obj_hook_table_.count(&self) == 0);
}

namespace lifecycle_sanity::detail {

constexpr std::pair<void const*, size_t> GetStorageRegion_(void const* beg,
                                                           size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(beg != nullptr);

    if (size == 0) { return { nullptr, 0 }; }

    return { beg, size };
}

template <typename T>
constexpr std::pair<void const*, size_t> GetFieldRegion_(meta::TypeWrapper<T>,
                                                         void const* beg,
                                                         size_t size) {
    if (size == 0) { return { nullptr, 0 }; }

    if constexpr (meta::IsReferenceType<T> ||
                  meta::IsTriviallyConstructible<T>) {
        return { nullptr, 0 };
    } else {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(beg != nullptr);

        return { beg, size };
    }
}

}  // namespace lifecycle_sanity::detail

#pragma pop_macro("FindAt_")
#pragma pop_macro("FindAt")

}  // namespace zeta::core::debug_utils
