#pragma once

#include <zeta/core/mem_check_utils.h>

#include <zeta/core/bin_tree.hpp>

namespace zeta::core::rbtree {

constexpr int black{ 0 };
constexpr int red{ 1 };

template <typename BinTreeNode>
BinTreeNode* InsertL(BinTreeNode* pos, BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* InsertR(BinTreeNode* pos, BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* Insert(BinTreeNode* pos_l, BinTreeNode* pos_r, BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* GeneralInsertL(BinTreeNode* root, BinTreeNode* pos,
                            BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* GeneralInsertR(BinTreeNode* root, BinTreeNode* pos,
                            BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* Extract(BinTreeNode* pos);

template <typename BinTreeNode>
void Sanitize(MemRecorder* dst_mr, BinTreeNode* root);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define ZETA_Core_RBTree_InsertBalance_F_(D, E)    \
    BinTreeNode* nu{ ng->Get##E##Ptr() };          \
                                                   \
    if (nu != nullptr && nu->GetPColor() == red) { \
        ng->SetPColor(red);                        \
        np->SetPColor(black);                      \
        nu->SetPColor(black);                      \
        n = ng;                                    \
        continue;                                  \
    }                                              \
                                                   \
    if (np->Get##E##Ptr() == n) {                  \
        bin_tree::Rotate##D(np);                   \
        Swap(n, np);                               \
    }                                              \
                                                   \
    ng->SetPColor(red);                            \
    np->SetPColor(black);                          \
    bin_tree::Rotate##E(ng);                       \
    break;

template <typename BinTreeNode>
BinTreeNode* InsertBalance_(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);

    for (;;) {
        BinTreeNode* np{ n->GetPPtr() };

        if (np == nullptr) {
            n->SetPColor(black);
            break;
        }

        if (np->GetPColor() == black) { break; }

        BinTreeNode* ng{ np->GetPPtr() };

        if (ng == nullptr) {
            np->SetPColor(black);
            break;
        }

        if (ng->GetLPtr() == np) {
            ZETA_Core_RBTree_InsertBalance_F_(L, R);
        } else {
            ZETA_Core_RBTree_InsertBalance_F_(R, L);
        }
    }

    return GetMostLink(n, bin_tree::GetPPtr<BinTreeNode>).first;
}

#define ZETA_Core_RBTree_Insert_(D, E)                                        \
    ZETA_Core_DebugAssert(pos != n);                                          \
    ZETA_Core_DebugAssert(n != nullptr);                                      \
                                                                              \
    ZETA_Core_DebugAssert(n->GetPPtr() == nullptr);                           \
    ZETA_Core_DebugAssert(n->Get##D##Ptr() == nullptr);                       \
    ZETA_Core_DebugAssert(n->Get##E##Ptr() == nullptr);                       \
                                                                              \
    if (pos == nullptr) {                                                     \
        if (n->GetPColor() != black) { n->SetPColor(black); }                 \
        return n;                                                             \
    }                                                                         \
                                                                              \
    if (n->GetPColor() != red) { n->SetPColor(red); }                         \
                                                                              \
    BinTreeNode* pos_d{ pos->Get##D##Ptr() };                                 \
                                                                              \
    if (pos_d == nullptr) {                                                   \
        bin_tree::Attatch##D(pos, n);                                         \
    } else {                                                                  \
        bin_tree::Attatch##E(                                                 \
            GetMostLink(pos_d, bin_tree::Get##E##Ptr<BinTreeNode>).first, n); \
    }                                                                         \
                                                                              \
    return InsertBalance_(n);

template <typename BinTreeNode>
BinTreeNode* InsertL(BinTreeNode* pos, BinTreeNode* n) {
    ZETA_Core_RBTree_Insert_(L, R);
}

template <typename BinTreeNode>
BinTreeNode* InsertR(BinTreeNode* pos, BinTreeNode* n) {
    ZETA_Core_RBTree_Insert_(R, L);
}

template <typename BinTreeNode>
BinTreeNode* Insert(BinTreeNode* pos_l, BinTreeNode* pos_r, BinTreeNode* n) {
    ZETA_Core_DebugAssert(pos_l != n);
    ZETA_Core_DebugAssert(pos_r != n);
    ZETA_Core_DebugAssert(n != nullptr);

    ZETA_Core_DebugAssert(n->GetPPtr() == nullptr);
    ZETA_Core_DebugAssert(n->GetLPtr() == nullptr);
    ZETA_Core_DebugAssert(n->GetRPtr() == nullptr);

    ZETA_Core_DebugAssert(pos_l == nullptr || bin_tree::StepR(pos_l) == pos_r);
    ZETA_Core_DebugAssert(pos_r == nullptr || bin_tree::StepL(pos_r) == pos_l);

    if (pos_l == nullptr && pos_r == nullptr) {
        if (n->GetPColor() != black) { n->SetPColor(black); }

        return n;
    }

    if (n->GetPColor() != red) { n->SetPColor(red); }

    if (pos_l != nullptr && pos_l->GetRPtr() == nullptr) {
        bin_tree::AttatchR(pos_l, n);
    } else {
        bin_tree::AttatchL(pos_r, n);
    }

    return InsertBalance_(n);
}

#define ZETA_Core_RBTree_GeneralInsert_(D, E)                            \
                                                                         \
    if (pos == nullptr) {                                                \
        return GetMostLink(root, bin_tree::Get##E##Ptr<BinTreeNode>)     \
            .first->Insert##E(n);                                        \
    }                                                                    \
                                                                         \
    ZETA_Core_DebugAssert(                                               \
        root == GetMostLink(pos, bin_tree::GetPPtr<BinTreeNode>).first); \
                                                                         \
    return pos->Insert##D(n);

template <typename BinTreeNode>
BinTreeNode* GeneralInsertL(BinTreeNode* root, BinTreeNode* pos,
                            BinTreeNode* n) {
    ZETA_Core_RBTree_GeneralInsert_(L, R);
}

template <typename BinTreeNode>
BinTreeNode* GeneralInsertR(BinTreeNode* root, BinTreeNode* pos,
                            BinTreeNode* n) {
    ZETA_Core_RBTree_GeneralInsert_(R, L);
}

template <typename BinTreeNode>
static void ExtractBalance_(BinTreeNode* n) {
    for (;;) {
        if (n->GetPColor() == red) {
            n->SetPColor(black);
            break;
        }

        BinTreeNode* np{ n->GetPPtr() };
        if (np == nullptr) { break; }

        BinTreeNode* (*GetD)(BinTreeNode* n);
        BinTreeNode* (*GetE)(BinTreeNode* n);
        void (*RotateD)(BinTreeNode* n);
        void (*RotateE)(BinTreeNode* n);

        if (np->GetLPtr() == n) {
            GetD = bin_tree::GetLPtr<BinTreeNode>;
            GetE = bin_tree::GetRPtr<BinTreeNode>;
            RotateD = bin_tree::RotateL<BinTreeNode>;
            RotateE = bin_tree::RotateR<BinTreeNode>;
        } else {
            GetD = bin_tree::GetRPtr<BinTreeNode>;
            GetE = bin_tree::GetLPtr<BinTreeNode>;
            RotateD = bin_tree::RotateR<BinTreeNode>;
            RotateE = bin_tree::RotateL<BinTreeNode>;
        }

        BinTreeNode* ns{ GetE(np) };

        if (ns != nullptr && ns->GetPColor() == red) {
            np->SetPColor(red);
            ns->SetPColor(black);
            RotateD(np);
            ns = GetE(np);
        }

        BinTreeNode* nsd{ GetD(ns) };
        BinTreeNode* nse{ GetE(ns) };

        int nse_color{ nse == nullptr ? black : nse->GetPColor() };

        if ((nsd == nullptr || nsd->GetPColor() == black) &&
            nse_color == black) {
            ns->SetPColor(red);
            n = np;
            continue;
        }

        if (nse_color == black) {
            ns->SetPColor(red);
            nsd->SetPColor(black);
            RotateE(ns);
            nse = ns;
            ns = nsd;
            nsd = GetD(nsd);
        }

        ns->SetPColor(np->GetPColor());
        nse->SetPColor(black);
        np->SetPColor(black);
        RotateD(np);

        break;
    }
}

template <typename BinTreeNode>
BinTreeNode* Extract(BinTreeNode* pos) {
    ZETA_Core_DebugAssert(pos != nullptr);

    BinTreeNode* n{ pos };

    BinTreeNode* root;

    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    if (nl != nullptr && nr != nullptr) {
        BinTreeNode* m{
            GetRandom() % 2 == 0
                ? GetMostLink(nr, bin_tree::GetLPtr<BinTreeNode>).first
                : GetMostLink(nl, bin_tree::GetRPtr<BinTreeNode>).first
        };

        bin_tree::Swap(n, m);

        int nc{ n->GetPColor() };
        int mc{ m->GetPColor() };

        if (nc != mc) {
            n->SetPColor(mc);
            m->SetPColor(nc);
        }

        root = GetMostLink(m, bin_tree::GetPPtr<BinTreeNode>).first;
    } else {
        root = GetMostLink(n, bin_tree::GetPPtr<BinTreeNode>).first;
    }

    if (n->GetPColor() == black) {
        nl = n->GetLPtr();
        nr = n->GetRPtr();

        if (nl != nullptr) {
            bin_tree::RotateR(n);
            nl->SetPColor(black);
        } else if (nr != nullptr) {
            bin_tree::RotateL(n);
            nr->SetPColor(black);
        } else {
            ExtractBalance_(n);
        }
    }

    root = GetMostLink(root, bin_tree::GetPPtr<BinTreeNode>).first;
    if (root == n) { return nullptr; }

    bin_tree::Detach(n);
    return root;
}

template <typename BinTreeNode>
size_t Sanitize_(MemRecorder* dst_mr, BinTreeNode* n) {
    if (n == nullptr) { return 0; }

    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    if (nl != nullptr) { ZETA_Core_DebugAssert(nl->GetPPtr() == n); }
    if (nr != nullptr) { ZETA_Core_DebugAssert(nr->GetPPtr() == n); }

    size_t lbh{ Sanitize_(dst_mr, nl) };

    if (dst_mr != nullptr) { dst_mr->Record(n, sizeof(void*)); }

    size_t rbh{ Sanitize_(dst_mr, nr) };

    ZETA_Core_DebugAssert(lbh == rbh);

    int nc{ n->GetPColor() };

    ZETA_Core_DebugAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugAssert(nl == nullptr || nl->GetPColor() == black);
    ZETA_Core_DebugAssert(nr == nullptr || nr->GetPColor() == black);

    return lbh;
}

template <typename BinTreeNode>
void Sanitize(MemRecorder* dst_mr, BinTreeNode* root) {
    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(root->GetPPtr() == nullptr);
    ZETA_Core_DebugAssert(root->GetPColor() == black);

    Sanitize_(dst_mr, root);
}

}  // namespace zeta::core::rbtree
