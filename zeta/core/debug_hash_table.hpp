#pragma once

#include <unordered_set>
#include <zeta/core/assoc_cntr.hpp>

namespace zeta::core {

template <typename ElemHash, typename ElemCompare>
struct DebugHashTable;

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare>
struct DebugHashTable {
    static assoc_cntr::AssocCntrVTable const assoc_cntr_vtable;

    static constexpr bool elem_tag{ false };
    static constexpr bool key_tag{ true };

    using FnHash = assoc_cntr::FnHash;
    using FnCompare = assoc_cntr::FnCompare;

    using ElemKeyWrapper = Pair<bool, void const*>;

    struct ElemKeyHash {
        ElemHash elem_hash;

        mutable FnHash key_hash;

        unsigned long long operator()(ElemKeyWrapper const& a) const;
    };

    struct ElemKeyEq {
        ElemCompare elem_compare;

        mutable FnCompare key_elem_compare;

        bool operator()(Pair<bool, void const*> const& a,
                        Pair<bool, void const*> const& b) const;
    };

    using hash_table_t =
        std::unordered_multiset<ElemKeyWrapper, ElemKeyHash&, ElemKeyEq&>;

    // -------------------------------------------------------------------------

    size_t width;

    hash_table_t* hash_table;

    ElemKeyHash elem_key_hash;
    ElemKeyEq elem_key_eq;

    // -------------------------------------------------------------------------

    static void Init(void* debug_ht);

    static void Deinit(void* debug_ht);

    // -------------------------------------------------------------------------

    static size_t GetWidth(void const* debug_ht);

    static size_t GetSize(void const* debug_ht);

    static size_t GetCapacity(void const* debug_ht);

    // -------------------------------------------------------------------------

    static void GetRBCursor(void const* debug_ht, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* PeekL(void* debug_ht, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekL(void const* debug_ht, void* dst_cursor,
                                  void* dst_elem);

    static void* Refer(void* debug_ht, void const* pos_cursor);

    static void const* ConstRefer(void const* debug_ht, void const* pos_cursor);

    template <typename Key, typename KeyHash, typename KeyElemCompare>
    static void* TplFind(void* debug_ht, Key const& key,
                         KeyHash const& key_hash,
                         KeyElemCompare const& key_elem_compare,
                         void* dst_cursor);

    template <typename Key, typename KeyHash, typename KeyElemCompare>
    static void const* ConstTplFind(void const* debug_ht, Key const& key,
                                    KeyHash const& key_hash,
                                    KeyElemCompare const& key_elem_compare,
                                    void* dst_cursor);

    static void* Find(void* debug_ht, void const* key, FnHash const& key_hash,
                      FnCompare const& key_elem_compare, void* dst_cursor);

    static void const* ConstFind(void const* debug_ht, void const* key,
                                 FnHash const& key_hash,
                                 FnCompare const& key_elem_compare,
                                 void* dst_cursor);

    static void* Insert(void* debug_ht, void const* key, void* dst_cursor);

    static void Erase(void* debug_ht, void* pos_cursor);

    static void EraseAll(void* debug_ht);

    // -------------------------------------------------------------------------

    static void CopyCursor(void const* debug_ht, void const* cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void const* debug_ht, void const* cursor_a,
                               void const* cursor_b);

    static void CursorStepL(void const* debug_ht, void* cursor);

    static void CursorStepR(void const* debug_ht, void* cursor);

    // -------------------------------------------------------------------------

    static assoc_cntr::AssocCntrRef GetAsscocCntrRef(void* debug_ht);

    static assoc_cntr::ConstAssocCntrRef GetAsscocCntrRef(void const* debug_ht);

    // -------------------------------------------------------------------------

    static bool CheckCntr(void const* debug_ht);

    static bool CheckCursor(void const* debug_ht, void const* cursor);
};

}  // namespace zeta::core
