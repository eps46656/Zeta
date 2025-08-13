#include <zeta/core/debugger.h>
#include <zeta/core/lin_space_allocator.h>
#include <zeta/core/ptr_utils.h>
#include <zeta/core/utils.h>

#if ZETA_Core_EnableDebug

#define Check_(lsa) Zeta_Core_LinSpaceAllocator_Check((lsa))

#else

#define Check_(lsa) ZETA_Core_Unused((lsa))

#endif

#define vacant_color (0)
#define occupied_color (1)

#define Node Zeta_Core_LinSpaceAllocator_Node
#define GNode Zeta_Core_ShortRelCnt3RBTreeNode
#define SNode Zeta_Core_ShortRelRBTreeNode

#define GNode_(x) ZETA_Core_Concat(GNode, _, x)
#define SNode_(x) ZETA_Core_Concat(SNode, _, x)

#define GNodeToNode(x) ZETA_Core_MemberToStruct(Node, gn, (x))
#define SNodeToNode(x) ZETA_Core_MemberToStruct(Node, sn, (x))

static Node* FindNodeFromIdx_(Zeta_Core_LinSpaceAllocator* lsa, size_t idx) {
    ZETA_Core_DebugAssert(lsa->beg <= idx && idx <= lsa->end);

    void* gn;
    size_t tail_idx;

    Zeta_Core_BinTree(GNode, AccessL)(&gn, &tail_idx, lsa->gt_root,
                                      idx - lsa->beg);

    ZETA_Core_DebugAssert(gn != NULL);
    ZETA_Core_DebugAssert(tail_idx == 0);

    return GNodeToNode(gn);
}

static Node* FindNode_(Zeta_Core_LinSpaceAllocator* lsa, size_t size) {
    SNode* ret = NULL;

    for (SNode* sn = &lsa->st_root->sn; sn != NULL;) {
        if (Zeta_Core_BinTree(SNode, GetSize)(sn) < size) {
            sn = SNode_(GetR)(sn);
        } else {
            ret = sn;
            sn = SNode_(GetL)(sn);
        }
    }

    return ret == NULL ? NULL : SNodeToNode(ret);
}

static Node* FindNiceNode_(Zeta_Core_LinSpaceAllocator* lsa, size_t size) {
    Node* best_fit_n = FindNode_(lsa, size);

    if (best_fit_n == NULL) { return NULL; }

    if (lsa->strategy ==
            ZETA_Core_LinSpaceAllocator_AllocationStrategy_BestFit ||
        Zeta_Core_BinTree(SNode, GetSize)(best_fit_n) == size) {
        return best_fit_n;
    }

    Node* twice_best_fit_n = FindNode_(lsa, size * 2);

    return twice_best_fit_n == NULL
               ? Zeta_Core_GetMostLink(lsa->st_root, SNode_(GetR))
               : twice_best_fit_n;
}

static Node* AllocateNode_(Zeta_Core_LinSpaceAllocator* lsa) {
    ZETA_Core_PrintCurPos;

    Node* n = ZETA_Core_Allocator_SafeAllocate(lsa->node_allocator,
                                               alignof(Node), sizeof(Node));

    ZETA_Core_PrintCurPos;

    ZETA_Core_RelColorPtr_Set(&n->gn.p, alignof(Node), n, n, vacant_color);

    ZETA_Core_PrintCurPos;

    ZETA_Core_RelColorPtr_Set(&n->gn.l, alignof(Node), n, n, vacant_color);
    ZETA_Core_RelPtr_SetPtr(&n->gn.r, n, n);

    ZETA_Core_PrintCurPos;

    ZETA_Core_RelColorPtr_Set(&n->sn.p, alignof(Node), n, n, 0);
    ZETA_Core_RelPtr_SetPtr(&n->sn.l, n, n);
    ZETA_Core_RelPtr_SetPtr(&n->sn.r, n, n);

    return n;
}

static void DeallocateNode_(Zeta_Core_LinSpaceAllocator* lsa, Node* n) {
    ZETA_Core_Allocator_Deallocate(lsa->node_allocator, n);
}

