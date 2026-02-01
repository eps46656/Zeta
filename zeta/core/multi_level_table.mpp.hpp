#if !defined(EnData)
#error "EnData is not defined."
#endif

#pragma push_macro("Skip")

#if EnData

#if defined(ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table)
#define Skip 1
#else
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table
#define Skip 0
#endif

#else

#if defined(ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table)
#define Skip 1
#else
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table
#define Skip 0
#endif

#endif

#if !Skip

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/utils.hpp>

#pragma push_macro("NameSpace")
#pragma push_macro("TplDeclParamList")
#pragma push_macro("TplArgList")

#if EnData

#define NameSpace multi_level_data_table
#define TplDeclParamList \
    typename NavNodeAllocatorLike, typename DataNodeAllocatorLike
#define TplArgList NavNodeAllocatorLike, DataNodeAllocatorLike

#else

#define NameSpace multi_level_ptr_table
#define TplDeclParamList typename NavNodeAllocatorLike
#define TplArgList NavNodeAllocatorLike

#endif

namespace zeta::core::NameSpace {

constexpr unsigned max_level{ 12 };

constexpr unsigned min_branch_num{ 2 };
constexpr unsigned max_branch_num{ ZETA_Core_ullong_width };

// -----------------------------------------------------------------------------

struct NavNode;

template <TplDeclParamList>
struct Cntr;

// -----------------------------------------------------------------------------

struct NavNode {
    unsigned long long active_map;
    void* ptrs[];
};

ZETA_Core_StaticAssert(offsetof(NavNode, active_map) == 0);

template <TplDeclParamList>
struct Cntr {
    unsigned level;

    unsigned short const* branch_nums;

#if EnData
    size_t stride;
#endif

    size_t size;

    void* root;

    NavNodeAllocatorLike nav_node_alctr;

#if EnData
    DataNodeAllocatorLike data_node_alctr;
#endif
};

namespace ops {

/**
 * @brief Initialize the cntr.
 *
 * @param cntr The target cntr.
 */
template <TplDeclParamList>
void Init(Cntr<TplArgList>* cntr);

/**
 * @brief Deinitialize the cntr.
 *
 * @param cntr The target cntr.
 */
template <TplDeclParamList>
void Deinit(Cntr<TplArgList>* cntr);

/**
 * @brief Get the size of cntr. Assume the value does not overflow max range
 * of size_t.
 *
 * @param cntr The target cntr.
 */
template <TplDeclParamList>
size_t GetSize(Cntr<TplArgList>* cntr);

/**
 * @brief Get the total capacity of cntr. Assume the value does not overflow
 * max range of size_t.
 *
 * @param cntr The target cntr.
 */
template <TplDeclParamList>
size_t GetCapacity(Cntr<TplArgList>* cntr);

/**
 * @brief Get the reference of target entry by indexes.
 *
 * @param cntr The target cntr.
 * @param idx The index of target entry in each level.
 *
 * @return The reference of target entry. If the it is not inserted, return
 * nullptr.
 */
template <TplDeclParamList>
void* Access(Cntr<TplArgList>* cntr, size_t* idxes);

template <TplDeclParamList>
void* FindFirst(Cntr<TplArgList>* cntr, size_t* dst_idxes);

template <TplDeclParamList>
void* FindLast(Cntr<TplArgList>* cntr, size_t* dst_idxes);

/**
 * @brief Find the first entry before idx.
 *
 * @param cntr The target cntr.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
template <TplDeclParamList>
void* FindPrev(Cntr<TplArgList>* cntr, size_t* idxes, bool included);

/**
 * @brief Find the first entry after idx.
 *
 * @param cntr The target cntr.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
template <TplDeclParamList>
void* FindNext(Cntr<TplArgList>* cntr, size_t* idxes, bool included);

/**
 * @brief Insert a new entry at idxes then return its reference. If
 * it has already existed, remaining its origin value.
 *
 * @param cntr The target cntr.
 * @param idxes The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
template <TplDeclParamList>
Pair<void*, bool> Insert(Cntr<TplArgList>* cntr, size_t* idxes);

/**
 * @brief Erase the target entry by indexes. If it has not existen.
 *
 * @param cntr The target cntr.
 * @param idx The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
template <TplDeclParamList>
bool Erase(Cntr<TplArgList>* cntr, size_t* idxes);

/**
 * @brief Erase all existed entries.
 *
 * @param cntr The target cntr.
 */
template <TplDeclParamList>
void EraseAll(Cntr<TplArgList>* cntr);

template <TplDeclParamList>
void Sanitize(Cntr<TplArgList>* cntr, MemRecorder* dst_nav_node
#if EnData
              ,
              MemRecorder* dst_data_node
#endif
);

}  // namespace ops

}  // namespace zeta::core::NameSpace

#pragma pop_macro("NameSpace")
#pragma pop_macro("TplArgList")
#pragma pop_macro("TplDeclParamList")

#endif

#pragma pop_macro("Skip")
