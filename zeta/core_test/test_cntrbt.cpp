#include <zeta/core/bin_tree.ipp>
#include <zeta/core/bin_tree_node_tpl.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/random.hpp>

// -----------------------------------------------------------------------------

using bin_tree_node_t =
    zeta::core::BinTreeNodeTpl<void*, true, false, false, true>;

struct Node {
    bin_tree_node_t n;
};

struct NodeCup {
    Node* linked_node;
    size_t size;
};

// -----------------------------------------------------------------------------

std::vector<NodeCup> vec;

size_t size_sum;

bin_tree_node_t* root;
bin_tree_node_t* rb;

// -----------------------------------------------------------------------------

struct MyRBTreeNodeOperator {
    static constexpr bool en_acc_size{ true };

    static constexpr size_t null_acc_size{ 0 };

    bin_tree_node_t* GetP(bin_tree_node_t* n) const { return n->GetPPtr(); }
    bin_tree_node_t* GetL(bin_tree_node_t* n) const { return n->GetLPtr(); }
    bin_tree_node_t* GetR(bin_tree_node_t* n) const { return n->GetRPtr(); }

    void SetP(bin_tree_node_t* n, bin_tree_node_t* m) const { n->SetPPtr(m); }
    void SetL(bin_tree_node_t* n, bin_tree_node_t* m) const { n->SetLPtr(m); }
    void SetR(bin_tree_node_t* n, bin_tree_node_t* m) const { n->SetRPtr(m); }

    int GetColor(bin_tree_node_t* n) const { return n->GetPColor(); }
    void SetColor(bin_tree_node_t* n, int color) const { n->SetPColor(color); }

    size_t GetAccSize(bin_tree_node_t const* n) const {
        return n->GetAccSize();
    }

    void SetAccSize(bin_tree_node_t* n, size_t acc_size) const {
        n->SetAccSize(acc_size);
    }
} rbtn_opr;

// -----------------------------------------------------------------------------

void CompareLR() {
    bin_tree_node_t* n{ zeta::core::GetMostLink(root, [&](auto x) {
                            return rbtn_opr.GetL(x);
                        }).first };

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
        ZETA_Core_DebugAssert(iter->size ==
                              zeta::core::bin_tree::GetSize(rbtn_opr, n));

        ++iter;
        n = zeta::core::bin_tree::StepR(rbtn_opr, n);
    }
}

void CompareRL() {
    bin_tree_node_t* n{ zeta::core::GetMostLink(root, [&](auto x) {
                            return rbtn_opr.GetR(x);
                        }).first };
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
        ZETA_Core_DebugAssert(iter->size ==
                              zeta::core::bin_tree::GetSize(rbtn_opr, n));

        ++iter;
        n = zeta::core::bin_tree::StepL(rbtn_opr, n);
    }
}

void Sanitize() {
    zeta::core::rbtree::Sanitize(rbtn_opr, nullptr, root);
    CompareLR();
    CompareRL();
}

// -----------------------------------------------------------------------------

size_t fallback_sign{ 0x479237197577 };

void Access(size_t idx) {
    auto [target_n, target_tail_idx]{ zeta::core::bin_tree::AccessL(
        rbtn_opr, root, idx) };

    auto target_iter{ vec.end() };

    for (auto iter{ vec.begin() }, end{ vec.end() }; iter != end; ++iter) {
        if (idx < iter->size) {
            target_iter = iter;
            break;
        }

        idx -= iter->size;
    }

    if (target_iter == vec.end()) {
        ZETA_Core_DebugAssert(target_n == nullptr);
    } else {
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n);
    }

    ZETA_Core_DebugAssert(target_tail_idx == idx);
}

void Insert(size_t idx, size_t size) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx <= vec.size());

    Node* new_node{ new Node };

    new_node->n.Init();

    zeta::core::bin_tree::SetSize(rbtn_opr, &new_node->n, size);

    ZETA_Core_DebugAssert(
        zeta::core::bin_tree::GetSize(rbtn_opr, &new_node->n) == size);

    size_sum += size;

    if (idx < vec.size()) {
        Node* ins_node{ vec[idx].linked_node };
        root =
            zeta::core::rbtree::InsertL(rbtn_opr, &ins_node->n, &new_node->n);
    } else if (vec.size() == 0) {
        root = &new_node->n;
    } else {
        Node* ins_node{ vec.back().linked_node };
        root =
            zeta::core::rbtree::InsertR(rbtn_opr, &ins_node->n, &new_node->n);
    }

    vec.insert(vec.begin() + static_cast<long long>(idx),
               {
                   .linked_node = new_node,
                   .size = size,
               });
}

// -----------------------------------------------------------------------------

void Erase(size_t idx) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx < vec.size());

    size_sum -= vec[idx].size;

    Node* target_node{ vec[idx].linked_node };
    root = zeta::core::rbtree::Extract(rbtn_opr, &target_node->n);
    delete target_node;

    vec.erase(vec.begin() + static_cast<long long>(idx));
}

// -----------------------------------------------------------------------------

void main1() {
    unsigned seed{ static_cast<unsigned>(time(nullptr)) };

    ZETA_Core_PrintVar(seed);

    size_sum = 0;
    root = nullptr;

    for (int i{ 0 }; i < 1024; ++i) {
        Insert(zeta::core_test::GetRandomInt<size_t, size_t>(0, vec.size()),
               zeta::core_test::GetRandomInt<size_t>(0, 16));

        Sanitize();
    }

    for (int _{ 0 }; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        for (int i{ 0 },
             end{ zeta::core_test::GetRandomInt<int>(1024, 1024 + 4096) };
             i < end; ++i) {
            Insert(zeta::core_test::GetRandomInt<size_t, size_t>(0, vec.size()),
                   zeta::core_test::GetRandomInt<size_t>(0, 16));

            Sanitize();
        }

        for (int i{ 0 }, end{ static_cast<int>(std::min(
                             vec.size(), zeta::core_test::GetRandomInt<size_t>(
                                             1024, 1024 + 4096))) };
             i < end; ++i) {
            Erase(zeta::core_test::GetRandomInt<size_t, size_t>(
                      0, vec.size() - 1) %
                  vec.size());
            Sanitize();
        }

        for (int i{ 0 }, end{ static_cast<int>(vec.size()) * 2 }; i < end;
             ++i) {
            size_t idx{ zeta::core_test::GetRandomInt<size_t, size_t>(
                0, size_sum * 2) };
            Access(idx);
        }
    }
}

// -----------------------------------------------------------------------------

int main() {
    main1();
    printf("ok\n");
    return 0;
}
