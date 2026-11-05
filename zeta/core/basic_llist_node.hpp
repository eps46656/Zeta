#pragma once

#include <zeta/core/debug_utils/lifecycle_sanity.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/llist.hpp>
#include <zeta/core/ptr_utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList(suffix) \
    typename Link##suffix, bool l_color_en##suffix, bool r_color_en##suffix

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList Link, l_color_en, r_color_en

namespace zeta::core::basic_llist_node {

template <NodeTplParamList(_)>
struct Node {
    using Link = Link_;

    static constexpr bool l_color_en{ l_color_en_ };
    static constexpr bool r_color_en{ r_color_en_ };

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    ptr_utils::AugPtrTpl<Link, l_color_en> l;
    ptr_utils::AugPtrTpl<Link, r_color_en> r;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<Link, l_color_en>::IsRelLink
    };

    constexpr Node();

    constexpr Node* GetLPtr(this Node& self);
    constexpr Node* GetRPtr(this Node& self);

    constexpr Node const* GetLPtr(this Node const& self);
    constexpr Node const* GetRPtr(this Node const& self);

    constexpr unsigned GetLColor(this Node const& self);
    constexpr unsigned GetRColor(this Node const& self);

    constexpr void SetLPtr(this Node& self, Node* m);
    constexpr void SetRPtr(this Node& self, Node* m);

    constexpr void SetLColor(this Node& self, unsigned color);
    constexpr void SetRColor(this Node& self, unsigned color);
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
    static constexpr bool IsConstType();

    static basic_llist_node::Node<NodeTplArgList> const* GetL(
        basic_llist_node::Node<NodeTplArgList> const* n);

    static basic_llist_node::Node<NodeTplArgList> const* GetR(
        basic_llist_node::Node<NodeTplArgList> const* n);
};

template <typename Link, bool l_color_en, bool r_color_en>
struct llist::NodeTraits<basic_llist_node::Node<Link, l_color_en, r_color_en> >
    : public llist::NodeTraits<
          basic_llist_node::Node<Link, l_color_en, r_color_en> const> {
    static constexpr bool IsConstType();

    static basic_llist_node::Node<NodeTplArgList>* GetL(
        basic_llist_node::Node<NodeTplArgList>* n);

    static basic_llist_node::Node<NodeTplArgList>* GetR(
        basic_llist_node::Node<NodeTplArgList>* n);

    static constexpr void SetL(basic_llist_node::Node<NodeTplArgList>* n,
                               basic_llist_node::Node<NodeTplArgList>* m);

    static constexpr void SetR(basic_llist_node::Node<NodeTplArgList>* n,
                               basic_llist_node::Node<NodeTplArgList>* m);
};

}  // namespace zeta::core

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
