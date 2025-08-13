// define LListNode

#if !defined(ZETA_Core_LList_MacroGuard)
#define ZETA_Core_LList_MacroGuard 1
#endif

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

#if ZETA_Core_LList_MacroGuard == 1

#define Zeta_Core_LList(llist_node_type, x) \
    ZETA_Core_Concat(Zeta_Core_LList_lt_, llist_node_type, _gt_, x)

#endif

#if !defined(LListNode)
#error "LListNode is not defined."
#endif

size_t Zeta_Core_LList(LListNode, Count)(void* n);

void Zeta_Core_LList(LListNode, InsertL)(void* n, void* m);
void Zeta_Core_LList(LListNode, InsertR)(void* n, void* m);

void Zeta_Core_LList(LListNode, Extract)(void* n);

void Zeta_Core_LList(LListNode, InsertSegL)(void* n, void* m_beg, void* m_end);
void Zeta_Core_LList(LListNode, InsertSegR)(void* n, void* m_beg, void* m_end);

void Zeta_Core_LList(LListNode, ExtractSeg)(void* n_beg, void* n_end);

ZETA_Core_ExternC_End;

#undef ZETA_Core_LList_MacroGuard
#define ZETA_Core_LList_MacroGuard 2
