#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList                                                    \
    ptr_utils::IsLinkType LinkType, typename PColorTag, typename LColorTag, \
        typename RColorTag, typename AccSizeTag,                            \
        basic_bin_tree_node::PrimaryColorTagEnum PrimaryColorTag

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
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Init()
    requires(!AccSizeTag::value)
{
    detail::InitLinks_(this);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Init(size_t acc_size)
    requires AccSizeTag::value
{
    detail::InitLinks_(this);
    this->acc_size = acc_size;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::IsConst(
    bin_tree::Tag, meta::TypeWrapper<Node>) {
    return false;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::IsConst(
    bin_tree::Tag, meta::TypeWrapper<Node const>) {
    return true;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::HasAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node>) {
    return AccSizeTag::value;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::HasAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node const>) {
    return AccSizeTag::value;
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetNullAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node>)
    requires AccSizeTag::value
{
    return 0;
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetNullAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node const>)
    requires AccSizeTag::value
{
    return 0;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr() {
    auto m{ static_cast<Node*>(this->p.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || PColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr() {
    auto m{ static_cast<Node*>(this->l.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || LColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr() {
    auto m{ static_cast<Node*>(this->r.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || RColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetP(bin_tree::Tag) {
    return this->GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetL(bin_tree::Tag) {
    return this->GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetR(bin_tree::Tag) {
    return this->GetRPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr() const {
    return const_cast<Node*>(this)->GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr() const {
    return const_cast<Node*>(this)->GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr() const {
    return const_cast<Node*>(this)->GetRPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetP(bin_tree::Tag) const {
    return this->GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetL(bin_tree::Tag) const {
    return this->GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetR(bin_tree::Tag) const {
    return this->GetRPtr();
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetPColor()
    const {
    return this->p.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetLColor()
    const {
    return this->l.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetRColor()
    const {
    return this->r.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetColor(
    rbtree::Tag) const {
    ZETA_Core_StaticAssert(PrimaryColorTag == PrimaryColorTagEnum::P ||
                           PrimaryColorTag == PrimaryColorTagEnum::L ||
                           PrimaryColorTag == PrimaryColorTagEnum::R);

    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::P) {
        return this->GetPColor();
    }

    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::L) {
        return this->GetLColor();
    }

    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::R) {
        return this->GetRColor();
    }
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetP(bin_tree::Tag,
                                                               Node* m) {
    if constexpr (IsRelLink || PColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->p.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetL(bin_tree::Tag,
                                                               Node* m) {
    if constexpr (IsRelLink || LColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->l.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetR(bin_tree::Tag,
                                                               Node* m) {
    if constexpr (IsRelLink || RColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->r.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetPColor(
    unsigned color) {
    this->p.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetLColor(
    unsigned color) {
    this->l.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetRColor(
    unsigned color) {
    this->r.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetColor(
    rbtree::Tag, unsigned color)
    requires(PrimaryColorTag == PrimaryColorTagEnum::P ||
             PrimaryColorTag == PrimaryColorTagEnum::L ||
             PrimaryColorTag == PrimaryColorTagEnum::R)
{
    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::P) {
        this->SetPColor(color);
    }

    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::L) {
        this->SetLColor(color);
    }

    if constexpr (PrimaryColorTag == PrimaryColorTagEnum::R) {
        this->SetRColor(color);
    }
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetAccSize(
    bin_tree::Tag) const
    requires AccSizeTag::value
{
    return this->acc_size;
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetAccSize(
    bin_tree::Tag, size_t acc_size)
    requires AccSizeTag::value
{
    this->acc_size = acc_size;
}

}  // namespace zeta::core

#pragma pop_macro("NodeTplParamList")
#pragma pop_macro("NodeTplArgList")
