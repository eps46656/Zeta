// TreeNode: The type of tree node.

// RelLinkType: Relative addressing integer type. If not defined, use ordinary
//              addressing mode.

// PColor: Boolean. Enable if color bit at p link.

// LColor: Boolean. Enable if color bit at l link.

// RColor: Boolean. Enable if color bit at r link.

// AccSizeType: Accumulation size integer type. If not defined, not enable
//              accumulation size.

#include <zeta/core/define.h>

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

ZETA_Core_DeclareStruct(TreeNode);

struct TreeNode {
#if defined(RelLinkType)
    RelLinkType p;
#else
    void* p;
#endif

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

#if defined(AccSizeType)
    AccSizeType acc_size;
#endif
} __attribute__((aligned(2)));

void TreeNode_(Init)(void* n);

void* TreeNode_(GetP)(void* n);
void* TreeNode_(GetL)(void* n);
void* TreeNode_(GetR)(void* n);

void TreeNode_(SetP)(void* n, void* m);
void TreeNode_(SetL)(void* n, void* m);
void TreeNode_(SetR)(void* n, void* m);

#if PColor

int TreeNode_(GetPColor)(void* n);
void TreeNode_(SetPColor)(void* n, int color);

#endif

#if LColor

int TreeNode_(GetLColor)(void* n);
void TreeNode_(SetLColor)(void* n, int color);

#endif

#if RColor

int TreeNode_(GetRColor)(void* n);
void TreeNode_(SetRColor)(void* n, int color);

#endif

#if defined(AccSizeType)

size_t TreeNode_(GetAccSize)(void* n);
void TreeNode_(SetAccSize)(void* n, size_t acc_size);

#endif

#pragma push_macro("TreeNode_")

ZETA_Core_ExternC_End;
