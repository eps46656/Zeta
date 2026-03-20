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
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/utils.hpp>

#if EnData

#pragma push_macro("NameSpace")
#define NameSpace multi_level_data_table

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList \
    typename TplNavNodeAllocatorLike, typename TplDataNodeAllocatorLike

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename NavNodeAllocatorLike, typename DataNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList NavNodeAllocatorLike, DataNodeAllocatorLike

#else

#pragma push_macro("NameSpace")
#define NameSpace multi_level_ptr_table

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList typename TplNavNodeAllocatorLike

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename NavNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList NavNodeAllocatorLike

#endif

namespace zeta::core::NameSpace {

constexpr unsigned max_level{ 12 };

constexpr unsigned min_branch_num{ 2 };
constexpr unsigned max_branch_num{ ZETA_Core_ullong_width };

struct NavNode;

template <CntrTplDeclParamList>
struct Cntr;

struct NavNode {
    unsigned long long active_map;
    void* ptrs[];
};

ZETA_Core_StaticAssert(offsetof(NavNode, active_map) == 0);

template <CntrTplDeclParamList>
struct Cntr {
    using NavNodeAllocatorLike = TplNavNodeAllocatorLike;

#if EnData
    using DataNodeAllocatorLike = TplDataNodeAllocatorLike;
#endif

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

/**
 * @brief Initialize the cntr.
 *
 * @param cntr The target cntr.
 */
template <CntrTplParamList, typename NavNodeAllocatorInitArg
#if EnData
          ,
          typename DataNodeAllocatorInitArg
#endif
          >
void Init(Cntr<CntrTplArgList>* cntr, unsigned level,
          unsigned short const* branch_nums
#if EnData
          ,
          size_t stride
#endif
          ,
          NavNodeAllocatorInitArg&& nav_node_alctr_init_arg
#if EnData
          ,
          DataNodeAllocatorInitArg&& data_node_alctr_init_arg
#endif
);

/**
 * @brief Deinitialize the cntr.
 *
 * @param cntr The target cntr.
 */
template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

/**
 * @brief Get the size of cntr. Assume the value does not overflow max range
 * of size_t.
 *
 * @param cntr The target cntr.
 */
template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList>* cntr);

/**
 * @brief Get the total capacity of cntr. Assume the value does not overflow
 * max range of size_t.
 *
 * @param cntr The target cntr.
 */
template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList>* cntr);

/**
 * @brief Get the reference of target entry by indexes.
 *
 * @param cntr The target cntr.
 * @param idx The index of target entry in each level.
 *
 * @return The reference of target entry. If the it is not inserted, return
 * nullptr.
 */
template <CntrTplParamList>
void* Access(Cntr<CntrTplArgList>* cntr, size_t* idxes);

template <CntrTplParamList>
void* FindFirst(Cntr<CntrTplArgList>* cntr, size_t* dst_idxes);

template <CntrTplParamList>
void* FindLast(Cntr<CntrTplArgList>* cntr, size_t* dst_idxes);

/**
 * @brief Find the first entry before idx.
 *
 * @param cntr The target cntr.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
template <CntrTplParamList>
void* FindPrev(Cntr<CntrTplArgList>* cntr, size_t* idxes, bool included);

/**
 * @brief Find the first entry after idx.
 *
 * @param cntr The target cntr.
 * @param idx The beginning index of searching, inclusivly.
 *
 * @return The reference of target entry.
 */
template <CntrTplParamList>
void* FindNext(Cntr<CntrTplArgList>* cntr, size_t* idxes, bool included);

/**
 * @brief Insert a new entry at idxes then return its reference. If
 * it has already existed, remaining its origin value.
 *
 * @param cntr The target cntr.
 * @param idxes The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
template <CntrTplParamList>
utils::Pair<void*, bool> Insert(Cntr<CntrTplArgList>* cntr, size_t* idxes);

/**
 * @brief Erase the target entry by indexes. If it has not existen.
 *
 * @param cntr The target cntr.
 * @param idx The indexes of target entry in each level.
 *
 * @return The reference of target entry.
 */
template <CntrTplParamList>
bool Erase(Cntr<CntrTplArgList>* cntr, size_t* idxes);

/**
 * @brief Erase all existed entries.
 *
 * @param cntr The target cntr.
 */
template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList>* cntr,
              mem_recorder::MemRecorder* dst_nav_node
#if EnData
              ,
              mem_recorder::MemRecorder* dst_data_node
#endif
);

}  // namespace zeta::core::NameSpace

#pragma pop_macro("NameSpace")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")

#endif

#pragma pop_macro("Skip")
