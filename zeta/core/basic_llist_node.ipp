#pragma once

#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/llist.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList \
    typename LinkType, typename LColorTag, typename RColorTag

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList LinkType, LColorTag, RColorTag

namespace zeta::core {

template <NodeTplParamList>
constexpr void basic_llist_node::Node<NodeTplArgList>::Construct(
    this Node& self) {
    if constexpr (LColorTag::value) {
        self.l.SetPtrColor(alignof(Node), &self, &self, 0);
    } else {
        self.l.SetPtr(alignof(Node), &self, &self);
    }

    if constexpr (RColorTag::value) {
        self.r.SetPtrColor(alignof(Node), &self, &self, 0);
    } else {
        self.r.SetPtr(alignof(Node), &self, &self);
    }
}

template <NodeTplParamList>
constexpr basic_llist_node::Node<NodeTplArgList>*
basic_llist_node::Node<NodeTplArgList>::GetLPtr(this Node& self) {
    return static_cast<Node*>(self.l.GetPtr(alignof(Node), &self));
}

template <NodeTplParamList>
constexpr basic_llist_node::Node<NodeTplArgList>*
basic_llist_node::Node<NodeTplArgList>::GetRPtr(this Node& self) {
    return static_cast<Node*>(self.r.GetPtr(alignof(Node), &self));
}

template <NodeTplParamList>
constexpr basic_llist_node::Node<NodeTplArgList> const*
basic_llist_node::Node<NodeTplArgList>::GetLPtr(this Node const& self) {
    return const_cast<Node&>(self).GetLPtr();
}

template <NodeTplParamList>
constexpr basic_llist_node::Node<NodeTplArgList> const*
basic_llist_node::Node<NodeTplArgList>::GetRPtr(this Node const& self) {
    return const_cast<Node&>(self).GetRPtr();
}

template <NodeTplParamList>
constexpr unsigned basic_llist_node::Node<NodeTplArgList>::GetLColor(
    this Node const& self) {
    return self.l.GetColor(alignof(Node), &self);
}

template <NodeTplParamList>
constexpr unsigned basic_llist_node::Node<NodeTplArgList>::GetRColor(
    this Node const& self) {
    return self.r.GetColor(alignof(Node), &self);
}

template <NodeTplParamList>
constexpr void basic_llist_node::Node<NodeTplArgList>::SetLPtr(this Node& self,
                                                               Node* m) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(m != nullptr);

    self.l.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_llist_node::Node<NodeTplArgList>::SetRPtr(this Node& self,
                                                               Node* m) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(m != nullptr);

    self.r.SetPtr(alignof(Node), &self, m);
}

template <NodeTplParamList>
constexpr void basic_llist_node::Node<NodeTplArgList>::SetLColor(
    this Node& self, unsigned color) {
    self.l.SetColor(alignof(Node), &self, color);
}

template <NodeTplParamList>
constexpr void basic_llist_node::Node<NodeTplArgList>::SetRColor(
    this Node& self, unsigned color) {
    self.r.SetColor(alignof(Node), &self, color);
}

template <NodeTplParamList>
constexpr bool
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList>>::IsConst() {
    return false;
}

template <NodeTplParamList>
constexpr bool
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList> const>::IsConst() {
    return true;
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList>*
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList>>::GetL(
    basic_llist_node::Node<NodeTplArgList>* n) {
    return n->GetLPtr();
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList> const*
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList> const>::GetL(
    basic_llist_node::Node<NodeTplArgList> const* n) {
    return n->GetLPtr();
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList>*
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList>>::GetR(
    basic_llist_node::Node<NodeTplArgList>* n) {
    return n->GetRPtr();
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList> const*
llist::NodeTraits<basic_llist_node::Node<NodeTplArgList> const>::GetR(
    basic_llist_node::Node<NodeTplArgList> const* n) {
    return n->GetRPtr();
}

template <NodeTplParamList>
constexpr void llist::NodeTraits<basic_llist_node::Node<NodeTplArgList>>::SetL(
    basic_llist_node::Node<NodeTplArgList>* n,
    basic_llist_node::Node<NodeTplArgList>* m) {
    n->SetLPtr(m);
}

template <NodeTplParamList>
constexpr void llist::NodeTraits<basic_llist_node::Node<NodeTplArgList>>::SetR(
    basic_llist_node::Node<NodeTplArgList>* n,
    basic_llist_node::Node<NodeTplArgList>* m) {
    n->SetRPtr(m);
}

}  // namespace zeta::core

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
