#include <zeta/core/bin_tree_node.h>
#include <zeta/core/debugger.h>
#include <zeta/core/utils.h>

#include <random>
#include <vector>

// -----------------------------------------------------------------------------

#define TreeNode ShortRelRBTreeNode

#define Zeta_TreeNode ZETA_Core_Concat(Zeta_Core_, TreeNode)

#define Zeta_TreeNode_(x) ZETA_Core_Concat(Zeta_TreeNode, _, x)

// -----------------------------------------------------------------------------

struct Node {
    Zeta_Core_OrdCntRBTreeNode n;
};

struct NodeCup {
    Node* linked_node;
    size_t size;
};

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

std::vector<NodeCup> vec;

size_t size_sum;

void* root;
void* rb;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void CompareLR() {
    void* n = Zeta_Core_GetMostLink(root, Zeta_TreeNode_(GetL));
    auto iter = vec.begin();
    auto end = vec.end();

    for (;;) {
        if (iter == end) {
            ZETA_Core_DebugAssert(n == NULL);
            return;
        }

        ZETA_Core_DebugAssert(n != NULL);

        Node* node = ZETA_Core_MemberToStruct(Node, n, n);

        ZETA_Core_DebugAssert(iter->linked_node == node);
        ZETA_Core_DebugAssert(iter->size ==
                              Zeta_Core_BinTree(TreeNode, GetSize)(n));

        ++iter;
        n = Zeta_Core_BinTree(TreeNode, StepR)(n);
    }
}

void CompareRL() {
    void* n = Zeta_Core_GetMostLink(root, Zeta_TreeNode_(GetR));
    auto iter = vec.rbegin();
    auto end = vec.rend();

    for (;;) {
        if (iter == end) {
            ZETA_Core_DebugAssert(n == NULL);
            return;
        }

        ZETA_Core_DebugAssert(n != NULL);

        Node* node = ZETA_Core_MemberToStruct(Node, n, n);

        ZETA_Core_DebugAssert(iter->linked_node == node);
        ZETA_Core_DebugAssert(iter->size ==
                              Zeta_Core_BinTree(TreeNode, GetSize)(n));

        ++iter;
        n = Zeta_Core_BinTree(TreeNode, StepL)(n);
    }
}

void Sanitize() {
    Zeta_Core_RBTree(TreeNode, Sanitize)(NULL, root);
    CompareLR();
    CompareRL();
}

// -----------------------------------------------------------------------------

size_t fallback_sign = 0x479237197577;

void Access(size_t idx) {
    void* target_n;
    size_t target_tail_idx;

    Zeta_Core_BinTree(TreeNode, AccessL)(&target_n, &target_tail_idx, root,
                                         idx);

    auto target_iter = vec.end();

    for (auto iter = vec.begin(), end = vec.end(); iter != end; ++iter) {
        if (idx < iter->size) {
            target_iter = iter;
            break;
        }

        idx -= iter->size;
    }

    if (target_iter == vec.end()) {
        ZETA_Core_DebugAssert(target_n == NULL);
    } else {
        ZETA_Core_DebugAssert(&target_iter->linked_node->n == target_n);
    }

    ZETA_Core_DebugAssert(target_tail_idx == idx);
}

void Insert(size_t idx, size_t size) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx <= vec.size());

    Node* new_node = new Node;
    Zeta_TreeNode_(Init)(&new_node->n);
    Zeta_Core_BinTree(TreeNode, SetSize)(&new_node->n, size);

    size_sum += size;

    if (idx < vec.size()) {
        Node* ins_node = vec[idx].linked_node;
        root = Zeta_Core_RBTree(TreeNode, InsertL)(&ins_node->n, &new_node->n);
    } else if (vec.size() == 0) {
        root = &new_node->n;
    } else {
        Node* ins_node = vec.back().linked_node;
        root = Zeta_Core_RBTree(TreeNode, InsertR)(&ins_node->n, &new_node->n);
    }

    vec.insert(vec.begin() + idx, (NodeCup){
                                      .linked_node = new_node,
                                      .size = size,
                                  });
}

// -----------------------------------------------------------------------------

void Erase(size_t idx) {
    ZETA_Core_DebugAssert(0 <= idx);
    ZETA_Core_DebugAssert(idx < vec.size());

    size_sum -= vec[idx].size;

    Node* target_node = vec[idx].linked_node;
    root = Zeta_Core_RBTree(TreeNode, Extract)(&target_node->n);
    delete target_node;

    vec.erase(vec.begin() + idx);
}

// -----------------------------------------------------------------------------

void main1() {
    unsigned seed = time(NULL);

    ZETA_Core_PrintVar(seed);

    std::mt19937_64 en{ seed };
    std::uniform_int_distribution<size_t> idx_generator{
        0, ZETA_Core_RangeMaxOf(size_t)
    };
    std::uniform_int_distribution<size_t> size_generator{ 0, 16 };

    size_sum = 0;
    root = NULL;

    ZETA_Core_PrintVar(ZETA_Core_ToStr(Zeta_TreeNode));
    ZETA_Core_PrintVar(sizeof(Zeta_TreeNode));

    for (int i = 0; i < 1024; ++i) {
        Insert(idx_generator(en) % (vec.size() + 1), size_generator(en));
        Sanitize();
    }

    for (int _ = 0; _ < 16; ++_) {
        for (int i = 0, end = idx_generator(en) % 1024 + 4096; i < end; ++i) {
            Insert(idx_generator(en) % (vec.size() + 1), size_generator(en));
            Sanitize();
        }

        for (int i = 0,
                 end = std::min(vec.size(), idx_generator(en) % 1024 + 4096);
             i < end; ++i) {
            Erase(idx_generator(en) % vec.size());
            Sanitize();
        }

        for (int i = 0, end = vec.size() * 2; i < end; ++i) {
            int idx = idx_generator(en) % (size_sum * 2);
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
