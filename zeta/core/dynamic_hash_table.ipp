#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/generic_hash_table.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/llist_node_tpl.ipp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
unsigned long long DynamicHashTable<
    ElemHash, ElemCompare, NodeAllocator,
    TableNodeAllocator>::NodeHash::operator()(GenericHashTableNode const* ghtn,
                                              unsigned long long salt) const {
    return this->elem_hash(ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data,
                           salt);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
int DynamicHashTable<ElemHash, ElemCompare, NodeAllocator, TableNodeAllocator>::
    NodeCompare::operator()(GenericHashTableNode const* ghtn_x,
                            GenericHashTableNode const* ghtn_y) const {
    return this->elem_compare(
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_x)->data,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_y)->data);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::Init(void* dht_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(dht != nullptr);

    ZETA_Core_DebugAssert(0 < dht->width);

    dht->width = UIntAlignUp(dht->width, alignof(Node));

    NodeAllocator::CheckAllocator(&dht->node_allocator);

    dht->lln = static_cast<LListNode*>(allocator::SafeAllocate(
        &dht->node_allocator, alignof(LListNode), sizeof(LListNode)));

    dht->lln->Init();

    GenericHashTableImpl::Init(&dht->ght);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::Deinit(void* dht_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    DynamicHashTable::EraseAll(dht);

    GenericHashTableImpl::Deinit(&dht->ght);

    NodeAllocator::Deallocate(&dht->node_allocator, dht->lln);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
assoc_cntr::AssocCntrAbilityFlagType DynamicHashTable<
    ElemHash, ElemCompare, NodeAllocator,
    TableNodeAllocator>::GetDynamicEnabledAbilityFlag(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));
    return dynamic_ability_flag;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
assoc_cntr::AssocCntrAbilityFlagType DynamicHashTable<
    ElemHash, ElemCompare, NodeAllocator,
    TableNodeAllocator>::GetDynamicDisabledAbilityFlag(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));
    return dynamic_ability_flag;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
