#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/generic_hash_table.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/llist.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

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
constexpr unsigned long long
dynamic_hash_table::NodeHasherWrapper<ElemHasherLike>::Hash(
    generic_hash_table::Node const* ghtn, unsigned long long salt) const {
    return hash::Hash(meta::GetInstRef(this->elem_hasher),
                      ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data, salt);
}

template <typename ElemComparatorLike>
template <comparison::IsOpTag OpTag>
constexpr auto
dynamic_hash_table::NodeComparatorWrapper<ElemComparatorLike>::Compare(
    OpTag op, generic_hash_table::Node const* ghtn_a,
    generic_hash_table::Node const* ghtn_b) const {
    return comparison::Compare(
        meta::GetInstRef(this->elem_cmptr), op,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_a)->data,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_b)->data);
}

template <typename KeyNodeComparatorLike>
template <comparison::IsOpTag OpTag>
constexpr auto
dynamic_hash_table::KeyNodeComparatorWrapper<KeyNodeComparatorLike>::Compare(
    OpTag op, void const* key_a, generic_hash_table::Node const* ghtn_b) const {
    return comparison::Compare(
        meta::GetInstRef(this->key_elem_cmptr), op, key_a,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_b)->data);
}

constexpr void dynamic_hash_table::Node::Construct() {
    this->lln.Construct();
    this->ghtn.Construct();
}

namespace dynamic_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_size);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_size % alignof(Node) == 0);
}

template <CntrTplParamList>
void CheckCursor_(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->cntr == &cntr);

    if (cntr.lln != cursor->lln) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr.ght.Contain(
            &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn));
    }
}

}  // namespace dynamic_hash_table::detail

template <CntrTplParamList>
template <typename ElemHasherLikeConstructArg,
          typename ElemComparatorLikeConstructArg,
          typename SaltRandomEngineLikeConstructArg,
          typename TableNodeAllocatorLikeConstructArg,
          typename NodeAllocatorLikeConstructArg>
constexpr dynamic_hash_table::Cntr<CntrTplArgList>::Cntr(
    size_t elem_size,
    generic_hash_table::RehashingConfig const& rehashing_config,
    ElemHasherLikeConstructArg&& elem_hasher_construct_arg,
    ElemComparatorLikeConstructArg&& elem_cmptr_construct_arg,
    SaltRandomEngineLikeConstructArg&& salt_random_engine_construct_arg,
    TableNodeAllocatorLikeConstructArg&& table_node_alctr_construct_arg,
    NodeAllocatorLikeConstructArg&& node_alctr_construct_arg)
    : ght{
          rehashing_config,
          meta::Forward<ElemHasherLikeConstructArg>(elem_hasher_construct_arg),
          meta::Forward<ElemComparatorLikeConstructArg>(
              elem_cmptr_construct_arg),
          meta::Forward<SaltRandomEngineLikeConstructArg>(
              salt_random_engine_construct_arg),
          meta::Forward<TableNodeAllocatorLikeConstructArg>(
              table_node_alctr_construct_arg),
      },
      node_alctr_like{ meta::Forward<NodeAllocatorLikeConstructArg>(
          node_alctr_construct_arg) } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_size);

    this->elem_size = elem_size =
        integral_math::AlignUp(elem_size, alignof(Node));

    this->lln = static_cast<LListNode*>(allocator::SafeAllocate(
        this->node_alctr_like, alignof(LListNode), sizeof(LListNode)));

    this->lln->Construct();
}

template <CntrTplParamList>
constexpr dynamic_hash_table::Cntr<CntrTplArgList>::~Cntr() {
    detail::CheckCntr_(*this);

    this->EraseAll();

    lifecycle::InvokeDestructor(this->ght);

    allocator::Deallocate(this->node_alctr_like, this->lln);
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr>) {
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
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(assoc_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           assoc_cntr::capability::const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return assoc_cntr::capability::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetDynamicEnabledCapabilityFlag(
    assoc_cntr::Tag) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
dynamic_hash_table::Cntr<CntrTplArgList>::GetDynamicDisabledCapabilityFlag(
    assoc_cntr::Tag) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr void* dynamic_hash_table::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);
    return const_cast<void*>(static_cast<void const*>(&self.ght));
}

