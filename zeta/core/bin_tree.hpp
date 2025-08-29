#pragma once

#include <zeta/core/utils.hpp>

namespace zeta::core::bin_tree {

template <typename BinTreeNode>
BinTreeNode* GetPPtr(BinTreeNode* n);
template <typename BinTreeNode>
BinTreeNode* GetLPtr(BinTreeNode* n);
template <typename BinTreeNode>
BinTreeNode* GetRPtr(BinTreeNode* n);

template <typename BinTreeNode>
size_t Count(BinTreeNode* n);

template <typename BinTreeNode>
size_t GetSize(BinTreeNode* n);

template <typename BinTreeNode>
void SetSize(BinTreeNode* n, size_t size);

template <typename BinTreeNode>
void AddDiffSize(BinTreeNode* n, size_t diff_size);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename BinTreeNode>
BinTreeNode* GetPPtr(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);
    return n->GetPPtr();
}

template <typename BinTreeNode>
BinTreeNode* GetLPtr(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);
    return n->GetLPtr();
}

template <typename BinTreeNode>
BinTreeNode* GetRPtr(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);
    return n->GetRPtr();
}

template <typename BinTreeNode>
size_t Count(BinTreeNode* n) {
    return n == nullptr ? 0 : Count(n->GetLPtr()) + 1 + Count(n->GetRPtr());
}

template <typename BinTreeNode>
size_t GetSize(BinTreeNode* n) {
    if (n == nullptr) { return BinTreeNode::null_acc_size; }

    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    return n->GetAccSize() -
           (nl == nullptr ? BinTreeNode::null_acc_size : nl->GetAccSize()) -
           (nr == nullptr ? BinTreeNode::null_acc_size : nr->GetAccSize());
}

template <typename BinTreeNode>
void SetSize(BinTreeNode* n, size_t size) {
    ZETA_Core_DebugAssert(n != nullptr);
    AddDiffSize(n, size - GetSize(n));
}

template <typename BinTreeNode>
void AddDiffSize(BinTreeNode* n, size_t diff_size) {
    if (diff_size == 0) { return; }

    for (; n != nullptr; n = n->GetPPtr()) {
        n->SetAccSize(n->GetAccSize() + diff_size);
    }
}

#define ZETA_Core_BinTree_Attatch_(D)                                        \
                                                                             \
    ZETA_Core_DebugAssert(pos != nullptr);                                   \
                                                                             \
    ZETA_Core_DebugAssert(pos->Get##D##Ptr() == nullptr);                    \
    ZETA_Core_DebugAssert(n == nullptr || n->GetPPtr() == nullptr);          \
                                                                             \
    if (n == nullptr) { return; }                                            \
                                                                             \
    pos->Set##D##Ptr(n);                                                     \
    n->SetPPtr(pos);                                                         \
                                                                             \
    if constexpr (BinTreeNode::has_acc_size) {                               \
        bin_tree::AddDiffSize(pos,                                           \
                              n->GetAccSize() - BinTreeNode::null_acc_size); \
    }

template <typename BinTreeNode>
void AttatchL(BinTreeNode* pos, BinTreeNode* n) {
    ZETA_Core_BinTree_Attatch_(L);
}

template <typename BinTreeNode>
void AttatchR(BinTreeNode* pos, BinTreeNode* n) {
    ZETA_Core_BinTree_Attatch_(R);
}

template <typename BinTreeNode>
void Detach(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);

    BinTreeNode* np{ n->GetPPtr() };

    if (np == nullptr) { return; }

    if (np->GetLPtr() == n) {
        np->SetLPtr(nullptr);
    } else {
        np->SetRPtr(nullptr);
    }

    n->SetPPtr(nullptr);

    if constexpr (BinTreeNode::has_acc_size) {
        bin_tree::AddDiffSize(np, BinTreeNode::null_acc_size - n->GetAccSize());
    }
}

