#include <zeta/core_test/BTree.h>
#include <zeta/core_test/utils.h>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_DEGREE (2)
#define MIN_DEGREE (MAX_DEGREE / 2)

ZETA_Core_DeclareStruct(MyBTreeNode);

struct MyBTreeNode {
    size_t degree;

    void* p;
    uintmax_t ks[MAX_DEGREE];
    void* cs[MAX_DEGREE + 1];

    size_t acc_size;
};

size_t MyBTreeNode_GetMinDegree(void* context) {
    ZETA_Core_Unused(context);
    return MIN_DEGREE;
}

size_t MyBTreeNode_GetMaxDegree(void* context) {
    ZETA_Core_Unused(context);
    return MAX_DEGREE;
}

size_t MyBTreeNode_GetDegree(void* context, void* n_) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    return n->degree;
}

void* MyBTreeNode_GetP(void* context, void* n_) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    return n->p;
}

uintmax_t MyBTreeNode_GetK(void* context, void* n_, size_t key_idx) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(key_idx < n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    return n->ks[key_idx];
}

void* MyBTreeNode_GetC(void* context, void* n_, size_t child_idx) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(child_idx <= n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    return n->cs[child_idx];
}

void MyBTreeNode_SetP(void* context, void* n_, void* p) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    n->p = p;
}

void MyBTreeNode_SetK(void* context, void* n_, size_t key_idx, uintmax_t key) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(key_idx < n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    n->ks[key_idx] = key;
}

void MyBTreeNode_SetC(void* context, void* n_, size_t child_idx, void* child) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(child_idx <= n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    n->cs[child_idx] = child;
}

void MyBTreeNode_Insert(void* context, void* n_, size_t child_idx, size_t cnt) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    ZETA_Core_DebugAssert(n->degree + cnt <= MAX_DEGREE);

    Zeta_Core_MemCopy(&n->ks[child_idx + cnt], &n->ks[child_idx],
                      sizeof(uintmax_t) * (n->degree - child_idx));
    Zeta_Core_MemCopy(&n->cs[child_idx + cnt], &n->cs[child_idx],
                      sizeof(void*) * (n->degree - child_idx + 1));

    n->degree += cnt;
}

void MyBTreeNode_PopL(void* context, void* n_, size_t cnt) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(cnt <= n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    Zeta_Core_MemCopy(&n->ks[0], &n->ks[cnt],
                      sizeof(uintmax_t) * (n->degree - cnt));
    Zeta_Core_MemCopy(&n->cs[0], &n->cs[cnt],
                      sizeof(void*) * (n->degree - cnt + 1));

    n->degree -= cnt;
}

void MyBTreeNode_PopR(void* context, void* n_, size_t cnt) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(cnt <= n->degree);
    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    n->degree -= cnt;
}

size_t MyBTreeNode_GetSize(void* context, uintmax_t key) {
    ZETA_Core_Unused(context);
    ZETA_Core_Unused(key);

    return 1;
}

size_t MyBTreeNode_GetAccSize(void* context, void* n_) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;

    if (n == NULL) { return 0; }

    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    return n->acc_size;
}

void MyBTreeNode_SetAccSize(void* context, void* n_, size_t acc_size) {
    ZETA_Core_Unused(context);

    MyBTreeNode* n = (MyBTreeNode*)n_;
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(n->degree <= MAX_DEGREE);

    n->acc_size = acc_size;
}

void* MyBTreeNode_AllocateNode(void* context) {
    ZETA_Core_Unused(context);

    MyBTreeNode* ret = (MyBTreeNode*)std::malloc(sizeof(MyBTreeNode));
    ZETA_Core_DebugAssert(ret != NULL);

    ret->degree = 0;
    ret->p = NULL;

    return ret;
}

void MyBTreeNode_DeallocateNode(void* context, void* n) {
    ZETA_Core_Unused(context);

    std::free(n);
}

void MyBTreeNode_DeplotBTreeOperator(void* context,
                                     Zeta_BTreeNodeOperator* btn_opr) {
    ZETA_Core_Unused(context);
    ZETA_Core_DebugAssert(btn_opr != NULL);

    btn_opr->context = NULL;

    btn_opr->GetMinDegree = MyBTreeNode_GetMinDegree;
    btn_opr->GetMaxDegree = MyBTreeNode_GetMaxDegree;

    btn_opr->GetDegree = MyBTreeNode_GetDegree;

    btn_opr->GetP = MyBTreeNode_GetP;
    btn_opr->GetK = MyBTreeNode_GetK;
    btn_opr->GetC = MyBTreeNode_GetC;

    btn_opr->SetP = MyBTreeNode_SetP;
    btn_opr->SetK = MyBTreeNode_SetK;
    btn_opr->SetC = MyBTreeNode_SetC;

    btn_opr->Insert = MyBTreeNode_Insert;
    btn_opr->PopL = MyBTreeNode_PopL;
    btn_opr->PopR = MyBTreeNode_PopR;

    btn_opr->GetSize = MyBTreeNode_GetSize;

    btn_opr->GetAccSize = MyBTreeNode_GetAccSize;
    btn_opr->SetAccSize = MyBTreeNode_SetAccSize;

    btn_opr->AllocateNode = MyBTreeNode_AllocateNode;
    btn_opr->DeallocateNode = MyBTreeNode_DeallocateNode;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void main1() {
    Zeta_BTreeNodeOperator btn_opr;
    MyBTreeNode_DeplotBTreeOperator(NULL, &btn_opr);

    void* root = NULL;
    void* n = NULL;

    ZETA_Core_PrintPos;

    Zeta_BTree_Check(&btn_opr, root);
    root = Zeta_BTree_Insert(&btn_opr, root, 0, 0);
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;

    n = Zeta_BTree_AccessL(&btn_opr, root, 0).n;

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);

    root = Zeta_BTree_Insert(&btn_opr, n, 0, 0);

    ZETA_Core_PrintPos;
    Zeta_BTree_Check(&btn_opr, root);
}

int main() {
    main1();
    printf("ok\n");
    return 0;
}
