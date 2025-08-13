#include <zeta/core/utils.h>

#if !defined(TreeNode)
#error "TreeNode is not defined."
#endif

#pragma push_macro("TreeNode_")
#pragma push_macro("AccSizeOnly")
#pragma push_macro("Attach_")
#pragma push_macro("Rotate_")
#pragma push_macro("StepP_")
#pragma push_macro("Step_")
#pragma push_macro("Access_")
#pragma push_macro("Advance_")

#define TreeNode_(x) ZETA_Core_Concat(TreeNode, _, x)

#if defined(AccSizeType)

#define AccSizeOnly(x) { x } ZETA_Core_StaticAssert(TRUE)

#else

#define AccSizeOnly(x)

#endif

static size_t Zeta_Core_BinTree(TreeNode, Count_)(void* n) {
    size_t ret = 1;

    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (nl != NULL) { ret += Zeta_Core_BinTree(TreeNode, Count_)(nl); }
    if (nr != NULL) { ret += Zeta_Core_BinTree(TreeNode, Count_)(nr); }

    return ret;
}

size_t Zeta_Core_BinTree(TreeNode, Count)(void* n) {
    if (n == NULL) { return 0; }

    return Zeta_Core_BinTree(TreeNode, Count_)(n);
}

#if defined(AccSizeType)

static void Zeta_Core_BinTree(TreeNode, AddDiffSize_)(void* n,
                                                      size_t diff_size) {
    if (diff_size == 0) { return; }

    for (; n != NULL; n = TreeNode_(GetP)(n)) {
        TreeNode_(SetAccSize)(n, TreeNode_(GetAccSize)(n) + diff_size);
    }
}

size_t Zeta_Core_BinTree(TreeNode, GetSize)(void* n) {
    size_t n_acc_size = TreeNode_(GetAccSize)(n);

    if (n == NULL) { return n_acc_size; }

    return n_acc_size - TreeNode_(GetAccSize)(TreeNode_(GetL)(n)) -
           TreeNode_(GetAccSize)(TreeNode_(GetR)(n));
}

void Zeta_Core_BinTree(TreeNode, SetSize)(void* n, size_t size) {
    ZETA_Core_DebugAssert(n != NULL);

    Zeta_Core_BinTree(TreeNode, AddDiffSize_)(
        n, size - Zeta_Core_BinTree(TreeNode, GetSize(n)));
}

void Zeta_Core_BinTree(TreeNode, SetDiffSize)(void* n, size_t diff_size) {
    ZETA_Core_DebugAssert(n != NULL);

    Zeta_Core_BinTree(TreeNode, AddDiffSize_)(n, diff_size);
}

#endif

