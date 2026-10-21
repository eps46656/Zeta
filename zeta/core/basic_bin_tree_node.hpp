#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/rbtree.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList(suffix)                             \
    ptr_utils::IsLink Link##suffix, bool p_color_en##suffix, \
        bool l_color_en##suffix, bool r_color_en##suffix,    \
        bool acc_size_en##suffix, PrimaryColor primary_color##suffix

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList(suffix)                                                \
    Link##suffix, p_color_en##suffix, l_color_en##suffix, r_color_en##suffix, \
        acc_size_en##suffix, primary_color##suffix

namespace zeta::core::basic_bin_tree_node {

enum struct PrimaryColor : unsigned char {
    Null = 0b000,
    P    = 0b001,
    L    = 0b010,
    R    = 0b100,
};

template <NodeTplParamList(_)>
struct NodeBase;

template <NodeTplParamList()>
    requires(!acc_size_en)
struct NodeBase<NodeTplArgList()> {
#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    ptr_utils::AugPtrTpl<Link, p_color_en> p;
    ptr_utils::AugPtrTpl<Link, l_color_en> l;
    ptr_utils::AugPtrTpl<Link, r_color_en> r;

    constexpr NodeBase();
};

template <NodeTplParamList()>
    requires(acc_size_en)
struct NodeBase<NodeTplArgList()> {
#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    ptr_utils::AugPtrTpl<Link, p_color_en> p;
    ptr_utils::AugPtrTpl<Link, l_color_en> l;
    ptr_utils::AugPtrTpl<Link, r_color_en> r;
    size_t acc_size;

    constexpr NodeBase();
};

template <NodeTplParamList(_)>
struct Node : public NodeBase<NodeTplArgList(_)> {
    using Link = Link_;

    static constexpr bool p_color_en{ p_color_en_ };
    static constexpr bool l_color_en{ l_color_en_ };
    static constexpr bool r_color_en{ r_color_en_ };
    static constexpr bool acc_size_en{ acc_size_en_ };

    static_assert(primary_color_ == PrimaryColor::Null ||
                  primary_color_ == PrimaryColor::P ||
                  primary_color_ == PrimaryColor::L ||
                  primary_color_ == PrimaryColor::R);

    static constexpr PrimaryColor primary_color{ primary_color };

    static constexpr bool has_priomary_color{
        primary_color == PrimaryColor::P || primary_color == PrimaryColor::L ||
        primary_color == PrimaryColor::R
    };

    static constexpr bool is_rel_link{
        ptr_utils::AugPtrTpl<Link, p_color_en>::is_rel_link
    };

    constexpr Node()
        requires(!acc_size_en);

    constexpr Node(size_t acc_size)
        requires acc_size_en;

    static constexpr bool IsConst(bin_tree::Tag, meta::TypeWrapper<Node>);

    static constexpr bool IsConst(bin_tree::Tag, meta::TypeWrapper<Node const>);

    static constexpr bool HasAccSize(bin_tree::Tag, meta::TypeWrapper<Node>);

    static constexpr bool HasAccSize(bin_tree::Tag,
                                     meta::TypeWrapper<Node const>);

    static constexpr size_t GetNullAccSize(bin_tree::Tag,
                                           meta::TypeWrapper<Node>)
        requires acc_size_en;

    static constexpr size_t GetNullAccSize(bin_tree::Tag,
                                           meta::TypeWrapper<Node const>)
        requires acc_size_en;

    constexpr Node* GetPPtr(this Node& self);
    constexpr Node* GetLPtr(this Node& self);
    constexpr Node* GetRPtr(this Node& self);

    constexpr Node* GetP(this Node& self, bin_tree::Tag);
    constexpr Node* GetL(this Node& self, bin_tree::Tag);
    constexpr Node* GetR(this Node& self, bin_tree::Tag);

    constexpr Node const* GetPPtr(this Node const& self);
    constexpr Node const* GetLPtr(this Node const& self);
    constexpr Node const* GetRPtr(this Node const& self);

    constexpr Node const* GetP(this Node const& self, bin_tree::Tag);
    constexpr Node const* GetL(this Node const& self, bin_tree::Tag);
    constexpr Node const* GetR(this Node const& self, bin_tree::Tag);

    constexpr unsigned GetPColor(this Node const& self);
    constexpr unsigned GetLColor(this Node const& self);
    constexpr unsigned GetRColor(this Node const& self);

    constexpr unsigned GetColor(this Node const& self, rbtree::Tag)
        requires has_priomary_color;

    constexpr void SetPPtr(this Node& self, Node* m);
    constexpr void SetLPtr(this Node& self, Node* m);
    constexpr void SetRPtr(this Node& self, Node* m);

    constexpr void SetP(this Node& self, bin_tree::Tag, Node* m);
    constexpr void SetL(this Node& self, bin_tree::Tag, Node* m);
    constexpr void SetR(this Node& self, bin_tree::Tag, Node* m);

    constexpr void SetPColor(this Node& self, unsigned color);
    constexpr void SetLColor(this Node& self, unsigned color);
    constexpr void SetRColor(this Node& self, unsigned color);

    constexpr void SetColor(this Node& self, rbtree::Tag, unsigned color)
        requires has_priomary_color;

    constexpr size_t GetAccSize(this Node const& self, bin_tree::Tag)
        requires acc_size_en;

    constexpr void SetAccSize(this Node& self, bin_tree::Tag, size_t acc_size)
        requires acc_size_en;
};

}  // namespace zeta::core::basic_bin_tree_node

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
