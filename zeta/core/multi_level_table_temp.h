#if !defined(ZETA_Core_MultiLevelTable_MacroGuard)
#define ZETA_Core_MultiLevelTable_MacroGuard 1
#endif

#include <zeta/core/allocator.h>
#include <zeta/core/mem_check_utils.h>

ZETA_Core_ExternC_Beg;

#pragma push_macro("Cntr")
#pragma push_macro("Cntr_")
#pragma push_macro("max_level")

#define ZETA_Core_MultiLevelTable_max_level (12)

#define ZETA_Core_MultiLevelTable_min_branch_num (2)
#define ZETA_Core_MultiLevelTable_max_branch_num ZETA_Core_ullong_width

#define max_level ZETA_Core_MultiLevelTable_max_level

#if DATA

#define Cntr Zeta_Core_MultiLevelDataTable

#else

#define Cntr Zeta_Core_MultiLevelPtrTable

#endif

#define Cntr_(x) ZETA_Core_Concat(Cntr, _, x)

ZETA_Core_DeclareStruct(Cntr);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_NavNode);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_Cursor);
ZETA_Core_DeclareStruct(Zeta_Core_MultiLevelTable_InsertResult);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct Cntr {
    int level;

    unsigned short const* branch_nums;

#if DATA
    size_t stride;
#endif

    size_t size;

    Zeta_Core_MultiLevelTable_NavNode* root;

    Zeta_Core_Allocator nav_node_allocator;

#if DATA
    Zeta_Core_Allocator dat_node_allocator;
#endif
};

#if ZETA_Core_MultiLevelTable_MacroGuard == 1

struct Zeta_Core_MultiLevelTable_NavNode {
    unsigned long long active_map;
    void* ptrs[];
};

ZETA_Core_StaticAssert(offsetof(Zeta_Core_MultiLevelTable_NavNode,
                                active_map) == 0);

struct Zeta_Core_MultiLevelTable_Cursor {
    unsigned short idxes[max_level];
};

struct Zeta_Core_MultiLevelTable_InsertResult {
    void* addr;
    bool_t inserted;
};

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * @brief Initialize the mlt.
 *
 * @param mlt The target mlt.
 */
void Cntr_(Init)(void* mlt);

/**
 * @brief Deinitialize the mlt.
 *
 * @param mlt The target mlt.
 */
void Cntr_(Deinit)(void* mlt);

/**
 * @brief Get the size of mlt. Assume the value does not overflow max range of
 * size_t.
 *
 * @param mlt The target mlt.
 */
size_t Cntr_(GetSize)(void* mlt);

/**
 * @brief Get the total capacity of mlt. Assume the value does not overflow
 * max range of size_t.
 *
 * @param mlt The target mlt.
 */
size_t Cntr_(GetCapacity)(void* mlt);

/**
 * @brief Get the reference of target entry by indexes.
 *
 * @param mlt The target mlt.
 * @param idx The index of target entry in each level.
 *
 * @return The reference of target entry. If the it is not inserted, return
 * NULL.
 */
void* Cntr_(Access)(void* mlt, size_t* idxes);

void* Cntr_(FindFirst)(void* mlt, size_t* dst_idxes);

void* Cntr_(FindLast)(void* mlt, size_t* dst_idxes);

/**
 * @brief Find the first entry before idx.
 *
 * @param mlt The target mlt.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
void* Cntr_(FindPrev)(void* mlt, size_t* idxes, bool_t included);

/**
 * @brief Find the first entry after idx.
 *
 * @param mlt The target mlt.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
void* Cntr_(FindNext)(void* mlt, size_t* idxes, bool_t included);

/**
 * @brief Insert a new entry at idxes then return its reference. If
 * it has already existed, remaining its origin value.
 *
 * @param mlt The target mlt.
 * @param idxes The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
Zeta_Core_MultiLevelTable_InsertResult Cntr_(Insert)(void* mlt, size_t* idxes);

/**
 * @brief Erase the target entry by indexes. If it has not existen.
 *
 * @param mlt The target mlt.
 * @param idx The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
bool_t Cntr_(Erase)(void* mlt, size_t* idxes);

/**
 * @brief Erase all existed entries.
 *
 * @param mlt The target mlt.
 */
void Cntr_(EraseAll)(void* mlt);

void Cntr_(Check)(void* mlt);

void Cntr_(CheckIdxes)(void* mlt, size_t const* idxes);

#if DATA
void Cntr_(Sanitize)(void* mlt, Zeta_Core_MemRecorder* dst_nav_node,
                     Zeta_Core_MemRecorder* dst_dat_node);
#else
void Cntr_(Sanitize)(void* mlt, Zeta_Core_MemRecorder* dst_nav_node);
#endif

#pragma pop_macro("Cntr")
#pragma pop_macro("Cntr_")
#pragma pop_macro("max_level")

ZETA_Core_ExternC_End;

#undef ZETA_Core_MultiLevelTable_MacroGuard
#define ZETA_Core_MultiLevelTable_MacroGuard 2
