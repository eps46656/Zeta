#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/llist_node_tpl.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDeclParamList")
#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")

#define CntrTplDeclParamList                                                 \
    typename ElemHashLike, typename ElemCompareLike, typename NodeAllocator, \
        typename TableNodeAllocatorLike

#define CntrTplParamList                                                     \
    typename ElemHashLike, typename ElemCompareLike, typename NodeAllocator, \
        typename TableNodeAllocatorLike

#define CntrTplArgList \
    ElemHashLike, ElemCompareLike, NodeAllocator, TableNodeAllocatorLike

namespace zeta::core::dynamic_hash_table {

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

using LListNode =
    LListNodeTpl<void*, value_wrapper::FalseType, value_wrapper::FalseType>;

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

// -----------------------------------------------------------------------------

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
void* PeekL(Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

template <CntrTplParamList>
void const* PeekL(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                  Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

template <CntrTplParamList>
void const* PeekR(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                  Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList>* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList>
void const* Derefer(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
                    bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(Cntr<CntrTplArgList>* cntr, void const* key, KeyHash const& key_hash,
           KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
           Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void const* Find(Cntr<CntrTplArgList> const* cntr, void const* key,
                 KeyHash const& key_hash,
                 KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
                 Cursor* dst_cursor, void* dst_elem);

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
void Sanitize(Cntr<CntrTplArgList>* cntr, MemRecorder* dst_table,
              MemRecorder* dst_node);

template <CntrTplParamList>
AssocCntrView<CntrTplArgList>* AsAssocCntrView(Cntr<CntrTplArgList>* cntr);

}  // namespace ops

template <CntrTplDeclParamList>
struct AssocCntrView {
    static constexpr bool IsConst(type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        AssocCntrView const*);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        AssocCntrView const*);

    static constexpr size_t GetCursorSize(AssocCntrView const* cntr);

    static size_t GetWidth(AssocCntrView const* cntr);

    static size_t GetSize(AssocCntrView const* cntr);

    static size_t GetCapacity(AssocCntrView const* cntr);

    static void GetLBCursor(AssocCntrView const* cntr, void* dst_cursor);

    static void GetRBCursor(AssocCntrView const* cntr, void* dst_cursor);

    static Conditional<false, void*, void*> PeekL(AssocCntrView* cntr,
                                                  bool lazy_copy_elem,
                                                  void* dst_cursor,
                                                  void* dst_elem);

    static void const* PeekL(AssocCntrView const* cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static Conditional<false, void*, void*> PeekR(AssocCntrView* cntr,
                                                  bool lazy_copy_elem,
                                                  void* dst_cursor,
                                                  void* dst_elem);

    static void const* PeekR(AssocCntrView const* cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static void* Derefer(AssocCntrView* cntr, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    static void const* Derefer(AssocCntrView const* cntr,
                               void const* pos_cursor, bool lazy_copy_elem,
                               void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(AssocCntrView* cntr, void const* key,
                      KeyHash const& key_hash,
                      KeyElemCompare const& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void const* Find(AssocCntrView const* cntr, void const* key,
                            KeyHash const& key_hash,
                            KeyElemCompare const& key_elem_compare,
                            bool lazy_copy_elem, void* dst_cursor,
                            void* dst_elem);

    static void* Insert(AssocCntrView* cntr, void const* elem,
                        void* dst_cursor);

    static void PopL(AssocCntrView* cntr, size_t cnt);

    static void PopR(AssocCntrView* cntr, size_t cnt);

    static void Erase(AssocCntrView* cntr, void* pos_cursor);

    static void EraseAll(AssocCntrView* cntr);

    static void CopyCursor(AssocCntrView const* cntr, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(AssocCntrView const* cntr, void const* cursor_a,
                               void const* cursor_b);

    static void CursorStepL(AssocCntrView const* cntr, void* cursor);

    static void CursorStepR(AssocCntrView const* cntr, void* cursor);
};

}  // namespace zeta::core::dynamic_hash_table

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
