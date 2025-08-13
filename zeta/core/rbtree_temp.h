// define TreeNode

#if !defined(ZETA_Core_RBTree_MacroGuard)
#define ZETA_Core_RBTree_MacroGuard 1
#endif

#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

#if ZETA_Core_RBTree_MacroGuard == 1

#define Zeta_Core_RBTree(tree_node_type, x) \
    ZETA_Core_Concat(Zeta_Core_RBTree_lt_, tree_node_type, _gt_, x)

#endif

#if !defined(TreeNode)
#error "TreeNode is not defined."
#endif

void* Zeta_Core_RBTree(TreeNode, InsertL)(void* pos, void* n);

void* Zeta_Core_RBTree(TreeNode, InsertR)(void* pos, void* n);

void* Zeta_Core_RBTree(TreeNode, Insert)(void* pos_l, void* pos_r, void* n);

void* Zeta_Core_RBTree(TreeNode, GeneralInsertL)(void* root, void* pos,
                                                 void* n);

void* Zeta_Core_RBTree(TreeNode, GeneralInsertR)(void* root, void* pos,
                                                 void* n);

void* Zeta_Core_RBTree(TreeNode, Extract)(void* n);

void Zeta_Core_RBTree(TreeNode, Sanitize)(Zeta_Core_MemRecorder* dst_mr,
                                          void* root);

ZETA_Core_ExternC_End;

#undef ZETA_Core_RBTree_MacroGuard
#define ZETA_Core_RBTree_MacroGuard 2
