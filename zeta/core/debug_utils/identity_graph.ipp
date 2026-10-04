#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/identity_graph.hpp>

namespace zeta::core::debug_utils {

namespace identity_graph::detail {

inline Id nxt_gen_id_{ 1024 };

constexpr Node* GenNode_(std::string const& name) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(null_id < nxt_gen_id_);

    Node* node{ new Node{
        .id = nxt_gen_id_,
        .name = name,
        .name_to_adj = {},
    } };

    id_to_node.insert({ nxt_gen_id_, node });

    ++nxt_gen_id_;

    return node;
}

}  // namespace identity_graph::detail

constexpr identity_graph::Node* identity_graph::GetNode(Id id) {
    auto iter{ id_to_node.find(id) };
    return iter == id_to_node.end() ? &null_node : iter->second;
}

constexpr identity_graph::Node* identity_graph::GetAdj(
    Node* node, bool create_if_not_exist, std::string const& name) {
    if (node == nullptr || node == &null_node) { return &null_node; }

    auto iter{ node->name_to_adj.find(name) };

    if (iter != node->name_to_adj.end()) {
        Node* adj_node{ (GetNode)(iter->second) };

        if (adj_node != &null_node) { return adj_node; }

        node->name_to_adj.erase(iter);
    }

    if (!create_if_not_exist) { return &null_node; }

    Node* new_node{ detail::GenNode_(name) };

    node->name_to_adj.insert({ name, new_node->id });

    return new_node;
}

template <typename Name0, typename Name1, typename... NameN>
constexpr identity_graph::Node* identity_graph::GetAdj(Node* node,
                                                       bool create_if_not_exist,
                                                       Name0 const& name0,
                                                       Name1 const& name1,
                                                       NameN const&... names) {
    node = (GetAdj)(node, create_if_not_exist, name0);
    node = (GetAdj)(node, create_if_not_exist, name1);

    ((node = (GetAdj)(node, create_if_not_exist, names)), ...);

    return node;
}

constexpr identity_graph::Node* identity_graph::AddAdj(Node* node,
                                                       std::string const& name,
                                                       Node* adj_node) {
    if (node == nullptr || node == &null_node || adj_node == nullptr ||
        adj_node == &null_node) {
        return &null_node;
    }

    Id act_adj_id{
        node->name_to_adj.insert({ name, adj_node->id }).first->second
    };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(act_adj_id == adj_node->id);

    return adj_node;
}

constexpr identity_graph::Node* identity_graph::SetAdj(Node* node,
                                                       std::string const& name,
                                                       Node* adj_node) {
    if (node == nullptr || node == &null_node || adj_node == nullptr ||
        adj_node == &null_node) {
        return &null_node;
    }

    node->name_to_adj.insert({ name, 0 }).first->second = adj_node->id;

    return adj_node;
}

constexpr identity_graph::Node* identity_graph::UnsetAdj(
    Node* node, std::string const& name) {
    if (node == nullptr || node == &null_node) { return &null_node; }

    auto iter{ node->name_to_adj.find(name) };

    if (iter == node->name_to_adj.end()) { return &null_node; }

    Id adj_id{ iter->second };

    node->name_to_adj.erase(iter);

    return (GetNode)(adj_id);
}

constexpr void identity_graph::RemoveNode(Id id) { id_to_node.erase(id); }

constexpr void identity_graph::RemoveNode(Node* node) {
    if (node == nullptr || node == &null_node) { return; }
    id_to_node.erase(node->id);
}

}  // namespace zeta::core::debug_utils
