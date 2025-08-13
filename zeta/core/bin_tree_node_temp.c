// defined TreeNode

// defined PColor
// defined LColor
// defined RColor

#include <zeta/core/debugger.h>
#include <zeta/core/ptr_utils.h>

ZETA_Core_ExternC_Beg;

#if !defined(TreeNode)
#error "TreeNode is not defined."
#endif

#if !defined(PColor)
#error "PColor is not defined."
#endif

#if !defined(LColor)
#error "LColor is not defined."
#endif

#if !defined(RColor)
#error "RColor is not defined."
#endif

#pragma push_macro("TreeNode_")

#define TreeNode_(x) ZETA_Core_Concat(TreeNode, _, x)

void TreeNode_(Init)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && PColor
    ZETA_Core_RelColorPtr_Set(&n->p, alignof(TreeNode), n, n, 0);
#elif defined(RelLinkType) && !PColor
    ZETA_Core_RelPtr_SetPtr(&n->p, n, n);
#elif !defined(RelLinkType) && PColor
    ZETA_ColorPtr_SetPtr(&n->p, alignof(TreeNode), n);
#else
    n->p = NULL;
#endif

#if defined(RelLinkType) && LColor
    ZETA_Core_RelColorPtr_Set(&n->l, alignof(TreeNode), n, n, 0);
#elif defined(RelLinkType) && !LColor
    ZETA_Core_RelPtr_SetPtr(&n->l, n, n);
#elif !defined(RelLinkType) && LColor
    ZETA_ColorPtr_SetPtr(&n->l, alignof(TreeNode), n);
#else
    n->l = NULL;
#endif

#if defined(RelLinkType) && RColor
    ZETA_Core_RelColorPtr_Set(&n->r, alignof(TreeNode), n, n, 0);
#elif defined(RelLinkType) && !RColor
    ZETA_Core_RelPtr_SetPtr(&n->r, n, n);
#elif !defined(RelLinkType) && RColor
    ZETA_ColorPtr_SetPtr(&n->r, alignof(TreeNode), n);
#else
    n->r = NULL;
#endif

#if defined(AccSizeType)
    n->acc_size = 0;
#endif
}

void* TreeNode_(GetP)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && PColor
    void* m = ZETA_Core_RelColorPtr_GetPtr(&n->p, alignof(TreeNode), n);
    return m == n ? NULL : m;
#elif defined(RelLinkType) && !PColor
    void* m = ZETA_Core_RelPtr_GetPtr(&n->p, n);
    return m == n ? NULL : m;
#elif !defined(RelLinkType) && PColor
    void* m = ZETA_ColorPtr_GetPtr(&n->p, alignof(TreeNode));
    return m == n ? NULL : m;
#else
    return n->p;
#endif
}

void* TreeNode_(GetL)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && LColor
    void* m = ZETA_Core_RelColorPtr_GetPtr(&n->l, alignof(TreeNode), n);
    return m == n ? NULL : m;
#elif defined(RelLinkType) && !LColor
    void* m = ZETA_Core_RelPtr_GetPtr(&n->l, n);
    return m == n ? NULL : m;
#elif !defined(RelLinkType) && LColor
    void* m = ZETA_ColorPtr_GetPtr(&n->l, alignof(TreeNode));
    return m == n ? NULL : m;
#else
    return n->l;
#endif
}

void* TreeNode_(GetR)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && RColor
    void* m = ZETA_Core_RelColorPtr_GetPtr(&n->r, alignof(TreeNode), n);
    return m == n ? NULL : m;
#elif defined(RelLinkType) && !RColor
    void* m = ZETA_Core_RelPtr_GetPtr(&n->r, n);
    return m == n ? NULL : m;
#elif !defined(RelLinkType) && RColor
    void* m = ZETA_ColorPtr_GetPtr(&n->r, alignof(TreeNode));
    return m == n ? NULL : m;
#else
    return n->r;
#endif
}

void TreeNode_(SetP)(void* n_, void* m) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && PColor
    ZETA_Core_RelColorPtr_SetPtr(&n->p, alignof(TreeNode), n,
                                 m == NULL ? n : m);
#elif defined(RelLinkType) && !PColor
    ZETA_Core_RelPtr_SetPtr(&n->p, n, m == NULL ? n : m);
#elif !defined(RelLinkType) && PColor
    ZETA_ColorPtr_SetPtr(&n->p, alignof(TreeNode), m == NULL ? n : m);
#else
    n->p = m;
#endif
}

void TreeNode_(SetL)(void* n_, void* m) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && LColor
    ZETA_Core_RelColorPtr_SetPtr(&n->l, alignof(TreeNode), n,
                                 m == NULL ? n : m);
#elif defined(RelLinkType) && !LColor
    ZETA_Core_RelPtr_SetPtr(&n->l, n, m == NULL ? n : m);
#elif !defined(RelLinkType) && LColor
    ZETA_ColorPtr_SetPtr(&n->l, alignof(TreeNode), m == NULL ? n : m);
#else
    n->l = m;
#endif
}

void TreeNode_(SetR)(void* n_, void* m) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && RColor
    ZETA_Core_RelColorPtr_SetPtr(&n->r, alignof(TreeNode), n,
                                 m == NULL ? n : m);
#elif defined(RelLinkType) && !RColor
    ZETA_Core_RelPtr_SetPtr(&n->r, n, m == NULL ? n : m);
#elif !defined(RelLinkType) && RColor
    ZETA_ColorPtr_SetPtr(&n->r, alignof(TreeNode), m == NULL ? n : m);
#else
    n->r = m;
#endif
}

#if PColor

int TreeNode_(GetPColor)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    return ZETA_Core_RelColorPtr_GetColor(&n->p, alignof(TreeNode), n);
#else
    return ZETA_ColorPtr_GetColor(&n->p, alignof(TreeNode));
#endif
}

void TreeNode_(SetPColor)(void* n_, int color) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    ZETA_Core_RelColorPtr_SetColor(&n->p, alignof(TreeNode), n, color);
#else
    ZETA_ColorPtr_SetColor(&n->p, alignof(TreeNode), color);
#endif
}

#endif

#if LColor

int TreeNode_(GetLColor)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    return ZETA_Core_RelColorPtr_GetColor(&n->l, alignof(TreeNode), n);
#else
    return ZETA_ColorPtr_GetColor(&n->l, alignof(TreeNode));
#endif
}

void TreeNode_(SetLColor)(void* n_, int color) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    ZETA_Core_RelColorPtr_SetColor(&n->l, alignof(TreeNode), n, color);
#else
    ZETA_ColorPtr_SetColor(&n->l, alignof(TreeNode), color);
#endif
}

#endif

#if RColor

int TreeNode_(GetRColor)(void* n_) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    return ZETA_Core_RelColorPtr_GetColor(&n->r, alignof(TreeNode), n);
#else
    return ZETA_ColorPtr_GetColor(&n->r, alignof(TreeNode));
#endif
}

void TreeNode_(SetRColor)(void* n_, int color) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    ZETA_Core_RelColorPtr_SetColor(&n->r, alignof(TreeNode), n, color);
#else
    ZETA_ColorPtr_SetColor(&n->r, alignof(TreeNode), color);
#endif
}

#endif

#if defined(AccSizeType)

size_t TreeNode_(GetAccSize)(void* n_) {
    TreeNode* n = n_;

    return n == NULL ? 0 : n->acc_size;
}

void TreeNode_(SetAccSize)(void* n_, size_t acc_size) {
    TreeNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

    n->acc_size = acc_size;
}

#endif

#pragma pop_macro("TreeNode_")

ZETA_Core_ExternC_End;
