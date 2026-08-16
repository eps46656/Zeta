#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/generic_hash_table.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                           \
    typename ElemHasherLike, typename ElemComparatorLike,          \
        typename SaltRandomEngineLike, typename NodeAllocatorLike, \
        typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList                                        \
    ElemHasherLike, ElemComparatorLike, SaltRandomEngineLike, \
        NodeAllocatorLike, TableNodeAllocatorLike

namespace zeta::core {

template <typename ElemHasherLike>
template <typename... Args>
constexpr void dynamic_hash_table::NodeHasherWrapper<ElemHasherLike>::Init(
    Args&&... args) {
    lifecycle::Init(this->elem_hasher, meta::Forward<Args>(args)...);
}

template <typename ElemHasherLike>
constexpr void dynamic_hash_table::NodeHasherWrapper<ElemHasherLike>::Deinit() {
    lifecycle::Deinit(this->elem_hasher);
}

template <typename ElemHasherLike>
constexpr unsigned long long
dynamic_hash_table::NodeHasherWrapper<ElemHasherLike>::Hash(
    generic_hash_table::Node const* ghtn, unsigned long long salt) const {
    return hash::Hash(meta::GetInstRef(this->elem_hasher),
                      ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data, salt);
}

template <typename ElemComparatorLike>
template <typename... Args>
constexpr void
dynamic_hash_table::NodeComparatorWrapper<ElemComparatorLike>::Init(
    Args&&... args) {
    lifecycle::Init(this->elem_cmptr, meta::Forward<Args>(args)...);
}

template <typename ElemComparatorLike>
constexpr void
dynamic_hash_table::NodeComparatorWrapper<ElemComparatorLike>::Deinit() {
    lifecycle::Deinit(this->elem_cmptr);
}

template <typename ElemComparatorLike>
template <comparison::IsOpType OpType>
constexpr auto
dynamic_hash_table::NodeComparatorWrapper<ElemComparatorLike>::Compare(
    OpType op, generic_hash_table::Node const* ghtn_a,
    generic_hash_table::Node const* ghtn_b) const {
    return comparison::Compare(
        meta::GetInstRef(this->elem_cmptr), op,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_a)->data,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_b)->data);
}

template <typename KeyNodeComparatorLike>
template <comparison::IsOpType OpType>
constexpr auto
dynamic_hash_table::KeyNodeComparatorWrapper<KeyNodeComparatorLike>::Compare(
    OpType op, void const* key_a,
    generic_hash_table::Node const* ghtn_b) const {
    return comparison::Compare(
        meta::GetInstRef(this->key_elem_cmptr), op, key_a,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_b)->data);
}

constexpr void dynamic_hash_table::Node::Init() {
    this->lln.Init();
    this->ghtn.Init();
}

namespace dynamic_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(0 < elem_size);

    ZETA_Core_DebugAssert(elem_size % alignof(Node) == 0);
}

template <CntrTplParamList>
void CheckCursor_(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(cursor->cntr == &cntr);

    if (cntr.lln != cursor->lln) {
        ZETA_Core_DebugAssert(cntr.ght.Contain(
            &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn));
    }
}

}  // namespace dynamic_hash_table::detail

