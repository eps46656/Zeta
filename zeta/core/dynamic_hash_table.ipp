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

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")

#define CntrTplParamList                                                     \
    typename ElemHashLike, typename ElemCompareLike, typename NodeAllocator, \
        typename TableNodeAllocatorLike

#define CntrTplArgList \
    ElemHashLike, ElemCompareLike, NodeAllocator, TableNodeAllocatorLike

namespace zeta::core::dynamic_hash_table {

template <typename ElemHashLike>
unsigned long long NodeHash<ElemHashLike>::operator()(
    generic_hash_table::Node const* ghtn, unsigned long long salt) const {
    return this->elem_hash(ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data,
                           salt);
}

template <typename ElemCompareLike>
int NodeCompare<ElemCompareLike>::operator()(
    generic_hash_table::Node const* ghtn_x,
    generic_hash_table::Node const* ghtn_y) const {
    return this->elem_compare(
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_x)->data,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_y)->data);
}

void Node::Init() {
    this->lln.Init();
    this->ghtn.Init();
}

namespace ops {

namespace detail {

template <CntrTplParamList>
void CheckCntr(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(0 < width);

    ZETA_Core_DebugAssert(width % alignof(Node) == 0);
}

template <CntrTplParamList>
void CheckCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor) {
    CheckCntr(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(cursor->cntr == cntr);

    if (cntr->lln != cursor->lln) {
        ZETA_Core_DebugAssert(generic_hash_table::ops::Contain(
            &cntr->ght,
            &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn));
    }
}

}  // namespace detail

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(0 < cntr->width);

    cntr->width = UIntAlignUp(cntr->width, alignof(Node));

    cntr->lln = static_cast<LListNode*>(allocator::SafeAllocate(
        &cntr->node_alctr, alignof(LListNode), sizeof(LListNode)));

    cntr->lln->Init();

    generic_hash_table::ops::Init(&cntr->ght);
}

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr(cntr);

    EraseAll(cntr);

    generic_hash_table::ops::Deinit(&cntr->ght);

    NodeAllocator::Deallocate(&cntr->node_alctr, cntr->lln);
}

