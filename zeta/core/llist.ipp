#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/llist.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>

namespace zeta::core::llist {

template <typename LListNode>
void CheckContract() {
    constexpr type_wrapper::TypeWrapper<LListNode*> lln_ptr_type_wrapper;

    LListNode* n{ nullptr };

#pragma push_macro("CheckMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(LListNode::method(__VA_ARGS__)), return_type>)

    CheckMethod(GetL, LListNode*, n);
    CheckMethod(GetR, LListNode*, n);

    CheckMethod(IsConst, TypeAny, lln_ptr_type_wrapper);

    if constexpr (!LListNode::IsConst(lln_ptr_type_wrapper)) {
        CheckMethod(SetL, TypeAny, n, n);
        CheckMethod(SetR, TypeAny, n, n);
    }
}

// -----------------------------------------------------------------------------

template <typename LListNode>
size_t Count(LListNode* n) {
    CheckContract<LListNode>();

    if (n == nullptr) { return 0; }

    size_t ret{ 1 };

    for (LListNode* m{ n }; (m = LListNode::GetR(n)) != n;) { ++ret; }

    return ret;
}

template <typename LListNode>
void InsertL(LListNode* n, LListNode* m) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(LListNode::GetL(m) == m);
    ZETA_Core_DebugAssert(LListNode::GetR(m) == m);

    LListNode* nl{ LListNode::GetL(n) };
    LListNode* nr{ n };

    LListNode::SetR(nl, m);
    LListNode::SetL(m, nl);

    LListNode::SetR(m, nr);
    LListNode::SetL(nr, m);
}

template <typename LListNode>
void InsertR(LListNode* n, LListNode* m) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(LListNode::GetL(m) == m);
    ZETA_Core_DebugAssert(LListNode::GetR(m) == m);

    LListNode* nl{ n };
    LListNode* nr{ LListNode::GetR(n) };

    LListNode::SetR(nl, m);
    LListNode::SetL(m, nl);

    LListNode::SetR(m, nr);
    LListNode::SetL(nr, m);
}

template <typename LListNode>
void Extract(LListNode* n) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);

    LListNode* nl{ LListNode::GetL(n) };
    LListNode* nr{ LListNode::GetR(n) };

    LListNode::SetR(nl, nr);
    LListNode::SetL(nr, nl);

    LListNode::SetR(n, n);
    LListNode::SetL(n, n);
}

template <typename LListNode>
void InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ LListNode::GetL(n) };
    LListNode* nr{ n };

    LListNode* m_beg_l{ LListNode::GetL(m_beg) };
    LListNode* m_end_r{ LListNode::GetR(m_end) };

    LListNode::SetR(m_beg_l, m_end_r);
    LListNode::SetL(m_end_r, m_beg_l);

    LListNode::SetR(nl, m_beg);
    LListNode::SetL(m_beg, nl);

    LListNode::SetR(m_end, nr);
    LListNode::SetL(nr, m_end);
}

template <typename LListNode>
void InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    LListNode* nl{ n };
    LListNode* nr{ LListNode::GetR(n) };

    LListNode* m_beg_l{ LListNode::GetL(m_beg) };
    LListNode* m_end_r{ LListNode::GetR(m_end) };

    LListNode::SetR(m_beg_l, m_end_r);
    LListNode::SetL(m_end_r, m_beg_l);

    LListNode::SetR(nl, m_beg);
    LListNode::SetL(m_beg, nl);

    LListNode::SetR(m_end, nr);
    LListNode::SetL(nr, m_end);
}

template <typename LListNode>
void ExtractSeg(LListNode* n_beg, LListNode* n_end) {
    CheckContract<LListNode>();

    ZETA_Core_StaticAssert(
        !LListNode::IsConst(type_wrapper::TypeWrapper<LListNode*>{}));

    ZETA_Core_DebugAssert(n_beg != nullptr);
    ZETA_Core_DebugAssert(n_end != nullptr);

    LListNode* nl{ LListNode::GetL(n_beg) };
    LListNode* nr{ LListNode::GetR(n_end) };

    LListNode::SetR(nl, nr);
    LListNode::SetL(nr, nl);

    LListNode::SetR(n_end, n_beg);
    LListNode::SetL(n_beg, n_end);
}

}  // namespace zeta::core::llist
