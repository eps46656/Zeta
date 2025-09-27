#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/rbtree.hpp>

namespace zeta::core::rbtree {

template <typename RBTreeNodeOperator, typename RBTreeNode>
void CheckRBTreeNodeOperator(RBTreeNodeOperator const& rbtn_opr) {
    bin_tree::CheckBinTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(
        rbtn_opr);

    RBTreeNode* n{ nullptr };

    ZETA_Core_StaticAssert(IsSame<decltype(rbtn_opr.GetColor(n)), int>);

    ZETA_Core_StaticAssert(
        IsSame<decltype((rbtn_opr.SetColor(n, size_t{ 0 }), 0)), int>);
}

// -----------------------------------------------------------------------------

#pragma push_macro("InsertBalance__")

#define InsertBalance__(D, E)                            \
    RBTreeNode* nu{ rbtn_opr.Get##E(ng) };               \
                                                         \
    if (nu != nullptr && rbtn_opr.GetColor(nu) == red) { \
        rbtn_opr.SetColor(ng, red);                      \
        rbtn_opr.SetColor(np, black);                    \
        rbtn_opr.SetColor(nu, black);                    \
        n = ng;                                          \
        continue;                                        \
    }                                                    \
                                                         \
    if (rbtn_opr.Get##E(np) == n) {                      \
        bin_tree::Rotate##D(rbtn_opr, np);               \
        Swap(n, np);                                     \
    }                                                    \
                                                         \
    rbtn_opr.SetColor(ng, red);                          \
    rbtn_opr.SetColor(np, black);                        \
    bin_tree::Rotate##E(rbtn_opr, ng);                   \
    break;

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* InsertBalance_(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* n) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    ZETA_Core_DebugAssert(n != nullptr);

    for (;;) {
        RBTreeNode* np{ rbtn_opr.GetP(n) };

        if (np == nullptr) {
            rbtn_opr.SetColor(n, black);
            break;
        }

        if (rbtn_opr.GetColor(np) == black) { break; }

        RBTreeNode* ng{ rbtn_opr.GetP(np) };

        if (ng == nullptr) {
            rbtn_opr.SetColor(np, black);
            break;
        }

        if (rbtn_opr.GetL(ng) == np) {
            InsertBalance__(L, R);
        } else {
            InsertBalance__(R, L);
        }
    }

    return bin_tree::GetMostP(rbtn_opr, n).first;
}

#pragma pop_macro("InsertBalance__")

#pragma push_macro("Insert_")

#define Insert_(D, E)                                                         \
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);        \
                                                                              \
    ZETA_Core_DebugAssert(pos != n);                                          \
    ZETA_Core_DebugAssert(n != nullptr);                                      \
                                                                              \
    ZETA_Core_DebugAssert(rbtn_opr.GetP(n) == nullptr);                       \
    ZETA_Core_DebugAssert(rbtn_opr.Get##D(n) == nullptr);                     \
    ZETA_Core_DebugAssert(rbtn_opr.Get##E(n) == nullptr);                     \
                                                                              \
    if (pos == nullptr) {                                                     \
        if (rbtn_opr.GetColor(n) != black) { rbtn_opr.SetColor(n, black); }   \
        return n;                                                             \
    }                                                                         \
                                                                              \
    if (rbtn_opr.GetColor(n) != red) { rbtn_opr.SetColor(n, red); }           \
                                                                              \
    RBTreeNode* pos_d{ rbtn_opr.Get##D(pos) };                                \
                                                                              \
    if (pos_d == nullptr) {                                                   \
        bin_tree::Attatch##D(rbtn_opr, pos, n);                               \
    } else {                                                                  \
        bin_tree::Attatch##E(rbtn_opr,                                        \
                             bin_tree::GetMost##E(rbtn_opr, pos_d).first, n); \
    }                                                                         \
                                                                              \
    return InsertBalance_(rbtn_opr, n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* InsertL(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos,
                    RBTreeNode* n) {
    Insert_(L, R);
}

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* InsertR(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos,
                    RBTreeNode* n) {
    Insert_(R, L);
}

#pragma pop_macro("Insert_")

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* Insert(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos_l,
                   RBTreeNode* pos_r, RBTreeNode* n) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    ZETA_Core_DebugAssert(pos_l != n);
    ZETA_Core_DebugAssert(pos_r != n);
    ZETA_Core_DebugAssert(n != nullptr);

    ZETA_Core_DebugAssert(rbtn_opr.GetP(n) == nullptr);
    ZETA_Core_DebugAssert(rbtn_opr.GetL(n) == nullptr);
    ZETA_Core_DebugAssert(rbtn_opr.GetR(n) == nullptr);

    ZETA_Core_DebugAssert(pos_l == nullptr ||
                          bin_tree::StepR(rbtn_opr, pos_l) == pos_r);
    ZETA_Core_DebugAssert(pos_r == nullptr ||
                          bin_tree::StepL(rbtn_opr, pos_r) == pos_l);

    if (pos_l == nullptr && pos_r == nullptr) {
        if (rbtn_opr.GetColor(n) != black) { rbtn_opr.SetColor(n, black); }

        return n;
    }

    if (rbtn_opr.GetColor(n) != red) { rbtn_opr.SetColor(n, red); }

    if (pos_l != nullptr && rbtn_opr.GetR(pos_l) == nullptr) {
        bin_tree::AttatchR(rbtn_opr, pos_l, n);
    } else {
        bin_tree::AttatchL(rbtn_opr, pos_r, n);
    }

    return InsertBalance_(rbtn_opr, n);
}

#pragma push_macro("GeneralInsert_")

#define GeneralInsert_(D, E)                                                \
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);      \
                                                                            \
    if (pos == nullptr) {                                                   \
        return Insert##E(bin_tree::GetMost##E(rbtn_opr, root).first, n);    \
    }                                                                       \
                                                                            \
    ZETA_Core_DebugAssert(root == bin_tree::GetMostP(rbtn_opr, pos).first); \
                                                                            \
    return Insert##D(rbtn_opr, pos, n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* GeneralInsertL(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* root,
                           RBTreeNode* pos, RBTreeNode* n) {
    GeneralInsert_(L, R);
}

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* GeneralInsertR(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* root,
                           RBTreeNode* pos, RBTreeNode* n) {
    GeneralInsert_(R, L);
}

#pragma pop_macro("GeneralInsert_")

#pragma push_macro("ExtractBalance__")

#define ExtractBalance__(D, E)                                        \
    RBTreeNode* ns{ rbtn_opr.Get##E(np) };                            \
                                                                      \
    if (ns != nullptr && rbtn_opr.GetColor(ns) == red) {              \
        rbtn_opr.SetColor(np, red);                                   \
        rbtn_opr.SetColor(ns, black);                                 \
        bin_tree::Rotate##D(rbtn_opr, np);                            \
        ns = rbtn_opr.Get##E(np);                                     \
    }                                                                 \
                                                                      \
    RBTreeNode* nsd{ rbtn_opr.Get##D(ns) };                           \
    RBTreeNode* nse{ rbtn_opr.Get##E(ns) };                           \
                                                                      \
    int nse_color{ nse == nullptr ? black : rbtn_opr.GetColor(nse) }; \
                                                                      \
    if ((nsd == nullptr || rbtn_opr.GetColor(nsd) == black) &&        \
        nse_color == black) {                                         \
        rbtn_opr.SetColor(ns, red);                                   \
        n = np;                                                       \
        continue;                                                     \
    }                                                                 \
                                                                      \
    if (nse_color == black) {                                         \
        rbtn_opr.SetColor(ns, red);                                   \
        rbtn_opr.SetColor(nsd, black);                                \
        bin_tree::Rotate##E(rbtn_opr, ns);                            \
        nse = ns;                                                     \
        ns = nsd;                                                     \
        nsd = rbtn_opr.Get##D(nsd);                                   \
    }                                                                 \
                                                                      \
    rbtn_opr.SetColor(ns, rbtn_opr.GetColor(np));                     \
    rbtn_opr.SetColor(nse, black);                                    \
    rbtn_opr.SetColor(np, black);                                     \
    bin_tree::Rotate##D(rbtn_opr, np);                                \
                                                                      \
    break;

template <typename RBTreeNodeOperator, typename RBTreeNode>
void ExtractBalance_(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* n) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    for (;;) {
        if (rbtn_opr.GetColor(n) == red) {
            rbtn_opr.SetColor(n, black);
            break;
        }

        RBTreeNode* np{ rbtn_opr.GetP(n) };

        if (np == nullptr) { break; }

        if (rbtn_opr.GetL(np) == n) {
            ExtractBalance__(L, R);
        } else {
            ExtractBalance__(R, L);
        }
    }
}

#pragma pop_macro("ExtractBalance__")

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* Extract(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    ZETA_Core_DebugAssert(pos != nullptr);

    RBTreeNode* n{ pos };

    RBTreeNode* root;

    RBTreeNode* nl{ rbtn_opr.GetL(n) };
    RBTreeNode* nr{ rbtn_opr.GetR(n) };

    if (nl == nullptr || nr == nullptr) {
        root = bin_tree::GetMostP(rbtn_opr, n).first;
    } else {
        RBTreeNode* m{ GetRandom() % 2 == 0
                           ? bin_tree::GetMostL(rbtn_opr, nr).first
                           : bin_tree::GetMostR(rbtn_opr, nl).first };

        bin_tree::Swap(rbtn_opr, n, m);

        int nc{ rbtn_opr.GetColor(n) };
        int mc{ rbtn_opr.GetColor(m) };

        if (nc != mc) {
            rbtn_opr.SetColor(n, mc);
            rbtn_opr.SetColor(m, nc);
        }

        root = bin_tree::GetMostP(rbtn_opr, m).first;
    }

    if (rbtn_opr.GetColor(n) == black) {
        nl = rbtn_opr.GetL(n);
        nr = rbtn_opr.GetR(n);

        if (nl != nullptr) {
            bin_tree::RotateR(rbtn_opr, n);
            rbtn_opr.SetColor(nl, black);
        } else if (nr != nullptr) {
            bin_tree::RotateL(rbtn_opr, n);
            rbtn_opr.SetColor(nr, black);
        } else {
            ExtractBalance_(rbtn_opr, n);
        }
    }

    root = bin_tree::GetMostP(rbtn_opr, root).first;

    if (root == n) { return nullptr; }

    bin_tree::Detach(rbtn_opr, n);

    return root;
}

template <typename RBTreeNodeOperator, typename RBTreeNode>
size_t Sanitize_(RBTreeNodeOperator const& rbtn_opr, MemRecorder* dst_mr,
                 RBTreeNode* n) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    if (n == nullptr) { return 0; }

    RBTreeNode* nl{ rbtn_opr.GetL(n) };
    RBTreeNode* nr{ rbtn_opr.GetR(n) };

    if (nl != nullptr) { ZETA_Core_DebugAssert(rbtn_opr.GetP(nl) == n); }
    if (nr != nullptr) { ZETA_Core_DebugAssert(rbtn_opr.GetP(nr) == n); }

    size_t lbh{ Sanitize_(rbtn_opr, dst_mr, nl) };

    if (dst_mr != nullptr) { MemRecorder::Record(dst_mr, n, sizeof(void*)); }

    size_t rbh{ Sanitize_(rbtn_opr, dst_mr, nr) };

    ZETA_Core_DebugAssert(lbh == rbh);

    int nc{ rbtn_opr.GetColor(n) };

    ZETA_Core_DebugAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugAssert(nl == nullptr || rbtn_opr.GetColor(nl) == black);
    ZETA_Core_DebugAssert(nr == nullptr || rbtn_opr.GetColor(nr) == black);

    return lbh;
}

template <typename RBTreeNodeOperator, typename RBTreeNode>
void Sanitize(RBTreeNodeOperator const& rbtn_opr, MemRecorder* dst_mr,
              RBTreeNode* root) {
    CheckRBTreeNodeOperator<RBTreeNodeOperator, RBTreeNode>(rbtn_opr);

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(rbtn_opr.GetP(root) == nullptr);
    ZETA_Core_DebugAssert(rbtn_opr.GetColor(root) == black);

    Sanitize_(rbtn_opr, dst_mr, root);
}

}  // namespace zeta::core::rbtree
