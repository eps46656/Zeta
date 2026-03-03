#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList                                      \
    typename LinkType, typename PColorTag, typename EnLColor, \
        typename EnRColor, typename EnAccSize

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList LinkType, PColorTag, EnLColor, EnRColor, EnAccSize

namespace zeta::core {

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::Init() {
    if constexpr (PColorTag::value) {
        this->p.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->p.SetPtr(alignof(Node), this, IsRelLink ? this : nullptr);
    }

    if constexpr (EnLColor::value) {
        this->l.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->l.SetPtr(alignof(Node), this, IsRelLink ? this : nullptr);
    }

    if constexpr (EnRColor::value) {
        this->r.SetPtrColor(alignof(Node), this, this, 0);
    } else {
        this->r.SetPtr(alignof(Node), this, IsRelLink ? this : nullptr);
    }

    if constexpr (EnAccSize::value) { this->acc_size = 0; }
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

    if constexpr (IsRelLink || EnLColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <NodeTplParamList>
basic_bin_tree_node::Node<NodeTplArgList>*
basic_bin_tree_node::Node<NodeTplArgList>::GetRPtr() {
    auto m{ static_cast<Node*>(this->r.GetPtr(alignof(Node), this)) };

    if constexpr (IsRelLink || EnRColor::value) {
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
    if constexpr (IsRelLink || EnLColor::value) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(Node), this, m);
}

template <NodeTplParamList>
void basic_bin_tree_node::Node<NodeTplArgList>::SetRPtr(Node* m) {
    if constexpr (IsRelLink || EnRColor::value) { m = m == nullptr ? this : m; }

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

}  // namespace zeta::core

#pragma pop_macro("NodeTplParamList")
#pragma pop_macro("NodeTplArgList")