void Zeta_Core_LinSpaceAllocator_Init(void* lsa_) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    ZETA_Core_DebugAssert(lsa != NULL);

    ZETA_Core_PrintCurPos;

    size_t beg = lsa->beg;
    size_t end = lsa->end;

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(beg < end);

    ZETA_Core_DebugAssert(lsa->node_allocator.vtable != NULL);

    ZETA_Core_PrintCurPos;

    Node* n = AllocateNode_(lsa);
    n->gn.acc_size = end - beg;

    ZETA_Core_PrintCurPos;

    lsa->gt_root = GNodeToNode(
        Zeta_Core_RBTree(GNode, GeneralInsertL)(NULL, NULL, &n->gn));

    lsa->st_root = SNodeToNode(
        Zeta_Core_RBTree(SNode, GeneralInsertL)(NULL, NULL, &n->sn));
}

size_t Zeta_Core_LinSpaceAllocator_Allocate(void* lsa_, size_t size) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    if (size == 0) { return ZETA_Core_size_max; }

    Node* nm = FindNiceNode_(lsa, size);

    if (nm == NULL) { return ZETA_Core_size_max; }

    size_t nm_size = Zeta_Core_BinTree(SNode, GetSize)(&nm->sn);

    GNode_(SetLColor)(&nm->gn, occupied_color);

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&nm->sn));

    size_t ret = ({
        size_t l_acc_size;
        Zeta_Core_BinTree(SNode, GetAccSize)(&l_acc_size, NULL, nm);
        lsa->beg + l_acc_size;
    });

    size_t res_size = nm_size - size;

    if (res_size == 0) { return ret; }

    Zeta_Core_BinTree(SNode, SetSize)(&nm->sn, size);

    Node* new_nr = AllocateNode_(lsa);
    new_nr->gn.acc_size = res_size;

    lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, GeneralInsertR)(
        &lsa->gt_root->gn, &nm->gn, &new_nr->gn));

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertL)(
        &lsa->st_root->sn, &FindNode_(lsa, res_size + 1)->sn, &new_nr->sn));

    return ret;
}

void Zeta_Core_LinSpaceAllocator_Deallocate(void* lsa_, size_t idx) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    if (idx == ZETA_Core_size_max) { return; }

    Node* nm = FindNodeFromIdx_(lsa, idx);

    Node* nl = GNodeToNode(Zeta_Core_BinTree(GNode, StepL)(&nm->gn));
    Node* nr = GNodeToNode(Zeta_Core_BinTree(GNode, StepR)(&nm->gn));

    int nm_color = GNode_(GetLColor)(&nm->gn);
    ZETA_Core_DebugAssert(nm_color == occupied_color);

    int nl_color = nl == NULL ? occupied_color : GNode_(GetLColor)(&nl->gn);
    ZETA_Core_DebugAssert(nl_color == vacant_color ||
                          nl_color == occupied_color);

    int nr_color = nr == NULL ? occupied_color : GNode_(GetLColor)(&nr->gn);
    ZETA_Core_DebugAssert(nr_color == vacant_color ||
                          nr_color == occupied_color);

    bool_t l_merge = nl_color == vacant_color;
    bool_t r_merge = nr_color == vacant_color;

    size_t m_size = Zeta_Core_BinTree(SNode, GetSize)(&nm->sn);

    Node* ins_n = nm;
    size_t ins_size = m_size;

    if (l_merge) {
        ins_n = nl;
        ins_size += Zeta_Core_BinTree(SNode, GetSize)(&nl->sn);

        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, Extract)(&nm->gn));

        DeallocateNode_(lsa, nm);

        lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&nm->sn));
    } else {
        GNode_(SetL)(&nm->gn, vacant_color);
    }

    if (r_merge) {
        ins_size += Zeta_Core_BinTree(SNode, GetSize)(&nr->sn);

        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, Extract)(&nr->gn));
        lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&nr->sn));

        DeallocateNode_(lsa, nr);
    }

    Zeta_Core_BinTree(SNode, SetSize)(&ins_n->sn, ins_size);

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertL)(
        &lsa->st_root->sn, &FindNode_(lsa, ins_size + 1)->sn, &ins_n->sn));
}

size_t Zeta_Core_LinSpaceAllocator_GetVacantSizeL(void* lsa_) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetL)));

    return GNode_(GetLColor)(&n->gn) == vacant_color
               ? Zeta_Core_BinTree(SNode, GetSize)(&n->sn)
               : 0;
}

size_t Zeta_Core_LinSpaceAllocator_GetVacantSizeR(void* lsa_) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetR)));

    return GNode_(GetLColor)(&n->gn) == vacant_color
               ? Zeta_Core_BinTree(SNode, GetSize)(&n->sn)
               : 0;
}