template <CntrTplParamList>
constexpr meta::TypeWrapper<typename dynamic_hash_table::Cursor>
dynamic_hash_table::Cntr<CntrTplArgList>::GetCursorType(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetCursorSize(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return sizeof(Cursor);
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_size;
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.ght.GetNodeCnt();
}

template <CntrTplParamList>
constexpr size_t dynamic_hash_table::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::GetLBCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->lln = self.lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->lln = self.lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PeekL(
    this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    Cursor cursor{
        .cntr = &self,
        .lln = llist::GetR(self.lln),
    };

    void* elem{ self.lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PeekR(
    this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    Cursor cursor{
        .cntr = &self,
        .lln = llist::GetL(self.lln),
    };

    void* elem{ self.lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Derefer(
    this auto& self, assoc_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, assoc_cntr::ElemPtrView* dst_elem_ptr_view,
    void* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    void* elem{
        self.lln == pos_cursor->lln
            ? nullptr
            : ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data
    };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& self, assoc_cntr::Tag, void const* elem, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    self.Find(elem, self.ght.hasher.elem_hasher, self.ght.cmptr.elem_cmptr,
              lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& self, assoc_cntr::Tag, void const* key,
    KeyHasher const& key_hasher, KeyElemComparator const& key_elem_cmptr,
    bool lazy_copy_elem, assoc_cntr::ElemPtrView* dst_elem_ptr_view,
    Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(self);

    KeyNodeComparatorWrapper<KeyElemComparator const&> key_node_cmptr{
        .key_elem_cmptr = key_elem_cmptr,
    };

    void* ghtn{ self.ght.Find(key, key_hasher, key_node_cmptr) };

    if (ghtn == nullptr) {
        if (dst_elem_ptr_view != nullptr) {
            dst_elem_ptr_view->ptr = nullptr;
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::Null;
        }

        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &self;
            dst_cursor->lln = self.lln;
        }

        return;
    }

    Node* node{ ZETA_Core_MemberToStruct(Node, ghtn, ghtn) };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = node->data;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->lln = &node->lln;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, node->data, self.elem_size);
    }
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Provider>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& self, assoc_cntr::Tag, void const* key,
    KeyHasher const& key_hasher, KeyElemComparator const& key_elem_cmptr,
    Provider&& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    Node* node{ static_cast<Node*>(allocator::SafeAllocate(
        node_alctr, alignof(Node),
        __builtin_offsetof(Node, data[self.elem_size]))) };

    node->Construct();

    seq_endpoint::provider::Transfer(writer, node->data, self.elem_size,
                                     self.elem_size, 1);

    self.ght.Insert(key, key_hasher, key_elem_cmptr, &node->ghtn);

    llist::InsertL(self.lln, &node->lln);

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->lln = &node->lln;
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PopL(
    this Cntr& self, assoc_cntr::Tag, size_t cnt, acceptor&& reader) {
    detail::CheckCntr_(self);

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= self.GetElemCnt());

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetR(self.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        self.ght.Extract(&node->ghtn);

        seq_endpoint::acceptor::Transfer(reader, node->data, self.elem_size,
                                         self.elem_size, 1);

        allocator::Deallocate(node_alctr, node);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::PopR(
    this Cntr& self, assoc_cntr::Tag, size_t cnt, acceptor&& reader) {
    detail::CheckCntr_(self);

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= self.GetElemCnt());

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetL(self.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        self.ght.Extract(&node->ghtn);

        seq_endpoint::acceptor::Transfer(reader, node->data, self.elem_size,
                                         self.elem_size, 1);

        allocator::Deallocate(node_alctr, node);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::Erase(
    this Cntr& self, assoc_cntr::Tag, Cursor* pos_cursor, size_t cnt,
    acceptor&& reader) {
    detail::CheckCursor_(self, pos_cursor);

    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    for (; 0 < cnt; --cnt) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.lln != pos_cursor->lln);

        LListNode* lln{ pos_cursor->lln };

        pos_cursor->lln = llist::GetR(lln);

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(&node->lln);

        self.ght.Extract(&node->ghtn);

        seq_endpoint::acceptor::Transfer(reader, node->data, self.elem_size,
                                         self.elem_size, 1);

        allocator::Deallocate(node_alctr, node);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::EraseAll(
    this Cntr& self, assoc_cntr::Tag) {
    auto& node_alctr{ meta::GetInstRef(self.node_alctr_like) };

    for (;;) {
        LListNode* nxt_lln{ llist::GetR(self.lln) };

        if (nxt_lln == self.lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(nxt_lln);

        self.ght.Extract(&nxt_node->ghtn);

        allocator::Deallocate(node_alctr, nxt_node);
    }
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor* src_cursor,
    Cursor* dst_cursor) {
    detail::CheckCursor_(self, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool dynamic_hash_table::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    detail::CheckCursor_(self, cursor_a);
    detail::CheckCursor_(self, cursor_b);

    return cursor_a->lln == cursor_b->lln;
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& self, assoc_cntr::Tag, Cursor* cursor) {
    detail::CheckCursor_(self, cursor);

    cursor->lln = llist::GetL(cursor->lln);
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& self, assoc_cntr::Tag, Cursor* cursor) {
    detail::CheckCursor_(self, cursor);

    cursor->lln = llist::GetR(cursor->lln);
}

template <CntrTplParamList>
constexpr auto dynamic_hash_table::Cntr<CntrTplArgList>::GetEffFactor(
    this Cntr& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.ght.GetEffFactor(self.ght);
}

template <CntrTplParamList>
constexpr void dynamic_hash_table::Cntr<CntrTplArgList>::SanityCheck(
    void const* self_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_ != nullptr);

    auto& self{ *static_cast<Cntr<CntrTplArgList> const*>(self_) };

    detail::CheckCntr_(self);

    mem_recorder::MemRecorder htn_records;

    self.ght.Sanitize(dst_table, &htn_records);

    if (dst_node != nullptr) { dst_node->Record(cntr.lln, sizeof(LListNode)); }

    size_t node_size{ __builtin_offsetof(Node, data[self.elem_size]) };

    for (LListNode* lln{ self.lln };;) {
        lln = llist::GetR(lln);

        if (lln == self.lln) { break; }

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        if (dst_node != nullptr) { dst_node->Record(node, node_size); }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            htn_records.Unrecord(&node->ghtn));
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        mem_recorder::GetSize(htn_records) == 0);
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
