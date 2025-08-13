#include <zeta/core/llist_node.h>

#pragma push_macro("LListNode")
#pragma push_macro("RelLinkType")
#pragma push_macro("LColor")
#pragma push_macro("RColor")

// -----------------------------------------------------------------------------

#undef LListNode
#undef RelLinkType
#undef LColor
#undef RColor

#define LListNode Zeta_Core_OrdLListNode
#undef RelLinkType
#define LColor 0
#define RColor 0

#include <zeta/core/llist_node_temp.c>
//
#include <zeta/core/llist_temp.c>

// -----------------------------------------------------------------------------

#undef LListNode
#undef RelLinkType
#undef LColor
#undef RColor

#define LListNode Zeta_Core_OrdRBLListNode
#undef RelLinkType
#define LColor 1
#define RColor 0

#include <zeta/core/llist_node_temp.c>
//
#include <zeta/core/llist_temp.c>

// -----------------------------------------------------------------------------

#undef LListNode
#undef RelLinkType
#undef LColor
#undef RColor

#define LListNode Zeta_Core_Ord2RBLListNode
#undef RelLinkType
#define LColor 1
#define RColor 1

#include <zeta/core/llist_node_temp.c>
//
#include <zeta/core/llist_temp.c>

// -----------------------------------------------------------------------------

#undef LListNode
#undef RelLinkType
#undef LColor
#undef RColor

#define LListNode Zeta_Core_ShortRel2RBLListNode
#define RelLinkType int
#define LColor 1
#define RColor 1

#include <zeta/core/llist_node_temp.c>
//
#include <zeta/core/llist_temp.c>

// -----------------------------------------------------------------------------

#pragma pop_macro("LListNode")
#pragma pop_macro("RelLinkType")
#pragma pop_macro("LColor")
#pragma pop_macro("RColor")
