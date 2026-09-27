#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList                                                   \
    ptr_utils::IsLinkType LinkType, meta::IsValueWrapperT<bool> PColorTag, \
        meta::IsValueWrapperT<bool> LColorTag,                             \
        meta::IsValueWrapperT<bool> RColorTag, typename AccSizeTag,        \
        meta::IsValueWrapperT<basic_bin_tree_node::PrimaryColorTagEnum>    \
            PrimaryColorTag

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList \
    LinkType, PColorTag, LColorTag, RColorTag, AccSizeTag, PrimaryColorTag

namespace zeta::core {

namespace basic_bin_tree_node::detail {

template <NodeTplParamList>
void InitLinks_(Node<NodeTplArgList>& n) {
    using NodeType = Node<NodeTplArgList>;

    if constexpr (PColorTag::value) {
        n.p.SetPtrColor(alignof(NodeType), &n, &n, 0);
    } else {
        n.p.SetPtr(alignof(NodeType), &n, NodeType::is_rel_link ? &n : nullptr);
    }

    if constexpr (LColorTag::value) {
        n.l.SetPtrColor(alignof(NodeType), &n, &n, 0);
    } else {
        n.l.SetPtr(alignof(NodeType), &n, NodeType::is_rel_link ? &n : nullptr);
    }

    if constexpr (RColorTag::value) {
        n.r.SetPtrColor(alignof(NodeType), &n, &n, 0);
    } else {
        n.r.SetPtr(alignof(NodeType), &n, NodeType::is_rel_link ? &n : nullptr);
    }
}

}  // namespace basic_bin_tree_node::detail

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Construct(
    this Node& self)
    requires(!AccSizeTag::value)
{
    detail::InitLinks_(self);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Construct(
    this Node& self, size_t acc_size)
    requires AccSizeTag::value
{
    detail::InitLinks_(self);
    self.acc_size = acc_size;
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
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.p.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || PColorTag::value) {
        if (m == &self) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.l.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || LColorTag::value) {
        if (m == &self) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.r.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || RColorTag::value) {
        if (m == &self) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetP(this Node& self,
                                                bin_tree::Tag) {
    return self.GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetL(this Node& self,
                                                bin_tree::Tag) {
    return self.GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetR(this Node& self,
                                                bin_tree::Tag) {
    return self.GetRPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr(this Node const& self) {
    return const_cast<Node&>(self).GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr(this Node const& self) {
    return const_cast<Node&>(self).GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr(this Node const& self) {
    return const_cast<Node&>(self).GetRPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetP(this Node const& self,
                                                bin_tree::Tag) {
    return self.GetPPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetL(this Node const& self,
                                                bin_tree::Tag) {
    return self.GetLPtr();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList> const*
basic_bin_tree_node::Node<NodeTplArgList>::GetR(this Node const& self,
                                                bin_tree::Tag) {
    return self.GetRPtr();
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetPColor(
    this Node const& self) {
    return self.p.GetColor(alignof(Node), &self);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetLColor(
    this Node const& self) {
    return self.l.GetColor(alignof(Node), &self);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetRColor(
    this Node const& self) {
    return self.r.GetColor(alignof(Node), &self);
}

template <NodeTplParamList>
constexpr unsigned basic_bin_tree_node::Node<NodeTplArgList>::GetColor(
    this Node const& self, rbtree::Tag)
    requires has_priomary_color
{
    if constexpr (primary_color == PrimaryColorTagEnum::P) {
        return self.GetPColor();
    }

    if constexpr (primary_color == PrimaryColorTagEnum::L) {
        return self.GetLColor();
    }

    if constexpr (primary_color == PrimaryColorTagEnum::R) {
        return self.GetRColor();
    }
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetP(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || PColorTag::value) {
        m = m == nullptr ? &self : m;
    }

    self.p.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetL(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || LColorTag::value) {
        m = m == nullptr ? &self : m;
    }

    self.l.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetR(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || RColorTag::value) {
        m = m == nullptr ? &self : m;
    }

    self.r.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetPColor(
    this Node& self, unsigned color) {
    self.p.SetColor(alignof(Node), &self, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetLColor(
    this Node& self, unsigned color) {
    self.l.SetColor(alignof(Node), &self, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetRColor(
    this Node& self, unsigned color) {
    self.r.SetColor(alignof(Node), &self, color);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetColor(
    this Node& self, rbtree::Tag, unsigned color)
    requires has_priomary_color
{
    if constexpr (primary_color == PrimaryColorTagEnum::P) {
        self.SetPColor(color);
    }

    if constexpr (primary_color == PrimaryColorTagEnum::L) {
        self.SetLColor(color);
    }

    if constexpr (primary_color == PrimaryColorTagEnum::R) {
        self.SetRColor(color);
    }
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetAccSize(
    this Node const& self, bin_tree::Tag)
    requires AccSizeTag::value
{
    return self.acc_size;
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetAccSize(
    this Node& self, bin_tree::Tag, size_t acc_size)
    requires AccSizeTag::value
{
    self.acc_size = acc_size;
}

}  // namespace zeta::core

#pragma pop_macro("NodeTplParamList")
#pragma pop_macro("NodeTplArgList")