template <CntrTplParamList>
size_t GetCursorSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr(cntr);

    return cntr->width;
}

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr(cntr);

    return generic_hash_table::ops::GetSize(&cntr->ght);
}

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->lln = cntr->lln;
}

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor) {
    detail::CheckCntr(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->lln = cntr->lln;
}

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem) {
    detail::CheckCntr(cntr);

    Cursor cursor{
        .cntr = cntr,
        .lln = cntr->lln->GetRPtr(),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr->lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* PeekL(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                  Cursor* dst_cursor, void* dst_elem) {
    return PeekL(const_cast<Cntr<CntrTplArgList>*>(cntr), lazy_copy_elem,
                 dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem) {
    detail::CheckCntr(cntr);

    Cursor cursor{
        .cntr = cntr,
        .lln = cntr->lln->GetLPtr(),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr->lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* PeekR(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                  Cursor* dst_cursor, void* dst_elem) {
    return PeekR(const_cast<Cntr<CntrTplArgList>*>(cntr), lazy_copy_elem,
                 dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList>* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor(cntr, pos_cursor);

    if (cntr->lln == pos_cursor->lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* Derefer(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
                    bool lazy_copy_elem, void* dst_elem) {
    return Derefer(const_cast<Cntr<CntrTplArgList>*>(cntr), pos_cursor,
                   lazy_copy_elem, dst_elem);
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(Cntr<CntrTplArgList>* cntr, void const* key, KeyHash const& key_hash,
           KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
           Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr(cntr);

    void* ghtn{ generic_hash_table::ops::Find(
        &cntr->ght, key, key_hash, [&](void const* key, void const* ghtn) {
            return key_elem_compare(
                key, ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data);
        }) };

    if (ghtn == nullptr) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->lln = cntr->lln;
        }

        return nullptr;
    }

    Node* node{ ZETA_Core_MemberToStruct(Node, ghtn, ghtn) };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->lln = &node->lln;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        MemCopy(dst_elem, node->data, cntr->width);
    }

    return node->data;
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void const* Find(Cntr<CntrTplArgList> const* cntr, void const* key,
                 KeyHash const& key_hash,
                 KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
                 Cursor* dst_cursor, void* dst_elem) {
    return Find(const_cast<Cntr<CntrTplArgList>*>(cntr), key, key_hash,
                key_elem_compare, lazy_copy_elem, dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* Insert(Cntr<CntrTplArgList>* cntr, void const* elem, Cursor* dst_cursor) {
    detail::CheckCntr(cntr);

    ZETA_Core_DebugAssert(elem != nullptr);

    Node* node{ static_cast<Node*>(allocator::SafeAllocate(
        &cntr->node_alctr, alignof(Node), offsetof(Node, data[cntr->width]))) };

    node->Init();

    MemCopy(node->data, elem, cntr->width);

    generic_hash_table::ops::Insert(&cntr->ght, &node->ghtn);

    llist::InsertL(cntr->lln->AsView(), node->lln.AsView());

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->lln = &node->lln;
    }

    return node->data;
}

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr(cntr);

    ZETA_Core_DebugAssert(cnt <= GetSize(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ cntr->lln->GetRPtr() };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln->AsView());

        generic_hash_table::ops::Extract(&cntr->ght, &node->ghtn);

        NodeAllocator::Deallocate(&cntr->node_alctr, node);
    }
}

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr(cntr);

    ZETA_Core_DebugAssert(cnt <= GetSize(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ cntr->lln->GetLPtr() };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln->AsView());

        generic_hash_table::ops::Extract(&cntr->ght, &node->ghtn);

        NodeAllocator::Deallocate(&cntr->node_alctr, node);
    }
}

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor) {
    detail::CheckCursor(cntr, pos_cursor);

    ZETA_Core_DebugAssert(cntr->lln != pos_cursor->lln);

    LListNode* lln{ pos_cursor->lln };

    pos_cursor->lln = lln->GetRPtr();

    Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

    llist::Extract(node->lln.AsView());

    generic_hash_table::ops::Extract(&cntr->ght, &node->ghtn);

    NodeAllocator::Deallocate(&cntr->node_alctr, node);
}

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr);

    for (;;) {
        LListNode* nxt_lln{ cntr->lln->GetRPtr() };

        if (nxt_lln == cntr->lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(nxt_lln->AsView());

        generic_hash_table::ops::Extract(&cntr->ght, &nxt_node->ghtn);

        NodeAllocator::Deallocate(&cntr->node_alctr, nxt_node);
    }
}

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* src_cursor,
                Cursor* dst_cursor) {
    detail::CheckCursor(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b) {
    detail::CheckCursor(cntr, cursor_a);
    detail::CheckCursor(cntr, cursor_b);

    return cursor_a->lln == cursor_b->lln;
}

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    detail::CheckCursor(cntr, cursor);

    cursor->lln = cursor->lln->GetLPtr();
}

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    detail::CheckCursor(cntr, cursor);

    cursor->lln = cursor->lln->GetRPtr();
}

template <CntrTplParamList>
unsigned long long GetEffFactor(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr(cntr);

    return GetEffFactor(&cntr->ght);
}

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList>* cntr, MemRecorder* dst_table,
              MemRecorder* dst_node) {
    detail::CheckCntr(cntr);

    MemRecorder* htn_records{ MemRecorder::Create() };

    generic_hash_table::ops::Sanitize(&cntr->ght, dst_table, htn_records);

    MemRecorder::Record(dst_node, cntr->lln, sizeof(LListNode));

    size_t node_size{ offsetof(Node, data[cntr->width]) };

    for (LListNode* lln{ cntr->lln };;) {
        lln = lln->GetRPtr();

        if (lln == cntr->lln) { break; }

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        MemRecorder::Record(dst_node, node, node_size);

        ZETA_Core_DebugAssert(MemRecorder::Unrecord(htn_records, &node->ghtn));
    }

    ZETA_Core_DebugAssert(MemRecorder::GetSize(htn_records) == 0);

    MemRecorder::Destroy(htn_records);
}

template <CntrTplParamList>
AssocCntrView<CntrTplArgList>* AsAssocCntrView(Cntr<CntrTplArgList>* cntr) {
    return reinterpret_cast<AssocCntrView<CntrTplArgList>*>(cntr);
}

}  // namespace ops

template <CntrTplParamList>
constexpr bool AssocCntrView<CntrTplArgList>::IsConst(
    type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList>*>) {
    return false;
}

