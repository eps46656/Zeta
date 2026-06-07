#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList                                         \
    typename ElemHasherLike_, typename ElemComparatorLike_,          \
        typename SaltRandomEngineLike_, typename NodeAllocatorLike_, \
        typename TableNodeAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                           \
    typename ElemHasherLike, typename ElemComparatorLike,          \
        typename SaltRandomEngineLike, typename NodeAllocatorLike, \
        typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList                                        \
    ElemHasherLike, ElemComparatorLike, SaltRandomEngineLike, \
        NodeAllocatorLike, TableNodeAllocatorLike

namespace zeta::core::dynamic_hash_table {

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

using LListNode = basic_llist_node::Node<void*, value_wrapper::FalseType,
                                         value_wrapper::FalseType>;

struct Node {
    LListNode lln;

    generic_hash_table::Node ghtn;

    unsigned char data[] __attribute__((aligned(max_align)));

    void Init();
};

template <typename ElemHasherLike>
struct NodeHasher {
    ElemHasherLike elem_hasher;

    unsigned long long operator()(generic_hash_table::Node const* ghtn,
                                  unsigned long long salt) const;
};

template <typename ElemComparatorLike>
struct NodeComparator {
    ElemComparatorLike elem_cmptr;

    int operator()(generic_hash_table::Node const* ghtn_a,
                   generic_hash_table::Node const* ghtn_b) const;
};

struct Cursor {
    void const* cntr;
    LListNode* lln;
};

template <CntrTplDeclParamList>
struct Cntr {
    using ElemHasherLike = ElemHasherLike_;
    using ElemComparatorLike = ElemComparatorLike_;
    using SaltRandomEngineLike = SaltRandomEngineLike_;
    using NodeAllocatorLike = NodeAllocatorLike_;
    using TableNodeAllocatorLike = TableNodeAllocatorLike_;

    size_t elem_size;

    generic_hash_table::Cntr<NodeHasher<ElemHasherLike>,
                             NodeComparator<ElemComparatorLike>,
                             SaltRandomEngineLike, TableNodeAllocatorLike>
        ght;

    LListNode* lln;

    NodeAllocatorLike node_alctr;
};

template <CntrTplParamList, typename ElemHasherLikeInitArg,
          typename ElemComparatorInitArg, typename SaltRandomEngineInitArg,
          typename NodeAllocatorInitArg, typename TableNodeAllocatorInitArg>
void Init(Cntr<CntrTplArgList>& cntr, size_t elem_size,
          generic_hash_table::RehashingConfig const& rehashing_config,
          ElemHasherLikeInitArg&& elem_hasher_init_arg,
          ElemComparatorInitArg&& elem_cmptr_init_arg,
          SaltRandomEngineInitArg&& salt_random_engine_init_arg,
          NodeAllocatorInitArg&& node_alctr_init_arg,
          TableNodeAllocatorInitArg&& table_node_alctr_init_arg);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
    Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
    Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetCursorSize(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetElemSize(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename KeyHasher, typename KeyElemComparator>
void* Find(Cntr<CntrTplArgList> const& cntr, void const* key,
           KeyHasher const& key_hasher, KeyElemComparator const& key_elem_cmptr,
           bool lazy_copy_elem, Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Insert(Cntr<CntrTplArgList>& cntr, void const* elem, Cursor* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* src_cursor,
                Cursor* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
auto GetEffFactor(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const& cntr,
              mem_recorder::MemRecorder* dst_table,
              mem_recorder::MemRecorder* dst_node);

}  // namespace zeta::core::dynamic_hash_table

namespace zeta::core {

template <typename ElemHasherLike>
struct lifecycle::Traits<dynamic_hash_table::NodeHasher<ElemHasherLike>> {
    template <typename... Args>
    static void Init(
        dynamic_hash_table::NodeHasher<ElemHasherLike>& node_hasher,
        Args&&... args);

    static void Deinit(
        dynamic_hash_table::NodeHasher<ElemHasherLike>& node_hasher);
};

template <typename ElemComparatorLike>
struct lifecycle::Traits<
    dynamic_hash_table::NodeComparator<ElemComparatorLike>> {
    template <typename... Args>
    static void Init(
        dynamic_hash_table::NodeComparator<ElemComparatorLike>& node_cmptr,
        Args&&... args);

    static void Deinit(
        dynamic_hash_table::NodeComparator<ElemComparatorLike>& node_cmptr);
};

template <CntrTplParamList>
struct lifecycle::Traits<dynamic_hash_table::Cntr<CntrTplArgList>> {
    template <typename... Args>
    static void Init(dynamic_hash_table::Cntr<CntrTplArgList>& cntr,
                     Args&&... args);

    static void Deinit(dynamic_hash_table::Cntr<CntrTplArgList>& cntr);
};

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const> {
    static void* GetReferedInstPtr(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static constexpr size_t GetCursorSize(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static size_t GetElemSize(dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static size_t GetElemCnt(dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static size_t GetMaxElemCnt(
        dynamic_hash_table::Cntr<CntrTplArgList> const&);

    static void GetLBCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr, void* dst_cursor);

    static void GetRBCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr, void* dst_cursor);

    static void* PeekL(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename KeyHasher, typename KeyElemComparator>
    static void* Find(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                      void const* key, KeyHasher const& key_hasher,
                      KeyElemComparator const& key_elem_cmptr,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void CopyCursor(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
        void const* cursor_a, void const* cursor_b);

    static void CursorStepL(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr, void* cursor);

    static void CursorStepR(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr, void* cursor);
};

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>
    : public assoc_cntr::CntrTraits<
          dynamic_hash_table::Cntr<CntrTplArgList> const> {
    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static void* Insert(dynamic_hash_table::Cntr<CntrTplArgList>& cntr,
                        void const* elem, void* dst_cursor);

    static void PopL(dynamic_hash_table::Cntr<CntrTplArgList>& cntr,
                     size_t cnt);

    static void PopR(dynamic_hash_table::Cntr<CntrTplArgList>& cntr,
                     size_t cnt);

    static void Erase(dynamic_hash_table::Cntr<CntrTplArgList>& cntr,
                      void* pos_cursor);

    static void EraseAll(dynamic_hash_table::Cntr<CntrTplArgList>& cntr);
};

}  // namespace zeta::core

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
