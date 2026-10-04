#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/lifecycle.hpp>

ZETA_Core_ClangdPreambleBarrier;

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

static_assert(alignof(void*) % 4 == 0);

using LListNode = basic_llist_node::Node<void*, meta::AutoValueWrapper<false>,
                                         meta::AutoValueWrapper<false>>;

struct Node {
    LListNode lln;

    generic_hash_table::Node ghtn;

    unsigned char data[] __attribute__((aligned(max_align)));

    constexpr void Construct();
};

template <typename ElemHasherLike>
struct NodeHasherWrapper {
    ElemHasherLike elem_hasher;

    template <typename... Args>
    constexpr void Construct(Args&&... args);

    constexpr void Destruct();

    constexpr unsigned long long Hash(generic_hash_table::Node const* ghtn,
                                      unsigned long long salt) const;
};

template <typename ElemComparatorLike>
struct NodeComparatorWrapper {
    ElemComparatorLike elem_cmptr;

    constexpr ~NodeComparatorWrapper();

    template <comparison::IsOpTag OpTag>
    constexpr auto Compare(OpTag, generic_hash_table::Node const* ghtn_a,
                           generic_hash_table::Node const* ghtn_b) const;
};

template <typename KeyElemComparatorLike>
struct KeyNodeComparatorWrapper {
    KeyElemComparatorLike key_elem_cmptr;

    template <comparison::IsOpTag OpTag>
    constexpr auto Compare(OpTag, void const* key_a,
                           generic_hash_table::Node const* ghtn_b) const;
};

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

    generic_hash_table::Cntr<NodeHasherWrapper<HasherLike>,
                             NodeComparatorWrapper<ComparatorLike>,
                             SaltRandomEngineLike, TableNodeAllocatorLike>
        ght;

    LListNode* lln;

    NodeAllocatorLike node_alctr_like;

    template <typename ElemHasherLikeConstructArg,
              typename ElemComparatorLikeConstructArg,
              typename SaltRandomEngineLikeConstructArg,
              typename TableNodeAllocatorLikeConstructArg,
              typename NodeAllocatorLikeConstructArg>
    constexpr Cntr(
        size_t elem_size,
        generic_hash_table::RehashingConfig const& rehashing_config,
        ElemHasherLikeConstructArg&& elem_hasher_construct_arg,
        ElemComparatorLikeConstructArg&& elem_cmptr_construct_arg,
        SaltRandomEngineLikeConstructArg&& salt_random_engine_construct_arg,
        TableNodeAllocatorLikeConstructArg&& table_node_alctr_construct_arg,
        NodeAllocatorLikeConstructArg&& node_alctr_construct_arg);

    constexpr ~Cntr();

    static constexpr assoc_cntr::capability::Flag
        GetStaticEnabledCapabilityFlag(assoc_cntr::Tag,
                                       meta::TypeWrapper<Cntr>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticEnabledCapabilityFlag(assoc_cntr::Tag,
                                       meta::TypeWrapper<Cntr const>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticDisabledCapabilityFlag(assoc_cntr::Tag,
                                        meta::TypeWrapper<Cntr>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticDisabledCapabilityFlag(assoc_cntr::Tag,
                                        meta::TypeWrapper<Cntr const>);

    static constexpr assoc_cntr::capability::Flag
        GetDynamicEnabledCapabilityFlag(assoc_cntr::Tag);

    static constexpr assoc_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(assoc_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, assoc_cntr::Tag);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        assoc_cntr::Tag, meta::TypeWrapper<Cntr>);

    constexpr size_t GetCursorSize(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, assoc_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& self, assoc_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    constexpr void Find(this auto& self, assoc_cntr::Tag, void const* elem,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this auto& self, assoc_cntr::Tag, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <assoc_cntr::IsWriter Provider>
    constexpr void Insert(this Cntr& self, assoc_cntr::Tag, void const* elem,
                          Provider&& writer, Cursor* dst_cursor);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Provider>
    constexpr void Insert(this Cntr& self, assoc_cntr::Tag, void const* key,
                          KeyHasher const& key_hasher,
                          KeyElemComparator const& key_elem_cmptr,
                          Provider&& writer, Cursor* dst_cursor);

    template <assoc_cntr::IsReader acceptor>
    constexpr void PopL(this Cntr& self, assoc_cntr::Tag, size_t cnt,
                        acceptor&& reader);

    template <assoc_cntr::IsReader acceptor>
    constexpr void PopR(this Cntr& self, assoc_cntr::Tag, size_t cnt,
                        acceptor&& reader);

    template <assoc_cntr::IsReader acceptor>
    constexpr void Erase(this Cntr& self, assoc_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, acceptor&& reader);

    constexpr void EraseAll(this Cntr& self, assoc_cntr::Tag);

    constexpr void CopyCursor(this Cntr const& self, assoc_cntr::Tag,
                              Cursor* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, assoc_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr void CursorStepL(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* cursor);

    constexpr auto GetEffFactor(this Cntr& self, assoc_cntr::Tag);

    static constexpr void SanityCheck(
        void const* self, debug_utils::sanity::SanityCheckScope scope);
};

}  // namespace zeta::core::dynamic_hash_table

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
