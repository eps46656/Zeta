#if !defined(DATA)
#error "xmacro parameter "DATA" is not defined."
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/multi_level_table.hpp>

#pragma push_macro("Cntr")

#if DATA
#define Cntr MultiLevelDataTable
#else
#define Cntr MultiLevelPtrTable
#endif

namespace zeta::core {

struct Cntr : public MultiLevelTable {
    unsigned level;

    unsigned short const* branch_nums;

#if DATA
    size_t stride;
#endif

    size_t size;

    void* root;

    Allocator nav_node_allocator;

#if DATA
    Allocator dat_node_allocator;
#endif

    // -------------------------------------------------------------------------

    /**
     * @brief Initialize the mlt.
     *
     * @param mlt The target mlt.
     */
    static void Init(void* mlt);

    /**
     * @brief Deinitialize the mlt.
     *
     * @param mlt The target mlt.
     */
    static void Deinit(void* mlt);

    /**
     * @brief Get the size of mlt. Assume the value does not overflow max range
     * of size_t.
     *
     * @param mlt The target mlt.
     */
    static size_t GetSize(void* mlt);

    /**
     * @brief Get the total capacity of mlt. Assume the value does not overflow
     * max range of size_t.
     *
     * @param mlt The target mlt.
     */
    static size_t GetCapacity(void* mlt);

    /**
     * @brief Get the reference of target entry by indexes.
     *
     * @param mlt The target mlt.
     * @param idx The index of target entry in each level.
     *
     * @return The reference of target entry. If the it is not inserted, return
     * NULL.
     */
    static void* Access(void* mlt, size_t* idxes);

    static void* FindFirst(void* mlt, size_t* dst_idxes);

    static void* FindLast(void* mlt, size_t* dst_idxes);

    /**
     * @brief Find the first entry before idx.
     *
     * @param mlt The target mlt.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    static void* FindPrev(void* mlt, size_t* idxes, bool included);

    /**
     * @brief Find the first entry after idx.
     *
     * @param mlt The target mlt.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    static void* FindNext(void* mlt, size_t* idxes, bool included);

    /**
     * @brief Insert a new entry at idxes then return its reference. If
     * it has already existed, remaining its origin value.
     *
     * @param mlt The target mlt.
     * @param idxes The indexes of target entry in each level.
     *
     * @return The reference of target entry.
     */
    static Pair<void*, bool> Insert(void* mlt, size_t* idxes);

    /**
     * @brief Erase the target entry by indexes. If it has not existen.
     *
     * @param mlt The target mlt.
     * @param idx The indexes of target entry in each level.
     *
     * @return The reference of target entry.
     */
    static bool Erase(void* mlt, size_t* idxes);

    /**
     * @brief Erase all existed entries.
     *
     * @param mlt The target mlt.
     */
    static void EraseAll(void* mlt);

    static void Check(void* mlt);

    static void CheckIdxes(void* mlt, size_t const* idxes);

#if DATA
    static void Sanitize(void* mlt, MemRecorder* dst_nav_node,
                         MemRecorder* dst_dat_node);
#else
    static void Sanitize(void* mlt, MemRecorder* dst_nav_node);
#endif

    // -------------------------------------------------------------------------

#if DATA
    static constexpr size_t CalcDataNodeSize_(size_t stride, size_t branch_num);

    static void* AllocateDatNode_(size_t stride, size_t branch_num,
                                  Allocator dat_node_allocator);

    static void DeallocateDatNode_(size_t stride, size_t branch_num,
                                   Allocator dat_node_allocator, void* node);
#endif

#if DATA
    static size_t Sanitize_(MemRecorder* dst_nav_node,
                            MemRecorder* dst_dat_node, unsigned level_i,
                            unsigned short const* branch_nums, size_t stride,
                            void* node);
#else
    static size_t Sanitize_(MemRecorder* dst_nav_node, unsigned level_i,
                            unsigned short const* branch_nums, void* node);
#endif
};

}  // namespace zeta::core

#pragma pop_macro("Cntr")
