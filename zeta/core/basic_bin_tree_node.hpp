#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/rbtree.hpp>

#pragma push_macro("NodeTplParamList")
#define NodeTplParamList(suffix)                                        \
    ptr_utils::IsLinkType LinkType##suffix, typename PColorTag##suffix, \
        typename LColorTag##suffix, typename RColorTag##suffix,         \
        typename AccSizeTag##suffix,                                    \
        basic_bin_tree_node::PrimaryColorTagEnum PrimaryColorTag##suffix

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
struct Node0 {
    ZETA_Core_StaticAssert(meta::IsValueWrapperT<PColorTag_, bool>);
    ZETA_Core_StaticAssert(meta::IsValueWrapperT<LColorTag_, bool>);
    ZETA_Core_StaticAssert(meta::IsValueWrapperT<RColorTag_, bool>);
};

template <NodeTplParamList(_)>
struct Node1;

template <NodeTplParamList()>
    requires(!AccSizeTag::value)
struct Node1<NodeTplArgList()> : public Node0<NodeTplArgList()> {
    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
};

template <NodeTplParamList()>
    requires(AccSizeTag::value)
struct Node1<NodeTplArgList()> : public Node0<NodeTplArgList()> {
    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
    size_t acc_size;
};

template <NodeTplParamList(_)>
struct Node : public Node1<NodeTplArgList(_)> {
    using LinkType = LinkType_;
    using PColorTag = PColorTag_;
    using LColorTag = LColorTag_;
    using RColorTag = RColorTag_;
    using AccSizeTag = AccSizeTag_;

    static constexpr PrimaryColorTagEnum PrimaryColorTag{ PrimaryColorTag_ };

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, PColorTag>::IsRelLink
    };

    constexpr void Init()
        requires(!AccSizeTag::value);

    constexpr void Init(size_t acc_size)
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

    constexpr Node* GetPPtr();
    constexpr Node* GetLPtr();
    constexpr Node* GetRPtr();

    constexpr Node* GetP(bin_tree::Tag);
    constexpr Node* GetL(bin_tree::Tag);
    constexpr Node* GetR(bin_tree::Tag);

    constexpr Node const* GetPPtr() const;
    constexpr Node const* GetLPtr() const;
    constexpr Node const* GetRPtr() const;

    constexpr Node const* GetP(bin_tree::Tag) const;
    constexpr Node const* GetL(bin_tree::Tag) const;
    constexpr Node const* GetR(bin_tree::Tag) const;

    constexpr unsigned GetPColor() const;
    constexpr unsigned GetLColor() const;
    constexpr unsigned GetRColor() const;

    constexpr unsigned GetColor(rbtree::Tag) const;

    constexpr void SetPPtr(Node* m);
    constexpr void SetLPtr(Node* m);
    constexpr void SetRPtr(Node* m);

    constexpr void SetP(bin_tree::Tag, Node* m);
    constexpr void SetL(bin_tree::Tag, Node* m);
    constexpr void SetR(bin_tree::Tag, Node* m);

    constexpr void SetPColor(unsigned color);
    constexpr void SetLColor(unsigned color);
    constexpr void SetRColor(unsigned color);

    constexpr void SetColor(rbtree::Tag, unsigned color)
        requires(PrimaryColorTag == PrimaryColorTagEnum::P ||
                 PrimaryColorTag == PrimaryColorTagEnum::L ||
                 PrimaryColorTag == PrimaryColorTagEnum::R);

    constexpr size_t GetAccSize(bin_tree::Tag) const
        requires AccSizeTag::value;

    constexpr void SetAccSize(bin_tree::Tag, size_t acc_size)
        requires AccSizeTag::value;
};

}  // namespace zeta::core::basic_bin_tree_node

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
