#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::bin_tree {

template <typename BinTreeNode>
void CheckContract() {
    constexpr type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;

    BinTreeNode* btn_ptr{ nullptr };
    size_t size_val{ 0 };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(BinTreeNode::method(__VA_ARGS__)), return_type>)

    CheckMethod(GetP, BinTreeNode*, btn_ptr);
    CheckMethod(GetL, BinTreeNode*, btn_ptr);
    CheckMethod(GetR, BinTreeNode*, btn_ptr);

    CheckMethod(IsConst, bool, btn_ptr_type_wrapper);

    if constexpr (!BinTreeNode::IsConst(btn_ptr_type_wrapper)) {
        CheckMethod(SetP, TypeAny, btn_ptr, btn_ptr);
        CheckMethod(SetL, TypeAny, btn_ptr, btn_ptr);
        CheckMethod(SetR, TypeAny, btn_ptr, btn_ptr);
    }

    CheckMethod(IsAccSizeEnabled, bool, btn_ptr_type_wrapper);

    if constexpr (BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper)) {
        CheckMethod(GetNullAccSize, size_t, btn_ptr_type_wrapper);

        CheckMethod(GetAccSize, size_t, btn_ptr);

        if constexpr (!BinTreeNode::IsConst(btn_ptr_type_wrapper)) {
            CheckMethod(SetAccSize, TypeAny, btn_ptr, size_val);
        }
    }

#pragma pop_macro("CheckMethod")
}

// -----------------------------------------------------------------------------

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostP(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    return GetMostLink(n, [&](auto x) { return BinTreeNode::GetP(x); });
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostL(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    return GetMostLink(n, [&](auto x) { return BinTreeNode::GetL(x); });
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostR(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    return GetMostLink(n, [&](auto x) { return BinTreeNode::GetR(x); });
}

// -----------------------------------------------------------------------------

template <typename BinTreeNode>
void AddDiffSize(BinTreeNode* n, size_t diff_size) {
    CheckContract<BinTreeNode>();

    ZETA_Core_StaticAssert(
        !BinTreeNode::IsConst(type_wrapper::TypeWrapper<BinTreeNode*>{}));

    ZETA_Core_StaticAssert(BinTreeNode::IsAccSizeEnabled(
        type_wrapper::TypeWrapper<BinTreeNode*>{}));

    if (diff_size == 0) { return; }

    for (; n != nullptr; n = BinTreeNode::GetP(n)) {
        BinTreeNode::SetAccSize(n, BinTreeNode::GetAccSize(n) + diff_size);
    }
}

template <typename BinTreeNode>
size_t GetSize(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;

    ZETA_Core_StaticAssert(BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper));

    auto null_acc_size{ BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) };

    if (n == nullptr) { return null_acc_size; }

    BinTreeNode* nl{ BinTreeNode::GetL(n) };
    BinTreeNode* nr{ BinTreeNode::GetR(n) };

    return BinTreeNode::GetAccSize(n) -
           (nl == nullptr ? null_acc_size : BinTreeNode::GetAccSize(nl)) -
           (nr == nullptr ? null_acc_size : BinTreeNode::GetAccSize(nr));
}

template <typename BinTreeNode>
void SetSize(BinTreeNode* n, size_t size) {
    CheckContract<BinTreeNode>();

    ZETA_Core_StaticAssert(
        !BinTreeNode::IsConst(type_wrapper::TypeWrapper<BinTreeNode*>{}));

    ZETA_Core_StaticAssert(BinTreeNode::IsAccSizeEnabled(
        type_wrapper::TypeWrapper<BinTreeNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);

    AddDiffSize(n, size - GetSize(n));
}

// -----------------------------------------------------------------------------

#pragma push_macro("AttatchCore")

#define AttatchCore(D)                                                      \
    CheckContract<BinTreeNode>();                                           \
                                                                            \
    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;           \
                                                                            \
    ZETA_Core_DebugAssert(pos != nullptr);                                  \
                                                                            \
    ZETA_Core_DebugAssert(BinTreeNode::Get##D(pos) == nullptr);             \
    ZETA_Core_DebugAssert(n == nullptr || BinTreeNode::GetP(n) == nullptr); \
                                                                            \
    if (n == nullptr) { return; }                                           \
                                                                            \
    BinTreeNode::Set##D(pos, n);                                            \
    BinTreeNode::SetP(n, pos);                                              \
                                                                            \
    if constexpr (BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper)) {    \
        AddDiffSize(pos,                                                    \
                    BinTreeNode::GetAccSize(n) -                            \
                        BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper)); \
    }

