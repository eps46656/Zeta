#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/llist.hpp>
#include <zeta/core/type_traits.hpp>

namespace zeta::core::llist {

template <typename LListNodeOperator, typename LListNode>
void CheckContract(LListNodeOperator const& lln_opr) {
    LListNode* n{ nullptr };

    ZETA_Core_StaticAssert(IsAnyOf<decltype(lln_opr.GetL(n)), LListNode*>);
    ZETA_Core_StaticAssert(IsAnyOf<decltype(lln_opr.GetR(n)), LListNode*>);

    ZETA_Core_StaticAssert(IsAnyOf<decltype((lln_opr.SetL(n, n), 0)), int>);
    ZETA_Core_StaticAssert(IsAnyOf<decltype((lln_opr.SetR(n, n), 0)), int>);
}

// -----------------------------------------------------------------------------

template <typename LListNodeOperator, typename LListNode>
size_t Count(LListNodeOperator const& lln_opr, LListNode* n) {
    CheckContract<LListNodeOperator, LListNode>(lln_opr);

    if (n == nullptr) { return 0; }

    size_t ret{ 1 };

    for (LListNode* m{ n }; (m = lln_opr.GetR(n)) != n;) { ++ret; }

    return ret;
}

template <typename LListNodeOperator, typename LListNode>
void InsertL(LListNodeOperator const& lln_opr, LListNode* n, LListNode* m) {
    CheckContract<LListNodeOperator, LListNode>(lln_opr);

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(lln_opr.GetL(m) == m);
    ZETA_Core_DebugAssert(lln_opr.GetR(m) == m);

    LListNode* nl{ lln_opr.GetL(n) };
    LListNode* nr{ n };

    lln_opr.SetR(nl, m);
    lln_opr.SetL(m, nl);

    lln_opr.SetR(m, nr);
    lln_opr.SetL(nr, m);
}

template <typename LListNodeOperator, typename LListNode>
void InsertR(LListNodeOperator const& lln_opr, LListNode* n, LListNode* m) {
    CheckContract<LListNodeOperator, LListNode>(lln_opr);

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(lln_opr.GetL(m) == m);
    ZETA_Core_DebugAssert(lln_opr.GetR(m) == m);

    LListNode* nl{ n };
    LListNode* nr{ lln_opr.GetR(n) };

    lln_opr.SetR(nl, m);
    lln_opr.SetL(m, nl);

    lln_opr.SetR(m, nr);
    lln_opr.SetL(nr, m);
}

template <typename LListNodeOperator, typename LListNode>
void Extract(LListNodeOperator const& lln_opr, LListNode* n) {
    CheckContract<LListNodeOperator, LListNode>(lln_opr);

    ZETA_Core_DebugAssert(n != nullptr);

    LListNode* nl{ lln_opr.GetL(n) };
    LListNode* nr{ lln_opr.GetR(n) };

    lln_opr.SetR(nl, nr);
    lln_opr.SetL(nr, nl);

    lln_opr.SetR(n, n);
    lln_opr.SetL(n, n);
}

template <typename LListNodeOperator, typename LListNode>
void InsertSegL(LListNodeOperator const& lln_opr, LListNode* n,
                LListNode* m_beg, LListNode* m_end) {
    CheckContract<LListNodeOperator, LListNode>(lln_opr);

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ lln_opr.GetL(n) };
    LListNode* nr{ n };

    LListNode* m_beg_l{ lln_opr.GetL(m_beg) };
    LListNode* m_end_r{ lln_opr.GetR(m_end) };

    lln_opr.SetR(m_beg_l, m_end_r);
    lln_opr.SetL(m_end_r, m_beg_l);

    lln_opr.SetR(nl, m_beg);
    lln_opr.SetL(m_beg, nl);

    lln_opr.SetR(m_end, nr);
    lln_opr.SetL(nr, m_end);
}

template <typename LListNodeOperator, typename LListNode>
void InsertSegR(LListNodeOperator const& lln_opr, LListNode* n,
                LListNode* m_beg, LListNode* m_end) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ n };
    LListNode* nr{ lln_opr.GetR(n) };

    LListNode* m_beg_l{ lln_opr.GetL(m_beg) };
    LListNode* m_end_r{ lln_opr.GetR(m_end) };

    lln_opr.SetR(m_beg_l, m_end_r);
    lln_opr.SetL(m_end_r, m_beg_l);

    lln_opr.SetR(nl, m_beg);
    lln_opr.SetL(m_beg, nl);

    lln_opr.SetR(m_end, nr);
    lln_opr.SetL(nr, m_end);
}

template <typename LListNodeOperator, typename LListNode>
void ExtractSeg(LListNodeOperator const& lln_opr, LListNode* n_beg,
                LListNode* n_end) {
    ZETA_Core_DebugAssert(n_beg != nullptr);
    ZETA_Core_DebugAssert(n_end != nullptr);

    LListNode* nl{ lln_opr.GetL(n_beg) };
    LListNode* nr{ lln_opr.GetR(n_end) };

    lln_opr.SetR(nl, nr);
    lln_opr.SetL(nr, nl);

    lln_opr.SetR(n_end, n_beg);
    lln_opr.SetL(n_beg, n_end);
}

}  // namespace zeta::core::llist
