

#include <zeta/core/bin_tree_node.h>
#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/rbtree_temp.h>
#include <zeta/core/utils.h>

ZETA_Core_ExternC_Beg;

#if !defined(TreeNode)
#error "TreeNode is not defined."
#endif

#pragma push_macro("TreeNode_")
#pragma push_macro("black")
#pragma push_macro("red")
#pragma push_macro("Insert_")
#pragma push_macro("F_")
#pragma push_macro("GeneralInsert_")

#define TreeNode_(x) ZETA_Core_Concat(TreeNode, _, x)

#define black (0)
#define red (1)

#define Insert_(D, E)                                                     \
    ZETA_Core_DebugAssert(pos != n);                                      \
    ZETA_Core_DebugAssert(n != NULL);                                     \
                                                                          \
    ZETA_Core_DebugAssert(TreeNode_(GetP)(n) == NULL);                    \
    ZETA_Core_DebugAssert(TreeNode_(Get##D)(n) == NULL);                  \
    ZETA_Core_DebugAssert(TreeNode_(Get##E)(n) == NULL);                  \
                                                                          \
    if (pos == NULL) {                                                    \
        if (TreeNode_(GetPColor)(n) != black) {                           \
            TreeNode_(SetPColor)(n, black);                               \
        }                                                                 \
        return n;                                                         \
    }                                                                     \
                                                                          \
    if (TreeNode_(GetPColor)(n) != red) { TreeNode_(SetPColor)(n, red); } \
                                                                          \
    void* pos_d = TreeNode_(Get##D)(pos);                                 \
                                                                          \
    if (pos_d == NULL) {                                                  \
        Zeta_Core_BinTree(TreeNode, Attatch##D)(pos, n);                  \
    } else {                                                              \
        Zeta_Core_BinTree(TreeNode, Attatch##E)(                          \
            Zeta_Core_GetMostLink(pos_d, TreeNode_(Get##E)), n);          \
    }                                                                     \
                                                                          \
    return Zeta_Core_RBTree(TreeNode, InsertBalance_)(n);

#define F_(D, E)                                         \
    void* nu = TreeNode_(Get##E)(ng);                    \
                                                         \
    if (nu != NULL && TreeNode_(GetPColor)(nu) == red) { \
        TreeNode_(SetPColor)(ng, red);                   \
        TreeNode_(SetPColor)(np, black);                 \
        TreeNode_(SetPColor)(nu, black);                 \
        n = ng;                                          \
        continue;                                        \
    }                                                    \
                                                         \
    if (TreeNode_(Get##E)(np) == n) {                    \
        Zeta_Core_BinTree(TreeNode, Rotate##D)(np);      \
        ZETA_Core_Swap(n, np);                           \
    }                                                    \
                                                         \
    TreeNode_(SetPColor)(ng, red);                       \
    TreeNode_(SetPColor)(np, black);                     \
    Zeta_Core_BinTree(TreeNode, Rotate##E)(ng);          \
    break;

static void* Zeta_Core_RBTree(TreeNode, InsertBalance_)(void* n) {
    ZETA_Core_DebugAssert(n != NULL);

    for (;;) {
        void* np = TreeNode_(GetP)(n);

        if (np == NULL) {
            TreeNode_(SetPColor)(n, black);
            break;
        }

        if (TreeNode_(GetPColor)(np) == black) { break; }

        void* ng = TreeNode_(GetP)(np);

        if (ng == NULL) {
            TreeNode_(SetPColor)(np, black);
            break;
        }

        if (TreeNode_(GetL)(ng) == np) {
            F_(L, R);
        } else {
            F_(R, L);
        }
    }

    return Zeta_Core_GetMostLink(n, TreeNode_(GetP));
}

void* Zeta_Core_RBTree(TreeNode, InsertL)(void* pos, void* n) { Insert_(L, R); }

void* Zeta_Core_RBTree(TreeNode, InsertR)(void* pos, void* n) { Insert_(R, L); }

void* Zeta_Core_RBTree(TreeNode, Insert)(void* pos_l, void* pos_r, void* n) {
    ZETA_Core_DebugAssert(pos_l != n);
    ZETA_Core_DebugAssert(pos_r != n);
    ZETA_Core_DebugAssert(n != NULL);

    ZETA_Core_DebugAssert(TreeNode_(GetP)(n) == NULL);
    ZETA_Core_DebugAssert(TreeNode_(GetL)(n) == NULL);
    ZETA_Core_DebugAssert(TreeNode_(GetR)(n) == NULL);

    ZETA_Core_DebugAssert(pos_l == NULL ||
                          Zeta_Core_BinTree(TreeNode, StepR)(pos_l) == pos_r);
    ZETA_Core_DebugAssert(pos_r == NULL ||
                          Zeta_Core_BinTree(TreeNode, StepL)(pos_r) == pos_l);

    if (pos_l == NULL && pos_r == NULL) {
        if (TreeNode_(GetPColor)(n) != black) {
            TreeNode_(SetPColor)(n, black);
        }

        return n;
    }

    if (TreeNode_(GetPColor)(n) != red) { TreeNode_(SetPColor)(n, red); }

    if (pos_l != NULL && TreeNode_(GetR)(pos_l) == NULL) {
        Zeta_Core_BinTree(TreeNode, AttatchR)(pos_l, n);
    } else {
        Zeta_Core_BinTree(TreeNode, AttatchL)(pos_r, n);
    }

    return Zeta_Core_RBTree(TreeNode, InsertBalance_)(n);
}

#define GeneralInsert_(D, E)                                            \
                                                                        \
    if (pos == NULL) {                                                  \
        return Zeta_Core_RBTree(TreeNode, Insert##E)(                   \
            Zeta_Core_GetMostLink(root, TreeNode_(Get##E)), n);         \
    }                                                                   \
                                                                        \
    ZETA_Core_DebugAssert(root ==                                       \
                          Zeta_Core_GetMostLink(TreeNode_(GetP), pos)); \
                                                                        \
    return Zeta_Core_RBTree(TreeNode, Insert##D)(pos, n);

void* Zeta_Core_RBTree(TreeNode, GeneralInsertL)(void* root, void* pos,
                                                 void* n) {
    GeneralInsert_(L, R);
}

void* Zeta_Core_RBTree(TreeNode, GeneralInsertR)(void* root, void* pos,
                                                 void* n) {
    GeneralInsert_(R, L);
}

static void Zeta_Core_RBTree(TreeNode, ExtractBalance_)(void* n) {
    for (;;) {
        if (TreeNode_(GetPColor)(n) == red) {
            TreeNode_(SetPColor)(n, black);
            break;
        }

        void* np = TreeNode_(GetP)(n);
        if (np == NULL) { break; }

        void* (*GetD)(void* n);
        void* (*GetE)(void* n);
        void (*RotateD)(void* n);
        void (*RotateE)(void* n);

        if (TreeNode_(GetL)(np) == n) {
            GetD = TreeNode_(GetL);
            GetE = TreeNode_(GetR);
            RotateD = Zeta_Core_BinTree(TreeNode, RotateL);
            RotateE = Zeta_Core_BinTree(TreeNode, RotateR);
        } else {
            GetD = TreeNode_(GetR);
            GetE = TreeNode_(GetL);
            RotateD = Zeta_Core_BinTree(TreeNode, RotateR);
            RotateE = Zeta_Core_BinTree(TreeNode, RotateL);
        }

        void* ns = GetE(np);

        if (ns != NULL && TreeNode_(GetPColor)(ns) == red) {
            TreeNode_(SetPColor)(np, red);
            TreeNode_(SetPColor)(ns, black);
            RotateD(np);
            ns = GetE(np);
        }

        void* nsd = GetD(ns);
        void* nse = GetE(ns);

        int nse_color = nse == NULL ? black : TreeNode_(GetPColor)(nse);

        if ((nsd == NULL || TreeNode_(GetPColor)(nsd) == black) &&
            nse_color == black) {
            TreeNode_(SetPColor)(ns, red);
            n = np;
            continue;
        }

        if (nse_color == black) {
            TreeNode_(SetPColor)(ns, red);
            TreeNode_(SetPColor)(nsd, black);
            RotateE(ns);
            nse = ns;
            ns = nsd;
            nsd = GetD(nsd);
        }

        TreeNode_(SetPColor)(ns, TreeNode_(GetPColor)(np));
        TreeNode_(SetPColor)(nse, black);
        TreeNode_(SetPColor)(np, black);
        RotateD(np);

        break;
    }
}

void* Zeta_Core_RBTree(TreeNode, Extract)(void* pos) {
    ZETA_Core_DebugAssert(pos != NULL);

    void* n = pos;

    void* root;

    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (nl != NULL && nr != NULL) {
        void* m = Zeta_Core_GetRandom() % 2 == 0
                      ? Zeta_Core_GetMostLink(nr, TreeNode_(GetL))
                      : Zeta_Core_GetMostLink(nl, TreeNode_(GetR));

        Zeta_Core_BinTree(TreeNode, Swap)(n, m);

        int nc = TreeNode_(GetPColor)(n);
        int mc = TreeNode_(GetPColor)(m);

        if (nc != mc) {
            TreeNode_(SetPColor)(n, mc);
            TreeNode_(SetPColor)(m, nc);
        }

        root = Zeta_Core_GetMostLink(m, TreeNode_(GetP));
    } else {
        root = Zeta_Core_GetMostLink(n, TreeNode_(GetP));
    }

    if (TreeNode_(GetPColor)(n) == black) {
        nl = TreeNode_(GetL)(n);
        nr = TreeNode_(GetR)(n);

        if (nl != NULL) {
            Zeta_Core_BinTree(TreeNode, RotateR)(n);
            TreeNode_(SetPColor)(nl, black);
        } else if (nr != NULL) {
            Zeta_Core_BinTree(TreeNode, RotateL)(n);
            TreeNode_(SetPColor)(nr, black);
        } else {
            Zeta_Core_RBTree(TreeNode, ExtractBalance_)(n);
        }
    }

    root = Zeta_Core_GetMostLink(root, TreeNode_(GetP));
    if (root == n) { return NULL; }

    Zeta_Core_BinTree(TreeNode, Detach)(n);
    return root;
}

static size_t Zeta_Core_RBTree(TreeNode,
                               Sanitize_)(Zeta_Core_MemRecorder* dst_mr,
                                          void* n) {
    if (n == NULL) { return 0; }

    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (nl != NULL) { ZETA_Core_DebugAssert(TreeNode_(GetP)(nl) == n); }
    if (nr != NULL) { ZETA_Core_DebugAssert(TreeNode_(GetP)(nr) == n); }

    size_t lbh = Zeta_Core_RBTree(TreeNode, Sanitize_)(dst_mr, nl);

    if (dst_mr != NULL) {
        Zeta_Core_MemRecorder_Record(dst_mr, n, sizeof(void*));
    }

    size_t rbh = Zeta_Core_RBTree(TreeNode, Sanitize_)(dst_mr, nr);

    ZETA_Core_DebugAssert(lbh == rbh);

    int nc = TreeNode_(GetPColor)(n);

    ZETA_Core_DebugAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugAssert(nl == NULL || TreeNode_(GetPColor)(nl) == black);
    ZETA_Core_DebugAssert(nr == NULL || TreeNode_(GetPColor)(nr) == black);

    return lbh;
}

void Zeta_Core_RBTree(TreeNode, Sanitize)(Zeta_Core_MemRecorder* dst_mr,
                                          void* root) {
    if (root == NULL) { return; }

    ZETA_Core_DebugAssert(TreeNode_(GetP)(root) == NULL);
    ZETA_Core_DebugAssert(TreeNode_(GetPColor)(root) == black);

    Zeta_Core_RBTree(TreeNode, Sanitize_)(dst_mr, root);
}

#pragma pop_macro("TreeNode_")
#pragma pop_macro("black")
#pragma pop_macro("red")
#pragma pop_macro("Insert_")
#pragma pop_macro("F_")
#pragma pop_macro("GeneralInsert_")

ZETA_Core_ExternC_End;
