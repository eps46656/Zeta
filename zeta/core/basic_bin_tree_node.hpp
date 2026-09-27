#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/rbtree.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList(suffix)                                        \
    ptr_utils::IsLinkType LinkType##suffix,                             \
        meta::IsValueWrapperT<bool> PColorTag##suffix,                  \
        meta::IsValueWrapperT<bool> LColorTag##suffix,                  \
        meta::IsValueWrapperT<bool> RColorTag##suffix,                  \
        typename AccSizeTag##suffix,                                    \
        meta::IsValueWrapperT<basic_bin_tree_node::PrimaryColorTagEnum> \
            PrimaryColorTag##suffix

#pragma push_macro("NodeTplArgList")
#define NodeTplArgList(suffix)                                                 \
    LinkType##suffix, PColorTag##suffix, LColorTag##suffix, RColorTag##suffix, \
        AccSizeTag##suffix, PrimaryColorTag##suffix

namespace zeta::core::basic_bin_tree_node {

enum struct PrimaryColorTagEnum : unsigned char {
    Null = 0b000,
    P = 0b001,
    L = 0b010,
    R = 0b100,
};

template <NodeTplParamList(_)>
struct NodeBase;

template <NodeTplParamList()>
    requires(!AccSizeTag::value)
struct NodeBase<NodeTplArgList()> {
    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
};

template <NodeTplParamList()>
    requires(AccSizeTag::value)
struct NodeBase<NodeTplArgList()> {
    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
    size_t acc_size;
};

template <NodeTplParamList(_)>
struct Node : public NodeBase<NodeTplArgList(_)> {
    using LinkType = LinkType_;
    using PColorTag = PColorTag_;
    using LColorTag = LColorTag_;
    using RColorTag = RColorTag_;
    using AccSizeTag = AccSizeTag_;
    using PrimaryColorTag = PrimaryColorTag_;

    static constexpr PrimaryColorTagEnum primary_color{
        PrimaryColorTag::value
    };

    static constexpr bool has_priomary_color{
        primary_color == PrimaryColorTagEnum::P ||
        primary_color == PrimaryColorTagEnum::L ||
        primary_color == PrimaryColorTagEnum::R
    };

    static constexpr bool is_rel_link{
        ptr_utils::AugPtrTpl<LinkType, PColorTag>::is_rel_link
    };

    constexpr void Construct(this Node& self)
        requires(!AccSizeTag::value);

    constexpr void Construct(this Node& self, size_t acc_size)
        requires AccSizeTag::value;

    static constexpr bool IsConst(bin_tree::Tag, meta::TypeWrapper<Node>);

    static constexpr bool IsConst(bin_tree::Tag, meta::TypeWrapper<Node const>);

    static constexpr bool HasAccSize(bin_tree::Tag, meta::TypeWrapper<Node>);

    static constexpr bool HasAccSize(bin_tree::Tag,
                                     meta::TypeWrapper<Node const>);

    static constexpr size_t GetNullAccSize(bin_tree::Tag,
                                           meta::TypeWrapper<Node>)
        requires AccSizeTag::value;

    static constexpr size_t GetNullAccSize(bin_tree::Tag,
                                           meta::TypeWrapper<Node const>)
        requires AccSizeTag::value;

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
        requires AccSizeTag::value;

    constexpr void SetAccSize(this Node& self, bin_tree::Tag, size_t acc_size)
        requires AccSizeTag::value;
};

}  // namespace zeta::core::basic_bin_tree_node

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
