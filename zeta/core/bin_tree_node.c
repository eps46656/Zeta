#include <zeta/core/bin_tree_node.h>

#pragma push_macro("TreeNode")
#pragma push_macro("RelLinkType")
#pragma push_macro("PColor")
#pragma push_macro("LColor")
#pragma push_macro("RColor")
#pragma push_macro("AccSizeType")

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_OrdTreeNode
#undef RelLinkType
#define PColor 0
#define LColor 0
#define RColor 0
#undef AccSizeType

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_OrdRBTreeNode
#undef RelLinkType
#define PColor 1
#define LColor 0
#define RColor 0
#undef AccSizeType

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_Ord3RBTreeNode
#undef RelLinkType
#define PColor 1
#define LColor 1
#define RColor 1
#undef AccSizeType

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_OrdCntRBTreeNode
#undef RelLinkType
#define PColor 1
#define LColor 0
#define RColor 0
#define AccSizeType size_t

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_OrdCnt3RBTreeNode
#undef RelLinkType
#define PColor 1
#define LColor 1
#define RColor 1
#define AccSizeType size_t

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_ShortRelRBTreeNode
#define RelLinkType int
#define PColor 1
#define LColor 0
#define RColor 0
#define AccSizeType unsigned

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#undef TreeNode
#undef RelLinkType
#undef PColor
#undef LColor
#undef RColor
#undef AccSizeType

#define TreeNode Zeta_Core_ShortRelCnt3RBTreeNode
#define RelLinkType int
#define PColor 1
#define LColor 1
#define RColor 1
#define AccSizeType unsigned

#include <zeta/core/bin_tree_node_temp.c>
//
#include <zeta/core/bin_tree_temp.c>
//
#include <zeta/core/rbtree_temp.c>

// -----------------------------------------------------------------------------

#pragma pop_macro("TreeNode")
#pragma pop_macro("RelLinkType")
#pragma pop_macro("PColor")
#pragma pop_macro("LColor")
#pragma pop_macro("RColor")
#pragma pop_macro("AccSizeType")
