#pragma once

#include <array>
#include <set>
#include <unordered_map>
#include <zeta/core/debug_utils/identity_graph.hpp>
#include <zeta/core/type_identity.hpp>

namespace zeta::core::debug_utils::lifecycle_sanity {

struct RegionKey {
    identity_graph::Id const id;

    void const* const beg;
    size_t const size;
};

enum struct RegionLifeState : unsigned char {
    Mem = 0,
    Obj = 1,
};

struct RegionInfo;

struct RegionCompare {
    using is_transparent = void;

    constexpr bool operator()(RegionKey const& lhs, RegionKey const& rhs) const;

    constexpr bool operator()(RegionInfo const* lhs,
                              RegionInfo const* rhs) const;
};

struct RegionInfo {
    static constexpr size_t max_parent_rank{ 4 };

    identity_graph::Id const id;

    void const* const beg;
    size_t const size;

    RegionLifeState const life_state;

    unsigned depth;

    std::array<RegionInfo const*, max_parent_rank> const parents;

    std::set<RegionInfo const*, RegionCompare> children;

    constexpr operator RegionKey(this RegionInfo const& self);
};

static constexpr identity_graph::Id auto_gen_obj_id{ 1 };
static_assert(auto_gen_obj_id < identity_graph::min_id);

enum class RegionRelation {
    Disjoint  = 0,
    Contained = 1,
    Crossed   = 2,
};

struct LifecycleServer {
    std::set<RegionInfo, RegionCompare> region_table_;

    using RegionTableIterator = decltype(region_table_)::iterator;

    std::unordered_map<RegionInfo const*, RegionTableIterator>
        region_ptr_to_region_table_iter_;

    std::unordered_map<identity_graph::Id, RegionTableIterator,
                       identity_graph::IdHasher>
        region_id_to_region_table_iter_;

    constexpr RegionTableIterator FindRegion_(this LifecycleServer const& self,
                                              void const* beg, size_t size);

    constexpr RegionInfo const* FindRegion(this LifecycleServer const& self,
                                           identity_graph::Id region_id);

    constexpr RegionInfo const* FindRegion(this LifecycleServer const& self,
                                           void const* beg, size_t size);

    constexpr std::pair<RegionRelation, RegionInfo const*>
    FindDeepestContainingRegion(this LifecycleServer const& self,
                                void const* beg, size_t size);

    constexpr RegionInfo const* InsertRegion_(this LifecycleServer& self,
                                              void const* beg, size_t size,
                                              RegionLifeState life_state,
                                              RegionInfo const* parent);

    constexpr RegionInfo const* DeclareMem(this LifecycleServer& self,
                                           void const* beg, size_t size);

    constexpr RegionInfo const* ConstructObj(
        this LifecycleServer& self, void const* beg, size_t size,
        std::vector<std::pair<void const*, size_t>> const& sub_mem_regions);

    constexpr void RemoveRegion_(this LifecycleServer& self,
                                 RegionTableIterator iter);

    constexpr void UndeclareMem(this LifecycleServer& self, void const* beg,
                                size_t size);

    constexpr void UndeclareMem(this LifecycleServer& self,
                                identity_graph::Id mem_id);

    constexpr void DestructObj(this LifecycleServer& self,
                               identity_graph::Id obj_id);
};

inline LifecycleServer region_server;

struct ObjHook {
    char content;

    constexpr ObjHook(
        void const* beg, size_t size,
        std::vector<std::pair<void const*, size_t>> const& sub_mem_regions);

    constexpr ~ObjHook();

    constexpr ObjHook& operator=(this ObjHook& self,
                                 ObjHook const& other_obj_hook) = delete;

    constexpr void AssertIsValid(this ObjHook const& self);
};

namespace detail {

constexpr std::pair<void const*, size_t> GetStorageRegion_(void const* beg,
                                                           size_t size);

template <typename T>
constexpr std::pair<void const*, size_t> GetFieldRegion_(meta::TypeWrapper<T>,
                                                         void const* beg,
                                                         size_t size);

}  // namespace detail

#define ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(       \
    member_name)                                                          \
    zeta::core::debug_utils::lifecycle_sanity::detail::GetStorageRegion_( \
        &this->member_name, sizeof(member_name))

#define ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(member_name) \
    zeta::core::debug_utils::lifecycle_sanity::detail::GetFieldRegion_(       \
        zeta::core::meta::TypeWrapper<decltype(this->member_name)>{},         \
        &this->member_name, sizeof(member_name))

}  // namespace zeta::core::debug_utils::lifecycle_sanity
