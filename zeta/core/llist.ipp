#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/llist.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>

namespace zeta::core {

template <typename LListNode>
constexpr bool llist::ops::IsConst() {
    return Traits<LListNode>::IsConst();
}

template <typename LListNode>
constexpr LListNode* llist::ops::GetL(LListNode* n) {
    return Traits<LListNode>::GetL(n);
}

template <typename LListNode>
constexpr LListNode* llist::ops::GetR(LListNode* n) {
    return Traits<LListNode>::GetR(n);
}

template <typename LListNode>
void llist::ops::SetL(LListNode* n, LListNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());
    Traits<LListNode>::SetL(n, m);
}

template <typename LListNode>
void llist::ops::SetR(LListNode* n, LListNode* m) {
    ZETA_Core_StaticAssert(!(IsConst<LListNode>)());
    Traits<LListNode>::SetR(n, m);
}

template <typename LListNode>
void llist::ops::CheckContract() {
    LListNode* lln{ nullptr };

    constexpr bool is_const{ (IsConst<LListNode>)() };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...) \
    ZETA_Core_Unused([=]() { ops::method<LListNode>(__VA_ARGS__); })

    CheckMethod(GetL, lln);
    CheckMethod(GetR, lln);

    if constexpr (!is_const) {
        CheckMethod(SetL, lln, lln);
        CheckMethod(SetR, lln, lln);
    }
}

template <typename LListNode>
size_t llist::ops::Count(LListNode* n) {
    (CheckContract<LListNode>)();

    if (n == nullptr) { return 0; }

    size_t ret{ 1 };

    for (LListNode* m{ n }; (m = (GetR)(n)) != n;) { ++ret; }

    return ret;
}

template <typename LListNode>
void llist::ops::InsertL(LListNode* n, LListNode* m) {
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
void llist::ops::InsertR(LListNode* n, LListNode* m) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!LListNode::IsConst());

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
void llist::ops::Extract(LListNode* n) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!LListNode::IsConst());

    ZETA_Core_DebugAssert(n != nullptr);

    LListNode* nl{ (GetL)(n) };
    LListNode* nr{ (GetR)(n) };

    (SetR)(nl, nr);
    (SetL)(nr, nl);

    (SetR)(n, n);
    (SetL)(n, n);
}

template <typename LListNode>
void llist::ops::InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!LListNode::IsConst());

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
void llist::ops::InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!LListNode::IsConst());

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
void llist::ops::ExtractSeg(LListNode* n_beg, LListNode* n_end) {
    (CheckContract<LListNode>)();

    ZETA_Core_StaticAssert(!LListNode::IsConst());

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
