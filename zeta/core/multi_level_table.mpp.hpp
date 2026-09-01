// IWYU pragma: private
// IWYU pragma: friend "zeta/core/multi_level_table.hpp"

#if !defined(EnDataNode)
#error "EnDataNode is not defined."
#endif

#pragma push_macro("Skip")

#if EnDataNode

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
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

#if EnDataNode

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#else

#pragma push_macro("Namespace")
#define Namespace multi_level_ptr_table

#endif

namespace zeta::core::Namespace {

constexpr unsigned max_level{ 12 };

using BranchNum = unsigned short;

constexpr BranchNum min_branch_num{ 2 };
constexpr BranchNum max_branch_num{ integral::RangeMaxOf<BranchNum> / 2 };

template <integral::IsUnsignedIntegral ActiveMap_>
struct NavNode {
    using ActiveMap = ActiveMap_;

    ZETA_Core_DebugStructPadding;

    ActiveMap active_map;
    void* ptrs[];
};

#if EnDataNode

template <integral::IsIntegral ActiveMap_>
struct DataNode {
    using ActiveMap = ActiveMap_;

    ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<ActiveMap>);

    ZETA_Core_DebugStructPadding;

    ActiveMap active_map;
    unsigned char data[] __attribute__((aligned(max_align)));
};

#endif

template <integral::IsUnsignedIntegral ActiveMap_,
          typename NavNodeAllocatorLike_
#if EnDataNode
          ,
          typename DataNodeAllocatorLike_
#endif
          >
struct Cntr {
    using ActiveMap = ActiveMap_;

    using NavNodeAllocatorLike = NavNodeAllocatorLike_;

#if EnDataNode
    using DataNodeAllocatorLike = DataNodeAllocatorLike_;
#endif

    unsigned level;

    BranchNum const* branch_nums;

#if EnDataNode
    size_t elem_stride;
#endif

    size_t elem_cnt;

    void* root;

    NavNodeAllocatorLike nav_node_alctr_like;

#if EnDataNode
    DataNodeAllocatorLike data_node_alctr_like;
#endif

    /**
     * @brief Initialize the cntr.
     *
     * @param cntr The target cntr.
     */
    template <typename NavNodeAllocatorInitArg
#if EnDataNode
              ,
              typename DataNodeAllocatorInitArg
#endif
              >
    constexpr Cntr(NavNodeAllocatorInitArg&& nav_node_alctr_init_arg,
#if EnDataNode
                   DataNodeAllocatorInitArg&& data_node_alctr_init_arg,
#endif
                   unsigned level, BranchNum const* branch_nums
#if EnDataNode
                   ,
                   size_t stride
#endif
    );

    constexpr ~Cntr();

    /**
     * @brief Get the size of cntr. Assume the value does not overflow max range
     * of size_t.
     *
     * @param cntr The target cntr.
     */
    constexpr size_t GetElemCnt(this Cntr& cntr);

    /**
     * @brief Get the total capacity of cntr. Assume the value does not overflow
     * max range of size_t.
     *
     * @param cntr The target cntr.
     */
    constexpr size_t GetMaxElemCnt(this Cntr& cntr);

    /**
     * @brief Get the reference of target entry by indexes.
     *
     * @param cntr The target cntr.
     * @param branch_idx The branch index of target entry in each level.
     *
     * @return The reference of target entry. If the it is not inserted, return
     * nullptr.
     */
    template <elem_stream::provider::IsProvider SrcBranchIdxesProvier>
    constexpr auto Access(this auto& cntr,
                          SrcBranchIdxesProvier&& src_branch_idxes_provider)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindFirst(this auto& cntr,
                             DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindLast(this auto& cntr,
                            DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    /**
     * @brief Find the first entry before idx.
     *
     * @param cntr The target cntr.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
              elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindPrevIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
              elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindPrevExcl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    /**
     * @brief Find the first entry after idx.
     *
     * @param cntr The target cntr.
     * @param idx The beginning index of searching, inclusivly.
     *
     * @return The reference of target entry.
     */
    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
              elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindNextIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider,
              elem_stream::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindNextExcl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
    constexpr pair::Pair<void*, bool> Insert(
        this Cntr& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider);

    /**
     * @brief Erase the target entry by indexes. If it has not existen.
     *
     * @param cntr The target cntr.
     * @param src_branch_idxes_provider The branch indexes of target entry in
     * each level.
     *
     * @return The reference of target entry.
     */
    template <elem_stream::provider::IsProvider SrcBranchIdxesProvider>
    constexpr bool Erase(this Cntr& cntr,
                         SrcBranchIdxesProvider&& src_branch_idxes_provider);

    /**
     * @brief Erase all existed entries.
     *
     * @param cntr The target cntr.
     */
    constexpr void EraseAll(this Cntr& cntr);

    constexpr void Sanitize(this Cntr& cntr,
                            mem_recorder::MemRecorder* dst_nav_node
#if EnDataNode
                            ,
                            mem_recorder::MemRecorder* dst_data_node
#endif
    );
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")

#endif

#pragma pop_macro("Skip")
