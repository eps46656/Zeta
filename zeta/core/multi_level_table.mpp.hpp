// IWYU pragma: private
// IWYU pragma: friend "zeta/core/multi_level_table.hpp"

#if ZETA_Core_Editor
#pragma push_macro("DataNodeEn")
#endif

#if !defined(DataNodeEn)
#error "DataNodeEn is not defined."
#endif

#if DataNodeEn

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
#include <zeta/core/debug_utils/lifecycle_sanity.hpp>
#include <zeta/core/debug_utils/mem_recording.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/seq_endpoint.hpp>
#include <zeta/core/utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

#if DataNodeEn

#pragma push_macro("Namespace")
#define Namespace multi_level_data_table

#else

#pragma push_macro("Namespace")
#define Namespace multi_level_ptr_table

#endif

namespace zeta::core::Namespace {

constexpr unsigned level_max{ 12 };

using BranchNum = unsigned short;

constexpr BranchNum branch_num_min{ 2 };
constexpr BranchNum branch_num_max{ integral::RangeMaxOf<BranchNum> / 2 };

template <integral::IsUnsignedIntegralType ActiveMap_>
struct NavNode {
    using ActiveMap = ActiveMap_;

    ZETA_Core_DebugStructPadding;

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    ActiveMap active_map;
    void* ptrs[];

    constexpr NavNode(BranchNum branch_num);
};

#if DataNodeEn

template <integral::IsUnsignedIntegralType ActiveMap_, meta::IsObjectType Data_>
struct DataNode {
    using ActiveMap = ActiveMap_;
    using Data      = Data_;

    static_assert(integral::IsUnsignedIntegralType<ActiveMap>);

    ZETA_Core_DebugStructPadding;

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    ActiveMap active_map;
    alignas(Data) unsigned char data[];

    constexpr DataNode(BranchNum branch_num);
};

#endif

template <integral::IsUnsignedIntegralType ActiveMap_,
#if DataNodeEn
          meta::IsObjectType Data_,
#endif
          allocator::IsAllocatorLike NavNodeAllocatorLike_
#if DataNodeEn
          ,
          allocator::IsAllocatorLike DataNodeAllocatorLike_
#endif
          >
struct Cntr {
    using ActiveMap = ActiveMap_;

#if DataNodeEn
    using Data = Data_;
#endif

    using NavNodeAllocatorLike = NavNodeAllocatorLike_;

#if DataNodeEn
    using DataNodeAllocatorLike = DataNodeAllocatorLike_;
#endif

#if ZETA_Core_DebugEnable
    debug_utils::lifecycle_sanity::ObjHook obj_hook;
#endif

    unsigned level;

    BranchNum const* branch_nums;

    size_t elem_cnt;

    void* root;

    NavNodeAllocatorLike nav_node_alctr_like;

#if DataNodeEn
    DataNodeAllocatorLike data_node_alctr_like;
#endif

    template <typename NavNodeAllocatorConstructArg
#if DataNodeEn
              ,
              typename DataNodeAllocatorConstructArg
#endif
              >
    constexpr Cntr(lifecycle::DirectConstructTag, unsigned level,
                   BranchNum const* branch_nums, size_t elem_cnt, void* root,
                   NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if DataNodeEn
                   ,
                   DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    );

    template <typename NavNodeAllocatorConstructArg
#if DataNodeEn
              ,
              typename DataNodeAllocatorConstructArg
#endif
              >
    constexpr Cntr(unsigned level, BranchNum const* branch_nums,
                   NavNodeAllocatorConstructArg&& nav_node_alctr_construct_arg
#if DataNodeEn
                   ,
                   DataNodeAllocatorConstructArg&& data_node_alctr_construct_arg
#endif
    );

    constexpr ~Cntr();

    constexpr void Disown(this meta::IsNonConstWithoutReference auto&& self);

    constexpr size_t GetElemCnt(this Cntr const& self);

    constexpr size_t GetMaxElemCnt(this Cntr const& self);

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvier>
    constexpr auto Access(this auto&& self,
                          SrcBranchIdxesProvier&& src_branch_idxes_provider)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>, void>*;

    template <
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindFirst(this auto&& self,
                             DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>, void>*;

    template <
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindLast(this auto&& self,
                            DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>,
#if DataNodeEn
            Elem
#else
            void
#endif
            >*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindPrevIncl(
        this auto&& self, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>,
#if DataNodeEn
            Elem
#else
            void
#endif
            >*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindPrevExcl(
        this auto&& self, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>,
#if DataNodeEn
            Elem
#else
            void
#endif
            >*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindNextIncl(
        this auto&& self, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>,
#if DataNodeEn
            Elem
#else
            void
#endif
            >*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider,
        seq_endpoint::acceptor::IsAcceptor<BranchNum> DstBranchIdxesAcceptor>
    constexpr auto FindNextExcl(
        this auto&& self, SrcBranchIdxesProvider&& src_branch_idxes_provider,
        DstBranchIdxesAcceptor&& dst_branch_idxes_acceptor)
        -> meta::ConditionalAddConst<
            meta::IsConstTypeWithoutReference<decltype(self)>,
#if DataNodeEn
            Elem
#else
            void
#endif
            >*;

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider
#if DataNodeEn
        ,
        seq_endpoint::provider::IsProvider<Data> DataProvider
#endif
        >
    constexpr pair::Pair<void*, bool> Insert(
        this meta::IsNonConstWithoutReference auto&& self,
        SrcBranchIdxesProvider&& src_branch_idxes_provider
#if DataNodeEn
        ,
        DataProvider&& data_provider
#endif
    );

    template <
        seq_endpoint::provider::IsProvider<BranchNum> SrcBranchIdxesProvider
#if DataNodeEn
        ,
        seq_endpoint::acceptor::IsAcceptor<Data> DataAcceptor
#endif
        >
    constexpr bool Erase(this meta::IsNonConstWithoutReference auto&& self,
                         SrcBranchIdxesProvider&& src_branch_idxes_provider
#if DataNodeEn
                         ,
                         DataAcceptor&& data_acceptor
#endif
    );

#if DataNodeEn
    template <seq_endpoint::acceptor::IsAcceptor<BranchNum> DataAcceptor>
#endif
    constexpr void EraseAll(this meta::IsNonConstWithoutReference auto&& self
#if DataNodeEn
                            ,
                            DataAcceptor&& data_acceptor
#endif
    );

#if ZETA_Core_DebugEnable
    constexpr void SanityCheck(
        this Cntr const& self,
        debug_utils::mem_recording::MemRecorder& nav_node_recorder
#if DataNodeEn
        ,
        debug_utils::mem_recording::MemRecorder& data_node_recorder
#endif
    );
#endif
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")

#endif

#if ZETA_Core_Editor
#pragma pop_macro("DataNodeEn")
#endif
