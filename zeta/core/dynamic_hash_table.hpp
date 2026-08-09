#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList(suffix)                                  \
    typename HasherLike##suffix, typename ComparatorLike##suffix, \
        typename SaltRandomEngineLike##suffix,                    \
        typename NodeAllocatorLike##suffix,                       \
        typename TableNodeAllocatorLike##suffix

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList                                                   \
    HasherLike, ComparatorLike, SaltRandomEngineLike, NodeAllocatorLike, \
        TableNodeAllocatorLike

namespace zeta::core::dynamic_hash_table {

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

using LListNode = basic_llist_node::Node<void*, meta::AutoValueWrapper<false>,
                                         meta::AutoValueWrapper<false>>;

struct Node {
    LListNode lln;

    generic_hash_table::Node ghtn;

    unsigned char data[] __attribute__((aligned(max_align)));

    constexpr void Init();
};

template <typename ElemHasherLike>
struct HasherWrapper {
    ElemHasherLike elem_hasher;

    template <typename... Args>
    constexpr void Init(Args&&... args);

    constexpr void Deinit();

    constexpr unsigned long long Hash(generic_hash_table::Node const* ghtn,
                                      unsigned long long salt) const;
};

}  // namespace zeta::core::dynamic_hash_table

namespace zeta::core {

template <typename ElemHasherLike>
struct lifecycle::Traits<dynamic_hash_table::HasherWrapper<ElemHasherLike>>
    : public lifecycle::MemberFuncTraitsAdapter<
          dynamic_hash_table::HasherWrapper<ElemHasherLike>> {};

}  // namespace zeta::core

namespace zeta::core::dynamic_hash_table {

template <typename ElemComparatorLike>
struct ComparatorWrapper {
    ElemComparatorLike elem_cmptr;

    template <typename... Args>
    constexpr void Init(Args&&... args);

    constexpr void Deinit();

    template <comparison::IsOpType OpType>
    constexpr auto Compare(OpType, generic_hash_table::Node const* ghtn_a,
                           generic_hash_table::Node const* ghtn_b) const;
};

}  // namespace zeta::core::dynamic_hash_table

namespace zeta::core {

template <typename ComparatorLike>
struct lifecycle::Traits<dynamic_hash_table::ComparatorWrapper<ComparatorLike>>
    : public lifecycle::MemberFuncTraitsAdapter<
          dynamic_hash_table::ComparatorWrapper<ComparatorLike>> {};

}  // namespace zeta::core

namespace zeta::core::dynamic_hash_table {

struct Cursor {
    void const* cntr;
    LListNode* lln;
};

template <CntrTplParamList(_)>
struct Cntr {
    using HasherLike = HasherLike_;
    using ComparatorLike = ComparatorLike_;
    using SaltRandomEngineLike = SaltRandomEngineLike_;
    using NodeAllocatorLike = NodeAllocatorLike_;
    using TableNodeAllocatorLike = TableNodeAllocatorLike_;

    size_t elem_size;

    generic_hash_table::Cntr<HasherWrapper<HasherLike>,
                             ComparatorWrapper<ComparatorLike>,
                             SaltRandomEngineLike, TableNodeAllocatorLike>
        ght;

    LListNode* lln;

    NodeAllocatorLike node_alctr;

    template <typename HasherLikeInitArg, typename ComparatorInitArg,
              typename SaltRandomEngineInitArg, typename NodeAllocatorInitArg,
              typename TableNodeAllocatorInitArg>
    constexpr void Init(
        this Cntr& cntr, size_t elem_size,
        generic_hash_table::RehashingConfig const& rehashing_config,
        HasherLikeInitArg&& elem_hasher_init_arg,
        ComparatorInitArg&& elem_cmptr_init_arg,
        SaltRandomEngineInitArg&& salt_random_engine_init_arg,
        NodeAllocatorInitArg&& node_alctr_init_arg,
        TableNodeAllocatorInitArg&& table_node_alctr_init_arg);

    constexpr void Deinit(this Cntr& cntr);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    constexpr size_t GetElemSize(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr);

    constexpr void GetLBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void PeekL(this Cntr const& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this Cntr const& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this Cntr const& cntr, Cursor const* pos_cursor,
                           bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this auto&& cntr, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Writer>
    constexpr void Insert(this Cntr& cntr, void const* key,
                          KeyHasher const& key_hasher,
                          KeyElemComparator const& key_elem_cmptr,
                          Writer&& writer, Cursor* dst_cursor);

    constexpr void PopL(this Cntr& cntr, size_t cnt);

    constexpr void PopR(this Cntr& cntr, size_t cnt);

    constexpr void Erase(this Cntr& cntr, Cursor* pos_cursor);

    constexpr void EraseAll(this Cntr& cntr);

    constexpr void CopyCursor(this Cntr const& cntr, Cursor* src_cursor,
                              Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr void CursorStepL(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, Cursor* cursor);

    constexpr auto GetEffFactor(this Cntr& cntr);

    constexpr void Sanitize(this Cntr const& cntr,
                            mem_recorder::MemRecorder* dst_table,
                            mem_recorder::MemRecorder* dst_node);
};

}  // namespace zeta::core::dynamic_hash_table

namespace zeta::core {

template <CntrTplParamList()>
struct assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<
          dynamic_hash_table::Cntr<CntrTplArgList>,
          dynamic_hash_table::Cursor> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(dynamic_hash_table::Cntr<CntrTplArgList>&);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(dynamic_hash_table::Cntr<CntrTplArgList>&);
};

template <CntrTplParamList()>
struct assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<
          dynamic_hash_table::Cntr<CntrTplArgList> const,
          dynamic_hash_table::Cursor> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);
};

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
