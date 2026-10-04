#pragma once

#include <string>
#include <unordered_map>

namespace zeta::core::debug_utils::identity_graph {

using Id = unsigned long long;

struct Node {
    Id const id;

    std::string name;

    std::unordered_map<std::string, Id> name_to_adj;
};

constexpr Id null_id{ 0 };

inline Node null_node{
    .id = null_id,
    .name{ "null" },
    .name_to_adj{},
};

inline std::unordered_map<Id, Node*> id_to_node;

constexpr Node* GetNode(Id id);

constexpr Node* GetAdj(Node* node, bool create_if_not_exist,
                       std::string const& name);

template <typename Name0, typename Name1, typename... NameN>
constexpr Node* GetAdj(Node* node, bool create_if_not_exist, Name0 const& name0,
                       Name1 const& name1, NameN const&... nameN);

constexpr Node* AddAdj(Node* node, std::string const& name, Node* adj_node);

constexpr Node* SetAdj(Node* node, std::string const& name, Node* adj_node);

constexpr Node* UnsetAdj(Node* node, std::string const& name);

constexpr void RemoveNode(Id id);

constexpr void RemoveNode(Node* node);

}  // namespace zeta::core::debug_utils::identity_graph
