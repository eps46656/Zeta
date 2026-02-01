#include <vector>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/bin_tree_node_tpl.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core/value_wrapper.ipp>
#include <zeta/core_test/random.hpp>

// -----------------------------------------------------------------------------

using bin_tree_node_t =
    zeta::core::BinTreeNodeTpl<void*, zeta::core::value_wrapper::TrueType,
                               zeta::core::value_wrapper::TrueType,
                               zeta::core::value_wrapper::TrueType,
                               zeta::core::value_wrapper::TrueType>;

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

struct MyRBTreeNodeView {
    static constexpr bool IsConst(
        zeta::core::type_wrapper::TypeWrapper<MyRBTreeNodeView*>) {
        return false;
    }

    static constexpr bool IsConst(
        zeta::core::type_wrapper::TypeWrapper<MyRBTreeNodeView const*>) {
        return true;
    }

    static constexpr bool IsAccSizeEnabled(
        zeta::core::type_wrapper::TypeWrapper<MyRBTreeNodeView const*>) {
        return true;
    }

    static constexpr size_t GetNullAccSize(
        zeta::core::type_wrapper::TypeWrapper<MyRBTreeNodeView const*>) {
        return 0;
    }

    static MyRBTreeNodeView* GetP(MyRBTreeNodeView* n) {
        return reinterpret_cast<MyRBTreeNodeView*>(
            reinterpret_cast<bin_tree_node_t*>(n)->GetPPtr());
    }

    static MyRBTreeNodeView* GetL(MyRBTreeNodeView* n) {
        return reinterpret_cast<MyRBTreeNodeView*>(
            reinterpret_cast<bin_tree_node_t*>(n)->GetLPtr());
    }

    static MyRBTreeNodeView* GetR(MyRBTreeNodeView* n) {
        return reinterpret_cast<MyRBTreeNodeView*>(
            reinterpret_cast<bin_tree_node_t*>(n)->GetRPtr());
    }

    static MyRBTreeNodeView const* GetP(MyRBTreeNodeView const* n) {
        return reinterpret_cast<MyRBTreeNodeView const*>(
            reinterpret_cast<bin_tree_node_t const*>(n)->GetPPtr());
    }

    static MyRBTreeNodeView const* GetL(MyRBTreeNodeView const* n) {
        return reinterpret_cast<MyRBTreeNodeView const*>(
            reinterpret_cast<bin_tree_node_t const*>(n)->GetLPtr());
    }

    static MyRBTreeNodeView const* GetR(MyRBTreeNodeView const* n) {
        return reinterpret_cast<MyRBTreeNodeView const*>(
            reinterpret_cast<bin_tree_node_t const*>(n)->GetRPtr());
    }

    static void SetP(MyRBTreeNodeView* n, MyRBTreeNodeView* m) {
        reinterpret_cast<bin_tree_node_t*>(n)->SetPPtr(
            reinterpret_cast<bin_tree_node_t*>(m));
    }
    static void SetL(MyRBTreeNodeView* n, MyRBTreeNodeView* m) {
        reinterpret_cast<bin_tree_node_t*>(n)->SetLPtr(
            reinterpret_cast<bin_tree_node_t*>(m));
    }
    static void SetR(MyRBTreeNodeView* n, MyRBTreeNodeView* m) {
        reinterpret_cast<bin_tree_node_t*>(n)->SetRPtr(
            reinterpret_cast<bin_tree_node_t*>(m));
    }

    static unsigned GetColor(MyRBTreeNodeView const* n) {
        return reinterpret_cast<bin_tree_node_t const*>(n)->GetPColor();
    }

    static void SetColor(MyRBTreeNodeView* n, unsigned color) {
        reinterpret_cast<bin_tree_node_t*>(n)->SetPColor(color);
    }

    static size_t GetAccSize(MyRBTreeNodeView const* n) {
        return reinterpret_cast<bin_tree_node_t const*>(n)->GetAccSize();
    }