template <typename BinTreeNode>
void AttatchL(BinTreeNode* pos, BinTreeNode* n) {
    AttatchCore(L);
}

template <typename BinTreeNode>
void AttatchR(BinTreeNode* pos, BinTreeNode* n) {
    AttatchCore(R);
}

#pragma pop_macro("AttatchCore")

template <typename BinTreeNode>
void Detach(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    ZETA_Core_DebugAssert(n != nullptr);

    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;

    BinTreeNode* np{ BinTreeNode::GetP(n) };

    if (np == nullptr) { return; }

    if (BinTreeNode::GetL(np) == n) {
        BinTreeNode::SetL(np, nullptr);
    } else {
        BinTreeNode::SetR(np, nullptr);
    }

    BinTreeNode::SetP(n, nullptr);

    if constexpr (BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper)) {
        AddDiffSize(np, BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) -
                            BinTreeNode::GetAccSize(n));
    }
}

template <typename BinTreeNode>
void Swap(BinTreeNode* n, BinTreeNode* m) {
    CheckContract<BinTreeNode>();

    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    if (n == m) { return; }

    BinTreeNode* np{ BinTreeNode::GetP(n) };
    BinTreeNode* mp{ BinTreeNode::GetP(m) };

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

    BinTreeNode* nl{ BinTreeNode::GetL(n) };
    BinTreeNode* nr{ BinTreeNode::GetR(n) };

    BinTreeNode* ml{ BinTreeNode::GetL(m) };
    BinTreeNode* mr{ BinTreeNode::GetR(m) };

    if constexpr (BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper)) {
        auto null_acc_size{ BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) };

        n_acc_size = BinTreeNode::GetAccSize(n);
        m_acc_size = BinTreeNode::GetAccSize(m);

        n_size = n_acc_size -
                 (nl == nullptr ? null_acc_size : BinTreeNode::GetAccSize(nl)) -
                 (nr == nullptr ? null_acc_size : BinTreeNode::GetAccSize(nr));

        m_size = m_acc_size -
                 (ml == nullptr ? null_acc_size : BinTreeNode::GetAccSize(ml)) -
                 (mr == nullptr ? null_acc_size : BinTreeNode::GetAccSize(mr));
    }

    if (mp == n) {
        if (np == nullptr) {
        } else if (BinTreeNode::GetL(np) == n) {
            BinTreeNode::SetL(np, m);
        } else {
            BinTreeNode::SetR(np, m);
        }

        BinTreeNode::SetP(m, np);

        if (BinTreeNode::GetL(n) == m) {
            BinTreeNode::SetL(m, n);

            BinTreeNode::SetR(m, nr);
            if (nr != nullptr) { BinTreeNode::SetP(nr, m); }
        } else {
            BinTreeNode::SetL(m, nl);
            if (nl != nullptr) { BinTreeNode::SetP(nl, m); }

            BinTreeNode::SetR(m, n);
        }

        BinTreeNode::SetP(n, m);

        BinTreeNode::SetL(n, ml);
        if (ml != nullptr) { BinTreeNode::SetP(ml, n); }

        BinTreeNode::SetR(n, mr);
        if (mr != nullptr) { BinTreeNode::SetP(mr, n); }
    } else {
        if (np == nullptr) {
        } else if (BinTreeNode::GetL(np) == n) {
            BinTreeNode::SetL(np, m);
        } else {
            BinTreeNode::SetR(np, m);
        }

        BinTreeNode::SetP(m, np);

        BinTreeNode::SetL(m, nl);
        if (nl != nullptr) { BinTreeNode::SetP(nl, m); }

        BinTreeNode::SetR(m, nr);
        if (nr != nullptr) { BinTreeNode::SetP(nr, m); }

        if (mp == nullptr) {
        } else if (BinTreeNode::GetL(mp) == m) {
            BinTreeNode::SetL(mp, n);
        } else {
            BinTreeNode::SetR(mp, n);
        }

        BinTreeNode::SetP(n, mp);

        BinTreeNode::SetL(n, ml);
        if (ml != nullptr) { BinTreeNode::SetP(ml, n); }

        BinTreeNode::SetR(n, mr);
        if (mr != nullptr) { BinTreeNode::SetP(mr, n); }
    }

    if constexpr (BinTreeNode::IsAccSizeEnabled(
                      type_wrapper::TypeWrapper<BinTreeNode*>{})) {
        BinTreeNode::SetAccSize(n, m_acc_size);
        BinTreeNode::SetAccSize(m, n_acc_size);

        AddDiffSize(n, n_size - m_size);
        AddDiffSize(m, m_size - n_size);
    }
}