template <CntrTplParamList>
constexpr bool AssocCntrView<CntrTplArgList>::IsConst(
    type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList> const*>) {
    return false;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList>*>) {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,
        .GetWidth = true,
        .GetSize = true,
        .GetCapacity = true,
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
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList> const*>) {
    return GetStaticEnabledAbilityFlag(
               type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList>*>()) &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView<CntrTplArgList>*>) {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = false,
        .GetWidth = false,
        .GetSize = false,
        .GetCapacity = false,
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
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView const*>) {
    return GetStaticDisabledAbilityFlag(
               type_wrapper::TypeWrapper<Cntr<CntrTplArgList>*>()) |
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetDynamicEnabledAbilityFlag(
    AssocCntrView const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetDynamicDisabledAbilityFlag(
    AssocCntrView const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr size_t AssocCntrView<CntrTplArgList>::GetCursorSize(
    AssocCntrView const* cntr_view) {
    return ops::GetCursorSize(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view));
}

template <CntrTplParamList>
size_t AssocCntrView<CntrTplArgList>::GetWidth(AssocCntrView const* cntr_view) {
    return ops::GetWidth(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view));
}

template <CntrTplParamList>
size_t AssocCntrView<CntrTplArgList>::GetSize(AssocCntrView const* cntr_view) {
    return ops::GetSize(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view));
}

template <CntrTplParamList>
size_t AssocCntrView<CntrTplArgList>::GetCapacity(
    AssocCntrView const* cntr_view) {
    return ops::GetCapacity(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::GetLBCursor(AssocCntrView const* cntr_view,
                                                void* dst_cursor) {
    return ops::GetLBCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::GetRBCursor(AssocCntrView const* cntr_view,
                                                void* dst_cursor) {
    return ops::GetRBCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::PeekL(AssocCntrView* cntr_view,
                                           bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return ops::PeekL(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::PeekL(AssocCntrView const* cntr_view,
                                                 bool lazy_copy_elem,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    return ops::PeekL(reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::PeekR(AssocCntrView* cntr_view,
                                           bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return ops::PeekR(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::PeekR(AssocCntrView const* cntr_view,
                                                 bool lazy_copy_elem,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    return ops::PeekR(reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
                      lazy_copy_elem, static_cast<Cursor*>(dst_cursor),
                      dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::Derefer(AssocCntrView* cntr_view,
                                             void const* pos_cursor,
                                             bool lazy_copy_elem,
                                             void* dst_elem) {
    return ops::Derefer(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
                        static_cast<Cursor const*>(pos_cursor), lazy_copy_elem,
                        dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::Derefer(
    AssocCntrView const* cntr_view, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return ops::Derefer(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor const*>(pos_cursor), lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void* AssocCntrView<CntrTplArgList>::Find(
    AssocCntrView* cntr_view, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return ops::Find(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), key,
                     key_hash, key_elem_compare, lazy_copy_elem,
                     static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void const* AssocCntrView<CntrTplArgList>::Find(
    AssocCntrView const* cntr_view, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return ops::Find(reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
                     key, key_hash, key_elem_compare, lazy_copy_elem,
                     static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::Insert(AssocCntrView* cntr_view,
                                            void const* elem,
                                            void* dst_cursor) {
    return ops::Insert(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), elem,
                       static_cast<Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::PopL(AssocCntrView* cntr_view, size_t cnt) {
    return ops::PopL(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), cnt);
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::PopR(AssocCntrView* cntr_view, size_t cnt) {
    return ops::PopR(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), cnt);
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::Erase(AssocCntrView* cntr_view,
                                          void* pos_cursor) {
    return ops::Erase(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
                      static_cast<Cursor*>(pos_cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::EraseAll(AssocCntrView* cntr_view) {
    return ops::EraseAll(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::CopyCursor(AssocCntrView const* cntr_view,
                                               void const* cursor,
                                               void* dst_cursor) {
    return ops::CopyCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor const*>(cursor), static_cast<Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool AssocCntrView<CntrTplArgList>::AreEqualCursor(
    AssocCntrView const* cntr_view, void const* cursor_a,
    void const* cursor_b) {
    return ops::AreEqualCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor const*>(cursor_a),
        static_cast<Cursor const*>(cursor_b));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::CursorStepL(AssocCntrView const* cntr_view,
                                                void* cursor) {
    return ops::CursorStepL(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor*>(cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::CursorStepR(AssocCntrView const* cntr_view,
                                                void* cursor) {
    return ops::CursorStepR(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<Cursor*>(cursor));
}

}  // namespace zeta::core::dynamic_hash_table
