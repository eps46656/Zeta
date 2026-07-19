#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList                                       \
    typename LinkType, typename PColorTag, typename LColorTag, \
        typename RColorTag, typename AccSizeTag, typename PrimaryColorTag

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList \
    LinkType, PColorTag, LColorTag, RColorTag, AccSizeTag, PrimaryColorTag

namespace zeta::core {

namespace basic_bin_tree_node::detail {

template <NodeTplParamList>
void InitLinks_(Node<NodeTplArgList>* n) {
    using NodeType = Node<NodeTplArgList>;

    if constexpr (PColorTag::value) {
        n->p.SetPtrColor(alignof(NodeType), n, n, 0);
    } else {
        n->p.SetPtr(alignof(NodeType), n, NodeType::IsRelLink ? n : nullptr);
    }

    if constexpr (LColorTag::value) {
        n->l.SetPtrColor(alignof(NodeType), n, n, 0);
    } else {
        n->l.SetPtr(alignof(NodeType), n, NodeType::IsRelLink ? n : nullptr);
    }

    if constexpr (RColorTag::value) {
        n->r.SetPtrColor(alignof(NodeType), n, n, 0);
    } else {
        n->r.SetPtr(alignof(NodeType), n, NodeType::IsRelLink ? n : nullptr);
    }
}

}  // namespace basic_bin_tree_node::detail

template <NodeTplParamList>
template <typename, typename>
void basic_bin_tree_node::Node<NodeTplArgList>::Init() {
    detail::InitLinks_(this);
}

template <NodeTplParamList>
template <typename, typename>
void basic_bin_tree_node::Node<NodeTplArgList>::Init(size_t acc_size) {
    detail::InitLinks_(this);
    this->acc_size = acc_size;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr() {
    auto m{ static_cast<Node*>(this->p.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || PColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr() {
    auto m{ static_cast<Node*>(this->l.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || LColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr() {
    auto m{ static_cast<Node*>(this->r.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || RColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr() const {
    return const_cast<Node*>(this)->GetPPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr() const {
    return const_cast<Node*>(this)->GetLPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr() const {
    return const_cast<Node*>(this)->GetRPtr();
}

template <NodeTplParamList>
unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetPColor() const {
    return this->p.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetLColor() const {
    return this->l.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetRColor() const {
    return this->r.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetPPtr(Node* m) {
    if constexpr (IsRelLink || PColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->p.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetLPtr(Node* m) {
    if constexpr (IsRelLink || LColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->l.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetRPtr(Node* m) {
    if constexpr (IsRelLink || RColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->r.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetPColor(unsigned color) {
    this->p.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetLColor(unsigned color) {
    this->l.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetRColor(unsigned color) {
    this->r.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
template <typename, typename>
size_t basic_bin_tree_node::Node<NodeTplArgList>::GetAccSize() const {
    return this->acc_size;
}

template <NodeTplParamList>
template <typename, typename>
void basic_bin_tree_node::Node<NodeTplArgList>::SetAccSize(size_t acc_size) {
    this->acc_size = acc_size;
}

template <NodeTplParamList>
constexpr bool bin_tree::NodeTraits<
    basic_bin_tree_node::Node<NodeTplArgList> const>::IsConst() {
    return true;
}

template <NodeTplParamList>
constexpr bool bin_tree::NodeTraits<
    basic_bin_tree_node::Node<NodeTplArgList> const>::HasAccSize() {
    return AccSizeTag::value;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList> const>::GetP(
    Node const* n) {
    return n->GetPPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList> const>::GetL(
    Node const* n) {
    return n->GetLPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList> const*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList> const>::GetR(
    Node const* n) {
    return n->GetRPtr();
}

template <NodeTplParamList>
template <typename, typename>
constexpr size_t bin_tree::NodeTraits<
    basic_bin_tree_node::Node<NodeTplArgList> const>::GetNullAccSize() {
    return 0;
}

template <NodeTplParamList>
template <typename, typename>
size_t bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList> const>::
    GetAccSize(Node const* n) {
    return n->GetAccSize();
}

template <NodeTplParamList>
constexpr bool
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::IsConst() {
    return false;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::GetP(Node* n) {
    return n->GetPPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::GetL(Node* n) {
    return n->GetLPtr();
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::GetR(Node* n) {
    return n->GetRPtr();
}

template <NodeTplParamList>
void bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::SetP(
    Node* n, Node* m) {
    n->SetPPtr(m);
}

template <NodeTplParamList>
void bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::SetL(
    Node* n, Node* m) {
    n->SetLPtr(m);
}

template <NodeTplParamList>
void bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>>::SetR(
    Node* n, Node* m) {
    n->SetRPtr(m);
}

template <NodeTplParamList>
template <typename, typename>
void bin_tree::NodeTraits<
    basic_bin_tree_node::Node<NodeTplArgList>>::SetAccSize(Node* n,
                                                           size_t acc_size) {
    n->SetAccSize(acc_size);
}

#pragma push_macro("NodeTraitsCond")
#define NodeTraitsCond                                                \
    meta::EnableIf<meta::IsAnySame<                                   \
        PrimaryColorTag, basic_bin_tree_node::PrimaryColorTagEnum::P, \
        basic_bin_tree_node::PrimaryColorTagEnum::L,                  \
        basic_bin_tree_node::PrimaryColorTagEnum::R>>

template <NodeTplParamList>
unsigned rbtree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList> const,
                            NodeTraitsCond>::GetColor(Node const* n) {
    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::P>) {
        return n->GetPColor();
    }

    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::L>) {
        return n->GetLColor();
    }

    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::R>) {
        return n->GetRColor();
    }

    ZETA_Core_Unreachable();
}

template <NodeTplParamList>
void rbtree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList>,
                        NodeTraitsCond>::SetColor(Node* n, unsigned color) {
    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::P>) {
        n->SetPColor(color);
    }

    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::L>) {
        n->SetLColor(color);
    }

    if constexpr (meta::IsSame<PrimaryColorTag,
                               basic_bin_tree_node::PrimaryColorTagEnum::R>) {
        n->SetRColor(color);
    }
}

#pragma pop_macro("NodeTraitsCond")

}  // namespace zeta::core

#pragma pop_macro("NodeTplParamList")
#pragma pop_macro("NodeTplArgList")
