#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>

namespace zeta::core::rbtree {

constexpr unsigned black{ 1 };
constexpr unsigned red{ 2 };

constexpr size_t max_height{ integral::WidthOf<size_t> * 2 };

template <typename Node>
struct NodeTraits;

template <typename Node>
concept IsNode = requires(Node* n, unsigned color) {
    requires bin_tree::IsNode<Node>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::GetColor(n))>, unsigned>;

    requires bin_tree::IsConst<Node>() ||
                 meta::IsSame<
                     meta::RemoveCVRef<decltype(NodeTraits<Node>::SetColor(
                         n, color))>,
                     void>;
};

template <typename Node>
struct MemberFuncNodeTraitsAdapter {
    static constexpr decltype(auto) GetColor(Node* n);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) SetColor(Node* n, unsigned color);
};

template <IsNode Node>
constexpr unsigned GetColor(Node* n);

template <IsNode Node>
constexpr void SetColor(Node* n, unsigned color);

template <IsNode Node>
constexpr Node* InsertL(Node* pos, Node* n);

template <IsNode Node>
constexpr Node* InsertR(Node* pos, Node* n);

template <IsNode Node>
constexpr Node* Insert(Node* pos_l, Node* pos_r, Node* n);

template <IsNode Node>
constexpr Node* GeneralInsertL(Node* root, Node* pos, Node* n);

template <IsNode Node>
constexpr Node* GeneralInsertR(Node* root, Node* pos, Node* n);

template <IsNode Node>
constexpr Node* Extract(Node* pos);

template <IsNode Node>
constexpr void Sanitize(mem_recorder::MemRecorder* dst_mr, Node* root);

}  // namespace zeta::core::rbtree
