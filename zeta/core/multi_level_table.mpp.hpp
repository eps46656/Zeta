// IWYU pragma: private
// IWYU pragma: friend "zeta/core/multi_level_table.hpp"

#if !defined(EnDataNode)
#error "EnDataNode is not defined."
#endif

#if EnDataNode

#if !defined(ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table)
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table 1
#endif

#else

#if !defined(ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table)
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table 1
#endif

#endif

#if ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table == 1 || \
    ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table == 1

#if ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table == 1
#undef ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_data_table 2
#endif

#if ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table == 1
#undef ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table
#define ZETA_MacroGuard__multi_level_table_mpp_hpp__multi_level_ptr_table 2
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/sanity.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/seq_endpoint.hpp>
#include <zeta/core/utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

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

    static_assert(integral::IsUnsignedIntegral<ActiveMap>);

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
     * @brief Constructialize the cntr.
     *
     * @param cntr The target cntr.
     */
    template <typename NavNodeAllocatorConstructArg
#if EnDataNode
              ,
              typename DataNodeAllocatorConstructArg
#endif
              >
    constexpr Cntr(lifecycle::DirectConstructTag, unsigned level,
                   BranchNum const* branch_nums,
#if EnDataNode
                   size_t elem_stride,
#endif
                   size_t elem_cnt, void* root,
                   NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if EnDataNode
                   ,
                   DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    );

    /**
     * @brief Constructialize the cntr.
     *
     * @param cntr The target cntr.
     */
    template <typename NavNodeAllocatorConstructArg
#if EnDataNode
              ,
              typename DataNodeAllocatorConstructArg
#endif
              >
    constexpr Cntr(unsigned level, BranchNum const* branch_nums,
#if EnDataNode
                   size_t elem_stride,
#endif
                   NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if EnDataNode
                   ,
                   DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    );

    constexpr ~Cntr();

    constexpr void Destruct(this Cntr& cntr);

    constexpr void DisownDestruct(this Cntr& cntr);

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

    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvier>
    constexpr auto Access(this auto& cntr,
                          SrcBranchIdxesProvier&& src_branch_idxes_provider)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindFirst(this auto& cntr,
                             DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindLast(this auto& cntr,
                            DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider,
              seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindPrevIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider,
              seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
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
    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider,
              seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindNextIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider,
              seq_endpoint::acceptor::IsAcceptor DstBranchIdxesAcceptor>
    constexpr auto FindNextExcl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::Conditional<meta::IsConst<decltype(cntr)>, void const*, void*>;

    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider>
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
    template <seq_endpoint::provider::IsProvider SrcBranchIdxesProvider>
    constexpr bool Erase(this Cntr& cntr,
                         SrcBranchIdxesProvider&& src_branch_idxes_provider);

    /**
     * @brief Erase all existed entries.
     *
     * @param cntr The target cntr.
     */
    constexpr void EraseAll(this Cntr& cntr);

#if ZETA_Core_DebugUtils_Sanity_Enable
    static constexpr void SanityCheck(
        void const* cntr, debug_utils::sanity::SanityCheckScope scope);
#endif
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")

#endif
