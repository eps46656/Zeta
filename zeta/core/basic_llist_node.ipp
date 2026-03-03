#pragma once

#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/ptr_utils.ipp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList \
    typename LinkType, typename LColorTag, typename RColorTag

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList LinkType, LColorTag, RColorTag

namespace zeta::core {

template <NodeTplParamList>
void basic_llist_node::Node<NodeTplArgList>::Init() {
    if constexpr (LColorTag::value) {
        this->l.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->l.SetPtr(alignof(Node), this, this);
    }

    if constexpr (RColorTag::value) {
        this->r.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->r.SetPtr(alignof(Node), this, this);
    }
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList>*
basic_llist_node::Node<NodeTplArgList>::GetLPtr() {
    return static_cast<Node*>(this->l.GetPtr(alignof(Node), this));
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList>*
basic_llist_node::Node<NodeTplArgList>::GetRPtr() {
    return static_cast<Node*>(this->r.GetPtr(alignof(Node), this));
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList> const*
basic_llist_node::Node<NodeTplArgList>::GetLPtr() const {
    return const_cast<Node*>(this)->GetLPtr();
}

template <NodeTplParamList>
basic_llist_node::Node<NodeTplArgList> const*
basic_llist_node::Node<NodeTplArgList>::GetRPtr() const {
    return const_cast<Node*>(this)->GetRPtr();
}

template <NodeTplParamList>
int basic_llist_node::Node<NodeTplArgList>::GetLColor() const {
    return this->l.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
int basic_llist_node::Node<NodeTplArgList>::GetRColor() const {
    return this->r.GetColor(alignof(Node), this);
}

template <NodeTplParamList>
void basic_llist_node::Node<NodeTplArgList>::SetLPtr(Node* m) {
    ZETA_Core_DebugAssert(m != nullptr);

    this->l.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_llist_node::Node<NodeTplArgList>::SetRPtr(Node* m) {
    ZETA_Core_DebugAssert(m != nullptr);

    this->r.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_llist_node::Node<NodeTplArgList>::SetLColor(int color) {
    this->l.SetColor(alignof(Node), this, color);
}

template <NodeTplParamList>
void basic_llist_node::Node<NodeTplArgList>::SetRColor(int color) {
    this->r.SetColor(alignof(Node), this, color);
}

}  // namespace zeta::core

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
