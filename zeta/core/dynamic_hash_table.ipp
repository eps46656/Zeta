#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/basic_llist_node.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/dynamic_hash_table.hpp>
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
unsigned long long dynamic_hash_table::NodeHasher<ElemHasherLike>::operator()(
    generic_hash_table::Node const* ghtn, unsigned long long salt) const {
    return this->elem_hasher(ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data,
                             salt);
}

template <typename ElemHasherLike>
template <typename... Args>
void lifecycle::Traits<dynamic_hash_table::NodeHasher<ElemHasherLike>>::Init(
    dynamic_hash_table::NodeHasher<ElemHasherLike>& node_hasher,
    Args&&... args) {
    lifecycle::Init(node_hasher.elem_hasher, meta::Forward<Args>(args)...);
}

template <typename ElemHasherLike>
void lifecycle::Traits<dynamic_hash_table::NodeHasher<ElemHasherLike>>::Deinit(
    dynamic_hash_table::NodeHasher<ElemHasherLike>& node_hasher) {
    lifecycle::Deinit(node_hasher.elem_hasher);
}

template <typename ElemComparatorLike>
int dynamic_hash_table::NodeComparator<ElemComparatorLike>::operator()(
    generic_hash_table::Node const* ghtn_x,
    generic_hash_table::Node const* ghtn_y) const {
    return this->elem_cmptr(ZETA_Core_MemberToStruct(Node, ghtn, ghtn_x)->data,
                            ZETA_Core_MemberToStruct(Node, ghtn, ghtn_y)->data);
}

template <typename ElemComparatorLike>
template <typename... Args>
void lifecycle::Traits<dynamic_hash_table::NodeComparator<ElemComparatorLike>>::
    Init(dynamic_hash_table::NodeComparator<ElemComparatorLike>& node_cmptr,
         Args&&... args) {
    lifecycle::Init(node_cmptr.elem_cmptr, meta::Forward<Args>(args)...);
}

template <typename ElemComparatorLike>
void lifecycle::Traits<dynamic_hash_table::NodeComparator<ElemComparatorLike>>::
    Deinit(dynamic_hash_table::NodeComparator<ElemComparatorLike>& node_cmptr) {
    lifecycle::Deinit(node_cmptr.elem_cmptr);
}

inline void dynamic_hash_table::Node::Init() {
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
        ZETA_Core_DebugAssert(generic_hash_table::Contain(
            cntr.ght, &ZETA_Core_MemberToStruct(Node, lln, cursor->lln)->ghtn));
    }
}

}  // namespace dynamic_hash_table::detail

template <CntrTplParamList, typename ElemHasherLikeInitArg,
          typename ElemComparatorInitArg, typename SaltRandomEngineInitArg,
          typename NodeAllocatorInitArg, typename TableNodeAllocatorInitArg>
void dynamic_hash_table::Init(
    Cntr<CntrTplArgList>& cntr, size_t elem_size,
    generic_hash_table::RehashingConfig const& rehashing_config,
    ElemHasherLikeInitArg&& elem_hash_init_arg,
    ElemComparatorInitArg&& elem_cmptr_init_arg,
    SaltRandomEngineInitArg&& salt_random_engine_init_arg,
    NodeAllocatorInitArg&& node_alctr_init_arg,
    TableNodeAllocatorInitArg&& table_node_alctr_init_arg) {
    ZETA_Core_DebugAssert(0 < elem_size);

    cntr.elem_size = elem_size =
        integral_math::AlignUp(elem_size, alignof(Node));

    cntr.lln = static_cast<LListNode*>(allocator::SafeAllocate(
        cntr.node_alctr, alignof(LListNode), sizeof(LListNode)));

    cntr.lln->Init();

    generic_hash_table::Init(
        cntr.ght, rehashing_config,
        meta::Forward<ElemHasherLikeInitArg>(elem_hash_init_arg),
        meta::Forward<ElemComparatorInitArg>(elem_cmptr_init_arg),
        meta::Forward<SaltRandomEngineInitArg>(salt_random_engine_init_arg),
        meta::Forward<TableNodeAllocatorInitArg>(table_node_alctr_init_arg));

    lifecycle::Init(cntr.node_alctr,
                    meta::Forward<NodeAllocatorInitArg>(node_alctr_init_arg));
}

template <CntrTplParamList>
void dynamic_hash_table::Deinit(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    (EraseAll)(cntr);

    generic_hash_table::Deinit(cntr.ght);

    allocator::Deallocate(cntr.node_alctr, cntr.lln);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetCursorSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetElemSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return generic_hash_table::GetSize(cntr.ght);
}

template <CntrTplParamList>
size_t dynamic_hash_table::GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void dynamic_hash_table::GetLBCursor(Cntr<CntrTplArgList> const& cntr,
                                     Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->lln = cntr.lln;
}

