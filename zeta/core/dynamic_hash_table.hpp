#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/basic_llist_node.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList                           \
    typename ElemHashLike_, typename ElemCompareLike_, \
        typename NodeAllocator_, typename TableNodeAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                                     \
    typename ElemHashLike, typename ElemCompareLike, typename NodeAllocator, \
        typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    ElemHashLike, ElemCompareLike, NodeAllocator, TableNodeAllocatorLike

namespace zeta::core::dynamic_hash_table {

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

using LListNode = basic_llist_node::Node<void*, value_wrapper::FalseType,
                                         value_wrapper::FalseType>;

struct Node;

template <typename ElemHashLike>
struct NodeHash;

template <typename ElemCompareLike>
struct NodeCompare;

struct Cursor;

template <CntrTplDeclParamList>
struct Cntr;

template <CntrTplDeclParamList>
struct AssocCntrView;

struct Node {
    ZETA_Core_DebugStructPadding;

    LListNode lln;

    ZETA_Core_DebugStructPadding;

    generic_hash_table::Node ghtn;

    ZETA_Core_DebugStructPadding;

    unsigned char data[] __attribute__((aligned(alignof(max_align_t))));

    void Init();
};

template <typename ElemHashLike>
struct NodeHash {
    ElemHashLike elem_hash;

    unsigned long long operator()(generic_hash_table::Node const* ghtn,
                                  unsigned long long salt) const;
};

template <typename ElemCompareLike>
struct NodeCompare {
    ElemCompareLike elem_compare;

    int operator()(generic_hash_table::Node const* ghtn_x,
                   generic_hash_table::Node const* ghtn_y) const;
};

struct Cursor {
    void const* cntr;
    LListNode* lln;
};

template <CntrTplDeclParamList>
struct Cntr {
    using ElemHashLike = ElemHashLike_;
    using ElemCompareLike = ElemCompareLike_;
    using NodeAllocator = NodeAllocator_;
    using TableNodeAllocatorLike = TableNodeAllocatorLike_;

    size_t width;

    LListNode* lln;

    NodeAllocator node_alctr;

    generic_hash_table::Cntr<NodeHash<ElemHashLike>,
                             NodeCompare<ElemCompareLike>,
                             TableNodeAllocatorLike>
        ght;
};

namespace ops {

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
    Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
    Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetCursorSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(Cntr<CntrTplArgList> const* cntr, void const* key,
           KeyHash const& key_hash, KeyElemCompare const& key_elem_compare,
           bool lazy_copy_elem, Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Insert(Cntr<CntrTplArgList>* cntr, void const* elem, Cursor* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* src_cursor,
                Cursor* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplParamList>
unsigned long long GetEffFactor(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const* cntr,
              mem_recorder::MemRecorder* dst_table,
              mem_recorder::MemRecorder* dst_node);

}  // namespace ops

}  // namespace zeta::core::dynamic_hash_table

namespace zeta::core {

template <CntrTplParamList>
struct assoc_cntr::Traits<dynamic_hash_table::Cntr<CntrTplArgList> const,
                          void> {
    static void* GetReferedInst(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static constexpr size_t GetCursorSize(
        dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetWidth(dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetSize(dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetCapacity(dynamic_hash_table::Cntr<CntrTplArgList> const*);

    static void GetLBCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr, void* dst_cursor);

    static void GetRBCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr, void* dst_cursor);

    static void* PeekL(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                      void const* key, KeyHash const& key_hash,
                      KeyElemCompare const& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void CopyCursor(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
        void const* cursor_a, void const* cursor_b);

    static void CursorStepL(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr, void* cursor);

    static void CursorStepR(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr, void* cursor);
};

template <CntrTplParamList>
struct assoc_cntr::Traits<dynamic_hash_table::Cntr<CntrTplArgList>, void>
    : public assoc_cntr::Traits<dynamic_hash_table::Cntr<CntrTplArgList> const,
                                void> {
    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static void* Insert(dynamic_hash_table::Cntr<CntrTplArgList>* cntr,
                        void const* elem, void* dst_cursor);

    static void PopL(dynamic_hash_table::Cntr<CntrTplArgList>* cntr,
                     size_t cnt);

    static void PopR(dynamic_hash_table::Cntr<CntrTplArgList>* cntr,
                     size_t cnt);

    static void Erase(dynamic_hash_table::Cntr<CntrTplArgList>* cntr,
                      void* pos_cursor);

    static void EraseAll(dynamic_hash_table::Cntr<CntrTplArgList>* cntr);
};

}  // namespace zeta::core

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
