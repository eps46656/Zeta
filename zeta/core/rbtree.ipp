#pragma once

#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core::rbtree {

template <typename RBTreeNode>
void CheckContract() {
    bin_tree::CheckContract<RBTreeNode>();

    using rbtn_t = RemoveConst<RBTreeNode>;

    rbtn_t* rbtn_ptr{ nullptr };
    rbtn_t const* const_rbtn_ptr{ nullptr };

    unsigned unsigned_val{ 0 };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(RBTreeNode::method(__VA_ARGS__)), return_type>)

    CheckMethod(GetColor, unsigned, rbtn_ptr);
    CheckMethod(GetColor, unsigned, const_rbtn_ptr);

    CheckMethod(SetColor, TypeAny, rbtn_ptr, unsigned_val);

#pragma pop_macro("CheckMethod")
}

// -----------------------------------------------------------------------------

namespace detail {

#pragma push_macro("InsertBalance_F_")

#define InsertBalance_F_(D, E)                              \
    RBTreeNode* nu{ RBTreeNode::Get##E(ng) };               \
                                                            \
    if (nu != nullptr && RBTreeNode::GetColor(nu) == red) { \
        RBTreeNode::SetColor(ng, red);                      \
        RBTreeNode::SetColor(np, black);                    \
        RBTreeNode::SetColor(nu, black);                    \
        n = ng;                                             \
        continue;                                           \
    }                                                       \
                                                            \
    if (RBTreeNode::Get##E(np) == n) {                      \
        bin_tree::Rotate##D(np);                            \
        Swap(n, np);                                        \
    }                                                       \
                                                            \
    RBTreeNode::SetColor(ng, red);                          \
    RBTreeNode::SetColor(np, black);                        \
    bin_tree::Rotate##E(ng);

template <typename RBTreeNode>
RBTreeNode* InsertBalance_(RBTreeNode* n) {
    CheckContract<RBTreeNode>();

    ZETA_Core_DebugAssert(n != nullptr);

    for (;;) {
        RBTreeNode* np{ RBTreeNode::GetP(n) };

        if (np == nullptr) {
            RBTreeNode::SetColor(n, black);
            break;
        }

        if (RBTreeNode::GetColor(np) == black) { break; }

        RBTreeNode* ng{ RBTreeNode::GetP(np) };

        if (ng == nullptr) {
            RBTreeNode::SetColor(np, black);
            break;
        }

        if (RBTreeNode::GetL(ng) == np) {
            InsertBalance_F_(L, R);
        } else {
            InsertBalance_F_(R, L);
        }

        break;
    }

    return bin_tree::GetMostP(n).first;
}

#pragma pop_macro("InsertBalance_F_")

}  // namespace detail

#pragma push_macro("Insert_")

#define Insert_(D, E)                                                     \
    CheckContract<RBTreeNode>();                                          \
                                                                          \
    ZETA_Core_DebugAssert(pos != n);                                      \
    ZETA_Core_DebugAssert(n != nullptr);                                  \
                                                                          \
    ZETA_Core_DebugAssert(RBTreeNode::GetP(n) == nullptr);                \
    ZETA_Core_DebugAssert(RBTreeNode::Get##D(n) == nullptr);              \
    ZETA_Core_DebugAssert(RBTreeNode::Get##E(n) == nullptr);              \
                                                                          \
    if (pos == nullptr) {                                                 \
        if (RBTreeNode::GetColor(n) != black) {                           \
            RBTreeNode::SetColor(n, black);                               \
        }                                                                 \
        return n;                                                         \
    }                                                                     \
                                                                          \
    if (RBTreeNode::GetColor(n) != red) { RBTreeNode::SetColor(n, red); } \
                                                                          \
    RBTreeNode* pos_d{ RBTreeNode::Get##D(pos) };                         \
                                                                          \
    if (pos_d == nullptr) {                                               \
        bin_tree::Attatch##D(pos, n);                                     \
    } else {                                                              \
        bin_tree::Attatch##E(bin_tree::GetMost##E(pos_d).first, n);       \
    }                                                                     \
                                                                          \
    return detail::InsertBalance_(n);

template <typename RBTreeNode>
RBTreeNode* InsertL(RBTreeNode* pos, RBTreeNode* n) {
    Insert_(L, R);
}

template <typename RBTreeNode>
RBTreeNode* InsertR(RBTreeNode* pos, RBTreeNode* n) {
    Insert_(R, L);
}

#pragma pop_macro("Insert_")

template <typename RBTreeNode>
RBTreeNode* Insert(RBTreeNode* pos_l, RBTreeNode* pos_r, RBTreeNode* n) {
    CheckContract<RBTreeNode>();

    ZETA_Core_DebugAssert(pos_l != n);
    ZETA_Core_DebugAssert(pos_r != n);
    ZETA_Core_DebugAssert(n != nullptr);

    ZETA_Core_DebugAssert(RBTreeNode::GetP(n) == nullptr);
    ZETA_Core_DebugAssert(RBTreeNode::GetL(n) == nullptr);
    ZETA_Core_DebugAssert(RBTreeNode::GetR(n) == nullptr);

    ZETA_Core_DebugAssert(pos_l == nullptr || bin_tree::StepR(pos_l) == pos_r);
    ZETA_Core_DebugAssert(pos_r == nullptr || bin_tree::StepL(pos_r) == pos_l);

    if (pos_l == nullptr && pos_r == nullptr) {
        if (RBTreeNode::GetColor(n) != black) {
            RBTreeNode::SetColor(n, black);
        }

        return n;
    }

    if (RBTreeNode::GetColor(n) != red) { RBTreeNode::SetColor(n, red); }

    if (pos_l != nullptr && RBTreeNode::GetR(pos_l) == nullptr) {
        bin_tree::AttatchR(pos_l, n);
    } else {
        bin_tree::AttatchL(pos_r, n);
    }

    return detail::InsertBalance_(n);
}

#pragma push_macro("GeneralInsert_")

#define GeneralInsert_(D, E)                                      \
    CheckContract<RBTreeNode>();                                  \
                                                                  \
    if (pos == nullptr) {                                         \
        return Insert##E(bin_tree::GetMost##E(root).first, n);    \
    }                                                             \
                                                                  \
    ZETA_Core_DebugAssert(root == bin_tree::GetMostP(pos).first); \
                                                                  \
    return Insert##D(pos, n);

template <typename RBTreeNode>
RBTreeNode* GeneralInsertL(RBTreeNode* root, RBTreeNode* pos, RBTreeNode* n) {
    GeneralInsert_(L, R);
}

template <typename RBTreeNode>
RBTreeNode* GeneralInsertR(RBTreeNode* root, RBTreeNode* pos, RBTreeNode* n) {
    GeneralInsert_(R, L);
}

#pragma pop_macro("GeneralInsert_")

namespace detail {

#pragma push_macro("ExtractBalance_F_")

#define ExtractBalance_F_(D, E)                                               \
    RBTreeNode* ns{ RBTreeNode::Get##E(np) };                                 \
                                                                              \
    if (RBTreeNode::GetColor(ns) == red) {                                    \
        RBTreeNode::SetColor(np, red);                                        \
        RBTreeNode::SetColor(ns, black);                                      \
        bin_tree::Rotate##D(np);                                              \
        ns = RBTreeNode::Get##E(np);                                          \
    }                                                                         \
                                                                              \
    RBTreeNode* nsd{ RBTreeNode::Get##D(ns) };                                \
    RBTreeNode* nse{ RBTreeNode::Get##E(ns) };                                \
                                                                              \
    unsigned nse_color{ nse == nullptr ? black : RBTreeNode::GetColor(nse) }; \
                                                                              \
    if ((nsd == nullptr || RBTreeNode::GetColor(nsd) == black) &&             \
        nse_color == black) {                                                 \
        RBTreeNode::SetColor(ns, red);                                        \
        n = np;                                                               \
        continue;                                                             \
    }                                                                         \
                                                                              \
    if (nse_color == black) {                                                 \
        RBTreeNode::SetColor(ns, red);                                        \
        RBTreeNode::SetColor(nsd, black);                                     \
        bin_tree::Rotate##E(ns);                                              \
        nse = ns;                                                             \
        ns = nsd;                                                             \
        nsd = RBTreeNode::Get##D(nsd);                                        \
    }                                                                         \
                                                                              \
    RBTreeNode::SetColor(ns, RBTreeNode::GetColor(np));                       \
    RBTreeNode::SetColor(nse, black);                                         \
    RBTreeNode::SetColor(np, black);                                          \
    bin_tree::Rotate##D(np);

template <typename RBTreeNode>
void ExtractBalance_(RBTreeNode* n) {
    CheckContract<RBTreeNode>();

    for (;;) {
        if (RBTreeNode::GetColor(n) == red) {
            RBTreeNode::SetColor(n, black);
            break;
        }

        RBTreeNode* np{ RBTreeNode::GetP(n) };

        if (np == nullptr) { break; }

        if (RBTreeNode::GetL(np) == n) {
            ExtractBalance_F_(L, R);
        } else {
            ExtractBalance_F_(R, L);
        }

        break;
    }
}

#pragma pop_macro("ExtractBalance_F_")

}  // namespace detail

template <typename RBTreeNode>
RBTreeNode* Extract(RBTreeNode* pos) {
    CheckContract<RBTreeNode>();

    ZETA_Core_DebugAssert(pos != nullptr);

    RBTreeNode* n{ pos };

    RBTreeNode* root;

    RBTreeNode* nl{ RBTreeNode::GetL(n) };
    RBTreeNode* nr{ RBTreeNode::GetR(n) };

    if (nl == nullptr || nr == nullptr) {
        root = bin_tree::GetMostP(n).first;
    } else {
        RBTreeNode* m{ GetRandom() % 2 == 0 ? bin_tree::GetMostL(nr).first
                                            : bin_tree::GetMostR(nl).first };

        bin_tree::Swap(n, m);

        unsigned nc{ RBTreeNode::GetColor(n) };
        unsigned mc{ RBTreeNode::GetColor(m) };

        if (nc != mc) {
            RBTreeNode::SetColor(n, mc);
            RBTreeNode::SetColor(m, nc);
        }

        root = bin_tree::GetMostP(m).first;
    }

    if (RBTreeNode::GetColor(n) == black) {
        nl = RBTreeNode::GetL(n);
        nr = RBTreeNode::GetR(n);

        if (nl != nullptr) {
            bin_tree::RotateR(n);
            RBTreeNode::SetColor(nl, black);
        } else if (nr != nullptr) {
            bin_tree::RotateL(n);
            RBTreeNode::SetColor(nr, black);
        } else {
            detail::ExtractBalance_(n);
        }
    }

    root = bin_tree::GetMostP(root).first;

    if (root == n) { return nullptr; }

    bin_tree::Detach(n);

    return root;
}

namespace detail {

template <typename RBTreeNode>
size_t SanitizeRecursive_(MemRecorder* dst_mr, RBTreeNode* n) {
    CheckContract<RBTreeNode>();

    if (n == nullptr) { return 0; }

    RBTreeNode* nl{ RBTreeNode::GetL(n) };
    RBTreeNode* nr{ RBTreeNode::GetR(n) };

    if (nl != nullptr) { ZETA_Core_DebugAssert(RBTreeNode::GetP(nl) == n); }
    if (nr != nullptr) { ZETA_Core_DebugAssert(RBTreeNode::GetP(nr) == n); }

    size_t lbh{ SanitizeRecursive_(dst_mr, nl) };

    if (dst_mr != nullptr) { MemRecorder::Record(dst_mr, n, sizeof(void*)); }

    size_t rbh{ SanitizeRecursive_(dst_mr, nr) };

    ZETA_Core_DebugAssert(lbh == rbh);

    unsigned nc{ RBTreeNode::GetColor(n) };

    ZETA_Core_DebugAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugAssert(nl == nullptr || RBTreeNode::GetColor(nl) == black);
    ZETA_Core_DebugAssert(nr == nullptr || RBTreeNode::GetColor(nr) == black);

    return lbh;
}

}  // namespace detail

template <typename RBTreeNode>
void Sanitize(MemRecorder* dst_mr, RBTreeNode* root) {
    CheckContract<RBTreeNode>();

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(RBTreeNode::GetP(root) == nullptr);
    ZETA_Core_DebugAssert(RBTreeNode::GetColor(root) == black);

    detail::SanitizeRecursive_(dst_mr, root);
}

}  // namespace zeta::core::rbtree