// -----------------------------------------------------------------------------

#pragma push_macro("RotateCore")

#define RotateCore(D, E)                                                      \
    CheckContract<BinTreeNode>();                                             \
                                                                              \
    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;             \
                                                                              \
    ZETA_Core_DebugAssert(n != nullptr);                                      \
                                                                              \
    BinTreeNode* ne{ BinTreeNode::Get##E(n) };                                \
    ZETA_Core_DebugAssert(ne != nullptr);                                     \
                                                                              \
    BinTreeNode* ned{ BinTreeNode::Get##D(ne) };                              \
    BinTreeNode* np{ BinTreeNode::GetP(n) };                                  \
                                                                              \
    if (np != nullptr) {                                                      \
        if (BinTreeNode::Get##D(np) == n) {                                   \
            BinTreeNode::Set##D(np, ne);                                      \
        } else {                                                              \
            BinTreeNode::Set##E(np, ne);                                      \
        }                                                                     \
    }                                                                         \
                                                                              \
    BinTreeNode::SetP(ne, np);                                                \
                                                                              \
    BinTreeNode::Set##D(ne, n);                                               \
    BinTreeNode::SetP(n, ne);                                                 \
                                                                              \
    BinTreeNode::Set##E(n, ned);                                              \
    if (ned != nullptr) { BinTreeNode::SetP(ned, n); }                        \
                                                                              \
    if constexpr (BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper)) {      \
        size_t n_acc_size{ BinTreeNode::GetAccSize(n) };                      \
        size_t ne_acc_size{ BinTreeNode::GetAccSize(ne) };                    \
        size_t ned_acc_size{ ned == nullptr ? BinTreeNode::GetNullAccSize(    \
                                                  btn_ptr_type_wrapper)       \
                                            : BinTreeNode::GetAccSize(ned) }; \
                                                                              \
        BinTreeNode::SetAccSize(n, n_acc_size - ne_acc_size + ned_acc_size);  \
                                                                              \
        BinTreeNode::SetAccSize(ne, n_acc_size);                              \
    };

template <typename BinTreeNode>
void RotateL(BinTreeNode* n) {
    RotateCore(L, R);
}

template <typename BinTreeNode>
void RotateR(BinTreeNode* n) {
    RotateCore(R, L);
}

#pragma pop_macro("RotateCore")

// -----------------------------------------------------------------------------

#pragma push_macro("AccessCore")

#define AccessCore(D, E)                                                     \
    CheckContract<BinTreeNode>();                                            \
                                                                             \
    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;            \
                                                                             \
    ZETA_Core_StaticAssert(                                                  \
        BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper));                \
                                                                             \
    auto null_acc_size{ BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) }; \
                                                                             \
    size_t n_acc_size{ n == nullptr ? null_acc_size                          \
                                    : BinTreeNode::GetAccSize(n) };          \
                                                                             \
    if (n_acc_size <= idx) { return { nullptr, idx - n_acc_size }; }         \
                                                                             \
    while (n != nullptr) {                                                   \
        BinTreeNode* nd{ BinTreeNode::Get##D(n) };                           \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                    \
                                          : BinTreeNode::GetAccSize(nd) };   \
                                                                             \
        if (idx < nd_acc_size) {                                             \
            n = nd;                                                          \
            n_acc_size = nd_acc_size;                                        \
            continue;                                                        \
        }                                                                    \
                                                                             \
        BinTreeNode* ne{ BinTreeNode::Get##E(n) };                           \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size                    \
                                          : BinTreeNode::GetAccSize(ne) };   \
                                                                             \
        idx -= nd_acc_size;                                                  \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };             \
                                                                             \
        if (idx < n_size) { break; }                                         \
                                                                             \
        n = ne;                                                              \
        n_acc_size = ne_acc_size;                                            \
        idx -= n_size;                                                       \
    }                                                                        \
                                                                             \
    return { n, idx };

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessL(BinTreeNode* n, size_t idx) {
    AccessCore(L, R);
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessR(BinTreeNode* n, size_t idx) {
    AccessCore(R, L);
}

#pragma pop_macro("AccessCore")

#pragma push_macro("StepP_")

#define StepP_(D)                                         \
    CheckContract<BinTreeNode>();                         \
                                                          \
    ZETA_Core_StaticAssert(BinTreeNode::IsAccSizeEnabled( \
        type_wrapper::TypeWrapper<BinTreeNode*>{}));      \
                                                          \
    ZETA_Core_DebugAssert(n != nullptr);                  \
                                                          \
    for (;;) {                                            \
        BinTreeNode* np{ BinTreeNode::GetP(n) };          \
        if (np == nullptr) { return nullptr; }            \
        if (BinTreeNode::Get##E(np) == n) { return np; }  \
        n = np;                                           \
    }

template <typename BinTreeNode>
BinTreeNode* StepPL(BinTreeNode* n) {
    StepP_(L);
}

template <typename BinTreeNode>
BinTreeNode* StepPR(BinTreeNode* n) {
    StepP_(R);
}

#pragma pop_macro("StepP_")

#pragma push_macro("Step_")

#define Step_(D, E)                                      \
    CheckContract<BinTreeNode>();                        \
                                                         \
    ZETA_Core_DebugAssert(n != nullptr);                 \
                                                         \
    BinTreeNode* nd{ BinTreeNode::Get##D(n) };           \
                                                         \
    if (nd != nullptr) { return GetMost##E(nd).first; }  \
                                                         \
    for (;;) {                                           \
        BinTreeNode* np{ BinTreeNode::GetP(n) };         \
        if (np == nullptr) { return nullptr; }           \
        if (BinTreeNode::Get##E(np) == n) { return np; } \
        n = np;                                          \
    }

template <typename BinTreeNode>
BinTreeNode* StepL(BinTreeNode* n) {
    Step_(L, R);
}

template <typename BinTreeNode>
BinTreeNode* StepR(BinTreeNode* n) {
    Step_(R, L);
}

#pragma push_macro("Step_")

#pragma push_macro("Advance_")

#define Advance_(D, E)                                                       \
    CheckContract<BinTreeNode>();                                            \
                                                                             \
    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;            \
                                                                             \
    ZETA_Core_StaticAssert(                                                  \
        BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper));                \
                                                                             \
    auto null_acc_size{ BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) }; \
                                                                             \
    while (n != nullptr && 0 < step) {                                       \
        BinTreeNode* nd{ BinTreeNode::Get##D(n) };                           \
        BinTreeNode* ne{ BinTreeNode::Get##E(n) };                           \
                                                                             \
        size_t n_acc_size{ BinTreeNode::GetAccSize(n) };                     \
        size_t nd_acc_size{ nd == nullptr ? null_acc_size                    \
                                          : BinTreeNode::GetAccSize(nd) };   \
        size_t ne_acc_size{ ne == nullptr ? null_acc_size                    \
                                          : BinTreeNode::GetAccSize(ne) };   \
                                                                             \
        size_t n_size{ n_acc_size - nd_acc_size - ne_acc_size };             \
                                                                             \
        if (step < n_size) { break; }                                        \
                                                                             \
        step -= n_size;                                                      \
                                                                             \
        if (step < nd_acc_size) { return Access##E(nd, step); }              \
                                                                             \
        step -= nd_acc_size;                                                 \
                                                                             \
        for (;;) {                                                           \
            BinTreeNode* np{ BinTreeNode::GetP(n) };                         \
                                                                             \
            if (np == nullptr || BinTreeNode::Get##E(np) == n) {             \
                n = np;                                                      \
                break;                                                       \
            }                                                                \
                                                                             \
            n = np;                                                          \
        }                                                                    \
    }                                                                        \
                                                                             \
    return { n, step };

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceL(BinTreeNode* n, size_t step) {
    Advance_(L, R);
}

template <typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceR(BinTreeNode* n, size_t step) {
    Advance_(R, L);
}

#pragma pop_macro("Advance_")

template <typename BinTreeNode>
Pair<size_t, size_t> GetLRAccSize(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    type_wrapper::TypeWrapper<BinTreeNode*> btn_ptr_type_wrapper;

    ZETA_Core_StaticAssert(BinTreeNode::IsAccSizeEnabled(btn_ptr_type_wrapper));

    ZETA_Core_DebugAssert(n != nullptr);

    auto null_acc_size{ BinTreeNode::GetNullAccSize(btn_ptr_type_wrapper) };

    BinTreeNode* nl{ BinTreeNode::GetL(n) };
    BinTreeNode* nr{ BinTreeNode::GetR(n) };

    size_t l_acc_size{ nl == nullptr ? null_acc_size
                                     : BinTreeNode::GetAccSize(nl) };
    size_t r_acc_size{ nr == nullptr ? null_acc_size
                                     : BinTreeNode::GetAccSize(nr) };

    size_t n_acc_size{ BinTreeNode::GetAccSize(n) };

    for (;;) {
        BinTreeNode* np{ BinTreeNode::GetP(n) };
        if (np == nullptr) { break; }

        size_t np_acc_size{ BinTreeNode::GetAccSize(np) };
        size_t k{ np_acc_size - n_acc_size };

        if (BinTreeNode::GetL(np) == n) {
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

template <typename BinTreeNode>
void SanitizeCore(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    for (BinTreeNode* m{ n };;) {
        BinTreeNode* ml{ BinTreeNode::GetL(m) };
        BinTreeNode* mr{ BinTreeNode::GetR(m) };

        ZETA_Core_DebugAssert((ml == nullptr && mr == nullptr) || (ml != mr));

        if (ml != nullptr) {
            ZETA_Core_DebugAssert(BinTreeNode::GetP(ml) == m);
        }
        if (mr != nullptr) {
            ZETA_Core_DebugAssert(BinTreeNode::GetP(mr) == m);
        }

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
            BinTreeNode* mp{ BinTreeNode::GetP(m) };

            if (BinTreeNode::GetR(mp) == m) {
                m = mp;
                continue;
            }

            BinTreeNode* mpr{ BinTreeNode::GetR(mp) };

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

template <typename BinTreeNode>
void Sanitize(BinTreeNode* root) {
    CheckContract<BinTreeNode>();

    ZETA_Core_StaticAssert(
        BinTreeNode::IsConst(type_wrapper::TypeWrapper<BinTreeNode*>{}));

    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    if (root == nullptr) { return; }

    ZETA_Core_DebugAssert(BinTreeNode::GetP(root) == nullptr);

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = root;

    unsigned long long random_seed{ GetRandom() };

    while (0 < buffer_i) {
        BinTreeNode* n{ buffer[--buffer_i] };

        BinTreeNode* nl{ BinTreeNode::GetL(n) };
        BinTreeNode* nr{ BinTreeNode::GetR(n) };

        if (SimpleRandomRotate(&random_seed) % 2 == 1) { Swap(nl, nr); }

        ZETA_Core_DebugAssert((nl == nullptr && nr == nullptr) || (nl != nr));

        if (nl != nullptr) {
            ZETA_Core_DebugAssert(BinTreeNode::GetP(nl) == n);

            if (buffer_i == buffer_capacity) {
                detail::SanitizeCore(nl);
            } else {
                buffer[buffer_i++] = nl;
            }
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert(BinTreeNode::GetP(nr) == n);

            if (buffer_i == buffer_capacity) {
                detail::SanitizeCore(nr);
            } else {
                buffer[buffer_i++] = nr;
            }
        }
    }
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename BinTreeNode>
size_t CountCore(BinTreeNode* n) {
    size_t ret{ 0 };

    for (BinTreeNode* m; n != nullptr; n = BinTreeNode::GetR(n), ++ret) {
        for (m = GetMostL(n).first; m != n; m = StepR(m)) { ++ret; }
    }

    return ret;
}

}  // namespace detail

template <typename BinTreeNode>
size_t Count(BinTreeNode* n) {
    CheckContract<BinTreeNode>();

    constexpr size_t buffer_capacity{
        static_cast<size_t>(ZETA_Core_ullong_width) * 4
    };

    if (n == nullptr) { return 0; }

    BinTreeNode* buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = n;

    unsigned long long random_seed{ GetRandom() };

    size_t ret{ 0 };

    while (0 < buffer_i) {
        BinTreeNode* m{ buffer[--buffer_i] };

        ++ret;

        BinTreeNode* ml{ BinTreeNode::GetL(m) };
        BinTreeNode* mr{ BinTreeNode::GetR(m) };

        if (SimpleRandomRotate(&random_seed) % 2 == 1) { core::Swap(ml, mr); }

        if (ml != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::CountCore(ml);
            } else {
                buffer[buffer_i++] = ml;
            }
        }

        if (mr != nullptr) {
            if (buffer_i == buffer_capacity) {
                ret += detail::CountCore(mr);
            } else {
                buffer[buffer_i++] = mr;
            }
        }
    }

    return ret;
}

}  // namespace zeta::core::bin_tree
