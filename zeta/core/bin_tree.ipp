#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename BinTreeNode>
constexpr bool bin_tree::IsConst() {
    constexpr bool ret{ NodeTraits<BinTreeNode>::IsConst() };
    return ret;
}

template <typename BinTreeNode>
constexpr bool bin_tree::HasAccSize() {
    constexpr bool ret{ NodeTraits<BinTreeNode>::HasAccSize() };
    return ret;
}

template <typename BinTreeNode>
BinTreeNode* bin_tree::GetP(BinTreeNode* n) {
    return NodeTraits<BinTreeNode>::GetP(n);
}

template <typename BinTreeNode>
BinTreeNode* bin_tree::GetL(BinTreeNode* n) {
    return NodeTraits<BinTreeNode>::GetL(n);
}

template <typename BinTreeNode>
BinTreeNode* bin_tree::GetR(BinTreeNode* n) {
    return NodeTraits<BinTreeNode>::GetR(n);
}

template <typename BinTreeNode>
void bin_tree::SetP(BinTreeNode* n, BinTreeNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());
    NodeTraits<BinTreeNode>::SetP(n, m);
}

template <typename BinTreeNode>
void bin_tree::SetL(BinTreeNode* n, BinTreeNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());
    NodeTraits<BinTreeNode>::SetL(n, m);
}

template <typename BinTreeNode>
void bin_tree::SetR(BinTreeNode* n, BinTreeNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());
    NodeTraits<BinTreeNode>::SetR(n, m);
}

template <typename BinTreeNode>
constexpr size_t bin_tree::GetNullAccSize() {
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());
    constexpr size_t ret{ NodeTraits<BinTreeNode>::GetNullAccSize() };
    return ret;
}

template <typename BinTreeNode>
size_t bin_tree::GetAccSize(BinTreeNode* n) {
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());
    return NodeTraits<BinTreeNode>::GetAccSize(n);
}

template <typename BinTreeNode>
void bin_tree::SetAccSize(BinTreeNode* n, size_t acc_size) {
    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());
    NodeTraits<BinTreeNode>::SetAccSize(n, acc_size);
}

template <typename BinTreeNode>
void bin_tree::CheckContract() {
    BinTreeNode* btn{ nullptr };
    size_t size_val{ 0 };

    constexpr bool is_const{ (IsConst<BinTreeNode>)() };
    constexpr bool hash_acc_size{ (HasAccSize<BinTreeNode>)() };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                                               \
    ZETA_Core_Unused((                                                         \
        meta::Conditional<false, decltype((method<BinTreeNode>)(__VA_ARGS__)), \
                          int>{}))

    CheckMethod(GetP, btn);
    CheckMethod(GetL, btn);
    CheckMethod(GetR, btn);

    if constexpr (!is_const) {
        CheckMethod(SetP, btn, btn);
        CheckMethod(SetL, btn, btn);
        CheckMethod(SetR, btn, btn);
    }

    if constexpr (hash_acc_size) {
        CheckMethod(GetNullAccSize);

        CheckMethod(GetAccSize, btn);

        if constexpr (!is_const) { CheckMethod(SetAccSize, btn, size_val); }
    }

#pragma pop_macro("CheckMethod")
}

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::GetMostP(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    return utils::GetMostLink(n, [](BinTreeNode* x) { return (GetP)(x); });
}

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::GetMostL(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    return utils::GetMostLink(n, [](BinTreeNode* x) { return (GetL)(x); });
}

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::GetMostR(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    return utils::GetMostLink(n, [](BinTreeNode* x) { return (GetR)(x); });
}

template <typename BinTreeNode>
void bin_tree::AddDiffSize(BinTreeNode* n, size_t diff_size) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());

    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());

    if (diff_size == 0) { return; }

    for (; n != nullptr; n = (GetP)(n)) {
        (SetAccSize)(n, (GetAccSize)(n) + diff_size);
    }
}

template <typename BinTreeNode>
size_t bin_tree::GetSize(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());

    constexpr size_t null_acc_size{ (GetNullAccSize<BinTreeNode>)() };

    if (n == nullptr) { return null_acc_size; }

    BinTreeNode* nl{ (GetL)(n) };
    BinTreeNode* nr{ (GetR)(n) };

    return (GetAccSize)(n) -
           (nl == nullptr ? null_acc_size : (GetAccSize)(nl)) -
           (nr == nullptr ? null_acc_size : (GetAccSize)(nr));
}

