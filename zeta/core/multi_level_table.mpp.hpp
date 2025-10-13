#if !defined(EnData)
#error "EnData is not defined"
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/utils.hpp>

#pragma push_macro("Cntr")
#pragma push_macro("TplList")
#pragma push_macro("TypenameTplList")

#if EnData

#define Cntr MultiLevelDataTable
#define TplList NavNodeAllocator, DataNodeAllocator
#define TypenameTplList typename NavNodeAllocator, typename DataNodeAllocator

#else

#define Cntr MultiLevelPtrTable
#define TplList NavNodeAllocator
#define TypenameTplList typename NavNodeAllocator

#endif

namespace zeta::core {

template <TypenameTplList>
struct Cntr;

// -----------------------------------------------------------------------------

template <TypenameTplList>
struct Cntr {
    static constexpr unsigned max_level{ 12 };

    static constexpr unsigned min_branch_num{ 2 };
    static constexpr unsigned max_branch_num{ ZETA_Core_ullong_width };

    struct NavNode {
        unsigned long long active_map;
        void* ptrs[];
    };

    ZETA_Core_StaticAssert(offsetof(NavNode, active_map) == 0);

    // -------------------------------------------------------------------------

    static constexpr bool TestActiveMap_(unsigned long long active_map,
                                         unsigned idx);

    // -------------------------------------------------------------------------

    static void* AllocateNavNode_(size_t branch_num,
                                  NavNodeAllocator* nav_node_allocator);

    static void DeallocateNavNode_(NavNodeAllocator* nav_node_allocator,
                                   void* node);

#if EnData
    static size_t CalcDataNodeSize_(size_t stride, size_t branch_num);

    static void* AllocateDataNode_(size_t stride, size_t branch_num,
                                   DataNodeAllocator* data_node_allocator);

    static void DeallocateDataNode_(size_t stride, size_t branch_num,
                                    DataNodeAllocator* data_node_allocator,
                                    void* node);
#endif

    static void EraseAll_(Cntr* cntr, void* node, unsigned level_i);

    static size_t Sanitize_(MemRecorder* dst_nav_node,
#if EnData
                            MemRecorder* dst_data_node,
#endif
                            unsigned level_i, unsigned short const* branch_nums,
#if EnData
                            size_t stride,
#endif
                            void* node);

    // -------------------------------------------------------------------------

    /**
     * @brief Initialize the cntr.
     *
     * @param cntr The target cntr.
     */
    static void Init(void* cntr);

    /**
     * @brief Deinitialize the cntr.
     *
     * @param cntr The target cntr.
     */
    static void Deinit(void* cntr);

    /**
     * @brief Get the size of cntr. Assume the value does not overflow max range
     * of size_t.
     *
     * @param cntr The target cntr.
     */
    static size_t GetSize(void* cntr);

    /**
     * @brief Get the total capacity of cntr. Assume the value does not overflow
     * max range of size_t.
     *
     * @param cntr The target cntr.
     */
    static size_t GetCapacity(void* cntr);

    /**
     * @brief Get the reference of target entry by indexes.
     *
     * @param cntr The target cntr.
     * @param idx The index of target entry in each level.
     *
     * @return The reference of target entry. If the it is not inserted, return
     * NULL.
     */
    static void* Access(void* cntr, size_t* idxes);

    static void* FindFirst(void* cntr, size_t* dst_idxes);

    static void* FindLast(void* cntr, size_t* dst_idxes);

    /**
     * @brief Find the first entry before idx.
     *
     * @param cntr The target cntr.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    static void* FindPrev(void* cntr, size_t* idxes, bool included);

    /**
     * @brief Find the first entry after idx.
     *
     * @param cntr The target cntr.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    static void* FindNext(void* cntr, size_t* idxes, bool included);

    /**
     * @brief Insert a new entry at idxes then return its reference. If
     * it has already existed, remaining its origin value.
     *
     * @param cntr The target cntr.
     * @param idxes The indexes of target entry in each level.
     *
     * @return The reference of target entry.
     */
    static Pair<void*, bool> Insert(void* cntr, size_t* idxes);

    /**
     * @brief Erase the target entry by indexes. If it has not existen.
     *
     * @param cntr The target cntr.
     * @param idx The indexes of target entry in each level.
     *
     * @return The reference of target entry.
     */
    static bool Erase(void* cntr, size_t* idxes);

    /**
     * @brief Erase all existed entries.
     *
     * @param cntr The target cntr.
     */
    static void EraseAll(void* cntr);

    static void Check(void* cntr);

    static void CheckIdxes(void* cntr, size_t const* idxes);

    static void Sanitize(void* cntr, MemRecorder* dst_nav_node
#if EnData
                         ,
                         MemRecorder* dst_data_node
#endif
    );

    // -------------------------------------------------------------------------

    unsigned level;

    unsigned short const* branch_nums;

#if EnData
    size_t stride;
#endif

    size_t size;

    void* root;

    NavNodeAllocator nav_node_allocator;

#if EnData
    DataNodeAllocator data_node_allocator;
#endif
};

}  // namespace zeta::core

#pragma pop_macro("Cntr")
#pragma pop_macro("TplList")
#pragma pop_macro("TypenameTplList")