void Zeta_Core_LinSpaceAllocator_ExtendL(void* lsa_, size_t cnt) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    if (cnt == 0) { return; }

    ZETA_Core_DebugAssert(cnt <= lsa->beg);

    lsa->beg -= cnt;

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetL)));

    size_t n_size;

    if (GNode_(GetLColor)(&n->gn) == vacant_color) {
        lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&n->sn));

        n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn) + cnt;

        Zeta_Core_BinTree(SNode, SetDiffSize)(&n->sn, cnt);
    } else {
        n = AllocateNode_(lsa);
        n->gn.acc_size = cnt;

        n_size = cnt;

        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, GeneralInsertR)(
            &lsa->gt_root->gn, NULL, &n->gn));
    }

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertR)(
        &lsa->st_root->sn, &FindNode_(lsa, n_size + 1)->sn, &n->sn));
}

void Zeta_Core_LinSpaceAllocator_ExtendR(void* lsa_, size_t cnt) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    if (cnt == 0) { return; }

    ZETA_Core_DebugAssert(cnt <= ZETA_Core_size_max - lsa->end);

    lsa->end += cnt;

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetR)));

    size_t n_size;

    if (GNode_(GetLColor)(&n->gn) == vacant_color) {
        lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&n->sn));

        n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn) + cnt;

        Zeta_Core_BinTree(SNode, SetDiffSize)(&n->sn, cnt);
    } else {
        n = AllocateNode_(lsa);
        n->gn.acc_size = cnt;

        n_size = cnt;

        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, GeneralInsertL)(
            &lsa->gt_root->gn, NULL, &n->gn));
    }

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertL)(
        &lsa->st_root->sn, &FindNode_(lsa, n_size + 1)->sn, &n->sn));
}

bool_t Zeta_Core_LinSpaceAllocator_ShrinkL(void* lsa_, size_t cnt) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetL)));

    if (GNode_(GetLColor)(&n->gn) == occupied_color) { return FALSE; }

    size_t n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn);

    if (n_size < cnt) { return FALSE; }

    lsa->beg += cnt;

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&n->sn));

    if (cnt == n_size) {
        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, Extract)(&n->gn));
        DeallocateNode_(lsa, n);
        return TRUE;
    }

    n_size -= cnt;

    Zeta_Core_BinTree(SNode, SetDiffSize)(&n->sn, -cnt);

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertL)(
        &lsa->st_root->sn, &FindNode_(lsa, n_size + 1)->sn, &n->sn));

    return TRUE;
}

bool_t Zeta_Core_LinSpaceAllocator_ShrinkR(void* lsa_, size_t cnt) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

    Node* n =
        GNodeToNode(Zeta_Core_GetMostLink(&lsa->gt_root->gn, GNode_(GetR)));

    if (GNode_(GetLColor)(&n->gn) == occupied_color) { return FALSE; }

    size_t n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn);

    if (n_size < cnt) { return FALSE; }

    lsa->end -= cnt;

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, Extract)(&n->sn));

    if (cnt == n_size) {
        lsa->gt_root = GNodeToNode(Zeta_Core_RBTree(GNode, Extract)(&n->gn));
        DeallocateNode_(lsa, n);
        return TRUE;
    }

    n_size -= cnt;

    Zeta_Core_BinTree(SNode, SetDiffSize)(&n->sn, -cnt);

    lsa->st_root = SNodeToNode(Zeta_Core_RBTree(SNode, GeneralInsertL)(
        &lsa->st_root->sn, &FindNode_(lsa, n_size + 1)->sn, &n->sn));

    return TRUE;
}

void Zeta_Core_LinSpaceAllocator_Check(void* lsa_) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    ZETA_Core_DebugAssert(lsa != NULL);

    ZETA_Core_DebugAssert(lsa->beg <= lsa->end);

    ZETA_Core_DebugAssert(lsa->node_allocator.vtable != NULL);

    ZETA_Core_DebugAssert(lsa->gt_root != NULL);
}

#if ZETA_Core_EnableDebug

