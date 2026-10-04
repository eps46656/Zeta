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

template <integral::IsIntegral ActiveMap_, meta::IsContainerElem Data_>
struct DataNode {
    using ActiveMap = ActiveMap_;
    using Data = Data_;

    static_assert(integral::IsUnsignedIntegral<ActiveMap>);

    ZETA_Core_DebugStructPadding;

    ActiveMap active_map;
    alignas(Data) unsigned char data[];
};

#endif

template <integral::IsUnsignedIntegral ActiveMap_,
#if EnDataNode
          meta::IsContainerElem Data_,
#endif
          typename NavNodeAllocatorLike_
#if EnDataNode
          ,
          typename DataNodeAllocatorLike_
#endif
          >
struct Cntr {
    using ActiveMap = ActiveMap_;

#if EnDataNode
    using Data = Data_;
#endif

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

    constexpr size_t GetElemCnt(this Cntr& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr& cntr);

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvier>
    constexpr auto Access(this auto& cntr,
                          SrcBranchIdxesProvier&& src_branch_idxes_provider)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindFirst(this auto& cntr,
                             DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindLast(this auto& cntr,
                            DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindPrevIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindPrevExcl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindNextIncl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindNextExcl(
        this auto& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::MakeConstIf<void, meta::IsConst<decltype(cntr)>>*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider
#if EnDataNode
        ,
        seq_endpoint::provider::IsProvider<Data> DataProvider
#endif
        >
    constexpr pair::Pair<void*, bool> Insert(
        this Cntr& cntr, SrcBranchIdxesProvider&& src_branch_idxes_provider
#if EnDataNode
        ,
        DataProvider&& data_provider
#endif
    );

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider
#if EnDataNode
        ,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor
#endif
        >
    constexpr bool Erase(this Cntr& cntr,
                         SrcBranchIdxesProvider&& src_branch_idxes_provider
#if EnDataNode
                         ,
                         DataAcceptor&& data_acceptor
#endif
    );

#if EnDataNode
    template <seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor
#endif
              constexpr void EraseAll(this Cntr& cntr
#if EnDataNode
                                      ,
                                      DataAcceptor&& data_acceptor
#endif
              );

#if ZETA_Core_DebugUtils_Sanity_Enable
    static constexpr void SanityCheck(
        void const* cntr, debug_utils::sanity::SanityCheckScope scope);
#endif
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")

#endif
