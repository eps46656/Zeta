#include <vector>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/random.hpp>

struct BinTreeNode
    : public zeta::core::basic_bin_tree_node::Node<
          void*, zeta::core::meta::AutoValueWrapper<true>,
          zeta::core::meta::AutoValueWrapper<true>,
          zeta::core::meta::AutoValueWrapper<false>,
          zeta::core::meta::AutoValueWrapper<true>,
          zeta::core::basic_bin_tree_node::PrimaryColorTagEnum::Null> {};

struct Node {
    BinTreeNode n;
};

struct NodeCup {
    Node* linked_node;
    size_t size;
};

std::vector<NodeCup> vec;

size_t size_sum;

BinTreeNode* root;
BinTreeNode* rb;

template <>
struct zeta::core::bin_tree::NodeTraits<BinTreeNode> {
    static constexpr bool IsConst() { return false; }

    static constexpr bool HasAccSize() { return true; }

    static constexpr BinTreeNode* GetP(BinTreeNode* n) {
        return static_cast<BinTreeNode*>(n->GetPPtr());
    }

    static constexpr BinTreeNode* GetL(BinTreeNode* n) {
        return static_cast<BinTreeNode*>(n->GetLPtr());
    }

    static constexpr BinTreeNode* GetR(BinTreeNode* n) {
        return static_cast<BinTreeNode*>(n->GetRPtr());
    }

    static constexpr void SetP(BinTreeNode* n, BinTreeNode* m) {
        n->SetPPtr(m);
    }

    static constexpr void SetL(BinTreeNode* n, BinTreeNode* m) {
        n->SetLPtr(m);
    }

    static constexpr void SetR(BinTreeNode* n, BinTreeNode* m) {
        n->SetRPtr(m);
    }

    static constexpr size_t GetNullAccSize() { return 0; }

    static constexpr size_t GetAccSize(BinTreeNode const* n) {
        return n->GetAccSize();
    }

    static constexpr void SetAccSize(BinTreeNode* n, size_t acc_size) {
        n->SetAccSize(acc_size);
    }
};

template <>
struct zeta::core::bin_tree::NodeTraits<BinTreeNode const> {
    static constexpr bool IsConst() { return true; }

    static constexpr bool HasAccSize() { return true; }

    static constexpr BinTreeNode const* GetP(BinTreeNode const* n) {
        return static_cast<BinTreeNode const*>(n->GetPPtr());
    }

    static constexpr BinTreeNode const* GetL(BinTreeNode const* n) {
        return static_cast<BinTreeNode const*>(n->GetLPtr());
    }

    static constexpr BinTreeNode const* GetR(BinTreeNode const* n) {
        return static_cast<BinTreeNode const*>(n->GetRPtr());
    }

    static constexpr size_t GetNullAccSize() { return 0; }

    static constexpr size_t GetAccSize(BinTreeNode const* n) {
        return n->GetAccSize();
    }
};

template <>
struct zeta::core::rbtree::NodeTraits<BinTreeNode const> {
    static unsigned GetColor(BinTreeNode const* n) { return n->GetPColor(); }
};

template <>
struct zeta::core::rbtree::NodeTraits<BinTreeNode> {
    static unsigned GetColor(BinTreeNode const* n) { return n->GetPColor(); }

    static void SetColor(BinTreeNode* n, unsigned color) {
        n->SetPColor(color);
    }
};

inline void CompareLR() {
    BinTreeNode* n{ zeta::core::bin_tree::GetMostL(root).first };

    auto iter{ vec.begin() };
    auto end{ vec.end() };

    for (;;) {
        if (iter == end) {
            ZETA_Core_DebugAssert(n == nullptr);
            return;
        }

        ZETA_Core_DebugAssert(n != nullptr);

        Node* node{ ZETA_Core_MemberToStruct(Node, n, n) };

        ZETA_Core_DebugAssert(iter->linked_node == node);
        ZETA_Core_DebugAssert(iter->size == zeta::core::bin_tree::GetSize(n));

        ++iter;
        n = zeta::core::bin_tree::StepR(n);
    }
}

inline void CompareRL() {
    BinTreeNode* n{ zeta::core::bin_tree::GetMostR(root).first };
    auto iter{ vec.rbegin() };
    auto end{ vec.rend() };

    for (;;) {
        if (iter == end) {
            ZETA_Core_DebugAssert(n == nullptr);
            return;
        }

        ZETA_Core_DebugAssert(n != nullptr);

        Node* node{ ZETA_Core_MemberToStruct(Node, n, n) };

        ZETA_Core_DebugAssert(iter->linked_node == node);
        ZETA_Core_DebugAssert(iter->size == zeta::core::bin_tree::GetSize(n));

        ++iter;
        n = zeta::core::bin_tree::StepL(n);
    }
}

inline void Sanitize() {
    zeta::core::rbtree::Sanitize(nullptr, root);
    CompareLR();
    CompareRL();
}

size_t fallback_sign{ 0x479237197577 };

inline void AccessL(size_t idx) {
    auto [target_n_l,
          target_tail_idx_l]{ zeta::core::bin_tree::AccessL(root, idx) };

    ZETA_Core_DebugAssert(zeta::core::bin_tree::GetAccSize(root) == size_sum);

    auto target_iter{ vec.end() };

    for (auto iter{ vec.begin() }, end{ vec.end() }; iter != end; ++iter) {
        if (idx < iter->size) {
            target_iter = iter;
            break;
        }

        idx -= iter->size;
    }

    if (target_iter == vec.end()) {
        ZETA_Core_DebugAssert(target_n_l == nullptr);
    } else {
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n_l);
    }

    ZETA_Core_DebugAssert(target_tail_idx_l == idx);
}

