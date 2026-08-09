#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/llist.hpp>
#include <zeta/core/ptr_utils.hpp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList(suffix)                           \
    typename LinkType##suffix, typename LColorTag##suffix, \
        typename RColorTag##suffix

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList LinkType, LColorTag, RColorTag

namespace zeta::core::basic_llist_node {

template <NodeTplParamList(_)>
struct Node {
    using LinkType = LinkType_;
    using LColorTag = LColorTag_;
    using RColorTag = RColorTag_;

    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, LColorTag>::IsRelLink
    };

    void Init();

    Node* GetLPtr();
    Node* GetRPtr();

    Node const* GetLPtr() const;
    Node const* GetRPtr() const;

    int GetLColor() const;
    int GetRColor() const;

    void SetLPtr(Node* m);
    void SetRPtr(Node* m);

    void SetLColor(int color);
    void SetRColor(int color);
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

}  // namespace zeta::core::basic_llist_node

namespace zeta::core {

template <NodeTplParamList()>
struct llist::NodeTraits<basic_llist_node::Node<NodeTplArgList> const> {
    static constexpr bool IsConst();

    static basic_llist_node::Node<NodeTplArgList> const* GetL(
        basic_llist_node::Node<NodeTplArgList> const* n);

    static basic_llist_node::Node<NodeTplArgList> const* GetR(
        basic_llist_node::Node<NodeTplArgList> const* n);
};

template <typename LinkType, typename LColorTag, typename RColorTag>
struct llist::NodeTraits<
    basic_llist_node::Node<LinkType, LColorTag, RColorTag> >
    : public llist::NodeTraits<
          basic_llist_node::Node<LinkType, LColorTag, RColorTag> const> {
    static constexpr bool IsConst();

    static basic_llist_node::Node<NodeTplArgList>* GetL(
        basic_llist_node::Node<NodeTplArgList>* n);

    static basic_llist_node::Node<NodeTplArgList>* GetR(
        basic_llist_node::Node<NodeTplArgList>* n);

    static void SetL(basic_llist_node::Node<NodeTplArgList>* n,
                     basic_llist_node::Node<NodeTplArgList>* m);

    static void SetR(basic_llist_node::Node<NodeTplArgList>* n,
                     basic_llist_node::Node<NodeTplArgList>* m);
};

}  // namespace zeta::core

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
