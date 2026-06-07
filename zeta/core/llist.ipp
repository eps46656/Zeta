#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/llist.hpp>

namespace zeta::core {

template <typename LListNode>
constexpr bool llist::IsConst() {
    return NodeTraits<LListNode>::IsConst();
}

template <typename LListNode>
constexpr LListNode* llist::GetL(LListNode* n) {
    return NodeTraits<LListNode>::GetL(n);
}

template <typename LListNode>
constexpr LListNode* llist::GetR(LListNode* n) {
    return NodeTraits<LListNode>::GetR(n);
}

template <typename LListNode>
void llist::SetL(LListNode* n, LListNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());
    NodeTraits<LListNode>::SetL(n, m);
}

template <typename LListNode>
void llist::SetR(LListNode* n, LListNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());
    NodeTraits<LListNode>::SetR(n, m);
}

template <typename LListNode>
void llist::CheckContract() {
    LListNode* lln{ nullptr };

    constexpr bool is_const{ (IsConst<LListNode>)() };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...) \
    ZETA_Core_Unused([&]() { (method<LListNode>)(__VA_ARGS__); })

    CheckMethod(GetL, lln);
    CheckMethod(GetR, lln);

    if constexpr (!is_const) {
        CheckMethod(SetL, lln, lln);
        CheckMethod(SetR, lln, lln);
    }

#pragma pop_macro("CheckMethod")
}

template <typename LListNode>
size_t llist::Count(LListNode* n) {
    (CheckContract<LListNode>)();

    if (n == nullptr) { return 0; }

    size_t ret{ 1 };

    for (LListNode* m{ n }; (m = (GetR)(n)) != n;) { ++ret; }

    return ret;
}

template <typename LListNode>
void llist::InsertL(LListNode* n, LListNode* m) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert((GetL)(m) == m);
    ZETA_Core_DebugAssert((GetR)(m) == m);

    LListNode* nl{ (GetL)(n) };
    LListNode* nr{ n };

    (SetR)(nl, m);
    (SetL)(m, nl);

    (SetR)(m, nr);
    (SetL)(nr, m);
}

template <typename LListNode>
void llist::InsertR(LListNode* n, LListNode* m) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert((GetL)(m) == m);
    ZETA_Core_DebugAssert((GetR)(m) == m);

    LListNode* nl{ n };
    LListNode* nr{ (GetR)(n) };

    (SetR)(nl, m);
    (SetL)(m, nl);

    (SetR)(m, nr);
    (SetL)(nr, m);
}

template <typename LListNode>
void llist::Extract(LListNode* n) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n != nullptr);

    LListNode* nl{ (GetL)(n) };
    LListNode* nr{ (GetR)(n) };

    (SetR)(nl, nr);
    (SetL)(nr, nl);

    (SetR)(n, n);
    (SetL)(n, n);
}

template <typename LListNode>
void llist::InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ (GetL)(n) };
    LListNode* nr{ n };

    LListNode* m_beg_l{ (GetL)(m_beg) };
    LListNode* m_end_r{ (GetR)(m_end) };

    (SetR)(m_beg_l, m_end_r);
    (SetL)(m_end_r, m_beg_l);

    (SetR)(nl, m_beg);
    (SetL)(m_beg, nl);

    (SetR)(m_end, nr);
    (SetL)(nr, m_end);
}

template <typename LListNode>
void llist::InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ n };
    LListNode* nr{ (GetR)(n) };

    LListNode* m_beg_l{ (GetL)(m_beg) };
    LListNode* m_end_r{ (GetR)(m_end) };

    (SetR)(m_beg_l, m_end_r);
    (SetL)(m_end_r, m_beg_l);

    (SetR)(nl, m_beg);
    (SetL)(m_beg, nl);

    (SetR)(m_end, nr);
    (SetL)(nr, m_end);
}

template <typename LListNode>
void llist::ExtractSeg(LListNode* n_beg, LListNode* n_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());

    ZETA_Core_DebugAssert(n_beg != nullptr);
    ZETA_Core_DebugAssert(n_end != nullptr);

    LListNode* nl{ (GetL)(n_beg) };
    LListNode* nr{ (GetR)(n_end) };

    (SetR)(nl, nr);
    (SetL)(nr, nl);

    (SetR)(n_end, n_beg);
    (SetL)(n_beg, n_end);
}

}  // namespace zeta::core
