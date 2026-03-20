#pragma once

#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/generic_hash_table.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                                             \
    typename ElemHashLike, typename ElemCompareLike,                 \
        typename NodeAllocatorLike, typename TableNodeAllocatorLike, \
        typename SaltRandomEngineLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList                                                        \
    ElemHashLike, ElemCompareLike, NodeAllocatorLike, TableNodeAllocatorLike, \
        SaltRandomEngineLike

namespace zeta::core {

template <typename ElemHashLike>
unsigned long long dynamic_hash_table::NodeHash<ElemHashLike>::operator()(
    generic_hash_table::Node const* ghtn, unsigned long long salt) const {
    return this->elem_hash(ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data,
                           salt);
}

template <typename ElemCompareLike>
int dynamic_hash_table::NodeCompare<ElemCompareLike>::operator()(
    generic_hash_table::Node const* ghtn_x,
    generic_hash_table::Node const* ghtn_y) const {
    return this->elem_compare(
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_x)->data,
        ZETA_Core_MemberToStruct(Node, ghtn, ghtn_y)->data);
}

inline void dynamic_hash_table::Node::Init() {
    this->lln.Init();
    this->ghtn.Init();
}

namespace dynamic_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(0 < width);

    ZETA_Core_DebugAssert(width % alignof(Node) == 0);
}

template <CntrTplParamList>
void CheckCursor_(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    ZETA_Core_DebugAssert(cursor->cntr == cntr);

    if (cntr->lln != cursor->lln) {
        ZETA_Core_DebugAssert(generic_hash_table::Contain(
            &cntr->ght,
            &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn));
    }
}

}  // namespace dynamic_hash_table::detail

template <CntrTplParamList, typename NodeAllocatorInitArg,
          typename ElemHashLikeInitArg, typename ElemCompareInitArg,
          typename TableNodeAllocatorInitArg, typename SaltRandomEngineInitArg>
void dynamic_hash_table::Init(
    Cntr<CntrTplArgList>* cntr, size_t width,
    NodeAllocatorInitArg&& node_alctr_init_arg,
    generic_hash_table::RehashingConfig const& rehashing_config,
    ElemHashLikeInitArg&& elem_hash_init_arg,
    ElemCompareInitArg&& elem_compare_init_arg,
    TableNodeAllocatorInitArg&& table_node_alctr_init_arg,
    SaltRandomEngineInitArg&& salt_random_engine_init_arg) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    ZETA_Core_DebugAssert(0 < width);

    cntr->width = width = integral_math::AlignUp(width, alignof(Node));

    lifecycle::Init(cntr->node_alctr,
                    meta::Forward<NodeAllocatorInitArg>(node_alctr_init_arg));

    cntr->lln = static_cast<LListNode*>(allocator::SafeAllocate(
        cntr->node_alctr, alignof(LListNode), sizeof(LListNode)));

    cntr->lln->Init();

    generic_hash_table::Init(
        &cntr->ght, rehashing_config,
        meta::Forward<ElemHashLikeInitArg>(elem_hash_init_arg),
        meta::Forward<ElemCompareInitArg>(elem_compare_init_arg),
        meta::Forward<TableNodeAllocatorInitArg>(table_node_alctr_init_arg),
        meta::Forward<SaltRandomEngineInitArg>(salt_random_engine_init_arg));
}