#define Attach_(D)                                                      \
                                                                        \
    ZETA_Core_DebugAssert(n != NULL);                                   \
                                                                        \
    ZETA_Core_DebugAssert(TreeNode_(Get##D)(n) == NULL);                \
    ZETA_Core_DebugAssert(m == NULL || TreeNode_(GetP)(m) == NULL);     \
                                                                        \
    if (m == NULL) { return; }                                          \
                                                                        \
    TreeNode_(Set##D)(n, m);                                            \
    TreeNode_(SetP)(m, n);                                              \
                                                                        \
    AccSizeOnly({                                                       \
        Zeta_Core_BinTree(TreeNode, AddDiffSize_)(                      \
            n, TreeNode_(GetAccSize)(m) - TreeNode_(GetAccSize)(NULL)); \
    });

void Zeta_Core_BinTree(TreeNode, AttatchL)(void* n, void* m) { Attach_(L); }

void Zeta_Core_BinTree(TreeNode, AttatchR)(void* n, void* m) { Attach_(R); }

void Zeta_Core_BinTree(TreeNode, Detach)(void* n) {
    ZETA_Core_DebugAssert(n != NULL);

    void* np = TreeNode_(GetP)(n);

    if (np == NULL) { return; }

    if (TreeNode_(GetL)(np) == n) {
        TreeNode_(SetL)(np, NULL);
    } else {
        TreeNode_(SetR)(np, NULL);
    }

    TreeNode_(SetP)(n, NULL);

    AccSizeOnly({
        Zeta_Core_BinTree(TreeNode, AddDiffSize_)(
            np, TreeNode_(GetAccSize)(NULL) - TreeNode_(GetAccSize)(n));
    });
}

static void Zeta_Core_BinTree(TreeNode, EraseAll_B_)(
    void* n, void (*Callback)(void* callback_context, void* n),
    void* callback_context) {
    ZETA_Core_DebugAssert(n != NULL);

    void* head = NULL;

    unsigned long long random_seed = Zeta_Core_GetRandom();

    for (;;) {
        if (n == NULL) {
            if (head == NULL) { break; }
            n = head;
            head = TreeNode_(GetP)(head);
        }

        void* nl = TreeNode_(GetL)(n);
        void* nr = TreeNode_(GetR)(n);

        if (Callback != NULL) { Callback(callback_context, n); }

        if (nl == NULL) {
            n = nr;
            continue;
        }

        if (nr == NULL) {
            n = nl;
            continue;
        }

        if (Zeta_Core_SimpleRandomRotate(&random_seed) % 2 == 0) {
            TreeNode_(SetP)(nl, head);
            head = nl;
            n = nr;
        } else {
            TreeNode_(SetP)(nr, head);
            head = nr;
            n = nl;
        }
    }
}

static void Zeta_Core_BinTree(TreeNode, EraseAll_A_)(
    void* n, void (*Callback)(void* callback_context, void* n),
    void* callback_context) {
    size_t buffer_capacity =
        ZETA_Core_GetMaxOf(24, ZETA_Core_ullong_width * 3 / 4);

    void* buffer[buffer_capacity];
    size_t buffer_i = 0;

    buffer[buffer_i++] = n;

    while (0 < buffer_i) {
        n = buffer[--buffer_i];

        void* nl = TreeNode_(GetL)(n);
        void* nr = TreeNode_(GetR)(n);

        if (Callback != NULL) { Callback(callback_context, n); }

        if (nl != NULL) {
            if (buffer_i == buffer_capacity) {
                Zeta_Core_BinTree(TreeNode, EraseAll_B_)(nl, Callback,
                                                         callback_context);
            } else {
                buffer[buffer_i++] = nl;
            }
        }

        if (nr != NULL) {
            if (buffer_i == buffer_capacity) {
                Zeta_Core_BinTree(TreeNode, EraseAll_B_)(nr, Callback,
                                                         callback_context);
            } else {
                buffer[buffer_i++] = nr;
            }
        }
    }
}

void Zeta_Core_BinTree(TreeNode, ErassAll)(
    void* root, void (*Callback)(void* callback_context, void* n),
    void* callback_context) {
    ZETA_Core_DebugAssert(TreeNode_(GetP)(root) == NULL);

    Zeta_Core_BinTree(TreeNode, EraseAll_A_)(root, Callback, callback_context);
}

void Zeta_Core_BinTree(TreeNode, Swap)(void* n, void* m) {
    ZETA_Core_DebugAssert(n != NULL);
    ZETA_Core_DebugAssert(m != NULL);

    if (n == m) { return; }

    void* np = TreeNode_(GetP)(n);
    void* mp = TreeNode_(GetP)(m);

    if (np == m) {
        ZETA_Core_Swap(n, m);
        ZETA_Core_Swap(np, mp);
    }

    size_t n_acc_size;
    size_t m_acc_size;

    size_t n_size;
    size_t m_size;

    ZETA_Core_Unused(n_acc_size);
    ZETA_Core_Unused(m_acc_size);

    ZETA_Core_Unused(n_size);
    ZETA_Core_Unused(m_size);

    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    void* ml = TreeNode_(GetL)(m);
    void* mr = TreeNode_(GetR)(m);

    AccSizeOnly({
        n_acc_size = TreeNode_(GetAccSize)(n);
        m_acc_size = TreeNode_(GetAccSize)(m);

        n_size =
            n_acc_size - TreeNode_(GetAccSize)(nl) - TreeNode_(GetAccSize)(nr);
        m_size =
            m_acc_size - TreeNode_(GetAccSize)(ml) - TreeNode_(GetAccSize)(mr);
    });

    if (mp == n) {
        if (np != NULL) {
            if (TreeNode_(GetL)(np) == n) {
                TreeNode_(SetL)(np, m);
            } else {
                TreeNode_(SetR)(np, m);
            }
        }

        TreeNode_(SetP)(m, np);

        if (TreeNode_(GetL)(n) == m) {
            TreeNode_(SetL)(m, n);

            TreeNode_(SetR)(m, nr);
            if (nr != NULL) { TreeNode_(SetP)(nr, m); }
        } else {
            TreeNode_(SetL)(m, nl);
            if (nl != NULL) { TreeNode_(SetP)(nl, m); }

            TreeNode_(SetR)(m, n);
        }

        TreeNode_(SetP)(n, m);

        TreeNode_(SetL)(n, ml);
        if (ml != NULL) { TreeNode_(SetP)(ml, n); }

        TreeNode_(SetR)(n, mr);
        if (mr != NULL) { TreeNode_(SetP)(mr, n); }
    } else {
        if (np != NULL) {
            if (TreeNode_(GetL)(np) == n) {
                TreeNode_(SetL)(np, m);
            } else {
                TreeNode_(SetR)(np, m);
            }
        }

        TreeNode_(SetP)(m, np);

        TreeNode_(SetL)(m, nl);
        if (nl != NULL) { TreeNode_(SetP)(nl, m); }

        TreeNode_(SetR)(m, nr);
        if (nr != NULL) { TreeNode_(SetP)(nr, m); }

        if (mp != NULL) {
            if (TreeNode_(GetL)(mp) == m) {
                TreeNode_(SetL)(mp, n);
            } else {
                TreeNode_(SetR)(mp, n);
            }
        }

        TreeNode_(SetP)(n, mp);

        TreeNode_(SetL)(n, ml);
        if (ml != NULL) { TreeNode_(SetP)(ml, n); }

        TreeNode_(SetR)(n, mr);
        if (mr != NULL) { TreeNode_(SetP)(mr, n); }
    }

    AccSizeOnly({
        TreeNode_(SetAccSize)(n, m_acc_size);
        TreeNode_(SetAccSize)(m, n_acc_size);

        Zeta_Core_BinTree(TreeNode, AddDiffSize_)(n, n_size - m_size);
        Zeta_Core_BinTree(TreeNode, AddDiffSize_)(m, m_size - n_size);
    });
}

#define Rotate_(D, E)                                                      \
                                                                           \
    ZETA_Core_DebugAssert(n != NULL);                                      \
                                                                           \
    void* ne = TreeNode_(Get##E)(n);                                       \
    ZETA_Core_DebugAssert(ne != NULL);                                     \
                                                                           \
    void* ned = TreeNode_(Get##D)(ne);                                     \
    void* np = TreeNode_(GetP)(n);                                         \
                                                                           \
    if (np != NULL) {                                                      \
        if (TreeNode_(Get##D)(np) == n) {                                  \
            TreeNode_(Set##D)(np, ne);                                     \
        } else {                                                           \
            TreeNode_(Set##E)(np, ne);                                     \
        }                                                                  \
    }                                                                      \
                                                                           \
    TreeNode_(SetP)(ne, np);                                               \
                                                                           \
    TreeNode_(Set##D)(ne, n);                                              \
    TreeNode_(SetP)(n, ne);                                                \
                                                                           \
    TreeNode_(Set##E)(n, ned);                                             \
    if (ned != NULL) { TreeNode_(SetP)(ned, n); }                          \
                                                                           \
    AccSizeOnly({                                                          \
        size_t n_acc_size = TreeNode_(GetAccSize)(n);                      \
        size_t ne_acc_size = TreeNode_(GetAccSize)(ne);                    \
        size_t ned_acc_size = TreeNode_(GetAccSize)(ned);                  \
                                                                           \
        TreeNode_(SetAccSize)(n, n_acc_size - ne_acc_size + ned_acc_size); \
                                                                           \
        TreeNode_(SetAccSize)(ne, n_acc_size);                             \
    });

void Zeta_Core_BinTree(TreeNode, RotateL)(void* n) { Rotate_(L, R); }

void Zeta_Core_BinTree(TreeNode, RotateR)(void* n) { Rotate_(R, L); }

#define StepP_(E)                                      \
                                                       \
    ZETA_Core_DebugAssert(n != NULL);                  \
                                                       \
    for (;;) {                                         \
        void* np = TreeNode_(GetP)(n);                 \
        if (np == NULL) { return NULL; }               \
        if (TreeNode_(Get##E)(np) == n) { return np; } \
        n = np;                                        \
    }

void* Zeta_Core_BinTree(TreeNode, StepPL)(void* n) { StepP_(R); }

void* Zeta_Core_BinTree(TreeNode, StepPR)(void* n) { StepP_(L); }

#define Step_(D, E)                                                          \
                                                                             \
    ZETA_Core_DebugAssert(n != NULL);                                        \
                                                                             \
    void* nd = TreeNode_(Get##D)(n);                                         \
                                                                             \
    if (nd != NULL) { return Zeta_Core_GetMostLink(nd, TreeNode_(Get##E)); } \
                                                                             \
    for (;;) {                                                               \
        void* np = TreeNode_(GetP)(n);                                       \
        if (np == NULL) { return NULL; }                                     \
        if (TreeNode_(Get##E)(np) == n) { return np; }                       \
        n = np;                                                              \
    }

void* Zeta_Core_BinTree(TreeNode, StepL)(void* n) { Step_(L, R); }

void* Zeta_Core_BinTree(TreeNode, StepR)(void* n) { Step_(R, L); }

#if defined(AccSizeType)

#define Access_(D, E)                                                   \
    if (dst_n == NULL && dst_tail_idx == NULL) { return; }              \
                                                                        \
    size_t n_acc_size = TreeNode_(GetAccSize)(n);                       \
                                                                        \
    if (n_acc_size <= idx) {                                            \
        if (dst_n != NULL) { *dst_n = NULL; }                           \
        if (dst_tail_idx != NULL) { *dst_tail_idx = idx - n_acc_size; } \
        return;                                                         \
    }                                                                   \
                                                                        \
    while (n != NULL) {                                                 \
        void* nd = TreeNode_(Get##D)(n);                                \
        size_t nd_acc_size = TreeNode_(GetAccSize)(nd);                 \
                                                                        \
        if (idx < nd_acc_size) {                                        \
            n = nd;                                                     \
            n_acc_size = nd_acc_size;                                   \
            continue;                                                   \
        }                                                               \
                                                                        \
        void* ne = TreeNode_(Get##E)(n);                                \
        size_t ne_acc_size = TreeNode_(GetAccSize)(ne);                 \
                                                                        \
        idx -= nd_acc_size;                                             \
        size_t n_size = n_acc_size - nd_acc_size - ne_acc_size;         \
                                                                        \
        if (idx < n_size) { break; }                                    \
                                                                        \
        n = ne;                                                         \
        n_acc_size = ne_acc_size;                                       \
        idx -= n_size;                                                  \
    }                                                                   \
                                                                        \
    if (dst_n != NULL) { *dst_n = n; }                                  \
    if (dst_tail_idx != NULL) { *dst_tail_idx = idx; }

void Zeta_Core_BinTree(TreeNode, AccessL)(void** dst_n, size_t* dst_tail_idx,
                                          void* n, size_t idx) {
    Access_(L, R);
}

void Zeta_Core_BinTree(TreeNode, AccessR)(void** dst_n, size_t* dst_tail_idx,
                                          void* n, size_t idx) {
    Access_(R, L);
}

#define Advance_(D, E)                                                      \
    if (dst_n == NULL && dst_tail_idx == NULL) { return; }                  \
                                                                            \
    while (n != NULL && 0 < step) {                                         \
        void* nd = TreeNode_(Get##D)(n);                                    \
        void* ne = TreeNode_(Get##E)(n);                                    \
                                                                            \
        size_t n_acc_size = TreeNode_(GetAccSize)(n);                       \
        size_t nd_acc_size = TreeNode_(GetAccSize)(nd);                     \
        size_t ne_acc_size = TreeNode_(GetAccSize)(ne);                     \
                                                                            \
        size_t n_size = n_acc_size - nd_acc_size - ne_acc_size;             \
                                                                            \
        if (step < n_size) { break; }                                       \
                                                                            \
        step -= n_size;                                                     \
                                                                            \
        if (step < nd_acc_size) {                                           \
            Zeta_Core_BinTree(TreeNode, Access##E)(dst_n, dst_tail_idx, nd, \
                                                   step);                   \
            return;                                                         \
        }                                                                   \
                                                                            \
        step -= nd_acc_size;                                                \
                                                                            \
        for (;;) {                                                          \
            void* np = TreeNode_(GetP)(n);                                  \
                                                                            \
            if (np == NULL || TreeNode_(Get##E)(np) == n) {                 \
                n = np;                                                     \
                break;                                                      \
            }                                                               \
                                                                            \
            n = np;                                                         \
        }                                                                   \
    }                                                                       \
                                                                            \
    if (dst_n != NULL) { *dst_n = n; }                                      \
    if (dst_tail_idx != NULL) { *dst_tail_idx = step; }

void Zeta_Core_BinTree(TreeNode, AdvanceL)(void** dst_n, size_t* dst_tail_idx,
                                           void* n, size_t step) {
    Advance_(L, R);
}

void Zeta_Core_BinTree(TreeNode, AdvanceR)(void** dst_n, size_t* dst_tail_idx,
                                           void* n, size_t step) {
    Advance_(R, L);
}

void Zeta_Core_BinTree(TreeNode, GetAccSize)(size_t* dst_l_acc_size,
                                             size_t* dst_r_acc_size, void* n) {
    ZETA_Core_DebugAssert(n != NULL);

    if (dst_l_acc_size == NULL && dst_r_acc_size == NULL) { return; }

    size_t l_acc_size = TreeNode_(GetAccSize)(TreeNode_(GetL)(n));
    size_t r_acc_size = TreeNode_(GetAccSize)(TreeNode_(GetR)(n));

    size_t n_acc_size = TreeNode_(GetAccSize)(n);

    for (;;) {
        void* np = TreeNode_(GetP)(n);
        if (np == NULL) { break; }

        size_t np_acc_size = TreeNode_(GetAccSize)(np);
        size_t k = np_acc_size - n_acc_size;

        if (TreeNode_(GetL)(np) == n) {
            r_acc_size += k;
        } else {
            l_acc_size += k;
        }

        n = np;
        n_acc_size = np_acc_size;
    }

    if (dst_l_acc_size != NULL) { *dst_l_acc_size = l_acc_size; }
    if (dst_r_acc_size != NULL) { *dst_r_acc_size = r_acc_size; }
}

#endif

static void Zeta_Core_BinTree(TreeNode, Sanitize_)(void* n) {
    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (nl != NULL) {
        ZETA_Core_DebugAssert(TreeNode_(GetP)(nl) == n);
        Zeta_Core_BinTree(TreeNode, Sanitize_)(nl);
    }

    if (nr != NULL) {
        ZETA_Core_DebugAssert(TreeNode_(GetP)(nr) == n);
        Zeta_Core_BinTree(TreeNode, Sanitize_)(nr);
    }
}

void Zeta_Core_BinTree(TreeNode, Sanitize)(void* root) {
    if (root == NULL) { return; }

    ZETA_Core_DebugAssert(TreeNode_(GetP)(root) == NULL);

    Zeta_Core_BinTree(TreeNode, Sanitize_)(root);
}

#pragma pop_macro("TreeNode_")
#pragma pop_macro("AccSizeOnly")
#pragma pop_macro("Attach_")
#pragma pop_macro("Rotate_")
#pragma pop_macro("StepP_")
#pragma pop_macro("Step_")
#pragma pop_macro("Access_")
#pragma pop_macro("Advance_")
