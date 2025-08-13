// LListNode: The name of linked list node.

// RelLinkType: Relative addressing link type. If not defined, use directional
//              addressing.

// LColor: Enable the color bit of l link.

// RColor: Enable the color bit of r link.

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

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

ZETA_Core_DeclareStruct(LListNode);

struct LListNode {
#if defined(RelLinkType)
    RelLinkType l;
#else
    void* l;
#endif

#if defined(RelLinkType)
    RelLinkType r;
#else
    void* r;
#endif
} __attribute__((aligned(2)));

void LListNode_(Init)(void* n);

void* LListNode_(GetL)(void* n);
void* LListNode_(GetR)(void* n);

void LListNode_(SetL)(void* n, void* m);
void LListNode_(SetR)(void* n, void* m);

#if LColor

int LListNode_(GetLColor)(void* n);
void LListNode_(SetLColor)(void* n, int color);

#endif

#if RColor

int LListNode_(GetRColor)(void* n);
void LListNode_(SetRColor)(void* n, int color);

#endif

#pragma pop_macro("LListNode_")

ZETA_Core_ExternC_End;