template <CntrTplParamList>
void dynamic_hash_table::Deinit(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    EraseAll(cntr);

    generic_hash_table::Deinit(&cntr->ght);

    allocator::Deallocate(cntr->node_alctr, cntr->lln);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetCursorSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetWidth(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->width;
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return generic_hash_table::GetSize(&cntr->ght);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetCapacity(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void dynamic_hash_table::GetLBCursor(Cntr<CntrTplArgList> const* cntr,
                                     Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->lln = cntr->lln;
}

template <CntrTplParamList>
void dynamic_hash_table::GetRBCursor(Cntr<CntrTplArgList> const* cntr,
                                     Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->lln = cntr->lln;
}

template <CntrTplParamList>
void* dynamic_hash_table::PeekL(Cntr<CntrTplArgList> const* cntr,
                                bool lazy_copy_elem, Cursor* dst_cursor,
                                void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = cntr,
        .lln = llist::GetR(cntr->lln),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr->lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void* dynamic_hash_table::PeekR(Cntr<CntrTplArgList> const* cntr,
                                bool lazy_copy_elem, Cursor* dst_cursor,
                                void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = cntr,
        .lln = llist::GetL(cntr->lln),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr->lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void* dynamic_hash_table::Derefer(Cntr<CntrTplArgList> const* cntr,
                                  Cursor const* pos_cursor, bool lazy_copy_elem,
                                  void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    if (cntr->lln == pos_cursor->lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* dynamic_hash_table::Find(Cntr<CntrTplArgList> const* cntr,
                               void const* key, KeyHash const& key_hash,
                               KeyElemCompare const& key_elem_compare,
                               bool lazy_copy_elem, Cursor* dst_cursor,
                               void* dst_elem) {
    detail::CheckCntr_(cntr);

    void* ghtn{ generic_hash_table::Find(
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
        utils::MemCopy(dst_elem, node->data, cntr->width);
    }

    return node->data;
}

template <CntrTplParamList>
void* dynamic_hash_table::Insert(Cntr<CntrTplArgList>* cntr, void const* elem,
                                 Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(elem != nullptr);

    Node* node{ static_cast<Node*>(allocator::SafeAllocate(
        cntr->node_alctr, alignof(Node), offsetof(Node, data[cntr->width]))) };

    node->Init();

    utils::MemCopy(node->data, elem, cntr->width);

    generic_hash_table::Insert(&cntr->ght, &node->ghtn);

    llist::InsertL(cntr->lln, node->lln);

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->lln = &node->lln;
    }

    return node->data;
}

template <CntrTplParamList>
void dynamic_hash_table::PopL(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= GetSize(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetR(cntr->lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        generic_hash_table::Extract(&cntr->ght, &node->ghtn);

        allocator::Deallocate(cntr->node_alctr, node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::PopR(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= GetSize(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetL(cntr->lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        generic_hash_table::Extract(&cntr->ght, &node->ghtn);

        allocator::Deallocate(cntr->node_alctr, node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(cntr->lln != pos_cursor->lln);

    LListNode* lln{ pos_cursor->lln };

    pos_cursor->lln = llist::GetR(lln);

    Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

    llist::Extract(node->lln);

    generic_hash_table::Extract(&cntr->ght, &node->ghtn);

    allocator::Deallocate(cntr->node_alctr, node);
}

template <CntrTplParamList>
void dynamic_hash_table::EraseAll(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr);

    for (;;) {
        LListNode* nxt_lln{ llist::GetR(cntr->lln) };

        if (nxt_lln == cntr->lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(nxt_lln);

        generic_hash_table::Extract(&cntr->ght, &nxt_node->ghtn);

        allocator::Deallocate(cntr->node_alctr, nxt_node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::CopyCursor(Cntr<CntrTplArgList> const* cntr,
                                    Cursor const* src_cursor,
                                    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool dynamic_hash_table::AreEqualCursor(Cntr<CntrTplArgList> const* cntr,
                                        Cursor const* cursor_a,
                                        Cursor const* cursor_b) {
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->lln == cursor_b->lln;
}

template <CntrTplParamList>
void dynamic_hash_table::CursorStepL(Cntr<CntrTplArgList> const* cntr,
                                     Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetL(cursor->lln);
}

template <CntrTplParamList>
void dynamic_hash_table::CursorStepR(Cntr<CntrTplArgList> const* cntr,
                                     Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetR(cursor->lln);
}

template <CntrTplParamList>
unsigned long long dynamic_hash_table::GetEffFactor(
    Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    return GetEffFactor(&cntr->ght);
}

template <CntrTplParamList>
void dynamic_hash_table::Sanitize(Cntr<CntrTplArgList> const* cntr,
                                  mem_recorder::MemRecorder* dst_table,
                                  mem_recorder::MemRecorder* dst_node) {
    detail::CheckCntr_(cntr);

    mem_recorder::MemRecorder htn_records;

    generic_hash_table::Sanitize(&cntr->ght, dst_table, &htn_records);

    mem_recorder::Record(dst_node, cntr->lln, sizeof(LListNode));

    size_t node_size{ offsetof(Node, data[cntr->width]) };

    for (LListNode* lln{ cntr->lln };;) {
        lln = llist::GetR(lln);

        if (lln == cntr->lln) { break; }

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        mem_recorder::Record(dst_node, node, node_size);

        ZETA_Core_DebugAssert(
            mem_recorder::Unrecord(&htn_records, &node->ghtn));
    }

    ZETA_Core_DebugAssert(mem_recorder::GetSize(&htn_records) == 0);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const,
                             void>::
    GetReferedInst(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return const_cast<dynamic_hash_table::Cntr<CntrTplArgList>::Cntr*>(cntr);
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>,
                       void>::GetStaticEnabledAbilityFlag() {
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
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const,
                       void>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>,
                                  void>::GetStaticEnabledAbilityFlag() &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>,
                       void>::GetStaticDisabledAbilityFlag() {
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
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const,
                       void>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>,
                                  void>::GetStaticDisabledAbilityFlag() |
           assoc_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const, void>::
    GetDynamicEnabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const, void>::
    GetDynamicDisabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr size_t assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetCursorSize(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return dynamic_hash_table::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetWidth(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return dynamic_hash_table::GetWidth(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetSize(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return dynamic_hash_table::GetSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetCapacity(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return dynamic_hash_table::GetCapacity(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetLBCursor(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       void* dst_cursor) {
    dynamic_hash_table::GetLBCursor(
        cntr, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::GetRBCursor(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       void* dst_cursor) {
    dynamic_hash_table::GetRBCursor(
        cntr, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::PeekL(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                 bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::PeekL(
        cntr, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::PeekR(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                 bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::PeekR(
        cntr, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::
    CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const, void>::Derefer(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
        void const* pos_cursor, bool lazy_copy_elem, void* dst_elem) {
    return dynamic_hash_table::Derefer(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::
    CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const, void>::Find(
        dynamic_hash_table::Cntr<CntrTplArgList> const* cntr, void const* key,
        KeyHash const& key_hash, KeyElemCompare const& key_elem_compare,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::Find(
        cntr, key, key_hash, key_elem_compare, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>, void>::
    Insert(dynamic_hash_table::Cntr<CntrTplArgList>* cntr, void const* elem,
           void* dst_cursor) {
    return dynamic_hash_table::Insert(
        cntr, elem, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>, void>::
    PopL(dynamic_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    dynamic_hash_table::PopL(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>, void>::
    PopR(dynamic_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    dynamic_hash_table::PopR(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>, void>::
    Erase(dynamic_hash_table::Cntr<CntrTplArgList>* cntr, void* pos_cursor) {
    dynamic_hash_table::Erase(
        cntr, static_cast<dynamic_hash_table::Cursor*>(pos_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>, void>::
    EraseAll(dynamic_hash_table::Cntr<CntrTplArgList>* cntr) {
    dynamic_hash_table::EraseAll(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::CopyCursor(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                      void const* src_cursor, void* dst_cursor) {
    dynamic_hash_table::CopyCursor(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(src_cursor),
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::AreEqualCursor(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                          void const* cursor_a, void const* cursor_b) {
    return dynamic_hash_table::AreEqualCursor(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(cursor_a),
        static_cast<dynamic_hash_table::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::CursorStepL(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       void* cursor) {
    dynamic_hash_table::CursorStepL(
        cntr, static_cast<dynamic_hash_table::Cursor*>(cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList> const,
    void>::CursorStepR(dynamic_hash_table::Cntr<CntrTplArgList> const* cntr,
                       void* cursor) {
    dynamic_hash_table::CursorStepR(
        cntr, static_cast<dynamic_hash_table::Cursor*>(cursor));
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