size_t DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                        TableNodeAllocator>::GetCursorSize(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    return sizeof(Cursor);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
size_t DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                        TableNodeAllocator>::GetWidth(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    return dht->width;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
size_t DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                        TableNodeAllocator>::GetSize(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    return GenericHashTableImpl::GetSize(&dht->ght);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
size_t DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                        TableNodeAllocator>::GetCapacity(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    return ZETA_Core_max_capacity;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::GetLBCursor(void const* dht_,
                                                       void* dst_cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->dht = dht;
    dst_cursor->lln = dht->lln;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::GetRBCursor(void const* dht_,
                                                       void* dst_cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    dst_cursor->dht = dht;
    dst_cursor->lln = dht->lln;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::PeekL(void* dht_, void* dst_cursor_,
                                                  void* dst_elem) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    Cursor cursor{
        .dht = dht,
        .lln = dht->lln->GetRPtr(),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->dht = cursor.dht;
        dst_cursor->lln = cursor.lln;
    }

    void* elem{ dht->lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem != nullptr && elem != nullptr) {
        MemCopy(dst_elem, elem, dht->width);
    }

    return elem;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void const* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                             TableNodeAllocator>::ConstPeekL(void const* dht,
                                                             void* dst_cursor,
                                                             void* dst_elem) {
    return PeekL(const_cast<void*>(dht), dst_cursor, dst_elem);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::PeekR(void* dht_, void* dst_cursor_,
                                                  void* dst_elem) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    Cursor cursor{
        .dht = dht,
        .lln = dht->lln->GetLPtr(),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->dht = cursor.dht;
        dst_cursor->lln = cursor.lln;
    }

    void* elem{ dht->lln == cursor.lln
                    ? nullptr
                    : ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (dst_elem != nullptr && elem != nullptr) {
        MemCopy(dst_elem, elem, dht->width);
    }

    return elem;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void const* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                             TableNodeAllocator>::ConstPeekR(void const* dht,
                                                             void* dst_cursor,
                                                             void* dst_elem) {
    return PeekR(const_cast<void*>(dht), dst_cursor, dst_elem);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::Refer(void* dht_,
                                                  void const* pos_cursor_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    auto pos_cursor{ static_cast<Cursor const*>(pos_cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(dht, pos_cursor));

    return dht->lln == pos_cursor->lln
               ? nullptr
               : ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void const*
DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                 TableNodeAllocator>::ConstRefer(void const* dht,
                                                 void const* pos_cursor) {
    return Refer(const_cast<void*>(dht), pos_cursor);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
template <typename KeyHash, typename KeyElemCompare>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::TplFind(void* dht_, void const* key,
                                                    KeyHash const& key_hash,
                                                    KeyElemCompare const&
                                                        key_elem_compare,
                                                    void* dst_cursor_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    void* ghtn{ GenericHashTableImpl::Find(
        &dht->ght, key, key_hash, [&](void const* key, void const* ghtn) {
            return key_elem_compare(
                key, ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data);
        }) };

    if (ghtn == nullptr) {
        if (dst_cursor != nullptr) {
            dst_cursor->dht = dht;
            dst_cursor->lln = dht->lln;
        }

        return nullptr;
    }

    Node* node{ ZETA_Core_MemberToStruct(Node, ghtn, ghtn) };

    if (dst_cursor != nullptr) {
        dst_cursor->dht = dht;
        dst_cursor->lln = &node->lln;
    }

    return node->data;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
template <typename Key, typename KeyHash, typename KeyElemCompare>
void const* DynamicHashTable<
    ElemHash, ElemCompare, NodeAllocator,
    TableNodeAllocator>::ConstTplFind(void const* dht, Key const& key,
                                      KeyHash const& key_hash,
                                      KeyElemCompare const& key_elem_compare,
                                      void* dst_cursor) {
    return TplFind(const_cast<void*>(dht), key, key_hash, key_elem_compare,
                   dst_cursor);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::FnFind(void* dht, void const* key,
                                                   FnHash const& key_hash,
                                                   FnCompare const&
                                                       key_elem_compare,
                                                   void* dst_cursor) {
    return TplFind(dht, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void const* DynamicHashTable<
    ElemHash, ElemCompare, NodeAllocator,
    TableNodeAllocator>::ConstFnFind(void const* dht, void const* key,
                                     FnHash const& key_hash,
                                     FnCompare const& key_elem_compare,
                                     void* dst_cursor) {
    return ConstTplFind(dht, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void* DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                       TableNodeAllocator>::Insert(void* dht_, void const* elem,
                                                   void* dst_cursor_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    ZETA_Core_DebugAssert(elem != nullptr);

    Node* node{ static_cast<Node*>(
        allocator::SafeAllocate(&dht->node_allocator, alignof(Node),
                                offsetof(Node, data[dht->width]))) };

    node->lln.Init();
    node->ghtn.Init();

    MemCopy(node->data, elem, dht->width);

    GenericHashTableImpl::Insert(&dht->ght, &node->ghtn);

    llist::InsertL(lln_opr, dht->lln, &node->lln);

    if (dst_cursor != nullptr) {
        dst_cursor->dht = dht;
        dst_cursor->lln = &node->lln;
    }

    return node->data;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::Erase(void* dht_,
                                                 void* pos_cursor_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    auto pos_cursor{ static_cast<Cursor*>(pos_cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(dht, pos_cursor));

    ZETA_Core_DebugAssert(dht->lln != pos_cursor->lln);

    LListNode* lln{ pos_cursor->lln };

    pos_cursor->lln = lln->GetRPtr();

    Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

    llist::Extract(lln_opr, &node->lln);

    GenericHashTableImpl::Extract(&dht->ght, &node->ghtn);

    NodeAllocator::Deallocate(&dht->node_allocator, node);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::EraseAll(void* dht_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(dht);

    for (;;) {
        LListNode* nxt_lln{ dht->lln->GetRPtr() };

        if (nxt_lln == dht->lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(lln_opr, nxt_lln);

        GenericHashTableImpl::Extract(&dht->ght, &nxt_node->ghtn);

        NodeAllocator::Deallocate(&dht->node_allocator, nxt_node);
    }
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::CopyCursor(void const* dht_,
                                                      void const* cursor_,
                                                      void* dst_cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    auto cursor{ static_cast<Cursor const*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(dht, cursor));

    auto dst_cursor{ static_cast<Cursor*>(dst_cursor_) };

    *dst_cursor = *cursor;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
bool DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::AreEqualCursor(void const* dht_,
                                                          void const* cursor_a_,
                                                          void const*
                                                              cursor_b_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };

    auto cursor_a{ static_cast<Cursor const*>(cursor_a_) };
    auto cursor_b{ static_cast<Cursor const*>(cursor_b_) };

    ZETA_Core_DebugAssert(CheckCursor(dht, cursor_a));
    ZETA_Core_DebugAssert(CheckCursor(dht, cursor_b));

    return cursor_a->lln == cursor_b->lln;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::CursorStepL(void const* dht_,
                                                       void* cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    auto cursor{ static_cast<Cursor*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(dht, cursor));

    cursor->lln = cursor->lln->GetLPtr();
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::CursorStepR(void const* dht_,
                                                       void* cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };
    auto cursor{ static_cast<Cursor*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(dht, cursor));

    cursor->lln = cursor->lln->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
bool DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::CheckCntr(void const* dht_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };

    if (!(dht != nullptr)) { return false; }

    size_t width{ dht->width };

    if (!(0 < width)) { return false; }

    if (!(width % alignof(Node) == 0)) { return false; }

    if (!NodeAllocator::CheckAllocator(&dht->node_allocator)) { return false; }

    if (!GenericHashTableImpl::CheckCntr(&dht->ght)) { return false; }

    return true;
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
bool DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::CheckCursor(void const* dht_,
                                                       void const* cursor_) {
    auto dht{ static_cast<DynamicHashTable const*>(dht_) };

    if (!CheckCntr(dht)) { return false; }

    auto cursor{ static_cast<Cursor const*>(cursor_) };

    if (!(cursor != nullptr)) { return false; }

    if (!(cursor->dht == dht)) { return false; }

    if (dht->lln == cursor->lln) { return true; }

    if (!(GenericHashTableImpl::Contain(
            &dht->ght,
            &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn))) {
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
unsigned long long
DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                 TableNodeAllocator>::GetEffFactor(void* dht_) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(dht);

    return GenericHashTableImpl::GetEffFactor(&dht->ght);
}

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
void DynamicHashTable<ElemHash, ElemCompare, NodeAllocator,
                      TableNodeAllocator>::Sanitize(void* dht_,
                                                    MemRecorder* dst_table,
                                                    MemRecorder* dst_node) {
    auto dht{ static_cast<DynamicHashTable*>(dht_) };
    ZETA_Core_DebugAssert(CheckCntr(dht));

    MemRecorder* htn_records{ MemRecorder::Create() };

    GenericHashTableImpl::Sanitize(&dht->ght, dst_table, htn_records);

    MemRecorder::Record(dst_node, dht->lln, sizeof(LListNode));

    size_t node_size{ offsetof(Node, data[dht->width]) };

    for (LListNode* lln{ dht->lln };;) {
        lln = lln->GetRPtr();

        if (lln == dht->lln) { break; }

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        MemRecorder::Record(dst_node, node, node_size);

        ZETA_Core_DebugAssert(MemRecorder::Unrecord(htn_records, &node->ghtn));
    }

    ZETA_Core_DebugAssert(MemRecorder::GetSize(htn_records) == 0);

    MemRecorder::Destroy(htn_records);
}

}  // namespace zeta::core