template <CntrTplParamList>
template <typename ElemHasherLikeInitArg, typename ElemComparatorLikeInitArg,
          typename SaltRandomEngineLikeInitArg,
          typename NodeAllocatorLikeInitArg,
          typename TableNodeAllocatorLikeInitArg>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Init(
    this Cntr& cntr, size_t elem_size,
    generic_hash_table::RehashingConfig const& rehashing_config,
    ElemHasherLikeInitArg&& elem_hash_init_arg,
    ElemComparatorLikeInitArg&& elem_cmptr_init_arg,
    SaltRandomEngineLikeInitArg&& salt_random_engine_init_arg,
    NodeAllocatorLikeInitArg&& node_alctr_like_init_arg,
    TableNodeAllocatorLikeInitArg&& table_node_alctr_init_arg) {
    ZETA_Core_DebugAssert(0 < elem_size);

    cntr.elem_size = elem_size =
        integral_math::AlignUp(elem_size, alignof(Node));

    cntr.lln = static_cast<LListNode*>(allocator::SafeAllocate(
        cntr.node_alctr, alignof(LListNode), sizeof(LListNode)));

    cntr.lln->Init();

    cntr.ght.Init(
        rehashing_config,
        meta::Forward<ElemHasherLikeInitArg>(elem_hash_init_arg),
        meta::Forward<ElemComparatorLikeInitArg>(elem_cmptr_init_arg),
        meta::Forward<SaltRandomEngineLikeInitArg>(salt_random_engine_init_arg),
        meta::Forward<TableNodeAllocatorLikeInitArg>(
            table_node_alctr_init_arg));

    lifecycle::Init(cntr.node_alctr, meta::Forward<NodeAllocatorLikeInitArg>(
                                         node_alctr_like_init_arg));
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Deinit(
    this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    cntr.EraseAll();

    cntr.ght.Deinit();

    allocator::Deallocate(cntr.node_alctr, cntr.lln);
}

template <CntrTplParamList>
constexpr void* dynamic_hash_table::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);
    return const_cast<void*>(static_cast<void const*>(&cntr.ght));
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetCursorSize(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.ght.GetNodeCnt();
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::GetLBCursor(
    this Cntr const& cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->lln = cntr.lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->lln = cntr.lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PeekL(
    this auto& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = &cntr,
        .lln = llist::GetR(cntr.lln),
    };

    void* elem{ cntr.lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PeekR(
    this auto& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = &cntr,
        .lln = llist::GetL(cntr.lln),
    };

    void* elem{ cntr.lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Derefer(
    this auto& cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    void* elem{
        cntr.lln == pos_cursor->lln
            ? nullptr
            : ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data
    };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& cntr, void const* elem, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    cntr.Find(elem, cntr.ght.hasher.elem_hasher, cntr.ght.cmptr.elem_cmptr,
              lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    KeyNodeComparatorWrapper<KeyElemComparator const&> key_node_cmptr{
        .key_elem_cmptr = key_elem_cmptr,
    };

    void* ghtn{ cntr.ght.Find(key, key_hasher, key_node_cmptr) };

    if (ghtn == nullptr) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->lln = cntr.lln;
        }

        return;
    }

    Node* node{ ZETA_Core_MemberToStruct(Node, ghtn, ghtn) };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = node->data;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->lln = &node->lln;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, node->data, cntr.elem_size);
    }
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Writer&& writer,
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    Node* node{ static_cast<Node*>(allocator::SafeAllocate(
        cntr.node_alctr, alignof(Node),
        __builtin_offsetof(Node, data[cntr.elem_size]))) };

    node->Init();

    elem_stream::provider::Transfer(writer, node->data, cntr.elem_size,
                                    cntr.elem_size, 1);

    cntr.ght.Insert(key, key_hasher, key_elem_cmptr, &node->ghtn);

    llist::InsertL(cntr.lln, &node->lln);

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->lln = &node->lln;
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PopL(this Cntr& cntr,
                                                              size_t cnt,
                                                              Reader&& reader) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= cntr.GetElemCnt());

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetR(cntr.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        cntr.ght.Extract(&node->ghtn);

        elem_stream::acceptor::Transfer(reader, node->data, cntr.elem_size,
                                        cntr.elem_size, 1);

        allocator::Deallocate(cntr.node_alctr, node);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PopR(this Cntr& cntr,
                                                              size_t cnt,
                                                              Reader&& reader) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= cntr.GetElemCnt());

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetL(cntr.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        cntr.ght.Extract(&node->ghtn);

        elem_stream::acceptor::Transfer(reader, node->data, cntr.elem_size,
                                        cntr.elem_size, 1);

        allocator::Deallocate(cntr.node_alctr, node);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Erase(
    this Cntr& cntr, Cursor* pos_cursor, size_t cnt, Reader&& reader) {
    detail::CheckCursor_(cntr, pos_cursor);

    for (; 0 < cnt; --cnt) {
        ZETA_Core_DebugAssert(cntr.lln != pos_cursor->lln);

        LListNode* lln{ pos_cursor->lln };

        pos_cursor->lln = llist::GetR(lln);

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(&node->lln);

        cntr.ght.Extract(&node->ghtn);

        elem_stream::acceptor::Transfer(reader, node->data, cntr.elem_size,
                                        cntr.elem_size, 1);

        allocator::Deallocate(cntr.node_alctr, node);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::EraseAll(
    this Cntr& cntr) {
    for (;;) {
        LListNode* nxt_lln{ llist::GetR(cntr.lln) };

        if (nxt_lln == cntr.lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(nxt_lln);

        cntr.ght.Extract(&nxt_node->ghtn);

        allocator::Deallocate(cntr.node_alctr, nxt_node);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& cntr, Cursor* src_cursor, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool dynamic_hash_table::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& cntr, Cursor const* cursor_a, Cursor const* cursor_b) {
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->lln == cursor_b->lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& cntr, Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetL(cursor->lln);
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& cntr, Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetR(cursor->lln);
}

template <CntrTplParamList>
constexpr auto dynamic_hash_table::Cntr<CntrTplArgList>::GetEffFactor(
    this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.ght.GetEffFactor(cntr.ght);
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Sanitize(
    this Cntr const& cntr, mem_recorder::MemRecorder* dst_table,
    mem_recorder::MemRecorder* dst_node) {
    detail::CheckCntr_(cntr);

    mem_recorder::MemRecorder htn_records;

    cntr.ght.Sanitize(dst_table, &htn_records);

    if (dst_node != nullptr) {
        mem_recorder::Record(*dst_node, cntr.lln, sizeof(LListNode));
    }

    size_t node_size{ __builtin_offsetof(Node, data[cntr.elem_size]) };

    for (LListNode* lln{ cntr.lln };;) {
        lln = llist::GetR(lln);

        if (lln == cntr.lln) { break; }

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        if (dst_node != nullptr) {
            mem_recorder::Record(*dst_node, node, node_size);
        }

        ZETA_Core_DebugAssert(mem_recorder::Unrecord(htn_records, &node->ghtn));
    }

    ZETA_Core_DebugAssert(mem_recorder::GetSize(htn_records) == 0);
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
    GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::FlagBuilder{
        .GetCursorSize = true,
        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,
        .GetLBCursor = true,
        .GetRBCursor = true,
        .PeekL = true,
        .PeekR = true,
        .Derefer = true,
        .Find = true,
        .Insert = true,
        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,
        .CopyCursor = true,
        .AreEqualCursor = true,
        .CompareCursor = false,
        .GetCursorDist = false,
        .GetCursorIdx = false,
        .CursorStepL = true,
        .CursorStepR = true,
        .CursorAdvanceL = false,
        .CursorAdvanceR = false,
    }();
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
    GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::FlagBuilder{
        .GetCursorSize = false,
        .GetElemSize = false,
        .GetElemCnt = false,
        .GetMaxElemCnt = false,
        .GetLBCursor = false,
        .GetRBCursor = false,
        .PeekL = false,
        .PeekR = false,
        .Derefer = false,
        .Find = false,
        .Insert = false,
        .PopL = false,
        .PopR = false,
        .Erase = false,
        .EraseAll = false,
        .CopyCursor = false,
        .AreEqualCursor = false,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,
        .CursorStepL = false,
        .CursorStepR = false,
        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
    GetDynamicEnabledCapabilityFlag(dynamic_hash_table::Cntr<CntrTplArgList>&) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
    GetDynamicDisabledCapabilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList>&) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
               GetStaticEnabledCapabilityFlag() &
           assoc_cntr::capability::const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
               GetStaticDisabledCapabilityFlag() |
           assoc_cntr::capability::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledCapabilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledCapabilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::capability::empty_capability_flag;
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