template <typename BinTreeNode>
void Swap(BinTreeNode* n, BinTreeNode* m) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    if (n == m) { return; }

    BinTreeNode* np{ n->GetPPtr() };
    BinTreeNode* mp{ m->GetPPtr() };

    if (np == m) {
        Swap(n, m);
        Swap(np, mp);
    }

    size_t n_acc_size;
    size_t m_acc_size;

    size_t n_size;
    size_t m_size;

    ZETA_Core_Unused(n_acc_size);
    ZETA_Core_Unused(m_acc_size);

    ZETA_Core_Unused(n_size);
    ZETA_Core_Unused(m_size);

    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    BinTreeNode* ml{ m->GetLPtr() };
    BinTreeNode* mr{ m->GetRPtr() };

    if constexpr (BinTreeNode::has_acc_size) {
        n_acc_size = n->GetAccSize();
        m_acc_size = m->GetAccSize();

        n_size =
            n_acc_size -
            (nl == nullptr ? BinTreeNode::null_acc_size : nl->GetAccSize()) -
            (nr == nullptr ? BinTreeNode::null_acc_size : nr->GetAccSize());

        m_size =
            m_acc_size -
            (ml == nullptr ? BinTreeNode::null_acc_size : ml->GetAccSize()) -
            (mr == nullptr ? BinTreeNode::null_acc_size : mr->GetAccSize());
    }

    if (mp == n) {
        if (np == nullptr) {
        } else if (np->GetLPtr() == n) {
            np->SetLPtr(m);
        } else {
            np->SetRPtr(m);
        }

        m->SetPPtr(np);

        if (n->GetLPtr() == m) {
            m->SetLPtr(n);

            m->SetRPtr(nr);
            if (nr != nullptr) { nr->SetPPtr(m); }
        } else {
            m->SetLPtr(nl);
            if (nl != nullptr) { nl->SetPPtr(m); }

            m->SetRPtr(n);
        }

        n->SetPPtr(m);

        n->SetLPtr(ml);
        if (ml != nullptr) { ml->SetPPtr(n); }

        n->SetRPtr(mr);
        if (mr != nullptr) { mr->SetPPtr(n); }
    } else {
        if (np == nullptr) {
        } else if (np->GetLPtr() == n) {
            np->SetLPtr(m);
        } else {
            np->SetRPtr(m);
        }

        m->SetPPtr(np);

        m->SetLPtr(nl);
        if (nl != nullptr) { nl->SetPPtr(m); }

        m->SetRPtr(nr);
        if (nr != nullptr) { nr->SetPPtr(m); }

        if (mp == nullptr) {
        } else if (mp->GetLPtr() == m) {
            mp->SetLPtr(n);
        } else {
            mp->SetRPtr(n);
        }

        n->SetPPtr(mp);

        n->SetLPtr(ml);
        if (ml != nullptr) { ml->SetPPtr(n); }

        n->SetRPtr(mr);
        if (mr != nullptr) { mr->SetPPtr(n); }
    }

    if constexpr (BinTreeNode::has_acc_size) {
        n->SetAccSize(m_acc_size);
        m->SetAccSize(n_acc_size);

        bin_tree::AddDiffSize(n, n_size - m_size);
        bin_tree::AddDiffSize(m, m_size - n_size);
    }
}