static void SanitizeGT_(Zeta_Core_MemRecorder* dst_gt_table,
                        Zeta_Core_MemRecorder* st_table, Node* n) {
    if (n == NULL) { return; }

    size_t n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn);

    ZETA_Core_DebugAssert(0 < n_size);

    int n_color = GNode_(GetLColor)(&n->gn);

    ZETA_Core_DebugAssert(n_color == vacant_color || n_color == occupied_color);

    Zeta_Core_MemRecorder_Record(dst_gt_table, n, sizeof(Node));

    if (n_color == vacant_color) {
        ZETA_Core_DebugAssert(Zeta_Core_MemRecorder_Unrecord(st_table, n));
    }

    SanitizeGT_(dst_gt_table, st_table, GNodeToNode(GNode_(GetL)(&n->gn)));
    SanitizeGT_(dst_gt_table, st_table, GNodeToNode(GNode_(GetR)(&n->gn)));

    Node* nl = GNodeToNode(Zeta_Core_BinTree(GNode, StepL)(&n->gn));
    Node* nr = GNodeToNode(Zeta_Core_BinTree(GNode, StepR)(&n->gn));

    ZETA_Core_DebugAssert(nl == NULL || n_color == occupied_color ||
                          GNode_(GetLColor)(&nl->gn) == occupied_color);
    ZETA_Core_DebugAssert(nr == NULL || n_color == occupied_color ||
                          GNode_(GetLColor)(&nr->gn) == occupied_color);
}

static void SanitizeST_(Zeta_Core_MemRecorder* dst_st_table, Node* n) {
    if (n == NULL) { return; }

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(GNode_(GetLColor)(&n->gn) == vacant_color);

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemRecorder_Record(dst_st_table, n, sizeof(Node));

    ZETA_Core_PrintCurPos;

    size_t n_size = Zeta_Core_BinTree(SNode, GetSize)(&n->sn);
    ZETA_Core_DebugAssert(0 < n_size);

    ZETA_Core_PrintCurPos;

    SanitizeST_(dst_st_table, SNodeToNode(SNode_(GetL)(&n->sn)));

    ZETA_Core_PrintCurPos;

    SanitizeST_(dst_st_table, SNodeToNode(SNode_(GetR)(&n->sn)));

    Node* nl = SNodeToNode(Zeta_Core_BinTree(SNode, StepL)(&n->sn));
    Node* nr = SNodeToNode(Zeta_Core_BinTree(SNode, StepR)(&n->sn));

    ZETA_Core_DebugAssert(nl == NULL ||
                          Zeta_Core_BinTree(SNode, GetSize)(&nl->sn) <= n_size);
    ZETA_Core_DebugAssert(nr == NULL ||
                          n_size <= Zeta_Core_BinTree(SNode, GetSize)(&nr->sn));
}

#endif

void Zeta_Core_LinSpaceAllocator_Sanitize(void* lsa_,
                                          Zeta_Core_MemRecorder* dst_ns) {
    Zeta_Core_LinSpaceAllocator* lsa = lsa_;
    Check_(lsa);

#if !ZETA_Core_EnableDebug
    ZETA_Core_Unused(dst_ns);
#else
    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(GNode_(GetP)(&lsa->gt_root->gn) == NULL);

    ZETA_Core_DebugAssert(lsa->st_root == NULL ||
                          SNode_(GetP)(&lsa->st_root->sn) == NULL);

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemRecorder* gt_table =
        dst_ns == NULL ? Zeta_Core_MemRecorder_Create() : dst_ns;

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemRecorder_Clear(gt_table);

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemRecorder* st_table = Zeta_Core_MemRecorder_Create();

    ZETA_Core_PrintCurPos;

    Zeta_Core_RBTree(GNode, Sanitize)(NULL, &lsa->gt_root->gn);

    ZETA_Core_PrintCurPos;

    Zeta_Core_RBTree(SNode, Sanitize)(NULL, &lsa->st_root->sn);

    ZETA_Core_PrintCurPos;

    ZETA_Core_DebugAssert(lsa->end - lsa->beg ==
                          GNode_(GetAccSize)(&lsa->gt_root->gn));

    ZETA_Core_PrintCurPos;

    SanitizeST_(st_table, lsa->st_root);

    ZETA_Core_PrintCurPos;

    SanitizeGT_(gt_table, st_table, lsa->gt_root);

    ZETA_Core_DebugAssert(Zeta_Core_MemRecorder_GetSize(st_table) == 0);

    if (dst_ns == NULL) { Zeta_Core_MemRecorder_Destroy(gt_table); }

    Zeta_Core_MemRecorder_Destroy(st_table);
#endif
}
