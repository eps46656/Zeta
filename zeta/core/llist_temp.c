// define LListNode

#include <zeta/core/debugger.h>

ZETA_Core_ExternC_Beg;

#if !defined(LListNode)
#error "LListNode is not defined."
#endif

#pragma push_macro("LListNode_")

#define Zeta_Core_LList(llist_node_type, x) \
    ZETA_Core_Concat(Zeta_Core_LList_lt_, llist_node_type, _gt_, x)

#define LListNode_(x) ZETA_Core_Concat(LListNode, _, x)

size_t Zeta_Core_LList(LListNode, Count)(void* n) {
    if (n == NULL) { return 0; }

    size_t ret = 1;

    for (void* m = LListNode_(GetR)(n); m != n; m = LListNode_(GetR)(m)) {
        ++ret;
    }

    return ret;
}

void Zeta_Core_LList(LListNode, InsertL)(void* n, void* m) {
    ZETA_Core_DebugAssert(n != NULL);
    ZETA_Core_DebugAssert(m != NULL);

    ZETA_Core_DebugAssert(LListNode_(GetL)(m) == m);
    ZETA_Core_DebugAssert(LListNode_(GetR)(m) == m);

    void* nl = LListNode_(GetL)(n);
    void* nr = n;

    LListNode_(SetR)(nl, m);
    LListNode_(SetL)(m, nl);

    LListNode_(SetR)(m, nr);
    LListNode_(SetL)(nr, m);
}

void Zeta_Core_LList(LListNode, InsertR)(void* n, void* m) {
    ZETA_Core_DebugAssert(n != NULL);
    ZETA_Core_DebugAssert(m != NULL);

    ZETA_Core_DebugAssert(LListNode_(GetL)(m) == m);
    ZETA_Core_DebugAssert(LListNode_(GetR)(m) == m);

    void* nl = n;
    void* nr = LListNode_(GetR)(n);

    LListNode_(SetR)(nl, m);
    LListNode_(SetL)(m, nl);

    LListNode_(SetR)(m, nr);
    LListNode_(SetL)(nr, m);
}

void Zeta_Core_LList(LListNode, Extract)(void* n) {
    ZETA_Core_DebugAssert(n != NULL);

    void* nl = LListNode_(GetL)(n);
    void* nr = LListNode_(GetR)(n);

    LListNode_(SetR)(nl, nr);
    LListNode_(SetL)(nr, nl);

    LListNode_(SetR)(n, n);
    LListNode_(SetL)(n, n);
}

void Zeta_Core_LList(LListNode, InsertSegL)(void* n, void* m_beg, void* m_end) {
    void* nl = LListNode_(GetL)(n);
    void* nr = n;

    void* m_beg_l = LListNode_(GetL)(m_beg);
    void* m_end_r = LListNode_(GetR)(m_end);

    LListNode_(SetR)(m_beg_l, m_end_r);
    LListNode_(SetL)(m_end_r, m_beg_l);

    LListNode_(SetR)(nl, m_beg);
    LListNode_(SetL)(m_beg, nl);

    LListNode_(SetR)(m_end, nr);
    LListNode_(SetL)(nr, m_end);
}

void Zeta_Core_LList(LListNode, InsertSegR)(void* n, void* m_beg, void* m_end) {
    void* nl = n;
    void* nr = LListNode_(GetR)(n);

    void* m_beg_l = LListNode_(GetL)(m_beg);
    void* m_end_r = LListNode_(GetR)(m_end);

    LListNode_(SetR)(m_beg_l, m_end_r);
    LListNode_(SetL)(m_end_r, m_beg_l);

    LListNode_(SetR)(nl, m_beg);
    LListNode_(SetL)(m_beg, nl);

    LListNode_(SetR)(m_end, nr);
    LListNode_(SetL)(nr, m_end);
}

void Zeta_Core_LList(LListNode, ExtractSeg)(void* n_beg, void* n_end) {
    void* n_beg_l = LListNode_(GetL)(n_beg);
    void* n_end_r = LListNode_(GetR)(n_end);

    LListNode_(SetR)(n_beg_l, n_end_r);
    LListNode_(SetL)(n_end_r, n_beg_l);

    LListNode_(SetR)(n_end, n_beg);
    LListNode_(SetL)(n_beg, n_end);
}

#pragma pop_macro("LListNode_")

ZETA_Core_ExternC_End;