template <CntrTplParamList>
void dynamic_hash_table::GetRBCursor(Cntr<CntrTplArgList> const& cntr,
                                     Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->lln = cntr.lln;
}

template <CntrTplParamList>
void* dynamic_hash_table::PeekL(Cntr<CntrTplArgList> const& cntr,
                                bool lazy_copy_elem, Cursor* dst_cursor,
                                void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = &cntr,
        .lln = llist::GetR(cntr.lln),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr.lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList>
void* dynamic_hash_table::PeekR(Cntr<CntrTplArgList> const& cntr,
                                bool lazy_copy_elem, Cursor* dst_cursor,
                                void* dst_elem) {
    detail::CheckCntr_(cntr);

    Cursor cursor{
        .cntr = &cntr,
        .lln = llist::GetL(cntr.lln),
    };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cursor.cntr;
        dst_cursor->lln = cursor.lln;
    }

    if (cntr.lln == cursor.lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, cursor.lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList>
void* dynamic_hash_table::Derefer(Cntr<CntrTplArgList> const& cntr,
                                  Cursor const* pos_cursor, bool lazy_copy_elem,
                                  void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    if (cntr.lln == pos_cursor->lln) { return nullptr; }

    void* elem{ ZETA_Core_MemberToStruct(Node, lln, pos_cursor->lln)->data };

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList, typename KeyHasher, typename KeyElemComparator>
void* dynamic_hash_table::Find(Cntr<CntrTplArgList> const& cntr,
                               void const* key, KeyHasher const& key_hasher,
                               KeyElemComparator const& key_elem_cmptr,
                               bool lazy_copy_elem, Cursor* dst_cursor,
                               void* dst_elem) {
    detail::CheckCntr_(cntr);

    void* ghtn{ generic_hash_table::Find(
        cntr.ght, key, key_hasher, [&](void const* key, void const* ghtn) {
            return key_elem_cmptr(
                key, ZETA_Core_MemberToStruct(Node, ghtn, ghtn)->data);
        }) };

    if (ghtn == nullptr) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->lln = cntr.lln;
        }

        return nullptr;
    }

    Node* node{ ZETA_Core_MemberToStruct(Node, ghtn, ghtn) };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->lln = &node->lln;
    }

    if (!lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, node->data, cntr.elem_size);
    }

    return node->data;
}

template <CntrTplParamList>
void* dynamic_hash_table::Insert(Cntr<CntrTplArgList>& cntr, void const* elem,
                                 Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(elem != nullptr);

    Node* node{ static_cast<Node*>(allocator::SafeAllocate(
        cntr.node_alctr, alignof(Node),
        __builtin_offsetof(Node, data[cntr.elem_size]))) };

    node->Init();

    utils::MemCopy(node->data, elem, cntr.elem_size);

    generic_hash_table::Insert(cntr.ght, &node->ghtn);

    llist::InsertL(cntr.lln, &node->lln);

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->lln = &node->lln;
    }

    return node->data;
}

