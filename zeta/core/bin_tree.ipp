#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <bin_tree::IsNode Node>
constexpr bool bin_tree::IsConst() {
    constexpr bool ret{ Node::IsConst(Tag{}, meta::TypeWrapper<Node>{}) };

    return ret;
}

template <bin_tree::IsNode Node>
constexpr bool bin_tree::HasAccSize() {
    constexpr bool ret{ Node::HasAccSize(Tag{}, meta::TypeWrapper<Node>{}) };

    return ret;
}

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::GetP(Node* n) {
    return n->GetP(Tag{});
}

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::GetL(Node* n) {
    return n->GetL(Tag{});
}

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::GetR(Node* n) {
    return n->GetR(Tag{});
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::SetP(Node* n, Node* m) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());

    n->SetP(Tag{}, m);
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::SetL(Node* n, Node* m) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());

    n->SetL(Tag{}, m);
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::SetR(Node* n, Node* m) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());

    n->SetR(Tag{}, m);
}

template <bin_tree::IsNode Node>
constexpr size_t bin_tree::GetNullAccSize() {
    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    constexpr size_t ret{ Node::GetNullAccSize(Tag{},
                                               meta::TypeWrapper<Node>{}) };

    return ret;
}

template <bin_tree::IsNode Node>
constexpr size_t bin_tree::GetAccSize(Node* n) {
    ZETA_Core_StaticAssert((HasAccSize<Node>)());
    return n->GetAccSize(Tag{});
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::SetAccSize(Node* n, size_t acc_size) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());
    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    n->SetAccSize(Tag{}, acc_size);
}

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::GetMostP(Node* n) {
    return utils::GetMostLink(n, (GetP<Node>));
}

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::GetMostL(Node* n) {
    return utils::GetMostLink(n, (GetL<Node>));
}

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::GetMostR(Node* n) {
    return utils::GetMostLink(n, (GetR<Node>));
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::AddDiffSize(Node* n, size_t diff_size) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());

    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    if (diff_size == 0) { return; }

    for (; n != nullptr; n = (GetP)(n)) {
        (SetAccSize)(n, (GetAccSize)(n) + diff_size);
    }
}

