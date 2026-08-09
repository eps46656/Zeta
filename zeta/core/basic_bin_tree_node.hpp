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

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !AccSizeTag::value;
        }
    constexpr void Init();

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires AccSizeTag::value;
        }
    constexpr void Init(size_t acc_size);

    constexpr Node* GetPPtr();
    constexpr Node* GetLPtr();
    constexpr Node* GetRPtr();

    constexpr Node const* GetPPtr() const;
    constexpr Node const* GetLPtr() const;
    constexpr Node const* GetRPtr() const;

    constexpr unsigned GetPColor() const;
    constexpr unsigned GetLColor() const;
    constexpr unsigned GetRColor() const;

    constexpr unsigned GetColor() const;

    constexpr void SetPPtr(Node* m);
    constexpr void SetLPtr(Node* m);
    constexpr void SetRPtr(Node* m);

    constexpr void SetPColor(unsigned color);
    constexpr void SetLColor(unsigned color);
    constexpr void SetRColor(unsigned color);

    constexpr void SetColor(unsigned color);

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    constexpr size_t GetAccSize() const;

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    constexpr void SetAccSize(size_t acc_size);
};

}  // namespace zeta::core::basic_bin_tree_node

namespace zeta::core {

template <NodeTplParamList()>
struct bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList()>>
    : public bin_tree::MemberFuncNodeTraitsAdapter<
          basic_bin_tree_node::Node<NodeTplArgList()>> {
    static constexpr bool IsConst();

    static constexpr bool HasAccSize();
};

template <NodeTplParamList()>
struct bin_tree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList()> const>
    : public bin_tree::MemberFuncNodeTraitsAdapter<
          basic_bin_tree_node::Node<NodeTplArgList()> const> {
    static constexpr bool IsConst();

    static constexpr bool HasAccSize();
};

template <NodeTplParamList()>
struct rbtree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList()>>
    : public rbtree::MemberFuncNodeTraitsAdapter<
          basic_bin_tree_node::Node<NodeTplArgList()>> {};

template <NodeTplParamList()>
struct rbtree::NodeTraits<basic_bin_tree_node::Node<NodeTplArgList()> const>
    : public rbtree::MemberFuncNodeTraitsAdapter<
          basic_bin_tree_node::Node<NodeTplArgList()> const> {};

}  // namespace zeta::core

#pragma pop_macro("NodeTplArgList")
#pragma pop_macro("NodeTplParamList")
