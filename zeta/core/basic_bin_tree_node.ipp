#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList                                                       \
    ptr_utils::IsLink Link, bool p_color_en, bool l_color_en, bool r_color_en, \
        bool acc_size_en, basic_bin_tree_node::PrimaryColor primary_color

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList \
    Link, p_color_en, l_color_en, r_color_en, acc_size_en, primary_color

namespace zeta::core {

template <NodeTplParamList>
    requires(!acc_size_en)
constexpr basic_bin_tree_node::NodeBase<NodeTplArgList>::NodeBase()
#if ZETA_Core_DebugEnable
    : obj_hook{
          this,
          sizeof(*this),
          {
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(p),
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(l),
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(r),
          },
      }
#endif
{
}

template <NodeTplParamList>
    requires(acc_size_en)
constexpr basic_bin_tree_node::NodeBase<NodeTplArgList>::NodeBase()
#if ZETA_Core_DebugEnable
    : obj_hook{
          this,
          sizeof(*this),
          {
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(p),
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(l),
              ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(r),
          },
      }
#endif
{
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>::Node()
    requires(!acc_size_en)
{
    this->Reset();
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>::Node(size_t acc_size)
    requires acc_size_en
{
    this->Reset(acc_size);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Reset()
    requires(!acc_size_en)
{
    if constexpr (p_color_en) {
        this->p.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->p.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }

    if constexpr (l_color_en) {
        this->l.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->l.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }

    if constexpr (r_color_en) {
        this->r.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->r.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::Reset(size_t acc_size)
    requires acc_size_en
{
    if constexpr (p_color_en) {
        this->p.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->p.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }

    if constexpr (l_color_en) {
        this->l.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->l.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }

    if constexpr (r_color_en) {
        this->r.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->r.SetPtr(alignof(Node), this, is_rel_link ? this : nullptr);
    }

    this->acc_size = acc_size;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::IsConstType(
    bin_tree::Tag, meta::TypeWrapper<Node>) {
    return false;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::IsConstType(
    bin_tree::Tag, meta::TypeWrapper<Node const>) {
    return true;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::HasAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node>) {
    return acc_size_en;
}

template <NodeTplParamList>
constexpr bool basic_bin_tree_node::Node<NodeTplArgList>::HasAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node const>) {
    return acc_size_en;
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetNullAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node>)
    requires acc_size_en
{
    return 0;
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetNullAccSize(
    bin_tree::Tag, meta::TypeWrapper<Node const>)
    requires acc_size_en
{
    return 0;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetPPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.p.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || p_color_en) {
        if (m == &self) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetLPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.l.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || l_color_en) {
        if (m == &self) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
constexpr basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr(this Node& self) {
    auto m{ static_cast<Node*>(self.r.GetPtr(alignof(Node), &self)) };

    if constexpr (is_rel_link || r_color_en) {
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
    if constexpr (primary_color == PrimaryColor::P) { return self.GetPColor(); }
    if constexpr (primary_color == PrimaryColor::L) { return self.GetLColor(); }
    if constexpr (primary_color == PrimaryColor::R) { return self.GetRColor(); }
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetP(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || p_color_en) { m = m == nullptr ? &self : m; }

    self.p.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetL(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || l_color_en) { m = m == nullptr ? &self : m; }

    self.l.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetR(this Node& self,
                                                               bin_tree::Tag,
                                                               Node* m) {
    if constexpr (is_rel_link || r_color_en) { m = m == nullptr ? &self : m; }

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
    if constexpr (primary_color == PrimaryColor::P) { self.SetPColor(color); }

    if constexpr (primary_color == PrimaryColor::L) { self.SetLColor(color); }

    if constexpr (primary_color == PrimaryColor::R) { self.SetRColor(color); }
}

template <NodeTplParamList>
constexpr size_t basic_bin_tree_node::Node<NodeTplArgList>::GetAccSize(
    this Node const& self, bin_tree::Tag)
    requires acc_size_en
{
    return self.acc_size;
}

template <NodeTplParamList>
constexpr void basic_bin_tree_node::Node<NodeTplArgList>::SetAccSize(
    this Node& self, bin_tree::Tag, size_t acc_size)
    requires acc_size_en
{
    self.acc_size = acc_size;
}

}  // namespace zeta::core

#pragma pop_macro("NodeTplParamList")
#pragma pop_macro("NodeTplArgList")