template <bin_tree::IsNode Node>
constexpr size_t bin_tree::GetSize(Node* n) {
    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    constexpr size_t null_acc_size{ (GetNullAccSize<Node>)() };

    if (n == nullptr) { return null_acc_size; }

    Node* nl{ (GetL)(n) };
    Node* nr{ (GetR)(n) };

    return (GetAccSize)(n) -
           (nl == nullptr ? null_acc_size : (GetAccSize)(nl)) -
           (nr == nullptr ? null_acc_size : (GetAccSize)(nr));
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::SetSize(Node* n, size_t size) {
    ZETA_Core_StaticAssert(!(IsConst<Node>)());

    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    ZETA_Core_DebugAssert(n != nullptr);

    (AddDiffSize)(n, size - (GetSize)(n));
}

#pragma push_macro("Attatch_")

#define Attatch_(D)                                                     \
                                                                        \
    ZETA_Core_DebugAssert(pos != nullptr);                              \
                                                                        \
    ZETA_Core_DebugAssert((Get##D)(pos) == nullptr);                    \
    ZETA_Core_DebugAssert(n == nullptr || (GetP)(n) == nullptr);        \
                                                                        \
    if (n == nullptr) { return; }                                       \
                                                                        \
    (Set##D)(pos, n);                                                   \
    (SetP)(n, pos);                                                     \
                                                                        \
    if constexpr ((HasAccSize<Node>())) {                               \
        (AddDiffSize)(pos, (GetAccSize)(n) - (GetNullAccSize<Node>)()); \
    }

template <bin_tree::IsNode Node>
constexpr void bin_tree::AttatchL(Node* pos, Node* n) {
    Attatch_(L);
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::AttatchR(Node* pos, Node* n) {
    Attatch_(R);
}

#pragma pop_macro("Attatch_")

template <bin_tree::IsNode Node>
constexpr void bin_tree::Detach(Node* n) {
    ZETA_Core_DebugAssert(n != nullptr);

    Node* np{ (GetP)(n) };

    if (np == nullptr) { return; }

    if ((GetL)(np) == n) {
        (SetL)(np, static_cast<Node*>(nullptr));
    } else {
        (SetR)(np, static_cast<Node*>(nullptr));
    }

    (SetP)(n, static_cast<Node*>(nullptr));

    if constexpr ((HasAccSize<Node>)()) {
        (AddDiffSize)(np, (GetNullAccSize<Node>)() - (GetAccSize)(n));
    }
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::Swap(Node* n, Node* m) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    if (n == m) { return; }

    Node* np{ (GetP)(n) };
    Node* mp{ (GetP)(m) };

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

    Node* nl{ (GetL)(n) };
    Node* nr{ (GetR)(n) };

    Node* ml{ (GetL)(m) };
    Node* mr{ (GetR)(m) };

    if constexpr ((HasAccSize<Node>)()) {
        constexpr size_t null_acc_size{ (GetNullAccSize<Node>)() };

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

    if constexpr ((HasAccSize<Node>)()) {
        (SetAccSize)(n, m_acc_size);
        (SetAccSize)(m, n_acc_size);

        (AddDiffSize)(n, n_size - m_size);
        (AddDiffSize)(m, m_size - n_size);
    }
}

#pragma push_macro("Rotate_")

#define Rotate_(D, E)                                                  \
                                                                       \
    ZETA_Core_DebugAssert(n != nullptr);                               \
                                                                       \
    Node* ne{ (Get##E)(n) };                                           \
    ZETA_Core_DebugAssert(ne != nullptr);                              \
                                                                       \
    Node* ned{ (Get##D)(ne) };                                         \
    Node* np{ (GetP)(n) };                                             \
                                                                       \
    if (np != nullptr) {                                               \
        if ((Get##D)(np) == n) {                                       \
            (Set##D)(np, ne);                                          \
        } else {                                                       \
            (Set##E)(np, ne);                                          \
        }                                                              \
    }                                                                  \
                                                                       \
    (SetP)(ne, np);                                                    \
                                                                       \
    (Set##D)(ne, n);                                                   \
    (SetP)(n, ne);                                                     \
                                                                       \
    (Set##E)(n, ned);                                                  \
    if (ned != nullptr) { (SetP)(ned, n); }                            \
                                                                       \
    if constexpr ((HasAccSize<Node>)()) {                              \
        size_t n_acc_size{ (GetAccSize)(n) };                          \
        size_t ne_acc_size{ (GetAccSize)(ne) };                        \
        size_t ned_acc_size{ ned == nullptr ? (GetNullAccSize<Node>)() \
                                            : (GetAccSize)(ned) };     \
                                                                       \
        (SetAccSize)(n, n_acc_size - ne_acc_size + ned_acc_size);      \
                                                                       \
        (SetAccSize)(ne, n_acc_size);                                  \
    };

template <bin_tree::IsNode Node>
constexpr void bin_tree::RotateL(Node* n) {
    Rotate_(L, R);
}

template <bin_tree::IsNode Node>
constexpr void bin_tree::RotateR(Node* n) {
    Rotate_(R, L);
}

#pragma pop_macro("Rotate_")

#pragma push_macro("Access_")

#define Access_(D, E)                                                    \
                                                                         \
    ZETA_Core_StaticAssert((HasAccSize<Node>)());                        \
                                                                         \
    constexpr size_t null_acc_size{ (GetNullAccSize<Node>)() };          \
                                                                         \
    size_t n_acc_size{ n == nullptr ? null_acc_size : (GetAccSize)(n) }; \
                                                                         \
    if (n_acc_size <= idx) { return { nullptr, idx - n_acc_size }; }     \
                                                                         \
    while (n != nullptr) {                                               \
        Node* nd{ (Get##D)(n) };                                         \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                \
                                          : (GetAccSize)(nd) };          \
                                                                         \
        if (idx < nd_acc_size) {                                         \
            n = nd;                                                      \
            n_acc_size = nd_acc_size;                                    \
            continue;                                                    \
        }                                                                \
                                                                         \
        Node* ne{ (Get##E)(n) };                                         \
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

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::AccessL(Node* n, size_t idx) {
    Access_(L, R);
}

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::AccessR(Node* n, size_t idx) {
    Access_(R, L);
}

#pragma pop_macro("Access_")

#pragma push_macro("StepP_")

#define StepP_(D)                                 \
                                                  \
    ZETA_Core_StaticAssert((HasAccSize<Node>)()); \
                                                  \
    ZETA_Core_DebugAssert(n != nullptr);          \
                                                  \
    for (;;) {                                    \
        Node* np{ (GetP)(n) };                    \
        if (np == nullptr) { return nullptr; }    \
        if ((Get##D)(np) == n) { return np; }     \
        n = np;                                   \
    }

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::StepPL(Node* n) {
    StepP_(L);
}

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::StepPR(Node* n) {
    StepP_(R);
}

#pragma pop_macro("StepP_")

#pragma push_macro("Step_")

#define Step_(D, E)                                       \
                                                          \
    ZETA_Core_DebugAssert(n != nullptr);                  \
                                                          \
    Node* nd{ (Get##D)(n) };                              \
                                                          \
    if (nd != nullptr) { return (GetMost##E)(nd).first; } \
                                                          \
    for (;;) {                                            \
        Node* np{ (GetP)(n) };                            \
        if (np == nullptr) { return nullptr; }            \
        if ((Get##E)(np) == n) { return np; }             \
        n = np;                                           \
    }

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::StepL(Node* n) {
    Step_(L, R);
}

template <bin_tree::IsNode Node>
constexpr Node* bin_tree::StepR(Node* n) {
    Step_(R, L);
}

#pragma push_macro("Step_")

#pragma push_macro("Advance_")

#define Advance_(D, E)                                            \
                                                                  \
    ZETA_Core_StaticAssert((HasAccSize<Node>)());                 \
                                                                  \
    constexpr size_t null_acc_size{ (GetNullAccSize<Node>)() };   \
                                                                  \
    while (n != nullptr && 0 < step) {                            \
        Node* nd{ (Get##D)(n) };                                  \
        Node* ne{ (Get##E)(n) };                                  \
                                                                  \
        size_t n_acc_size{ (GetAccSize)(n) };                     \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size         \
                                          : (GetAccSize)(nd) };   \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size         \
                                          : (GetAccSize)(ne) };   \
                                                                  \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };  \
                                                                  \
        if (step < n_size) { break; }                             \
                                                                  \
        step -= n_size;                                           \
                                                                  \
        if (step < nd_acc_size) { return (Access##E)(nd, step); } \
                                                                  \
        step -= nd_acc_size;                                      \
                                                                  \
        for (;;) {                                                \
            Node* np{ (GetP)(n) };                                \
                                                                  \
            if (np == nullptr || (Get##E)(np) == n) {             \
                n = np;                                           \
                break;                                            \
            }                                                     \
                                                                  \
            n = np;                                               \
        }                                                         \
    }                                                             \
                                                                  \
    return { n, step };

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::AdvanceL(Node* n, size_t step) {
    Advance_(L, R);
}

template <bin_tree::IsNode Node>
constexpr pair::Pair<Node*, size_t> bin_tree::AdvanceR(Node* n, size_t step) {
    Advance_(R, L);
}

#pragma pop_macro("Advance_")

template <bin_tree::IsNode Node>
constexpr pair::Pair<size_t, size_t> bin_tree::GetLRAccSize(Node* n) {
    ZETA_Core_StaticAssert((HasAccSize<Node>)());

    ZETA_Core_DebugAssert(n != nullptr);

    constexpr size_t null_acc_size{ (GetNullAccSize<Node>)() };

    Node* nl{ (GetL)(n) };
    Node* nr{ (GetR)(n) };

    size_t l_acc_size{ nl == nullptr ? null_acc_size : (GetAccSize)(nl) };
    size_t r_acc_size{ nr == nullptr ? null_acc_size : (GetAccSize)(nr) };

    size_t n_acc_size{ (GetAccSize)(n) };

    for (;;) {
        Node* np{ (GetP)(n) };
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

template <bin_tree::IsNode Node>
constexpr void SanitizeCore(Node* n) {
    for (Node* m{ n };;) {
        Node* ml{ (GetL)(m) };
        Node* mr{ (GetR)(m) };

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
            Node* mp{ (GetP)(m) };

            if ((GetR)(mp) == m) {
                m = mp;
                continue;
            }

            Node* mpr{ (GetR)(mp) };

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

template <bin_tree::IsNode Node>
constexpr void bin_tree::Sanitize(Node* root) {
    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert((GetP)(root) == nullptr);

    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    Node* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = root;

    unsigned long long random_seed{ utils::GetRandom() };

    while (0 < buffer_i) {
        Node* n{ buffer[--buffer_i] };

        Node* nl{ (GetL)(n) };
        Node* nr{ (GetR)(n) };

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

template <bin_tree::IsNode Node>
constexpr size_t Count_(Node* n) {
    size_t ret{ 0 };

    for (Node* m; n != nullptr; n = (GetR)(n), ++ret) {
        for (m = GetMostL(n).first; m != n; m = StepR(m)) { ++ret; }
    }

    return ret;
}

}  // namespace bin_tree::detail

template <bin_tree::IsNode Node>
constexpr size_t bin_tree::Count(Node* n) {
    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    constexpr size_t random_swap_th{ buffer_capacity / 2 };

    if (n == nullptr) { return 0; }

    Node* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = n;

    unsigned long long random_seed{ utils::GetRandom() };

    size_t ret{ 0 };

    while (0 < buffer_i) {
        Node* m{ buffer[--buffer_i] };

        ++ret;

        Node* ml{ (GetL)(m) };
        Node* mr{ (GetR)(m) };

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
