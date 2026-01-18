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

namespace zeta::core {

ZETA_Core_StaticAssert(alignof(void*) % 4 == 0);

template <typename ElemHash, typename ElemCompare, typename NodeAllocator,
          typename TableNodeAllocator>
struct DynamicHashTable {
    using FnHash = assoc_cntr::FnHash;
    using FnCompare = assoc_cntr::FnCompare;

    using LListNode =
        LListNodeTpl<void*, value_wrapper::StaticValueWrapper<false>,
                     value_wrapper::StaticValueWrapper<false>>;

    static constexpr LListNodeTplOperator lln_opr;

    struct NodeHash {
        ElemHash elem_hash;

        unsigned long long operator()(GenericHashTableNode const* ghtn,
                                      unsigned long long salt) const;
    };

    struct NodeCompare {
        ElemCompare elem_compare;

        int operator()(GenericHashTableNode const* ghtn_x,
                       GenericHashTableNode const* ghtn_y) const;
    };

    using GenericHashTableImpl =
        GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>;

    struct Node {
        ZETA_Core_DebugStructPadding;

        LListNode lln;

        ZETA_Core_DebugStructPadding;

        GenericHashTableNode ghtn;

        ZETA_Core_DebugStructPadding;

        unsigned char data[] __attribute__((aligned(alignof(max_align_t))));
    };

    struct Cursor {
        void const* dht;
        LListNode* lln;
    };

    static constexpr assoc_cntr::AssocCntrAbilityFlagType
        static_enabled_ability_flag{ assoc_cntr::AssocCntrAbilityFlagBuilder{
            .GetLBCursor = true,
            .GetRBCursor = true,

            .PeekL = true,
            .PeekR = true,

            .CompareCursor = false,
            .GetCursorDist = false,
            .GetCursorIdx = false,
            .CursorStepL = true,
            .CursorStepR = true,
            .CursorAdvanceL = false,
            .CursorAdvanceR = false,
        }() };

    static constexpr assoc_cntr::AssocCntrAbilityFlagType
        static_disabled_ability_flag{ assoc_cntr::AssocCntrAbilityFlagBuilder{
            .GetLBCursor = false,
            .GetRBCursor = false,

            .PeekL = false,
            .PeekR = false,

            .CompareCursor = true,
            .GetCursorDist = true,
            .GetCursorIdx = true,
            .CursorStepL = false,
            .CursorStepR = false,
            .CursorAdvanceL = true,
            .CursorAdvanceR = true,
        }() };

    static constexpr assoc_cntr::AssocCntrAbilityFlagType dynamic_ability_flag{
        assoc_cntr::AssocCntrAbilityFlagBuilder{
            .GetLBCursor = false,
            .GetRBCursor = false,

            .PeekL = false,
            .PeekR = false,

            .CompareCursor = false,
            .GetCursorDist = false,
            .GetCursorIdx = false,
            .CursorStepL = false,
            .CursorStepR = false,
            .CursorAdvanceL = false,
            .CursorAdvanceR = false,
        }()
    };

    // -------------------------------------------------------------------------

    static unsigned long long GHTNodeHash_(
        DynamicHashTable const* dht, GenericHashTableImpl::Node const* ghtn,
        unsigned long long salt);

    // -------------------------------------------------------------------------

    size_t width;

    LListNode* lln;

    NodeAllocator node_allocator;

    GenericHashTableImpl ght;

    // -------------------------------------------------------------------------

    static void Init(void* dht);

    static void Deinit(void* dht);

    // -------------------------------------------------------------------------

    static assoc_cntr::AssocCntrAbilityFlagType GetDynamicEnabledAbilityFlag(
        void const* dht);

    static assoc_cntr::AssocCntrAbilityFlagType GetDynamicDisabledAbilityFlag(
        void const* dht);

    static size_t GetCursorSize(void const* dht);

    static size_t GetWidth(void const* dht);

    static size_t GetSize(void const* dht);

    static size_t GetCapacity(void const* dht);

    static void GetLBCursor(void const* dht, void* dst_cursor);

    static void GetRBCursor(void const* dht, void* dst_cursor);

    static void* PeekL(void* dht, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekL(void const* dht, void* dst_cursor,
                                  void* dst_elem);

    static void* PeekR(void* dht, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekR(void const* dht, void* dst_cursor,
                                  void* dst_elem);

    static void* Refer(void* dht, void const* pos_cursor);

    static void const* ConstRefer(void const* dht, void const* pos_cursor);

    template <typename KeyHash, typename KeyElemCompare>
    static void* TplFind(void* dht, void const* key, KeyHash const& key_hash,
                         KeyElemCompare const& key_elem_compare,
                         void* dst_cursor);

    template <typename Key, typename KeyHash, typename KeyElemCompare>
    static void const* ConstTplFind(void const* dht, Key const& key,
                                    KeyHash const& key_hash,
                                    KeyElemCompare const& key_elem_compare,
                                    void* dst_cursor);

    static void* FnFind(void* dht, void const* key, FnHash const& key_hash,
                        FnCompare const& key_elem_compare, void* dst_cursor);

    static void const* ConstFnFind(void const* dht, void const* key,
                                   FnHash const& key_hash,
                                   FnCompare const& key_elem_compare,
                                   void* dst_cursor);

    static void* Insert(void* dht, void const* elem, void* dst_cursor);

    static void Erase(void* dht, void* pos_cursor);

    static void EraseAll(void* dht);

    static void CopyCursor(void const* dht, void const* cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void const* dht, void const* cursor_a,
                               void const* cursor_b);

    static void CursorStepL(void const* dht, void* cursor);

    static void CursorStepR(void const* dht, void* cursor);

    static bool CheckCntr(void const* dht);

    static bool CheckCursor(void const* dht, void const* cursor);

    static unsigned long long GetEffFactor(void* dht);

    static void Sanitize(void* dht, MemRecorder* dst_table,
                         MemRecorder* dst_node);
};

}  // namespace zeta::core
