#include <zeta/core/debugger.h>
#include <zeta/core/ptr_utils.h>

#if !defined(LListNode)
#error "LListNode is not defined."
#endif

#if !defined(LColor)
#error "LColor is not defined."
#endif

#if !defined(RColor)
#error "RColor is not defined."
#endif

#pragma push_macro("LListNode_")

#define LListNode_(x) ZETA_Core_Concat(LListNode, _, x)

ZETA_Core_ExternC_Beg;

void LListNode_(Init)(void* n_) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && LColor
    ZETA_Core_RelColorPtr_Set(&n->l, alignof(LListNode), n, n, 0);
#elif defined(RelLinkType) && !LColor
    ZETA_Core_RelPtr_SetPtr(&n->l, n, n);
#elif !defined(RelLinkType) && LColor
    ZETA_ColorPtr_SetPtr(&n->l, alignof(LListNode), n);
#else
    n->l = n;
#endif

#if defined(RelLinkType) && RColor
    ZETA_Core_RelColorPtr_Set(&n->r, alignof(LListNode), n, n, 0);
#elif defined(RelLinkType) && !RColor
    ZETA_Core_RelPtr_SetPtr(&n->r, n, n);
#elif !defined(RelLinkType) && RColor
    ZETA_ColorPtr_SetPtr(&n->r, alignof(LListNode), n);
#else
    n->r = n;
#endif
}

void* LListNode_(GetL)(void* n_) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && LColor
    return ZETA_Core_RelColorPtr_GetPtr(&n->l, alignof(LListNode), n);
#elif defined(RelLinkType) && !LColor
    return ZETA_Core_RelPtr_GetPtr(&n->l, n);
#elif !defined(RelLinkType) && LColor
    return ZETA_ColorPtr_GetPtr(&n->l, alignof(LListNode));
#else
    return n->l;
#endif
}

void* LListNode_(GetR)(void* n_) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && RColor
    return ZETA_Core_RelColorPtr_GetPtr(&n->r, alignof(LListNode), n);
#elif defined(RelLinkType) && !RColor
    return ZETA_Core_RelPtr_GetPtr(&n->r, n);
#elif !defined(RelLinkType) && RColor
    return ZETA_ColorPtr_GetPtr(&n->r, alignof(LListNode));
#else
    return n->r;
#endif
}

void LListNode_(SetL)(void* n_, void* m) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && LColor
    ZETA_Core_RelColorPtr_SetPtr(&n->l, alignof(LListNode), n, m);
#elif defined(RelLinkType) && !LColor
    ZETA_Core_RelPtr_SetPtr(&n->l, n, m);
#elif !defined(RelLinkType) && LColor
    ZETA_ColorPtr_SetPtr(&n->l, alignof(LListNode), m);
#else
    n->l = m;
#endif
}

void LListNode_(SetR)(void* n_, void* m) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType) && RColor
    ZETA_Core_RelColorPtr_SetPtr(&n->r, alignof(LListNode), n, m);
#elif defined(RelLinkType) && !RColor
    ZETA_Core_RelPtr_SetPtr(&n->r, n, m);
#elif !defined(RelLinkType) && RColor
    ZETA_ColorPtr_SetPtr(&n->r, alignof(LListNode), m);
#else
    n->r = m;
#endif
}

#if LColor

int LListNode_(GetLColor)(void* n_) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    return ZETA_Core_RelColorPtr_GetColor(&n->l, alignof(LListNode), n);
#else
    return ZETA_ColorPtr_GetColor(&n->l, alignof(LListNode));
#endif
}

void LListNode_(SetLColor)(void* n_, int color) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    ZETA_Core_RelColorPtr_SetColor(&n->l, alignof(LListNode), n, color);
#else
    ZETA_ColorPtr_SetColor(&n->l, alignof(LListNode), color);
#endif
}

#endif

#if RColor

int LListNode_(GetRColor)(void* n_) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    return ZETA_Core_RelColorPtr_GetColor(&n->r, alignof(LListNode), n);
#else
    return ZETA_ColorPtr_GetColor(&n->r, alignof(LListNode));
#endif
}

void LListNode_(SetRColor)(void* n_, int color) {
    LListNode* n = n_;
    ZETA_Core_DebugAssert(n != NULL);

#if defined(RelLinkType)
    ZETA_Core_RelColorPtr_SetColor(&n->r, alignof(LListNode), n, color);
#else
    ZETA_ColorPtr_SetColor(&n->r, alignof(LListNode), color);
#endif
}

#endif

ZETA_Core_ExternC_End;

#pragma pop_macro("LListNode_")
