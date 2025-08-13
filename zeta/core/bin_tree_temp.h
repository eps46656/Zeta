// define TreeNode

#if !defined(ZETA_Core_BinTree_MacroGuard)
#define ZETA_Core_BinTree_MacroGuard 1
#endif

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

#if ZETA_Core_BinTree_MacroGuard == 1

#define Zeta_Core_BinTree(tree_node, x) \
    ZETA_Core_Concat(Zeta_Core_BinTree_lt_, tree_node, _gt_, x)

#endif

#if !defined(TreeNode)
#error "TreeNode is not defined."
#endif

size_t Zeta_Core_BinTree(TreeNode, Count)(void* n);

#if defined(AccSizeType)

size_t Zeta_Core_BinTree(TreeNode, GetSize)(void* n);

void Zeta_Core_BinTree(TreeNode, SetSize)(void* n, size_t size);

void Zeta_Core_BinTree(TreeNode, SetDiffSize)(void* n, size_t diff_size);

#endif

void Zeta_Core_BinTree(TreeNode, AttatchL)(void* pos, void* n);

void Zeta_Core_BinTree(TreeNode, AttatchR)(void* pos, void* n);

void Zeta_Core_BinTree(TreeNode, Detach)(void* n);

void Zeta_Core_BinTree(TreeNode,
                       EraseAll)(void* root, void* callback_context,
                                 void (*Callback)(void* callback_context,
                                                  void* n));

void Zeta_Core_BinTree(TreeNode, Swap)(void* n, void* m);

void Zeta_Core_BinTree(TreeNode, RotateL)(void* n);

void Zeta_Core_BinTree(TreeNode, RotateR)(void* n);

void* Zeta_Core_BinTree(TreeNode, StepL)(void* n);

void* Zeta_Core_BinTree(TreeNode, StepR)(void* n);

#if defined(AccSizeType)

void Zeta_Core_BinTree(TreeNode, AccessL)(void** dst_n, size_t* dst_tail_idx,
                                          void* n, size_t idx);

void Zeta_Core_BinTree(TreeNode, AccessR)(void** dst_n, size_t* dst_tail_idx,
                                          void* n, size_t idx);

void Zeta_Core_BinTree(TreeNode, AdvanceL)(void** dst_n, size_t* dst_tail_idx,
                                           void* n, size_t step);

void Zeta_Core_BinTree(TreeNode, AdvanceR)(void** dst_n, size_t* dst_tail_idx,
                                           void* n, size_t step);

void Zeta_Core_BinTree(TreeNode, GetAccSize)(size_t* dst_l_acc_size,
                                             size_t* dst_r_acc_size, void* n);

#endif

void Zeta_Core_BinTree(TreeNode, ErassAll)(
    void* root, void (*Callback)(void* callback_context, void* n),
    void* callback_context);

void Zeta_Core_BinTree(TreeNode, Sanitize)(void* root);

ZETA_Core_ExternC_End;

#undef ZETA_Core_BinTree_MacroGuard
#define ZETA_Core_BinTree_MacroGuard 2
