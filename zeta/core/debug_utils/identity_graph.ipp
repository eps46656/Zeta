#pragma once

#include <unordered_map>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/identity_graph.hpp>

namespace zeta::core::debug_utils {

namespace identity_graph::detail {

struct Node_ {
    Id const id;

    std::string name;

    std::unordered_map<std::string, Id> name_to_adj;
};

inline std::unordered_map<Id, Node_, IdHasher> id_to_node_;

inline Id nxt_gen_id_{ 1024 };

constexpr Node_* GenNode_(std::string const& name) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(nxt_gen_id_ < max_id);

    auto p{ id_to_node_.insert({ nxt_gen_id_,
                                 {
                                     .id = nxt_gen_id_,
                                     .name = name,
                                     .name_to_adj = {},
                                 } }) };

    ++nxt_gen_id_;

    return &p.first->second;
}

}  // namespace identity_graph::detail

constexpr identity_graph::Id identity_graph::GetId(std::string const& name) {
    return detail::GenNode_(name)->id;
}

namespace identity_graph::detail {

constexpr Id GetAdj_(Id id, bool create_if_not_exist, std::string const& name) {
    if (id == null_id) { return null_id; }

    Node_* node{ ({
        auto iter{ id_to_node_.find(id) };

        if (iter == id_to_node_.end()) { return null_id; }

        &iter->second;
    }) };

    auto iter{ node->name_to_adj.find(name) };

    if (iter != node->name_to_adj.end()) {
        Id adj_id{ iter->second };

        if (id_to_node_.count(adj_id) == 1) { return adj_id; }

        node->name_to_adj.erase(iter);
    }

    if (!create_if_not_exist) { return null_id; }

    Node_* new_node{ detail::GenNode_(name) };

    node->name_to_adj.insert({ name, new_node->id });

    return new_node->id;
}

}  // namespace identity_graph::detail

template <typename Name0, typename... NameN>
constexpr identity_graph::Id identity_graph::GetAdj(Id id,
                                                    bool create_if_not_exist,
                                                    Name0 const& name0,
                                                    NameN const&... names) {
    id = id == null_id ? id : detail::GetAdj_(id, create_if_not_exist, name0);

    ((id =
          id == null_id ? id : detail::GetAdj_(id, create_if_not_exist, names)),
     ...);

    return id;
}

constexpr identity_graph::Id identity_graph::AddAdj(Id id,
                                                    std::string const& name,
                                                    Id adj_id) {
    if (id == null_id || adj_id == null_id) { return null_id; }

    detail::Node_* node{ ({
        auto iter{ detail::id_to_node_.find(id) };

        if (iter == detail::id_to_node_.end()) { return null_id; }

        &iter->second;
    }) };

    if (detail::id_to_node_.count(adj_id) == 0) { return null_id; }

    auto iter{ node->name_to_adj.insert({ name, adj_id }).first };

    Id act_adj_id{ iter->second };

    if (adj_id != act_adj_id) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            detail::id_to_node_.count(act_adj_id) == 0);

        iter->second = adj_id;
    }

    return adj_id;
}

constexpr identity_graph::Id identity_graph::SetAdj(Id id,
                                                    std::string const& name,
                                                    Id adj_id) {
    if (id == null_id || adj_id == null_id) { return null_id; }

    detail::Node_* node{ ({
        auto iter{ detail::id_to_node_.find(id) };

        if (iter == detail::id_to_node_.end()) { return null_id; }

        &iter->second;
    }) };

    if (detail::id_to_node_.count(adj_id) == 0) { return null_id; }

    node->name_to_adj.insert({ name, 0 }).first->second = adj_id;

    return adj_id;
}

constexpr identity_graph::Id identity_graph::UnsetAdj(Id id,
                                                      std::string const& name) {
    if (id == null_id) { return null_id; }

    detail::Node_* node{ ({
        auto iter{ detail::id_to_node_.find(id) };

        if (iter == detail::id_to_node_.end()) { return null_id; }

        &iter->second;
    }) };

    auto iter{ node->name_to_adj.find(name) };

    if (iter == node->name_to_adj.end()) { return null_id; }

    Id adj_id{ iter->second };

    node->name_to_adj.erase(iter);

    return detail::id_to_node_.count(adj_id) == 0 ? null_id : adj_id;
}

constexpr bool identity_graph::RemoveId(Id id) {
    return id != null_id && detail::id_to_node_.erase(id) == 1;
}

}  // namespace zeta::core::debug_utils
