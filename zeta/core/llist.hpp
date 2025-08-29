#pragma once

namespace zeta::core::llist {

template <typename LListNode>
size_t Count(LListNode* n);

template <typename LListNode>
void InsertL(LListNode* n, LListNode* m);
template <typename LListNode>
void InsertR(LListNode* n, LListNode* m);

template <typename LListNode>
void Extract(LListNode* n);

template <typename LListNode>
void InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end);

template <typename LListNode>
void InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end);

template <typename LListNode>
void ExtractSeg(LListNode* n_beg, LListNode* n_end);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename LListNode>
size_t Count(LListNode* n) {
    if (n == nullptr) { return 0; }

    size_t ret{ 1 };

    for (void* m{ n }; (m = m->GetR) != n;) { ++ret; }

    return ret;
}

template <typename LListNode>
void InsertL(LListNode* n, LListNode* m) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(m->GetLPtr() == m);
    ZETA_Core_DebugAssert(m->GetRPtr() == m);

    void* nl{ n->GetLPtr() };
    void* nr{ n };

    nl->SetRPtr(m);
    m->SetLPtr(nl);

    m->SetRPtr(nr);
    nr->SetLPtr(m);
}

template <typename LListNode>
void InsertR(LListNode* n, LListNode* m) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m != nullptr);

    ZETA_Core_DebugAssert(m->GetLPtr() == m);
    ZETA_Core_DebugAssert(m->GetRPtr() == m);

    void* nl{ n };
    void* nr{ n->GetRPtr() };

    nl->SetRPtr(m);
    m->SetLPtr(nl);

    m->SetRPtr(nr);
    nr->SetLPtr(m);
}

template <typename LListNode>
void Extract(LListNode* n) {
    ZETA_Core_DebugAssert(n != nullptr);

    void* nl{ n->GetLPtr() };
    void* nr{ n->GetRPtr() };

    nl->SetRPtr(nr);
    nr->SetLPtr(nl);

    n->SetRPtr(n);
    n->SetLPtr(n);
}

template <typename LListNode>
void InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    void* nl{ n->GetLPtr() };
    void* nr{ n };

    void* m_beg_l{ m_beg->GetLPtr() };
    void* m_end_r{ m_end->GetRPtr() };

    m_beg_l->SetRPtr(m_end_r);
    m_end_r->SetLPtr(m_beg_l);

    nl->SetRPtr(m_beg);
    m_beg->SetLPtr(nl);

    m_end->SetRPtr(nr);
    nr->SetLPtr(m_end);
}

template <typename LListNode>
void InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end) {
    ZETA_Core_DebugAssert(n != nullptr);
    ZETA_Core_DebugAssert(m_beg != nullptr);
    ZETA_Core_DebugAssert(m_end != nullptr);

    void* nl{ n };
    void* nr{ n->GetRPtr() };

    void* m_beg_l{ m_beg->GetLPtr() };
    void* m_end_r{ m_end->GetRPtr() };

    m_beg_l->SetRPtr(m_end_r);
    m_end_r->SetLPtr(m_beg_l);

    nl->SetRPtr(m_beg);
    m_beg->SetLPtr(nl);

    m_end->SetRPtr(nr);
    nr->SetLPtr(m_end);
}

template <typename LListNode>
void ExtractSeg(LListNode* n_beg, LListNode* n_end) {
    ZETA_Core_DebugAssert(n_beg != nullptr);
    ZETA_Core_DebugAssert(n_end != nullptr);

    void* nl{ n_beg->GetLPtr() };
    void* nr{ n_end->GetRPtr() };

    nl->SetRPtr(nr);
    nr->SetLPtr(nl);

    n_end->SetRPtr(n_beg);
    n_beg->SetLPtr(n_end);
}

}  // namespace zeta::core::llist