template <typename BinTreeNode>
void bin_tree::SetSize(BinTreeNode* n, size_t size) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_StaticAssert(!(IsConst<BinTreeNode>)());

    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());

    ZETA_Core_DebugAssert(n != nullptr);

    (AddDiffSize)(n, size - (GetSize)(n));
}

#pragma push_macro("Attatch_")

#define Attatch_(D)                                                            \
    (CheckContract<BinTreeNode>)();                                            \
                                                                               \
    ZETA_Core_DebugAssert(pos != nullptr);                                     \
                                                                               \
    ZETA_Core_DebugAssert((Get##D)(pos) == nullptr);                           \
    ZETA_Core_DebugAssert(n == nullptr || (GetP)(n) == nullptr);               \
                                                                               \
    if (n == nullptr) { return; }                                              \
                                                                               \
    (Set##D)(pos, n);                                                          \
    (SetP)(n, pos);                                                            \
                                                                               \
    if constexpr ((HasAccSize<BinTreeNode>())) {                               \
        (AddDiffSize)(pos, (GetAccSize)(n) - (GetNullAccSize<BinTreeNode>)()); \
    }

template <typename BinTreeNode>
void bin_tree::AttatchL(BinTreeNode* pos, BinTreeNode* n) {
    Attatch_(L);
}

template <typename BinTreeNode>
void bin_tree::AttatchR(BinTreeNode* pos, BinTreeNode* n) {
    Attatch_(R);
}

#pragma pop_macro("Attatch_")

template <typename BinTreeNode>
void bin_tree::Detach(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_DebugAssert(n != nullptr);

    BinTreeNode* np{ (GetP)(n) };

    if (np == nullptr) { return; }

    if ((GetL)(np) == n) {
        (SetL)(np, static_cast<BinTreeNode*>(nullptr));
    } else {
        (SetR)(np, static_cast<BinTreeNode*>(nullptr));
    }

    (SetP)(n, static_cast<BinTreeNode*>(nullptr));

    if constexpr ((HasAccSize<BinTreeNode>)()) {
        (AddDiffSize)(np, (GetNullAccSize<BinTreeNode>)() - (GetAccSize)(n));
    }
}

template <typename BinTreeNode>
void bin_tree::Swap(BinTreeNode* n, BinTreeNode* m) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    if (n == m) { return; }

    BinTreeNode* np{ (GetP)(n) };
    BinTreeNode* mp{ (GetP)(m) };

    if (np == m) {
        utils::Swap(n, m);
        utils::Swap(np, mp);
    }

    size_t n_acc_size;
    size_t m_acc_size;

    size_t n_size;
    size_t m_size;

    ZETA_Core_Unused(n_acc_size);
    ZETA_Core_Unused(m_acc_size);

    ZETA_Core_Unused(n_size);
    ZETA_Core_Unused(m_size);

    BinTreeNode* nl{ (GetL)(n) };
    BinTreeNode* nr{ (GetR)(n) };

    BinTreeNode* ml{ (GetL)(m) };
    BinTreeNode* mr{ (GetR)(m) };

    if constexpr ((HasAccSize<BinTreeNode>)()) {
        constexpr size_t null_acc_size{ (GetNullAccSize<BinTreeNode>)() };

        n_acc_size = (GetAccSize)(n);
        m_acc_size = (GetAccSize)(m);

        n_size = n_acc_size -
                 (nl == nullptr ? null_acc_size : (GetAccSize)(nl)) -
                 (nr == nullptr ? null_acc_size : (GetAccSize)(nr));

        m_size = m_acc_size -
                 (ml == nullptr ? null_acc_size : (GetAccSize)(ml)) -
                 (mr == nullptr ? null_acc_size : (GetAccSize)(mr));
    }

    if (mp == n) {
        if (np == nullptr) {
        } else if ((GetL)(np) == n) {
            (SetL)(np, m);
        } else {
            (SetR)(np, m);
        }

        (SetP)(m, np);

        if ((GetL)(n) == m) {
            (SetL)(m, n);

            (SetR)(m, nr);
            if (nr != nullptr) { (SetP)(nr, m); }
        } else {
            (SetL)(m, nl);
            if (nl != nullptr) { (SetP)(nl, m); }

            (SetR)(m, n);
        }

        (SetP)(n, m);

        (SetL)(n, ml);
        if (ml != nullptr) { (SetP)(ml, n); }

        (SetR)(n, mr);
        if (mr != nullptr) { (SetP)(mr, n); }
    } else {
        if (np == nullptr) {
        } else if ((GetL)(np) == n) {
            (SetL)(np, m);
        } else {
            (SetR)(np, m);
        }

        (SetP)(m, np);

        (SetL)(m, nl);
        if (nl != nullptr) { (SetP)(nl, m); }

        (SetR)(m, nr);
        if (nr != nullptr) { (SetP)(nr, m); }

        if (mp == nullptr) {
        } else if ((GetL)(mp) == m) {
            (SetL)(mp, n);
        } else {
            (SetR)(mp, n);
        }

        (SetP)(n, mp);

        (SetL)(n, ml);
        if (ml != nullptr) { (SetP)(ml, n); }

        (SetR)(n, mr);
        if (mr != nullptr) { (SetP)(mr, n); }
    }

    if constexpr ((HasAccSize<BinTreeNode>)()) {
        (SetAccSize)(n, m_acc_size);
        (SetAccSize)(m, n_acc_size);

        (AddDiffSize)(n, n_size - m_size);
        (AddDiffSize)(m, m_size - n_size);
    }
}

#pragma push_macro("Rotate_")

#define Rotate_(D, E)                                                         \
    (CheckContract<BinTreeNode>)();                                           \
                                                                              \
    ZETA_Core_DebugAssert(n != nullptr);                                      \
                                                                              \
    BinTreeNode* ne{ (Get##E)(n) };                                           \
    ZETA_Core_DebugAssert(ne != nullptr);                                     \
                                                                              \
    BinTreeNode* ned{ (Get##D)(ne) };                                         \
    BinTreeNode* np{ (GetP)(n) };                                             \
                                                                              \
    if (np != nullptr) {                                                      \
        if ((Get##D)(np) == n) {                                              \
            (Set##D)(np, ne);                                                 \
        } else {                                                              \
            (Set##E)(np, ne);                                                 \
        }                                                                     \
    }                                                                         \
                                                                              \
    (SetP)(ne, np);                                                           \
                                                                              \
    (Set##D)(ne, n);                                                          \
    (SetP)(n, ne);                                                            \
                                                                              \
    (Set##E)(n, ned);                                                         \
    if (ned != nullptr) { (SetP)(ned, n); }                                   \
                                                                              \
    if constexpr ((HasAccSize<BinTreeNode>)()) {                              \
        size_t n_acc_size{ (GetAccSize)(n) };                                 \
        size_t ne_acc_size{ (GetAccSize)(ne) };                               \
        size_t ned_acc_size{ ned == nullptr ? (GetNullAccSize<BinTreeNode>)() \
                                            : (GetAccSize)(ned) };            \
                                                                              \
        (SetAccSize)(n, n_acc_size - ne_acc_size + ned_acc_size);             \
                                                                              \
        (SetAccSize)(ne, n_acc_size);                                         \
    };

template <typename BinTreeNode>
void bin_tree::RotateL(BinTreeNode* n) {
    Rotate_(L, R);
}

template <typename BinTreeNode>
void bin_tree::RotateR(BinTreeNode* n) {
    Rotate_(R, L);
}

#pragma pop_macro("Rotate_")

#pragma push_macro("Access_")

#define Access_(D, E)                                                    \
    (CheckContract<BinTreeNode>)();                                      \
                                                                         \
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());                 \
                                                                         \
    constexpr size_t null_acc_size{ (GetNullAccSize<BinTreeNode>)() };   \
                                                                         \
    size_t n_acc_size{ n == nullptr ? null_acc_size : (GetAccSize)(n) }; \
                                                                         \
    if (n_acc_size <= idx) { return { nullptr, idx - n_acc_size }; }     \
                                                                         \
    while (n != nullptr) {                                               \
        BinTreeNode* nd{ (Get##D)(n) };                                  \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                \
                                          : (GetAccSize)(nd) };          \
                                                                         \
        if (idx < nd_acc_size) {                                         \
            n = nd;                                                      \
            n_acc_size = nd_acc_size;                                    \
            continue;                                                    \
        }                                                                \
                                                                         \
        BinTreeNode* ne{ (Get##E)(n) };                                  \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size                \
                                          : (GetAccSize)(ne) };          \
                                                                         \
        idx -= nd_acc_size;                                              \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };         \
                                                                         \
        if (idx < n_size) { break; }                                     \
                                                                         \
        n = ne;                                                          \
        n_acc_size = ne_acc_size;                                        \
        idx -= n_size;                                                   \
    }                                                                    \
                                                                         \
    return { n, idx };

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::AccessL(BinTreeNode* n, size_t idx) {
    Access_(L, R);
}

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::AccessR(BinTreeNode* n, size_t idx) {
    Access_(R, L);
}

#pragma pop_macro("Access_")

#pragma push_macro("StepP_")

#define StepP_(D)                                        \
    (CheckContract<BinTreeNode>)();                      \
                                                         \
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)()); \
                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                 \
                                                         \
    for (;;) {                                           \
        BinTreeNode* np{ (GetP)(n) };                    \
        if (np == nullptr) { return nullptr; }           \
        if ((Get##D)(np) == n) { return np; }            \
        n = np;                                          \
    }

template <typename BinTreeNode>
BinTreeNode* bin_tree::StepPL(BinTreeNode* n) {
    StepP_(L);
}

template <typename BinTreeNode>
BinTreeNode* bin_tree::StepPR(BinTreeNode* n) {
    StepP_(R);
}

#pragma pop_macro("StepP_")

#pragma push_macro("Step_")

#define Step_(D, E)                                       \
    (CheckContract<BinTreeNode>)();                       \
                                                          \
    ZETA_Core_DebugAssert(n != nullptr);                  \
                                                          \
    BinTreeNode* nd{ (Get##D)(n) };                       \
                                                          \
    if (nd != nullptr) { return (GetMost##E)(nd).first; } \
                                                          \
    for (;;) {                                            \
        BinTreeNode* np{ (GetP)(n) };                     \
        if (np == nullptr) { return nullptr; }            \
        if ((Get##E)(np) == n) { return np; }             \
        n = np;                                           \
    }

template <typename BinTreeNode>
BinTreeNode* bin_tree::StepL(BinTreeNode* n) {
    Step_(L, R);
}

template <typename BinTreeNode>
BinTreeNode* bin_tree::StepR(BinTreeNode* n) {
    Step_(R, L);
}

#pragma push_macro("Step_")

#pragma push_macro("Advance_")

#define Advance_(D, E)                                                 \
    (CheckContract<BinTreeNode>)();                                    \
                                                                       \
    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());               \
                                                                       \
    constexpr size_t null_acc_size{ (GetNullAccSize<BinTreeNode>)() }; \
                                                                       \
    while (n != nullptr && 0 < step) {                                 \
        BinTreeNode* nd{ (Get##D)(n) };                                \
        BinTreeNode* ne{ (Get##E)(n) };                                \
                                                                       \
        size_t n_acc_size{ (GetAccSize)(n) };                          \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size              \
                                          : (GetAccSize)(nd) };        \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size              \
                                          : (GetAccSize)(ne) };        \
                                                                       \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };       \
                                                                       \
        if (step < n_size) { break; }                                  \
                                                                       \
        step -= n_size;                                                \
                                                                       \
        if (step < nd_acc_size) { return (Access##E)(nd, step); }      \
                                                                       \
        step -= nd_acc_size;                                           \
                                                                       \
        for (;;) {                                                     \
            BinTreeNode* np{ (GetP)(n) };                              \
                                                                       \
            if (np == nullptr || (Get##E)(np) == n) {                  \
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
pair::Pair<BinTreeNode*, size_t> bin_tree::AdvanceL(BinTreeNode* n,
                                                    size_t step) {
    Advance_(L, R);
}

template <typename BinTreeNode>
pair::Pair<BinTreeNode*, size_t> bin_tree::AdvanceR(BinTreeNode* n,
                                                    size_t step) {
    Advance_(R, L);
}

#pragma pop_macro("Advance_")

template <typename BinTreeNode>
pair::Pair<size_t, size_t> bin_tree::GetLRAccSize(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    ZETA_Core_StaticAssert((HasAccSize<BinTreeNode>)());

    ZETA_Core_DebugAssert(n != nullptr);

    constexpr size_t null_acc_size{ (GetNullAccSize<BinTreeNode>)() };

    BinTreeNode* nl{ (GetL)(n) };
    BinTreeNode* nr{ (GetR)(n) };

    size_t l_acc_size{ nl == nullptr ? null_acc_size : (GetAccSize)(nl) };
    size_t r_acc_size{ nr == nullptr ? null_acc_size : (GetAccSize)(nr) };

    size_t n_acc_size{ (GetAccSize)(n) };

    for (;;) {
        BinTreeNode* np{ (GetP)(n) };
        if (np == nullptr) { break; }

        size_t np_acc_size{ (GetAccSize)(np) };
        size_t k{ np_acc_size - n_acc_size };

        if ((GetL)(np) == n) {
            r_acc_size += k;
        } else {
            l_acc_size += k;
        }

        n = np;
        n_acc_size = np_acc_size;
    }

    return { l_acc_size, r_acc_size };
}

namespace bin_tree::detail {

template <typename BinTreeNode>
void SanitizeCore(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    for (BinTreeNode* m{ n };;) {
        BinTreeNode* ml{ (GetL)(m) };
        BinTreeNode* mr{ (GetR)(m) };

        ZETA_Core_DebugAssert((ml == nullptr && mr == nullptr) || (ml != mr));

        if (ml != nullptr) { ZETA_Core_DebugAssert((GetP)(ml) == m); }
        if (mr != nullptr) { ZETA_Core_DebugAssert((GetP)(mr) == m); }

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
            BinTreeNode* mp{ (GetP)(m) };

            if ((GetR)(mp) == m) {
                m = mp;
                continue;
            }

            BinTreeNode* mpr{ (GetR)(mp) };

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

}  // namespace bin_tree::detail

template <typename BinTreeNode>
void bin_tree::Sanitize(BinTreeNode* root) {
    (CheckContract<BinTreeNode>)();

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert((GetP)(root) == nullptr);

    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = root;

    unsigned long long random_seed{ utils::GetRandom() };

    while (0 < buffer_i) {
        BinTreeNode* n{ buffer[--buffer_i] };

        BinTreeNode* nl{ (GetL)(n) };
        BinTreeNode* nr{ (GetR)(n) };

        if (utils::SimpleRandomRotate(&random_seed) % 2 == 1) {
            utils::Swap(nl, nr);
        }

        ZETA_Core_DebugAssert((nl == nullptr && nr == nullptr) || (nl != nr));

        if (nl != nullptr) {
            ZETA_Core_DebugAssert((GetP)(nl) == n);

            if (buffer_i == buffer_capacity) {
                detail::SanitizeCore(nl);
            } else {
                buffer[buffer_i++] = nl;
            }
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert((GetP)(nr) == n);

            if (buffer_i == buffer_capacity) {
                detail::SanitizeCore(nr);
            } else {
                buffer[buffer_i++] = nr;
            }
        }
    }
}

namespace bin_tree::detail {

template <typename BinTreeNode>
size_t Count_(BinTreeNode* n) {
    size_t ret{ 0 };

    for (BinTreeNode* m; n != nullptr; n = (GetR)(n), ++ret) {
        for (m = GetMostL(n).first; m != n; m = StepR(m)) { ++ret; }
    }

    return ret;
}

}  // namespace bin_tree::detail

template <typename BinTreeNode>
size_t bin_tree::Count(BinTreeNode* n) {
    (CheckContract<BinTreeNode>)();

    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    constexpr size_t random_swap_th{ buffer_capacity / 2 };

    if (n == nullptr) { return 0; }

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = n;

    unsigned long long random_seed{ utils::GetRandom() };

    size_t ret{ 0 };

    while (0 < buffer_i) {
        BinTreeNode* m{ buffer[--buffer_i] };

        ++ret;

        BinTreeNode* ml{ (GetL)(m) };
        BinTreeNode* mr{ (GetR)(m) };

        if (random_swap_th <= buffer_i &&
            utils::SimpleRandomRotate(&random_seed) % 2 == 1) {
            utils::Swap(ml, mr);
        }

        if (ml != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::Count_(ml);
            } else {
                buffer[buffer_i++] = ml;
            }
        }

        if (mr != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::Count_(mr);
            } else {
                buffer[buffer_i++] = mr;
            }
        }
    }

    return ret;
}

}  // namespace zeta::core
