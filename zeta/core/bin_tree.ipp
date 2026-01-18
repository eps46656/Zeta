#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::bin_tree {

template <typename BinTreeNodeOperator, typename BinTreeNode>
void CheckBinTreeNode(BinTreeNodeOperator const& btn_opr) {
    BinTreeNode* btn_ptr{ nullptr };
    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(btn_opr.method(__VA_ARGS__)), return_type>)

    CheckMethod(GetP, BinTreeNode*, btn_ptr);
    CheckMethod(GetL, BinTreeNode*, btn_ptr);
    CheckMethod(GetR, BinTreeNode*, btn_ptr);

    if constexpr (btn_opr.IsConst(btn_opr)) {
        CheckMethod(GetP, BinTreeNode*, btn_ptr);
        CheckMethod(GetL, BinTreeNode*, btn_ptr);
        CheckMethod(GetR, BinTreeNode*, btn_ptr);

        CheckMethod(SetP, TypeAny, btn_ptr, btn_ptr);
        CheckMethod(SetL, TypeAny, btn_ptr, btn_ptr);
        CheckMethod(SetR, TypeAny, btn_ptr, btn_ptr);
    }

    CheckMethod(IsAccSizeEnabled, bool);

    if constexpr (btn_opr.IsAccSizeEnabled()) {
        CheckMethod(GetNullAccSize, size_t);

        CheckMethod(GetAccSize, size_t, btn_ptr);

        if constexpr (btn_opr.IsConst(btn_opr)) {
            CheckMethod(SetAccSize, TypeAny, btn_ptr, size_val);
        }
    }

#pragma pop_macro("CheckMethod")
}

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostP(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    return GetMostLink(n, [&](auto x) { return btn_opr.GetP(x); });
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostL(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    return GetMostLink(n, [&](auto x) { return btn_opr.GetL(x); });
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostR(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    return GetMostLink(n, [&](auto x) { return btn_opr.GetR(x); });
}

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AddDiffSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n,
                 size_t diff_size) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());

    if (diff_size == 0) { return; }

    for (; n != nullptr; n = btn_opr.GetP(n)) {
        btn_opr.SetAccSize(n, btn_opr.GetAccSize(n) + diff_size);
    }
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
size_t GetSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());

    auto null_acc_size{ btn_opr.GetNullAccSize() };

    if (n == nullptr) { return null_acc_size; }

    BinTreeNode* nl{ btn_opr.GetL(n) };
    BinTreeNode* nr{ btn_opr.GetR(n) };

    return btn_opr.GetAccSize(n) -
           (nl == nullptr ? null_acc_size : btn_opr.GetAccSize(nl)) -
           (nr == nullptr ? null_acc_size : btn_opr.GetAccSize(nr));
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
void SetSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n, size_t size) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());

    ZETA_Core_DebugAssert(n != nullptr);

    AddDiffSize(btn_opr, n, size - GetSize(btn_opr, n));
}

// -----------------------------------------------------------------------------

#pragma push_macro("Attatch_")

