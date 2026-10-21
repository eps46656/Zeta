#pragma once

#include <string>
#include <zeta/core/integral.hpp>

namespace zeta::core::debug_utils::identity_graph {

using Id = uint64_t;

using IdHasher = std::hash<Id>;

constexpr Id null_id{ 0 };

constexpr Id min_id{ 1024 };
constexpr Id max_id{ integral::RangeMaxOf<Id> - 256 };

constexpr Id GetId(std::string const& name);

template <typename Name0, typename... NameN>
constexpr Id GetAdj(Id id, bool create_if_not_exist, Name0 const& name0,
                    NameN const&... nameN);

constexpr Id AddAdj(Id id, std::string const& name, Id adj_id);

constexpr Id SetAdj(Id id, std::string const& name, Id adj_id);

constexpr Id UnsetAdj(Id id, std::string const& name);

constexpr bool RemoveId(Id id);

}  // namespace zeta::core::debug_utils::identity_graph
