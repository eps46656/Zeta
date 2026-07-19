#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::basic_bin_tree_node {

struct PrimaryColorTagEnum {
    struct Null {};
    struct P {};
    struct L {};
    struct R {};
};

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag, typename PrimaryColorTag>
struct NodeBase;

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename PrimaryColorTag>
struct NodeBase<LinkType, PColorTag, LColorTag, RColorTag,
                value_wrapper::FalseType, PrimaryColorTag> {
    ZETA_Core_StaticAssert(
        meta::IsAnySame<PrimaryColorTag, PrimaryColorTagEnum::Null,
                        PrimaryColorTagEnum::P, PrimaryColorTagEnum::L,
                        PrimaryColorTagEnum::R>);

    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
};

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename PrimaryColorTag>
struct NodeBase<LinkType, PColorTag, LColorTag, RColorTag,
                value_wrapper::TrueType, PrimaryColorTag> {
    ZETA_Core_StaticAssert(
        meta::IsAnySame<PrimaryColorTag, PrimaryColorTagEnum::Null,
                        PrimaryColorTagEnum::P, PrimaryColorTagEnum::L,
                        PrimaryColorTagEnum::R>);

    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;
    size_t acc_size;
};

template <typename LinkType_, typename PColorTag_, typename LColorTag_,
          typename RColorTag_, typename AccSizeTag_, typename PrimaryColorTag_>
struct Node : public NodeBase<LinkType_, PColorTag_, LColorTag_, RColorTag_,
                              AccSizeTag_, PrimaryColorTag_> {
    using LinkType = LinkType_;
    using PColorTag = PColorTag_;
    using LColorTag = LColorTag_;
    using RColorTag = RColorTag_;
    using AccSizeTag = AccSizeTag_;
    using PrimaryColorTag = PrimaryColorTag_;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, PColorTag>::IsRelLink
    };

    template <typename _ = void,
              typename = meta::EnableIf<!AccSizeTag::value, _>>
    void Init();

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    void Init(size_t acc_size);

    Node* GetPPtr();
    Node* GetLPtr();
    Node* GetRPtr();

    Node const* GetPPtr() const;
    Node const* GetLPtr() const;
    Node const* GetRPtr() const;

    unsigned GetPColor() const;
    unsigned GetLColor() const;
    unsigned GetRColor() const;

    void SetPPtr(Node* m);
    void SetLPtr(Node* m);
    void SetRPtr(Node* m);

    void SetPColor(unsigned color);
    void SetLColor(unsigned color);
    void SetRColor(unsigned color);

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    size_t GetAccSize() const;

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    void SetAccSize(size_t acc_size);
};

}  // namespace zeta::core::basic_bin_tree_node

namespace zeta::core {

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag, typename PrimaryColorTag>
struct bin_tree::NodeTraits<
    basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                              AccSizeTag, PrimaryColorTag> const> {
    using Node =
        basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                  AccSizeTag, PrimaryColorTag>;

    static constexpr bool IsConst();

    static constexpr bool HasAccSize();

    static Node const* GetP(Node const* n);
    static Node const* GetL(Node const* n);
    static Node const* GetR(Node const* n);

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    static constexpr size_t GetNullAccSize();

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    static size_t GetAccSize(Node const* n);
};

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag, typename PrimaryColorTag>
struct bin_tree::NodeTraits<basic_bin_tree_node::Node<
    LinkType, PColorTag, LColorTag, RColorTag, AccSizeTag, PrimaryColorTag>>
    : public bin_tree::NodeTraits<
          basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                    AccSizeTag, PrimaryColorTag> const> {
    using Node =
        basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                  AccSizeTag, PrimaryColorTag>;

    static constexpr bool IsConst();

    static Node* GetP(Node* n);
    static Node* GetL(Node* n);
    static Node* GetR(Node* n);

    static void SetP(Node* n, Node* m);
    static void SetL(Node* n, Node* m);
    static void SetR(Node* n, Node* m);

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    static void SetAccSize(Node* n, size_t acc_size);
};

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag, typename PrimaryColorTag>
struct rbtree::NodeTraits<
    basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                              AccSizeTag, PrimaryColorTag> const,
    meta::EnableIf<meta::IsAnySame<
        PrimaryColorTag, basic_bin_tree_node::PrimaryColorTagEnum::P,
        basic_bin_tree_node::PrimaryColorTagEnum::L,
        basic_bin_tree_node::PrimaryColorTagEnum::R>>> {
    using Node =
        basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                  AccSizeTag, PrimaryColorTag>;

    static unsigned GetColor(Node const* n);
};

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag, typename PrimaryColorTag>
struct rbtree::NodeTraits<
    basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                              AccSizeTag, PrimaryColorTag>,
    meta::EnableIf<meta::IsAnySame<
        PrimaryColorTag, basic_bin_tree_node::PrimaryColorTagEnum::P,
        basic_bin_tree_node::PrimaryColorTagEnum::L,
        basic_bin_tree_node::PrimaryColorTagEnum::R>>>
    : public rbtree::NodeTraits<
          basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                    AccSizeTag, PrimaryColorTag> const> {
    using Node =
        basic_bin_tree_node::Node<LinkType, PColorTag, LColorTag, RColorTag,
                                  AccSizeTag, PrimaryColorTag>;

    static void SetColor(Node* n, unsigned color);
};

}  // namespace zeta::core
