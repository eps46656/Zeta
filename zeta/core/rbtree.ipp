#pragma once

#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename Node>
constexpr decltype(auto) rbtree::MemberFuncNodeTraitsAdapter<Node>::GetColor(
    Node* n) {
    return n->GetColor();
}

template <typename Node>
template <typename _>
    requires meta::IsSame<_, void>
constexpr decltype(auto) rbtree::MemberFuncNodeTraitsAdapter<Node>::SetColor(
    Node* n, unsigned color) {
    n->SetColor(color);
}

template <rbtree::IsNode Node>
constexpr unsigned rbtree::GetColor(Node* n) {
    return NodeTraits<Node>::GetColor(n);
}

template <rbtree::IsNode Node>
constexpr void rbtree::SetColor(Node* n, unsigned color) {
    ZETA_Core_StaticAssert(!bin_tree::IsConst<Node>());
    NodeTraits<Node>::SetColor(n, color);
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
    ZETA_Core_DebugAssert(n != nullptr);

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
#define Insert_(D, E)                                               \
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
    Node* pos_d{ (bin_tree::Get##D)(pos) };                         \
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
    ZETA_Core_DebugAssert(pos != nullptr);

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

            default: ZETA_Core_Unreachable();
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
size_t SanitizeRecursive_(mem_recorder::MemRecorder* dst_mr, Node* n) {
    if (n == nullptr) { return 0; }

    Node* nl{ bin_tree::GetL(n) };
    Node* nr{ bin_tree::GetR(n) };

    if (nl != nullptr) { ZETA_Core_DebugAssert(bin_tree::GetP(nl) == n); }
    if (nr != nullptr) { ZETA_Core_DebugAssert(bin_tree::GetP(nr) == n); }

    size_t lbh{ SanitizeRecursive_(dst_mr, nl) };

    if (dst_mr != nullptr) { mem_recorder::Record(*dst_mr, n, sizeof(void*)); }

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

template <rbtree::IsNode Node>
constexpr void rbtree::Sanitize(mem_recorder::MemRecorder* dst_mr, Node* root) {
    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(bin_tree::GetP(root) == nullptr);
    ZETA_Core_DebugAssert((GetColor)(root) == black);

    mem_recorder::MemRecorder* origin_dst_mr{ dst_mr };

    if (dst_mr == nullptr) { dst_mr = mem_recorder::Create(); }

    detail::SanitizeRecursive_(dst_mr, root);

    if (origin_dst_mr != dst_mr) {
        mem_recorder::Destroy(dst_mr);
        dst_mr = origin_dst_mr;
    }
}

}  // namespace zeta::core