    static void SetAccSize(MyRBTreeNodeView* n, size_t acc_size) {
        reinterpret_cast<bin_tree_node_t*>(n)->SetAccSize(acc_size);
    }
};

// -----------------------------------------------------------------------------

void CompareLR() {
    bin_tree_node_t* n{ reinterpret_cast<bin_tree_node_t*>(
        zeta::core::GetMostLink(
            reinterpret_cast<MyRBTreeNodeView*>(root),
            [&](auto x) { return MyRBTreeNodeView::GetL(x); })
            .first) };

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
                              zeta::core::bin_tree::GetSize(
                                  reinterpret_cast<MyRBTreeNodeView*>(n)));

        ++iter;
        n = reinterpret_cast<bin_tree_node_t*>(zeta::core::bin_tree::StepR(
            reinterpret_cast<MyRBTreeNodeView*>(n)));
    }
}

void CompareRL() {
    bin_tree_node_t* n{ reinterpret_cast<bin_tree_node_t*>(
        zeta::core::GetMostLink(
            reinterpret_cast<MyRBTreeNodeView*>(root),
            [&](MyRBTreeNodeView* x) { return MyRBTreeNodeView::GetR(x); })
            .first) };
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
                              zeta::core::bin_tree::GetSize(
                                  reinterpret_cast<MyRBTreeNodeView*>(n)));

        ++iter;
        n = reinterpret_cast<bin_tree_node_t*>(zeta::core::bin_tree::StepL(
            reinterpret_cast<MyRBTreeNodeView*>(n)));
    }
}

void Sanitize() {
    zeta::core::rbtree::Sanitize(nullptr,
                                 reinterpret_cast<MyRBTreeNodeView*>(root));
    CompareLR();
    CompareRL();
}

// -----------------------------------------------------------------------------

size_t fallback_sign{ 0x479237197577 };

void Access(size_t idx) {
    auto [target_n_, target_tail_idx]{ zeta::core::bin_tree::AccessL(
        reinterpret_cast<MyRBTreeNodeView*>(root), idx) };

    auto* target_n{ reinterpret_cast<bin_tree_node_t*>(target_n_) };

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

    zeta::core::bin_tree::SetSize(
        reinterpret_cast<MyRBTreeNodeView*>(&new_node->n), size);

    ZETA_Core_DebugAssert(
        zeta::core::bin_tree::GetSize(
            reinterpret_cast<MyRBTreeNodeView*>(&new_node->n)) == size);

    size_sum += size;

    if (idx < vec.size()) {
        Node* ins_node{ vec[idx].linked_node };
        root = reinterpret_cast<bin_tree_node_t*>(zeta::core::rbtree::InsertL(
            reinterpret_cast<MyRBTreeNodeView*>(&ins_node->n),
            reinterpret_cast<MyRBTreeNodeView*>(&new_node->n)));
    } else if (vec.size() == 0) {
        root = &new_node->n;
        MyRBTreeNodeView::SetColor(
            reinterpret_cast<MyRBTreeNodeView*>(&new_node->n),
            zeta::core::rbtree::black);
    } else {
        Node* ins_node{ vec.back().linked_node };
        root = reinterpret_cast<bin_tree_node_t*>(zeta::core::rbtree::InsertR(
            reinterpret_cast<MyRBTreeNodeView*>(&ins_node->n),
            reinterpret_cast<MyRBTreeNodeView*>(&new_node->n)));
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
    root = reinterpret_cast<bin_tree_node_t*>(zeta::core::rbtree::Extract(
        reinterpret_cast<MyRBTreeNodeView*>(&target_node->n)));
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
            size_t idx{ zeta::core_test::GetRandomInt<size_t>(0,
                                                              size_sum * 2) };
            Access(idx);
        }
    }
}

// -----------------------------------------------------------------------------

int main() {
    main1();
    std::cout << "ok\n";
    return 0;
}
