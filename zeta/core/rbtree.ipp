#pragma once

#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <rbtree::IsNode Node>
constexpr unsigned rbtree::GetColor(Node* n) {
    return n->GetColor(Tag{});
}

template <rbtree::IsNode Node>
constexpr void rbtree::SetColor(Node* n, unsigned color) {
    static_assert(!bin_tree::IsConst<Node>());

    n->SetColor(Tag{}, color);
}

namespace rbtree::detail {

#pragma push_macro("InsertBalance_F_")

#define InsertBalance_F_(D, E)                    \
    Node* nu{ (bin_tree::Get##E)(ng) };           \
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

template <IsNode Node>
constexpr Node* InsertBalance_(Node* n) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

    for (;;) {
        Node* np{ bin_tree::GetP(n) };

        if (np == nullptr) {
            (SetColor)(n, black);
            break;
        }

        if ((GetColor)(np) == black) { break; }

        Node* ng{ bin_tree::GetP(np) };

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
#define Insert_(D, E)                                                        \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos != n);                       \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);                   \
                                                                             \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetP(n) == nullptr);   \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::Get##D(n) == nullptr); \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::Get##E(n) == nullptr); \
                                                                             \
    if (pos == nullptr) {                                                    \
        if ((GetColor)(n) != black) { (SetColor)(n, black); }                \
        return n;                                                            \
    }                                                                        \
                                                                             \
    if ((GetColor)(n) != red) { (SetColor)(n, red); }                        \
                                                                             \
    Node* pos_d{ (bin_tree::Get##D)(pos) };                                  \
                                                                             \
    if (pos_d == nullptr) {                                                  \
        bin_tree::Attatch##D(pos, n);                                        \
    } else {                                                                 \
        bin_tree::Attatch##E(bin_tree::GetMost##E(pos_d).first, n);          \
    }                                                                        \
                                                                             \
    return detail::InsertBalance_(n);                                        \
                                                                             \
    static_assert(true)

template <rbtree::IsNode Node>
constexpr Node* rbtree::InsertL(Node* pos, Node* n) {
    Insert_(L, R);
}

template <rbtree::IsNode Node>
constexpr Node* rbtree::InsertR(Node* pos, Node* n) {
    Insert_(R, L);
}

#pragma pop_macro("Insert_")

template <rbtree::IsNode Node>
constexpr Node* rbtree::Insert(Node* pos_l, Node* pos_r, Node* n) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_l != n);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_r != n);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetP(n) == nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetL(n) == nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetR(n) == nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_l == nullptr ||
                                            bin_tree::StepR(pos_l) == pos_r);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_r == nullptr ||
                                            bin_tree::StepL(pos_r) == pos_l);

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
#define GeneralInsert_(D, E)                                                \
    if (pos == nullptr) {                                                   \
        return Insert##E(bin_tree::GetMost##E(root).first, n);              \
    }                                                                       \
                                                                            \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(root ==                         \
                                            bin_tree::GetMostP(pos).first); \
                                                                            \
    return Insert##D(pos, n);                                               \
                                                                            \
    static_assert(true)

template <rbtree::IsNode Node>
constexpr Node* rbtree::GeneralInsertL(Node* root, Node* pos, Node* n) {
    GeneralInsert_(L, R);
}

template <rbtree::IsNode Node>
constexpr Node* rbtree::GeneralInsertR(Node* root, Node* pos, Node* n) {
    GeneralInsert_(R, L);
}

#pragma pop_macro("GeneralInsert_")

namespace rbtree::detail {

#pragma push_macro("ExtractBalance_F_")
#define ExtractBalance_F_(D, E)                                               \
    Node* ns{ (bin_tree::Get##E)(np) };                                       \
                                                                              \
    if ((GetColor)(ns) == red) {                                              \
        (SetColor)(np, red);                                                  \
        (SetColor)(ns, black);                                                \
        bin_tree::Rotate##D(np);                                              \
        ns = (bin_tree::Get##E)(np);                                          \
    }                                                                         \
                                                                              \
    Node* nsd{ (bin_tree::Get##D)(ns) };                                      \
    Node* nse{ (bin_tree::Get##E)(ns) };                                      \
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

template <IsNode Node>
void ExtractBalance_(Node* n) {
    for (;;) {
        if ((GetColor)(n) == red) {
            (SetColor)(n, black);
            break;
        }

        Node* np{ bin_tree::GetP(n) };

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

template <rbtree::IsNode Node>
constexpr Node* rbtree::Extract(Node* pos) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos != nullptr);

    Node* n{ pos };

    Node* root;

    Node* nl{ bin_tree::GetL(n) };
    Node* nr{ bin_tree::GetR(n) };

    if (nl == nullptr || nr == nullptr) {
        root = bin_tree::GetMostP(n).first;

        if ((GetColor)(n) == black) {
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
    } else {
        unsigned side{ static_cast<unsigned>(utils::GetRandom() % 2U) };

        Node* m{ side == 0 ? bin_tree::GetMostL(nr).first
                           : bin_tree::GetMostR(nl).first };

        bin_tree::Swap(n, m);

        unsigned nc{ (GetColor)(n) };
        unsigned mc{ (GetColor)(m) };

        if (nc != mc) {
            (SetColor)(n, mc);
            (SetColor)(m, nc);
        }

        root = bin_tree::GetMostP(m).first;

        if (mc == black) {
            switch (side) {
            case 0:
                // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
                if ((nr = bin_tree::GetR(n)) == nullptr) {
                    detail::ExtractBalance_(n);
                } else {
                    bin_tree::RotateL(n);
                    (SetColor)(nr, black);
                }

                break;

            case 1:
                // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
                if ((nl = bin_tree::GetL(n)) == nullptr) {
                    detail::ExtractBalance_(n);
                } else {
                    bin_tree::RotateR(n);
                    (SetColor)(nl, black);
                }

                break;

            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }
        }
    }

    root = bin_tree::GetMostP(root).first;

    if (root == n) { return nullptr; }

    bin_tree::Detach(n);

    return root;
}

namespace rbtree::detail {

template <IsNode Node>
size_t SanitizeRecursive_(debug_utils::memory::MemRecorder* dst_mr, Node* n) {
    if (n == nullptr) { return 0; }

    Node* nl{ bin_tree::GetL(n) };
    Node* nr{ bin_tree::GetR(n) };

    if (nl != nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetP(nl) == n);
    }

    if (nr != nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetP(nr) == n);
    }

    size_t lbh{ (SanitizeRecursive_)(dst_mr, nl) };

    if (dst_mr != nullptr) { dst_mr->Add(n, sizeof(void*)); }

    size_t rbh{ (SanitizeRecursive_)(dst_mr, nr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(lbh == rbh);

    unsigned nc{ (GetColor)(n) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(nc == black || nc == red);

    if (nc == black) { return lbh + 1; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(nl == nullptr ||
                                            (GetColor)(nl) == black);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(nr == nullptr ||
                                            (GetColor)(nr) == black);

    return lbh;
}

}  // namespace rbtree::detail

template <rbtree::IsNode Node>
constexpr void rbtree::SanityCheck(debug_utils::memory::MemRecorder* dst_mr,
                                   Node* root) {
    if (root == nullptr) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetP(root) == nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert((GetColor)(root) == black);

    detail::SanitizeRecursive_(dst_mr, root);
}

}  // namespace zeta::core
