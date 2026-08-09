#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::bin_tree {

template <typename Node>
struct NodeTraits;

template <typename Node>
concept IsNode = requires(Node* n, size_t acc_size) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::GetP(n))>, Node*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::GetL(n))>, Node*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::GetR(n))>, Node*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::IsConst())>, bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(NodeTraits<Node>::HasAccSize())>, bool>;

    requires NodeTraits<Node>::IsConst() ||
                 requires { NodeTraits<Node>::SetP(n, n); };

    requires NodeTraits<Node>::IsConst() ||
                 requires { NodeTraits<Node>::SetL(n, n); };

    requires NodeTraits<Node>::IsConst() ||
                 requires { NodeTraits<Node>::SetR(n, n); };

    requires !NodeTraits<Node>::HasAccSize() ||
                 meta::IsSame<meta::RemoveCVRef<
                                  decltype(NodeTraits<Node>::GetNullAccSize())>,
                              size_t>;

    requires !NodeTraits<Node>::HasAccSize() ||
                 meta::IsSame<meta::RemoveCVRef<
                                  decltype(NodeTraits<Node>::GetAccSize())>,
                              size_t>;

    requires !NodeTraits<Node>::HasAccSize() ||
                 requires { NodeTraits<Node>::SetAccSize(n, acc_size); };
};

template <typename Node>
struct MemberFuncNodeTraitsAdapter {
    static constexpr decltype(auto) GetP(Node* n);

    static constexpr decltype(auto) GetL(Node* n);

    static constexpr decltype(auto) GetR(Node* n);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) SetP(Node* n, Node* m);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) SetL(Node* n, Node* m);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) SetR(Node* n, Node* m);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) GetNullAccSize();

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) GetAccSize(Node* n);

    template <typename _ = void>
        requires meta::IsSame<_, void>
    static constexpr decltype(auto) SetAccSize(Node* n, size_t acc_size);
};

template <IsNode Node>
constexpr bool IsConst();

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