inline void AccessR(size_t idx) {
    auto [target_n_r,
          target_tail_idx_r]{ zeta::core::bin_tree::AccessR(root, idx) };

    ZETA_Core_DebugAssert(zeta::core::bin_tree::GetAccSize(root) == size_sum);

    auto target_iter{ vec.rend() };

    for (auto iter{ vec.rbegin() }, end{ vec.rend() }; iter != end; ++iter) {
        if (idx < iter->size) {
            target_iter = iter;
            break;
        }

        idx -= iter->size;
    }

    if (target_iter == vec.rend()) {
        ZETA_Core_DebugAssert(target_n_r == nullptr);
    } else {
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n_r);
    }

    ZETA_Core_DebugAssert(target_tail_idx_r == idx);
}

inline void AccessLR(size_t idx) {
    auto [target_n_l,
          target_tail_idx_l]{ zeta::core::bin_tree::AccessL(root, idx) };

    auto [target_n_r, target_tail_idx_r]{ zeta::core::bin_tree::AccessR(
        root, size_sum - 1 - idx) };

    ZETA_Core_DebugAssert(zeta::core::bin_tree::GetAccSize(root) == size_sum);

    size_t last_size;

    auto target_iter{ vec.end() };

    for (auto iter{ vec.begin() }, end{ vec.end() }; iter != end; ++iter) {
        last_size = iter->size;

        if (idx < last_size) {
            target_iter = iter;
            break;
        }

        idx -= last_size;
    }

    if (target_iter == vec.end()) {
        ZETA_Core_DebugAssert(target_n_l == nullptr);
        ZETA_Core_DebugAssert(target_n_r == nullptr);

        ZETA_Core_DebugAssert(target_tail_idx_l == idx);
        ZETA_Core_DebugAssert(target_tail_idx_r == idx);
    } else {
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n_l);
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n_r);

        ZETA_Core_DebugAssert(target_tail_idx_l == idx);
        ZETA_Core_DebugAssert(target_tail_idx_r == last_size - 1 - idx);
    }
}

inline void Insert(size_t idx, size_t size) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx <= vec.size());

    Node* new_node{ new Node };

    new_node->n.Init(0);

    zeta::core::bin_tree::SetSize(&new_node->n, size);

    ZETA_Core_DebugAssert(zeta::core::bin_tree::GetSize(&new_node->n) == size);

    size_sum += size;

    if (idx < vec.size()) {
        Node* ins_node{ vec[idx].linked_node };
        root = zeta::core::rbtree::InsertL(&ins_node->n, &new_node->n);
    } else if (vec.size() == 0) {
        root = &new_node->n;
        zeta::core::rbtree::SetColor(&new_node->n, zeta::core::rbtree::black);
    } else {
        Node* ins_node{ vec.back().linked_node };
        root = zeta::core::rbtree::InsertR(&ins_node->n, &new_node->n);
    }

    vec.insert(vec.begin() + static_cast<long long>(idx),
               {
                   .linked_node = new_node,
                   .size = size,
               });
}

inline void Erase(size_t idx) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx < vec.size());

    size_sum -= vec[idx].size;

    Node* target_node{ vec[idx].linked_node };
    root = zeta::core::rbtree::Extract(&target_node->n);
    delete target_node;

    vec.erase(vec.begin() + static_cast<long long>(idx));
}

inline void main1() {
    unsigned seed{ static_cast<unsigned>(time(nullptr)) };

    ZETA_Core_PrintVar(seed);

    size_sum = 0;
    root = nullptr;

    for (int i{ 0 }; i < 1024; ++i) {
        Insert(zeta::core_test::GetRandomInt<size_t>(0, vec.size()),
               zeta::core_test::GetRandomInt<size_t>(0, 16));

        Sanitize();
    }

    for (int _{ 0 }; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        for (int i{ 0 },
             end{ zeta::core_test::GetRandomInt<int>(1024, 1024 + 4096) };
             i < end; ++i) {
            Insert(zeta::core_test::GetRandomInt<size_t>(0, vec.size()),
                   zeta::core_test::GetRandomInt<size_t>(0, 16));

            Sanitize();
        }

        for (int i{ 0 }, end{ static_cast<int>(std::min(
                             vec.size(), zeta::core_test::GetRandomInt<size_t>(
                                             1024, 1024 + 4096))) };
             i < end; ++i) {
            Erase(zeta::core_test::GetRandomInt<size_t>(0, vec.size() - 1) %
                  vec.size());
            Sanitize();
        }

        for (int i{ 0 }, end{ static_cast<int>(vec.size()) * 2 }; i < end;
             ++i) {
            AccessL(zeta::core_test::GetRandomInt<size_t>(0, size_sum * 2));
            AccessR(zeta::core_test::GetRandomInt<size_t>(0, size_sum * 2));
            AccessLR(zeta::core_test::GetRandomInt<size_t>(0, size_sum - 1));
        }
    }
}

int main() {
    main1();
    std::cout << "ok\n";
    return 0;
}