#define ZETA_Core_BinTree_Rotate_(D, E)                                  \
                                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                                 \
                                                                         \
    BinTreeNode* ne{ n->Get##E##Ptr() };                                 \
    ZETA_Core_DebugAssert(ne != nullptr);                                \
                                                                         \
    BinTreeNode* ned{ ne->Get##D##Ptr() };                               \
    BinTreeNode* np{ n->GetPPtr() };                                     \
                                                                         \
    if (np == nullptr) {                                                 \
    } else if (np->Get##D##Ptr() == n) {                                 \
        np->Set##D##Ptr(ne);                                             \
    } else {                                                             \
        np->Set##E##Ptr(ne);                                             \
    }                                                                    \
                                                                         \
    ne->SetPPtr(np);                                                     \
                                                                         \
    ne->Set##D##Ptr(n);                                                  \
    n->SetPPtr(ne);                                                      \
                                                                         \
    n->Set##E##Ptr(ned);                                                 \
    if (ned != nullptr) { ned->SetPPtr(n); }                             \
                                                                         \
    if constexpr (BinTreeNode::has_acc_size) {                           \
        size_t n_acc_size{ n->GetAccSize() };                            \
        size_t ne_acc_size{ ne->GetAccSize() };                          \
        size_t ned_acc_size{ ned == nullptr ? BinTreeNode::null_acc_size \
                                            : ned->GetAccSize() };       \
                                                                         \
        n->SetAccSize(n_acc_size - ne_acc_size + ned_acc_size);          \
                                                                         \
        ne->SetAccSize(n_acc_size);                                      \
    };

template <typename BinTreeNode>
void RotateL(BinTreeNode* n) {
    ZETA_Core_BinTree_Rotate_(L, R);
}

template <typename BinTreeNode>
void RotateR(BinTreeNode* n) {
    ZETA_Core_BinTree_Rotate_(R, L);
}

#define ZETA_Core_BinTree_StepP_(D)                \
                                                   \
    ZETA_Core_DebugAssert(n != nullptr);           \
                                                   \
    for (;;) {                                     \
        BinTreeNode* np{ n->GetPPtr() };           \
        if (np == nullptr) { return nullptr; }     \
        if (np->Get##E##Ptr() == n) { return np; } \
        n = np;                                    \
    }

template <typename BinTreeNode>
BinTreeNode* StepPL(BinTreeNode* n) {
    ZETA_Core_BinTree_StepP_(L);
}

template <typename BinTreeNode>
BinTreeNode* StepPR(BinTreeNode* n) {
    ZETA_Core_BinTree_StepP_(R);
}

#define ZETA_Core_BinTree_Step_(D, E)                           \
                                                                \
    ZETA_Core_DebugAssert(n != nullptr);                        \
                                                                \
    BinTreeNode* nd{ n->Get##D##Ptr() };                        \
                                                                \
    if (nd != nullptr) {                                        \
        return GetMostLink(nd, Get##E##Ptr<BinTreeNode>).first; \
    }                                                           \
                                                                \
    for (;;) {                                                  \
        BinTreeNode* np{ n->GetPPtr() };                        \
        if (np == nullptr) { return nullptr; }                  \
        if (np->Get##E##Ptr() == n) { return np; }              \
        n = np;                                                 \
    }

template <typename BinTreeNode>
BinTreeNode* StepL(BinTreeNode* n) {
    ZETA_Core_BinTree_Step_(L, R);
}

template <typename BinTreeNode>
BinTreeNode* StepR(BinTreeNode* n) {
    ZETA_Core_BinTree_Step_(R, L);
}

#define ZETA_Core_BinTree_Access_(D, E)                                \
    size_t n_acc_size{ n == nullptr ? BinTreeNode::null_acc_size       \
                                    : n->GetAccSize() };               \
                                                                       \
    if (n_acc_size <= idx) { return { nullptr, idx - n_acc_size }; }   \
                                                                       \
    while (n != nullptr) {                                             \
        BinTreeNode* nd{ n->Get##D##Ptr() };                           \
        size_t nd_acc_size{ nd == nullptr ? BinTreeNode::null_acc_size \
                                          : nd->GetAccSize() };        \
                                                                       \
        if (idx < nd_acc_size) {                                       \
            n = nd;                                                    \
            n_acc_size = nd_acc_size;                                  \
            continue;                                                  \
        }                                                              \
                                                                       \
        BinTreeNode* ne{ n->Get##E##Ptr() };                           \
        size_t ne_acc_size{ ne == nullptr ? BinTreeNode::null_acc_size \
                                          : ne->GetAccSize() };        \
                                                                       \
        idx -= nd_acc_size;                                            \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };       \
                                                                       \
        if (idx < n_size) { break; }                                   \
                                                                       \
        n = ne;                                                        \
        n_acc_size = ne_acc_size;                                      \
        idx -= n_size;                                                 \
    }                                                                  \
                                                                       \
    return { n, idx };

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessL(BinTreeNode* n, size_t idx) {
    ZETA_Core_BinTree_Access_(L, R);
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessR(BinTreeNode* n, size_t idx) {
    ZETA_Core_BinTree_Access_(R, L);
}

#define ZETA_Core_BinTree_Advance_(D, E)                               \
    while (n != nullptr && 0 < step) {                                 \
        BinTreeNode* nd{ n->Get##D##Ptr() };                           \
        BinTreeNode* ne{ n->Get##E##Ptr() };                           \
                                                                       \
        size_t n_acc_size{ n->GetAccSize() };                          \
        size_t nd_acc_size{ nd == nullptr ? BinTreeNode::null_acc_size \
                                          : nd->GetAccSize() };        \
        size_t ne_acc_size{ ne == nullptr ? BinTreeNode::null_acc_size \
                                          : ne->GetAccSize() };        \
                                                                       \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };       \
                                                                       \
        if (step < n_size) { break; }                                  \
                                                                       \
        step -= n_size;                                                \
                                                                       \
        if (step < nd_acc_size) { return Access##E(nd, step); }        \
                                                                       \
        step -= nd_acc_size;                                           \
                                                                       \
        for (;;) {                                                     \
            BinTreeNode* np{ n->GetPPtr() };                           \
                                                                       \
            if (np == nullptr || np->Get##E##Ptr() == n) {             \
                n = np;                                                \
                break;                                                 \
            }                                                          \
                                                                       \
            n = np;                                                    \
        }                                                              \
    }                                                                  \
                                                                       \
    return { n, step };

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceL(BinTreeNode* n, size_t step) {
    ZETA_Core_BinTree_Advance_(L, R);
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceR(BinTreeNode* n, size_t step) {
    ZETA_Core_BinTree_Advance_(R, L);
}

template <typename BinTreeNode>
Pair<size_t, size_t> GetLRAccSize(BinTreeNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);

    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    size_t l_acc_size{ nl == nullptr ? BinTreeNode::null_acc_size
                                     : nl->GetAccSize() };
    size_t r_acc_size{ nr == nullptr ? BinTreeNode::null_acc_size
                                     : nr->GetAccSize() };

    size_t n_acc_size{ n->GetAccSize() };

    for (;;) {
        BinTreeNode* np{ n->GetPPtr() };
        if (np == nullptr) { break; }

        size_t np_acc_size{ np->GetAccSize() };
        size_t k{ np_acc_size - n_acc_size };

        if (np->GetLPtr() == n) {
            r_acc_size += k;
        } else {
            l_acc_size += k;
        }

        n = np;
        n_acc_size = np_acc_size;
    }

    return { l_acc_size, r_acc_size };
}

template <typename BinTreeNode>
void Sanitize_(BinTreeNode* n) {
    BinTreeNode* nl{ n->GetLPtr() };
    BinTreeNode* nr{ n->GetRPtr() };

    if (nl != nullptr) {
        ZETA_Core_DebugAssert(nl->GetPPtr() == n);
        Sanitize_(nl);
    }

    if (nr != nullptr) {
        ZETA_Core_DebugAssert(nr->GetPPtr() == n);
        Sanitize_(nr);
    }
}

template <typename BinTreeNode>
void Sanitize(BinTreeNode* root) {
    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(root->GetPPtr() == nullptr);

    Sanitize_(root);
}

}  // namespace zeta::core::bin_tree
