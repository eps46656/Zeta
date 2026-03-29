#pragma once

#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename BinTreeNode>
unsigned rbtree::GetColor(BinTreeNode* n) {
    return NodeTraits<BinTreeNode>::GetColor(n);
}

template <typename BinTreeNode>
void rbtree::SetColor(BinTreeNode* n, unsigned color) {
    ZETA_Core_StaticAssert(!bin_tree::IsConst<BinTreeNode>());
    NodeTraits<BinTreeNode>::SetColor(n, color);
}

template <typename RBTreeNode>
void rbtree::CheckContract() {
    bin_tree::CheckContract<RBTreeNode>();

    RBTreeNode* rbtn{ nullptr };

    unsigned unsigned_val{ 0 };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...) \
    ZETA_Core_Unused(            \
        (meta::Conditional<false, decltype((method)(__VA_ARGS__)), int>{}))

    CheckMethod(GetColor, rbtn);
    CheckMethod(SetColor, rbtn, unsigned_val);

#pragma pop_macro("CheckMethod")
}

namespace rbtree::detail {

#pragma push_macro("InsertBalance_F_")

#define InsertBalance_F_(D, E)                    \
    RBTreeNode* nu{ (bin_tree::Get##E)(ng) };     \
                                                  \
    if (nu != nullptr && (GetColor)(nu) == red) { \
        (SetColor)(ng, red);                      \
        (SetColor)(np, black);                    \
        (SetColor)(nu, black);                    \
        n = ng;                                   \
        continue;                                 \
    }                                             \
                                                  \
    if ((bin_tree::Get##E)(np) == n) {            \
        bin_tree::Rotate##D(np);                  \
        utils::Swap(n, np);                       \
    }                                             \
                                                  \
    (SetColor)(ng, red);                          \
    (SetColor)(np, black);                        \
    bin_tree::Rotate##E(ng);

template <typename RBTreeNode>
RBTreeNode* InsertBalance_(RBTreeNode* n) {
    (CheckContract<RBTreeNode>)();

    ZETA_Core_DebugAssert(n != nullptr);

    for (;;) {
        RBTreeNode* np{ bin_tree::GetP(n) };

        if (np == nullptr) {
            (SetColor)(n, black);
            break;
        }

        if ((GetColor)(np) == black) { break; }

        RBTreeNode* ng{ bin_tree::GetP(np) };

        if (ng == nullptr) {
            (SetColor)(np, black);
            break;
        }

        if (bin_tree::GetL(ng) == np) {
            InsertBalance_F_(L, R);
        } else {
            InsertBalance_F_(R, L);
        }

        break;
    }

    return bin_tree::GetMostP(n).first;
}

#pragma pop_macro("InsertBalance_F_")

}  // namespace rbtree::detail

#pragma push_macro("Insert_")
#define Insert_(D, E)                                               \
    (CheckContract<RBTreeNode>)();                                  \
                                                                    \
    ZETA_Core_DebugAssert(pos != n);                                \
    ZETA_Core_DebugAssert(n != nullptr);                            \
                                                                    \
    ZETA_Core_DebugAssert(bin_tree::GetP(n) == nullptr);            \
    ZETA_Core_DebugAssert(bin_tree::Get##D(n) == nullptr);          \
    ZETA_Core_DebugAssert(bin_tree::Get##E(n) == nullptr);          \
                                                                    \
    if (pos == nullptr) {                                           \
        if ((GetColor)(n) != black) { (SetColor)(n, black); }       \
        return n;                                                   \
    }                                                               \
                                                                    \
    if ((GetColor)(n) != red) { (SetColor)(n, red); }               \
                                                                    \
    RBTreeNode* pos_d{ (bin_tree::Get##D)(pos) };                   \
                                                                    \
    if (pos_d == nullptr) {                                         \
        bin_tree::Attatch##D(pos, n);                               \
    } else {                                                        \
        bin_tree::Attatch##E(bin_tree::GetMost##E(pos_d).first, n); \
    }                                                               \
                                                                    \
    return detail::InsertBalance_(n);                               \
                                                                    \
    ZETA_Core_StaticAssert(true)

template <typename RBTreeNode>
RBTreeNode* rbtree::InsertL(RBTreeNode* pos, RBTreeNode* n) {
    Insert_(L, R);
}

template <typename RBTreeNode>
RBTreeNode* rbtree::InsertR(RBTreeNode* pos, RBTreeNode* n) {
    Insert_(R, L);
}

#pragma pop_macro("Insert_")

template <typename RBTreeNode>
RBTreeNode* rbtree::Insert(RBTreeNode* pos_l, RBTreeNode* pos_r,
                           RBTreeNode* n) {
    (CheckContract<RBTreeNode>)();

    ZETA_Core_DebugAssert(pos_l != n);
    ZETA_Core_DebugAssert(pos_r != n);
    ZETA_Core_DebugAssert(n != nullptr);

    ZETA_Core_DebugAssert(bin_tree::GetP(n) == nullptr);
    ZETA_Core_DebugAssert(bin_tree::GetL(n) == nullptr);
    ZETA_Core_DebugAssert(bin_tree::GetR(n) == nullptr);

    ZETA_Core_DebugAssert(pos_l == nullptr || bin_tree::StepR(pos_l) == pos_r);
    ZETA_Core_DebugAssert(pos_r == nullptr || bin_tree::StepL(pos_r) == pos_l);

    if (pos_l == nullptr && pos_r == nullptr) {
        if ((GetColor)(n) != black) { (SetColor)(n, black); }

        return n;
    }

    if ((GetColor)(n) != red) { (SetColor)(n, red); }

    if (pos_l != nullptr && bin_tree::GetR(pos_l) == nullptr) {
        bin_tree::AttatchR(pos_l, n);
    } else {
        bin_tree::AttatchL(pos_r, n);
    }

    return detail::InsertBalance_(n);
}

#pragma push_macro("GeneralInsert_")
#define GeneralInsert_(D, E)                                      \
    (CheckContract<RBTreeNode>)();                                \
                                                                  \
    if (pos == nullptr) {                                         \
        return Insert##E(bin_tree::GetMost##E(root).first, n);    \
    }                                                             \
                                                                  \
    ZETA_Core_DebugAssert(root == bin_tree::GetMostP(pos).first); \
                                                                  \
    return Insert##D(pos, n);                                     \
                                                                  \
    ZETA_Core_StaticAssert(true)

template <typename RBTreeNode>
RBTreeNode* rbtree::GeneralInsertL(RBTreeNode* root, RBTreeNode* pos,
                                   RBTreeNode* n) {
    GeneralInsert_(L, R);
}

template <typename RBTreeNode>
RBTreeNode* rbtree::GeneralInsertR(RBTreeNode* root, RBTreeNode* pos,
                                   RBTreeNode* n) {
    GeneralInsert_(R, L);
}

#pragma pop_macro("GeneralInsert_")

namespace rbtree::detail {

#pragma push_macro("ExtractBalance_F_")
#define ExtractBalance_F_(D, E)                                               \
    RBTreeNode* ns{ (bin_tree::Get##E)(np) };                                 \
                                                                              \
    if ((GetColor)(ns) == red) {                                              \
        (SetColor)(np, red);                                                  \
        (SetColor)(ns, black);                                                \
        bin_tree::Rotate##D(np);                                              \
        ns = (bin_tree::Get##E)(np);                                          \
    }                                                                         \
                                                                              \
    RBTreeNode* nsd{ (bin_tree::Get##D)(ns) };                                \
    RBTreeNode* nse{ (bin_tree::Get##E)(ns) };                                \
                                                                              \
    unsigned nse_color{ nse == nullptr ? black : (GetColor)(nse) };           \
                                                                              \
    if ((nsd == nullptr || (GetColor)(nsd) == black) && nse_color == black) { \
        (SetColor)(ns, red);                                                  \
        n = np;                                                               \
        continue;                                                             \
    }                                                                         \
                                                                              \
    if (nse_color == black) {                                                 \
        (SetColor)(ns, red);                                                  \
        (SetColor)(nsd, black);                                               \
        bin_tree::Rotate##E(ns);                                              \
        nse = ns;                                                             \
        ns = nsd;                                                             \
        nsd = (bin_tree::Get##D)(nsd);                                        \
    }                                                                         \
                                                                              \
    (SetColor)(ns, (GetColor)(np));                                           \
    (SetColor)(nse, black);                                                   \
    (SetColor)(np, black);                                                    \
    bin_tree::Rotate##D(np);

template <typename RBTreeNode>
void ExtractBalance_(RBTreeNode* n) {
    (CheckContract<RBTreeNode>)();

    for (;;) {
        if ((GetColor)(n) == red) {
            (SetColor)(n, black);
            break;
        }

        RBTreeNode* np{ bin_tree::GetP(n) };

        if (np == nullptr) { break; }

        if (bin_tree::GetL(np) == n) {
            ExtractBalance_F_(L, R);
        } else {
            ExtractBalance_F_(R, L);
        }

        break;
    }
}

#pragma pop_macro("ExtractBalance_F_")

}  // namespace rbtree::detail

template <typename RBTreeNode>
RBTreeNode* rbtree::Extract(RBTreeNode* pos) {
    (CheckContract<RBTreeNode>)();

    ZETA_Core_DebugAssert(pos != nullptr);

    RBTreeNode* n{ pos };

    RBTreeNode* root;

    RBTreeNode* nl{ bin_tree::GetL(n) };
    RBTreeNode* nr{ bin_tree::GetR(n) };

    if (nl == nullptr || nr == nullptr) {
        root = bin_tree::GetMostP(n).first;
    } else {
        RBTreeNode* m{ utils::GetRandom() % 2 == 0
                           ? bin_tree::GetMostL(nr).first
                           : bin_tree::GetMostR(nl).first };

        bin_tree::Swap(n, m);

        unsigned nc{ (GetColor)(n) };
        unsigned mc{ (GetColor)(m) };

        if (nc != mc) {
            (SetColor)(n, mc);
            (SetColor)(m, nc);
        }

        root = bin_tree::GetMostP(m).first;
    }

    if ((GetColor)(n) == black) {
        nl = bin_tree::GetL(n);
        nr = bin_tree::GetR(n);

        if (nl != nullptr) {
            bin_tree::RotateR(n);
            (SetColor)(nl, black);
        } else if (nr != nullptr) {
            bin_tree::RotateL(n);
            (SetColor)(nr, black);
        } else {
            detail::ExtractBalance_(n);
        }
    }

    root = bin_tree::GetMostP(root).first;

    if (root == n) { return nullptr; }

    bin_tree::Detach(n);

    return root;
}

namespace rbtree::detail {

template <typename RBTreeNode>
size_t SanitizeRecursive_(mem_recorder::MemRecorder* dst_mr, RBTreeNode* n) {
    (CheckContract<RBTreeNode>)();

    if (n == nullptr) { return 0; }

    RBTreeNode* nl{ bin_tree::GetL(n) };
    RBTreeNode* nr{ bin_tree::GetR(n) };

    if (nl != nullptr) { ZETA_Core_DebugAssert(bin_tree::GetP(nl) == n); }
    if (nr != nullptr) { ZETA_Core_DebugAssert(bin_tree::GetP(nr) == n); }

    size_t lbh{ SanitizeRecursive_(dst_mr, nl) };

    if (dst_mr != nullptr) { mem_recorder::Record(dst_mr, n, sizeof(void*)); }

    size_t rbh{ SanitizeRecursive_(dst_mr, nr) };

    ZETA_Core_DebugAssert(lbh == rbh);

    unsigned nc{ (GetColor)(n) };

    ZETA_Core_DebugAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugAssert(nl == nullptr || (GetColor)(nl) == black);
    ZETA_Core_DebugAssert(nr == nullptr || (GetColor)(nr) == black);

    return lbh;
}

}  // namespace rbtree::detail

template <typename RBTreeNode>
void rbtree::Sanitize(mem_recorder::MemRecorder* dst_mr, RBTreeNode* root) {
    (CheckContract<RBTreeNode>)();

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(bin_tree::GetP(root) == nullptr);
    ZETA_Core_DebugAssert((GetColor)(root) == black);

    detail::SanitizeRecursive_(dst_mr, root);
}

}  // namespace zeta::core