template <CntrTplParamList>
void dynamic_hash_table::PopL(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= GetElemCnt(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetR(cntr.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        generic_hash_table::Extract(cntr.ght, &node->ghtn);

        allocator::Deallocate(cntr.node_alctr, node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::PopR(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cnt <= GetElemCnt(cntr));

    for (; 0 < cnt; --cnt) {
        LListNode* lln{ llist::GetL(cntr.lln) };

        Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

        llist::Extract(lln);

        generic_hash_table::Extract(cntr.ght, &node->ghtn);

        allocator::Deallocate(cntr.node_alctr, node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::Erase(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(cntr.lln != pos_cursor->lln);

    LListNode* lln{ pos_cursor->lln };

    pos_cursor->lln = llist::GetR(lln);

    Node* node{ ZETA_Core_MemberToStruct(Node, lln, lln) };

    llist::Extract(&node->lln);

    generic_hash_table::Extract(cntr.ght, &node->ghtn);

    allocator::Deallocate(cntr.node_alctr, node);
}

template <CntrTplParamList>
void dynamic_hash_table::EraseAll(Cntr<CntrTplArgList>& cntr) {
    for (;;) {
        LListNode* nxt_lln{ llist::GetR(cntr.lln) };

        if (nxt_lln == cntr.lln) { break; }

        Node* nxt_node{ ZETA_Core_MemberToStruct(Node, lln, nxt_lln) };

        llist::Extract(nxt_lln);

        generic_hash_table::Extract(cntr.ght, &nxt_node->ghtn);

        allocator::Deallocate(cntr.node_alctr, nxt_node);
    }
}

template <CntrTplParamList>
void dynamic_hash_table::CopyCursor(Cntr<CntrTplArgList> const& cntr,
                                    Cursor const* src_cursor,
                                    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool dynamic_hash_table::AreEqualCursor(Cntr<CntrTplArgList> const& cntr,
                                        Cursor const* cursor_a,
                                        Cursor const* cursor_b) {
    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return cursor_a->lln == cursor_b->lln;
}

template <CntrTplParamList>
void dynamic_hash_table::CursorStepL(Cntr<CntrTplArgList> const& cntr,
                                     Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetL(cursor->lln);
}

template <CntrTplParamList>
void dynamic_hash_table::CursorStepR(Cntr<CntrTplArgList> const& cntr,
                                     Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    cursor->lln = llist::GetR(cursor->lln);
}

template <CntrTplParamList>
auto dynamic_hash_table::GetEffFactor(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    return generic_hash_table::GetEffFactor(cntr.ght);
}

template <CntrTplParamList>
void dynamic_hash_table::Sanitize(Cntr<CntrTplArgList> const& cntr,
                                  mem_recorder::MemRecorder* dst_table,
                                  mem_recorder::MemRecorder* dst_node) {
    detail::CheckCntr_(cntr);

    mem_recorder::MemRecorder htn_records;

    generic_hash_table::Sanitize(cntr.ght, dst_table, &htn_records);

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
template <typename... Args>
void lifecycle::Traits<dynamic_hash_table::Cntr<CntrTplArgList>>::Init(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr, Args&&... args) {
    dynamic_hash_table::Init(cntr, meta::Forward<Args>(args)...);
}

template <CntrTplParamList>
void lifecycle::Traits<dynamic_hash_table::Cntr<CntrTplArgList>>::Deinit(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr) {
    dynamic_hash_table::Deinit(cntr);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetReferedInstPtr(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return const_cast<dynamic_hash_table::Cntr<CntrTplArgList>::Cntr*>(cntr);
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList>>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
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
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticEnabledAbilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
               GetStaticEnabledAbilityFlag() &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    dynamic_hash_table::Cntr<CntrTplArgList>>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
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
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticDisabledAbilityFlag() {
    return assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::
               GetStaticDisabledAbilityFlag() |
           assoc_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledAbilityFlag(
        dynamic_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr size_t assoc_cntr::
    CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::GetCursorSize(
        dynamic_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return dynamic_hash_table::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetElemSize(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return dynamic_hash_table::GetElemSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetElemCnt(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return dynamic_hash_table::GetElemCnt(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetMaxElemCnt(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return dynamic_hash_table::GetMaxElemCnt(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetLBCursor(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* dst_cursor) {
    dynamic_hash_table::GetLBCursor(
        cntr, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    GetRBCursor(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* dst_cursor) {
    dynamic_hash_table::GetRBCursor(
        cntr, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    PeekL(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
          bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::PeekL(
        cntr, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    PeekR(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
          bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::PeekR(
        cntr, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    Derefer(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
            void const* pos_cursor, bool lazy_copy_elem, void* dst_elem) {
    return dynamic_hash_table::Derefer(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHasher, typename KeyElemComparator>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    Find(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr, void const* key,
         KeyHasher const& key_hasher, KeyElemComparator const& key_elem_compare,
         bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return dynamic_hash_table::Find(
        cntr, key, key_hasher, key_elem_compare, lazy_copy_elem,
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor), dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::Insert(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr, void const* elem,
    void* dst_cursor) {
    return dynamic_hash_table::Insert(
        cntr, elem, static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::PopL(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    dynamic_hash_table::PopL(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::PopR(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    dynamic_hash_table::PopR(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::Erase(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr, void* pos_cursor) {
    dynamic_hash_table::Erase(
        cntr, static_cast<dynamic_hash_table::Cursor*>(pos_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList>>::EraseAll(
    dynamic_hash_table::Cntr<CntrTplArgList>& cntr) {
    dynamic_hash_table::EraseAll(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    CopyCursor(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
               void const* src_cursor, void* dst_cursor) {
    dynamic_hash_table::CopyCursor(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(src_cursor),
        static_cast<dynamic_hash_table::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    AreEqualCursor(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                   void const* cursor_a, void const* cursor_b) {
    return dynamic_hash_table::AreEqualCursor(
        cntr, static_cast<dynamic_hash_table::Cursor const*>(cursor_a),
        static_cast<dynamic_hash_table::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    CursorStepL(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    dynamic_hash_table::CursorStepL(
        cntr, static_cast<dynamic_hash_table::Cursor*>(cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<dynamic_hash_table::Cntr<CntrTplArgList> const>::
    CursorStepR(dynamic_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    dynamic_hash_table::CursorStepR(
        cntr, static_cast<dynamic_hash_table::Cursor*>(cursor));
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
