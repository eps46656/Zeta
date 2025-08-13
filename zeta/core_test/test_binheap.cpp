#include <zeta/core/bin_heap.h>
#include <zeta/core/bin_tree_node.h>

#include <queue>
#include <random>

struct Node {
    Zeta_OrdCntRBTreeNode n;
    int val;
};

Zeta_BinTreeNodeOperator btn_opr;

Zeta_BinHeap bh;

std::priority_queue<int, std::vector<int>, std::greater<int>> queue;

void SetBntOpr() {
    Zeta_OrdCntRBTreeNode_DeployBinTreeNodeOperator(NULL, &btn_opr);
}

void BinHeap_Init() {
    SetBntOpr();

    bh.btn_opr = &btn_opr;
    Zeta_BinHeap_Init(&bh);
}

int BinHeap_Top() {
    ZETA_Core_DebugAssert(0 < bh.size);
    Node* node{ ZETA_Core_MemberToStruct(Node, n, bh.root) };
    return node->val;
}

void BinHeap_Push(int val) {
    Node* node{ new Node };
    Zeta_OrdCntRBTreeNode_Init(NULL, &node->n);
    node->val = val;

    Zeta_BinHeap_Insert(&bh, node);

    for (;;) {
        Zeta_OrdCntRBTreeNode* np{ static_cast<Zeta_OrdCntRBTreeNode*>(
            Zeta_OrdCntRBTreeNode_GetP(NULL, &node->n)) };

        if (np == nullptr) { break; }

        Node* node_p{ ZETA_Core_MemberToStruct(Node, n, np) };

        if (node_p->val <= val) { break; }

        Zeta_BinTree_Swap(&btn_opr, &node->n, np);
    }

    Zeta_BinHeap_UpdateRoot(&bh);
}

void BinHeap_Pop() {
    ZETA_Core_DebugAssert(0 < bh.size);

    void* n{ Zeta_BinHeap_Access(&bh, 0) };
    void* m{ Zeta_BinHeap_Extract(&bh, n) };

    Node* node{ ZETA_Core_MemberToStruct(Node, n, n) };
    delete node;

    if (m == NULL) { return; }

    int m_val{ ZETA_Core_MemberToStruct(Node, n, m)->val };

    for (;;) {
        void* ml{ Zeta_OrdCntRBTreeNode_GetL(NULL, m) };
        void* mr{ Zeta_OrdCntRBTreeNode_GetR(NULL, m) };

        int ml_val;
        int mr_val;

        if (ml != NULL) { ml_val = ZETA_Core_MemberToStruct(Node, n, ml)->val; }
        if (mr != NULL) { mr_val = ZETA_Core_MemberToStruct(Node, n, mr)->val; }

        if (ml == NULL && mr == NULL) { break; }

        if (ml == NULL) {
            if (m_val <= mr_val) { break; }
            Zeta_BinTree_Swap(&btn_opr, m, mr);
            continue;
        }

        if (mr == NULL) {
            if (m_val <= ml_val) { break; }
            Zeta_BinTree_Swap(&btn_opr, m, ml);
            continue;
        }

        if (ml_val <= mr_val) {
            if (m_val <= ml_val) { break; }
            Zeta_BinTree_Swap(&btn_opr, m, ml);
        } else {
            if (m_val <= mr_val) { break; }
            Zeta_BinTree_Swap(&btn_opr, m, mr);
        }
    }

    Zeta_BinHeap_UpdateRoot(&bh);
}

void BinHeap_Check() {
    ZETA_Core_DebugAssert(bh.size == queue.size());
    Zeta_BinHeap_Check(&bh);
}

void Queue_Init() {}

int Queue_Top() { return queue.top(); }

void Queue_Push(int val) { queue.push(val); }

void Queue_Pop() { queue.pop(); }

void main1() {
    BinHeap_Init();
    Queue_Init();

    ZETA_Core_PrintVar(bh.size);

    BinHeap_Push(3);
    ZETA_Core_PrintVar(bh.size);

    BinHeap_Push(7);
    ZETA_Core_PrintVar(bh.size);

    BinHeap_Push(5);
    ZETA_Core_PrintVar(bh.size);

    ZETA_Core_PrintVar(BinHeap_Top());
    BinHeap_Pop();

    ZETA_Core_PrintVar(bh.size);

    ZETA_Core_PrintVar(BinHeap_Top());
    BinHeap_Pop();

    ZETA_Core_PrintVar(bh.size);

    ZETA_Core_PrintVar(BinHeap_Top());
    BinHeap_Pop();

    ZETA_Core_PrintVar(bh.size);
}

void main2() {
    unsigned seed = time(NULL);

    ZETA_Core_PrintVar(seed);

    std::mt19937_64 en{ seed };
    std::uniform_int_distribution<size_t> val_generator{
        0, ZETA_Core_RangeMaxOf(int)
    };

    BinHeap_Init();
    Queue_Init();

    BinHeap_Check();

    int K{ 1024 * 64 };

    for (int i{ 0 }; i < K; ++i) {
        ZETA_Core_DebugAssert((size_t)i == bh.size);
        BinHeap_Push(i);
    }

    ZETA_Core_DebugAssert((size_t)K == bh.size);

    for (int i{ 0 }; i < K; ++i) {
        void* n{ Zeta_BinHeap_Access(&bh, i) };
        Node* node{ ZETA_Core_MemberToStruct(Node, n, n) };
        ZETA_Core_DebugAssert(i == node->val);
    }

    for (int i{ 0 }; i < K; ++i) {
        BinHeap_Pop();
        Zeta_BinHeap_Check(&bh);
    }

    for (int i{ 0 }; i <= 32; ++i) {
        BinHeap_Check();

        for (int i = 0; i < 1000; ++i) {
            int val = val_generator(en);

            Queue_Push(val);

            BinHeap_Push(val);
            BinHeap_Check();
        }

        for (int j{ 0 }; j < 8; ++j) {
            for (int i = 0; i < 1000; ++i) {
                int val = val_generator(en);

                Queue_Push(val);

                BinHeap_Push(val);
                BinHeap_Check();
            }

            for (int i = 0; i < 1000; ++i) {
                ZETA_Core_DebugAssert(Queue_Top() == BinHeap_Top());

                Queue_Pop();

                BinHeap_Pop();
                BinHeap_Check();
            }
        }

        while (!queue.empty()) {
            ZETA_Core_DebugAssert(Queue_Top() == BinHeap_Top());

            Queue_Pop();

            BinHeap_Pop();
            BinHeap_Check();
        }
    }
}

int main() {
    main2();
    printf("ok\n");
    return 0;
}