#define Attatch_(D)                                                      \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr); \
                                                                         \
    ZETA_Core_DebugAssert(pos != nullptr);                               \
                                                                         \
    ZETA_Core_DebugAssert(btn_opr.Get##D(pos) == nullptr);               \
    ZETA_Core_DebugAssert(n == nullptr || btn_opr.GetP(n) == nullptr);   \
                                                                         \
    if (n == nullptr) { return; }                                        \
                                                                         \
    btn_opr.Set##D(pos, n);                                              \
    btn_opr.SetP(n, pos);                                                \
                                                                         \
    if constexpr (btn_opr.IsAccSizeEnabled()) {                          \
        AddDiffSize(btn_opr, pos,                                        \
                    btn_opr.GetAccSize(n) - btn_opr.GetNullAccSize());   \
    }

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AttatchL(BinTreeNodeOperator const& btn_opr, BinTreeNode* pos,
              BinTreeNode* n) {
    Attatch_(L);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AttatchR(BinTreeNodeOperator const& btn_opr, BinTreeNode* pos,
              BinTreeNode* n) {
    Attatch_(R);
}

#pragma pop_macro("Attatch_")

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Detach(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_DebugAssert(n != nullptr);

    BinTreeNode* np{ btn_opr.GetP(n) };

    if (np == nullptr) { return; }

    if (btn_opr.GetL(np) == n) {
        btn_opr.SetL(np, static_cast<BinTreeNode*>(nullptr));
    } else {
        btn_opr.SetR(np, static_cast<BinTreeNode*>(nullptr));
    }

    btn_opr.SetP(n, static_cast<BinTreeNode*>(nullptr));

    if constexpr (btn_opr.IsAccSizeEnabled()) {
        AddDiffSize(btn_opr, np,
                    btn_opr.GetNullAccSize() - btn_opr.GetAccSize(n));
    }
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Swap(BinTreeNodeOperator const& btn_opr, BinTreeNode* n, BinTreeNode* m) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    if (n == m) { return; }

    BinTreeNode* np{ btn_opr.GetP(n) };
    BinTreeNode* mp{ btn_opr.GetP(m) };

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

    BinTreeNode* nl{ btn_opr.GetL(n) };
    BinTreeNode* nr{ btn_opr.GetR(n) };

    BinTreeNode* ml{ btn_opr.GetL(m) };
    BinTreeNode* mr{ btn_opr.GetR(m) };

    if constexpr (btn_opr.IsAccSizeEnabled()) {
        auto null_acc_size{ btn_opr.GetNullAccSize() };

        n_acc_size = btn_opr.GetAccSize(n);
        m_acc_size = btn_opr.GetAccSize(m);

        n_size = n_acc_size -
                 (nl == nullptr ? null_acc_size : btn_opr.GetAccSize(nl)) -
                 (nr == nullptr ? null_acc_size : btn_opr.GetAccSize(nr));

        m_size = m_acc_size -
                 (ml == nullptr ? null_acc_size : btn_opr.GetAccSize(ml)) -
                 (mr == nullptr ? null_acc_size : btn_opr.GetAccSize(mr));
    }

    if (mp == n) {
        if (np == nullptr) {
        } else if (btn_opr.GetL(np) == n) {
            btn_opr.SetL(np, m);
        } else {
            btn_opr.SetR(np, m);
        }

        btn_opr.SetP(m, np);

        if (btn_opr.GetL(n) == m) {
            btn_opr.SetL(m, n);

            btn_opr.SetR(m, nr);
            if (nr != nullptr) { btn_opr.SetP(nr, m); }
        } else {
            btn_opr.SetL(m, nl);
            if (nl != nullptr) { btn_opr.SetP(nl, m); }

            btn_opr.SetR(m, n);
        }

        btn_opr.SetP(n, m);

        btn_opr.SetL(n, ml);
        if (ml != nullptr) { btn_opr.SetP(ml, n); }

        btn_opr.SetR(n, mr);
        if (mr != nullptr) { btn_opr.SetP(mr, n); }
    } else {
        if (np == nullptr) {
        } else if (btn_opr.GetL(np) == n) {
            btn_opr.SetL(np, m);
        } else {
            btn_opr.SetR(np, m);
        }

        btn_opr.SetP(m, np);

        btn_opr.SetL(m, nl);
        if (nl != nullptr) { btn_opr.SetP(nl, m); }

        btn_opr.SetR(m, nr);
        if (nr != nullptr) { btn_opr.SetP(nr, m); }

        if (mp == nullptr) {
        } else if (btn_opr.GetL(mp) == m) {
            btn_opr.SetL(mp, n);
        } else {
            btn_opr.SetR(mp, n);
        }

        btn_opr.SetP(n, mp);

        btn_opr.SetL(n, ml);
        if (ml != nullptr) { btn_opr.SetP(ml, n); }

        btn_opr.SetR(n, mr);
        if (mr != nullptr) { btn_opr.SetP(mr, n); }
    }

    if constexpr (btn_opr.IsAccSizeEnabled()) {
        btn_opr.SetAccSize(n, m_acc_size);
        btn_opr.SetAccSize(m, n_acc_size);

        AddDiffSize(btn_opr, n, n_size - m_size);
        AddDiffSize(btn_opr, m, m_size - n_size);
    }
}

// -----------------------------------------------------------------------------

#pragma push_macro("Rotate_")

#define Rotate_(D, E)                                                    \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr); \
                                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                                 \
                                                                         \
    BinTreeNode* ne{ btn_opr.Get##E(n) };                                \
    ZETA_Core_DebugAssert(ne != nullptr);                                \
                                                                         \
    BinTreeNode* ned{ btn_opr.Get##D(ne) };                              \
    BinTreeNode* np{ btn_opr.GetP(n) };                                  \
                                                                         \
    if (np != nullptr) {                                                 \
        if (btn_opr.Get##D(np) == n) {                                   \
            btn_opr.Set##D(np, ne);                                      \
        } else {                                                         \
            btn_opr.Set##E(np, ne);                                      \
        }                                                                \
    }                                                                    \
                                                                         \
    btn_opr.SetP(ne, np);                                                \
                                                                         \
    btn_opr.Set##D(ne, n);                                               \
    btn_opr.SetP(n, ne);                                                 \
                                                                         \
    btn_opr.Set##E(n, ned);                                              \
    if (ned != nullptr) { btn_opr.SetP(ned, n); }                        \
                                                                         \
    if constexpr (btn_opr.IsAccSizeEnabled()) {                          \
        size_t n_acc_size{ btn_opr.GetAccSize(n) };                      \
        size_t ne_acc_size{ btn_opr.GetAccSize(ne) };                    \
        size_t ned_acc_size{ ned == nullptr ? btn_opr.GetNullAccSize()   \
                                            : btn_opr.GetAccSize(ned) }; \
                                                                         \
        btn_opr.SetAccSize(n, n_acc_size - ne_acc_size + ned_acc_size);  \
                                                                         \
        btn_opr.SetAccSize(ne, n_acc_size);                              \
    };

template <typename BinTreeNodeOperator, typename BinTreeNode>
void RotateL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    Rotate_(L, R);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
void RotateR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    Rotate_(R, L);
}

#pragma pop_macro("Rotate_")

// -----------------------------------------------------------------------------

#pragma push_macro("Access_")

#define Access_(D, E)                                                          \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr);       \
                                                                               \
    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());                        \
                                                                               \
    auto null_acc_size{ btn_opr.GetNullAccSize() };                            \
                                                                               \
    size_t n_acc_size{ n == nullptr ? null_acc_size : btn_opr.GetAccSize(n) }; \
                                                                               \
    if (n_acc_size <= idx) { return { nullptr, idx - n_acc_size }; }           \
                                                                               \
    while (n != nullptr) {                                                     \
        BinTreeNode* nd{ btn_opr.Get##D(n) };                                  \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                      \
                                          : btn_opr.GetAccSize(nd) };          \
                                                                               \
        if (idx < nd_acc_size) {                                               \
            n = nd;                                                            \
            n_acc_size = nd_acc_size;                                          \
            continue;                                                          \
        }                                                                      \
                                                                               \
        BinTreeNode* ne{ btn_opr.Get##E(n) };                                  \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size                      \
                                          : btn_opr.GetAccSize(ne) };          \
                                                                               \
        idx -= nd_acc_size;                                                    \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };               \
                                                                               \
        if (idx < n_size) { break; }                                           \
                                                                               \
        n = ne;                                                                \
        n_acc_size = ne_acc_size;                                              \
        idx -= n_size;                                                         \
    }                                                                          \
                                                                               \
    return { n, idx };

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessL(BinTreeNodeOperator const& btn_opr,
                                   BinTreeNode* n, size_t idx) {
    Access_(L, R);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessR(BinTreeNodeOperator const& btn_opr,
                                   BinTreeNode* n, size_t idx) {
    Access_(R, L);
}

#pragma pop_macro("Access_")

#pragma push_macro("StepP_")

#define StepP_(D)                                                        \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr); \
                                                                         \
    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());                  \
                                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                                 \
                                                                         \
    for (;;) {                                                           \
        BinTreeNode* np{ btn_opr.GetP(n) };                              \
        if (np == nullptr) { return nullptr; }                           \
        if (btn_opr.Get##E(np) == n) { return np; }                      \
        n = np;                                                          \
    }

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepPL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    StepP_(L);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepPR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    StepP_(R);
}

#pragma pop_macro("StepP_")

#pragma push_macro("Step_")

#define Step_(D, E)                                                      \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr); \
                                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                                 \
                                                                         \
    BinTreeNode* nd{ btn_opr.Get##D(n) };                                \
                                                                         \
    if (nd != nullptr) { return GetMost##E(btn_opr, nd).first; }         \
                                                                         \
    for (;;) {                                                           \
        BinTreeNode* np{ btn_opr.GetP(n) };                              \
        if (np == nullptr) { return nullptr; }                           \
        if (btn_opr.Get##E(np) == n) { return np; }                      \
        n = np;                                                          \
    }

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    Step_(L, R);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    Step_(R, L);
}

#pragma push_macro("Step_")

#pragma push_macro("Advance_")

#define Advance_(D, E)                                                   \
    CheckBinTreeNodeOperator<BinTreeNodeOperator, BinTreeNode>(btn_opr); \
                                                                         \
    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());                  \
                                                                         \
    auto null_acc_size{ btn_opr.GetNullAccSize() };                      \
                                                                         \
    while (n != nullptr && 0 < step) {                                   \
        BinTreeNode* nd{ btn_opr.Get##D(n) };                            \
        BinTreeNode* ne{ btn_opr.Get##E(n) };                            \
                                                                         \
        size_t n_acc_size{ btn_opr.GetAccSize(n) };                      \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                \
                                          : btn_opr.GetAccSize(nd) };    \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size                \
                                          : btn_opr.GetAccSize(ne) };    \
                                                                         \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };         \
                                                                         \
        if (step < n_size) { break; }                                    \
                                                                         \
        step -= n_size;                                                  \
                                                                         \
        if (step < nd_acc_size) { return Access##E(btn_opr, nd, step); } \
                                                                         \
        step -= nd_acc_size;                                             \
                                                                         \
        for (;;) {                                                       \
            BinTreeNode* np{ btn_opr.GetP(n) };                          \
                                                                         \
            if (np == nullptr || btn_opr.Get##E(np) == n) {              \
                n = np;                                                  \
                break;                                                   \
            }                                                            \
                                                                         \
            n = np;                                                      \
        }                                                                \
    }                                                                    \
                                                                         \
    return { n, step };

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceL(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n, size_t step) {
    Advance_(L, R);
}

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceR(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n, size_t step) {
    Advance_(R, L);
}

#pragma pop_macro("Advance_")

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<size_t, size_t> GetLRAccSize(BinTreeNodeOperator const& btn_opr,
                                  BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    ZETA_Core_StaticAssert(btn_opr.IsAccSizeEnabled());

    ZETA_Core_DebugAssert(n != nullptr);

    auto null_acc_size{ btn_opr.GetNullAccSize() };

    BinTreeNode* nl{ btn_opr.GetL(n) };
    BinTreeNode* nr{ btn_opr.GetR(n) };

    size_t l_acc_size{ nl == nullptr ? null_acc_size : btn_opr.GetAccSize(nl) };
    size_t r_acc_size{ nr == nullptr ? null_acc_size : btn_opr.GetAccSize(nr) };

    size_t n_acc_size{ btn_opr.GetAccSize(n) };

    for (;;) {
        BinTreeNode* np{ btn_opr.GetP(n) };
        if (np == nullptr) { break; }

        size_t np_acc_size{ btn_opr.GetAccSize(np) };
        size_t k{ np_acc_size - n_acc_size };

        if (btn_opr.GetL(np) == n) {
            r_acc_size += k;
        } else {
            l_acc_size += k;
        }

        n = np;
        n_acc_size = np_acc_size;
    }

    return { l_acc_size, r_acc_size };
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Sanitize_(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    for (BinTreeNode* m{ n };;) {
        BinTreeNode* ml{ btn_opr.GetL(m) };
        BinTreeNode* mr{ btn_opr.GetR(m) };

        ZETA_Core_DebugAssert((ml == nullptr && mr == nullptr) || (ml != mr));

        if (ml != nullptr) { ZETA_Core_DebugAssert(btn_opr.GetP(ml) == m); }
        if (mr != nullptr) { ZETA_Core_DebugAssert(btn_opr.GetP(mr) == m); }

        if (ml != nullptr) {
            m = ml;
            continue;
        }

        if (mr != nullptr) {
            m = mr;
            continue;
        }

        if (m == n) { break; }

        for (;;) {
            BinTreeNode* mp{ btn_opr.GetP(m) };

            if (btn_opr.GetR(mp) == m) {
                m = mp;
                continue;
            }

            BinTreeNode* mpr{ btn_opr.GetR(mp) };

            if (mp != n) {
                m = mpr == nullptr ? mp : mpr;
            } else if (mpr != nullptr) {
                n = m = mpr;
            } else {
                break;
            }
        }
    }
}

}  // namespace detail

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Sanitize(BinTreeNodeOperator const& btn_opr, BinTreeNode* root) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    constexpr size_t buffer_capacity{ ZETA_Core_ullong_width * 4 };

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(btn_opr.GetP(root) == nullptr);

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = root;

    unsigned long long random_seed{ GetRandom() };

    while (0 < buffer_i) {
        BinTreeNode* n{ buffer[--buffer_i] };

        BinTreeNode* nl{ btn_opr.GetL(n) };
        BinTreeNode* nr{ btn_opr.GetR(n) };

        if (SimpleRandomRotate(&random_seed) % 2 == 1) { Swap(nl, nr); }

        ZETA_Core_DebugAssert((nl == nullptr && nr == nullptr) || (nl != nr));

        if (nl != nullptr) {
            ZETA_Core_DebugAssert(btn_opr.GetP(nl) == n);

            if (buffer_i == buffer_capacity) {
                detail::Sanitize_(btn_opr, nl);
            } else {
                buffer[buffer_i++] = nl;
            }
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert(btn_opr.GetP(nr) == n);

            if (buffer_i == buffer_capacity) {
                detail::Sanitize_(btn_opr, nr);
            } else {
                buffer[buffer_i++] = nr;
            }
        }
    }
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename BinTreeNodeOperator, typename BinTreeNode>
size_t Count_(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    size_t ret{ 0 };

    for (BinTreeNode* m; n != nullptr; n = btn_opr.GetR(n), ++ret) {
        for (m = GetMostL(btn_opr, n).first; m != n; m = StepR(btn_opr, m)) {
            ++ret;
        }
    }

    return ret;
}

}  // namespace detail

template <typename BinTreeNodeOperator, typename BinTreeNode>
size_t Count(BinTreeNodeOperator const& btn_opr, BinTreeNode* n) {
    CheckBinTreeNode<BinTreeNodeOperator, BinTreeNode>(btn_opr);

    constexpr size_t buffer_capacity{ ZETA_Core_ullong_width * 4 };

    if (n == nullptr) { return 0; }

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = n;

    unsigned long long random_seed{ GetRandom() };

    size_t ret{ 0 };

    while (0 < buffer_i) {
        BinTreeNode* m{ buffer[--buffer_i] };

        ++ret;

        BinTreeNode* ml{ btn_opr.GetL(m) };
        BinTreeNode* mr{ btn_opr.GetR(m) };

        if (SimpleRandomRotate(&random_seed) % 2 == 1) { Swap(ml, mr); }

        if (ml != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::Count_(btn_opr, ml);
            } else {
                buffer[buffer_i++] = ml;
            }
        }

        if (mr != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::Count_(btn_opr, mr);
            } else {
                buffer[buffer_i++] = mr;
            }
        }
    }

    return ret;
}

}  // namespace zeta::core::bin_tree
