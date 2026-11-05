#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::bin_tree {

struct Tag {};

template <typename Node>
concept IsNode = requires(Node* n, Tag tag, size_t acc_size) {
    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(n->GetP(tag))>, Node*>;

    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(n->GetL(tag))>, Node*>;

    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(n->GetR(tag))>, Node*>;

    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(Node::IsConstType(
            tag, meta::TypeWrapper<Node>{}))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(Node::HasAccSize(
            tag, meta::TypeWrapper<Node>{}))>,
        bool>;

    requires meta::Implies(
        !Node::IsConstType(tag, meta::TypeWrapper<Node>{}),
        requires { n->SetP(tag, n); });

    requires meta::Implies(
        !Node::IsConstType(tag, meta::TypeWrapper<Node>{}),
        requires { n->SetL(tag, n); });

    requires meta::Implies(
        !Node::IsConstType(tag, meta::TypeWrapper<Node>{}),
        requires { n->SetR(tag, n); });

    requires meta::Implies(
        Node::HasAccSize(tag, meta::TypeWrapper<Node>{}),
        meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(Node::GetNullAccSize(
                tag, meta::TypeWrapper<Node>{}))>,
            size_t>);

    requires meta::Implies(
        Node::HasAccSize(tag, meta::TypeWrapper<Node>{}),
        meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(n->GetAccSize(tag))>,
            size_t>);

    requires meta::Implies(
        Node::HasAccSize(tag, meta::TypeWrapper<Node>{}),
        requires { n->SetAccSize(tag, acc_size); });
};

template <IsNode Node>
constexpr bool IsConstType();

template <IsNode Node>
constexpr bool HasAccSize();

template <IsNode Node>
constexpr Node* GetP(Node* n);

template <IsNode Node>
constexpr Node* GetL(Node* n);

template <IsNode Node>
constexpr Node* GetR(Node* n);

template <IsNode Node>
constexpr void SetP(Node* n, Node* m);

template <IsNode Node>
constexpr void SetL(Node* n, Node* m);

template <IsNode Node>
constexpr void SetR(Node* n, Node* m);

template <IsNode Node>
constexpr size_t GetNullAccSize();

template <IsNode Node>
constexpr size_t GetAccSize(Node* n);

template <IsNode Node>
constexpr void SetAccSize(Node* n, size_t acc_size);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> GetMostP(Node* n);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> GetMostL(Node* n);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> GetMostR(Node* n);

template <IsNode Node>
constexpr size_t Count(Node* n);

template <IsNode Node>
constexpr void AddDiffSize(Node* n, size_t diff_size);

template <IsNode Node>
constexpr size_t GetSize(Node* n);

template <IsNode Node>
constexpr void SetSize(Node* n, size_t size);

template <IsNode Node>
constexpr void AttatchL(Node* pos, Node* n);

template <IsNode Node>
constexpr void AttatchR(Node* pos, Node* n);

template <IsNode Node>
constexpr void Detach(Node* n);

template <IsNode Node>
constexpr void Swap(Node* n, Node* m);

template <IsNode Node>
constexpr void RotateL(Node* n);

template <IsNode Node>
constexpr void RotateR(Node* n);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> AccessL(Node* n, size_t idx);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> AccessR(Node* n, size_t idx);

template <IsNode Node>
constexpr Node* StepPL(Node* n);

template <IsNode Node>
constexpr Node* StepPR(Node* n);

template <IsNode Node>
constexpr Node* StepL(Node* n);

template <IsNode Node>
constexpr Node* StepR(Node* n);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> AdvanceL(Node* n, size_t step);

template <IsNode Node>
constexpr pair::Pair<Node*, size_t> AdvanceR(Node* n, size_t step);

template <IsNode Node>
constexpr pair::Pair<size_t, size_t> GetLRAccSize(Node* n);

template <IsNode Node>
constexpr void Sanitize(Node* root);

}  // namespace zeta::core::bin_tree
