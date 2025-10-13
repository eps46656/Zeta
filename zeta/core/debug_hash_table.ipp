#pragma once

#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename ElemHash, typename ElemCompare>
unsigned long long
DebugHashTable<ElemHash, ElemCompare>::ElemKeyHash::operator()(
    ElemKeyWrapper const& a) const {
    ZETA_Core_DebugAssert(a.first == elem_tag || a.first == key_tag);

    return a.first == key_tag ? this->key_hash(a.second, 0)
                              : this->elem_hash(a.second, 0);
}

template <typename ElemHash, typename ElemCompare>
bool DebugHashTable<ElemHash, ElemCompare>::ElemKeyEq::operator()(
    ElemKeyWrapper const& a, ElemKeyWrapper const& b) const {
    ZETA_Core_DebugAssert((a.first == elem_tag && b.first == key_tag) ||
                          (a.first == key_tag && b.first == elem_tag) ||
                          (a.first == elem_tag && b.first == elem_tag));

    if (a.first == key_tag) {
        return this->key_elem_compare(a.second, b.second) == 0;
    }

    if (b.first == key_tag) {
        return this->key_elem_compare(a.second, b.second) == 0;
    }

    return this->elem_compare(a.second, b.second) == 0;
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::Init(void* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(debug_ht != nullptr);

    debug_ht->hash_table =
        new hash_table_t{ 0, debug_ht->elem_key_hash, debug_ht->elem_key_eq };
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::Deinit(void* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    hash_table_t* hash_table{ debug_ht->hash_table };

    delete hash_table;

    debug_ht->hash_table = nullptr;
}

template <typename ElemHash, typename ElemCompare>
size_t DebugHashTable<ElemHash, ElemCompare>::GetWidth(void const* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    return debug_ht->width;
}

template <typename ElemHash, typename ElemCompare>
size_t DebugHashTable<ElemHash, ElemCompare>::GetSize(void const* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    hash_table_t* hash_table{ debug_ht->hash_table };

    return hash_table->size();
}

template <typename ElemHash, typename ElemCompare>
size_t DebugHashTable<ElemHash, ElemCompare>::GetCapacity(
    void const* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    return ZETA_Core_max_capacity;
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::GetRBCursor(void const* debug_ht_,
                                                        void* dst_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    hash_table_t* hash_table{ debug_ht->hash_table };

    auto dst_cursor{ static_cast<hash_table_t::iterator*>(dst_cursor_) };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor) hash_table_t::iterator{ hash_table->end() };
}

template <typename ElemHash, typename ElemCompare>
void* DebugHashTable<ElemHash, ElemCompare>::PeekL(void* debug_ht_,
                                                   void* dst_cursor_,
                                                   void* dst_elem) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    auto dst_cursor{ static_cast<hash_table_t::iterator*>(dst_cursor_) };

    hash_table_t* hash_table{ debug_ht->hash_table };

    auto pos_cursor{ hash_table->begin() };

    if (dst_cursor != nullptr) {
        new (dst_cursor) hash_table_t::iterator{ pos_cursor };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr) {
        MemCopy(dst_elem, elem, debug_ht->width);
    }

    return elem;
}

template <typename ElemHash, typename ElemCompare>
void const* DebugHashTable<ElemHash, ElemCompare>::ConstPeekL(
    void const* debug_ht, void* dst_cursor, void* dst_elem) {
    return PeekL(const_cast<void*>(debug_ht), dst_cursor, dst_elem);
}

template <typename ElemHash, typename ElemCompare>
void* DebugHashTable<ElemHash, ElemCompare>::Refer(void* debug_ht_,
                                                   void const* pos_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    auto pos_cursor{ static_cast<hash_table_t::iterator const*>(pos_cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, pos_cursor));

    hash_table_t* hash_table{ debug_ht->hash_table };

    return *pos_cursor == hash_table->end()
               ? nullptr
               : const_cast<void*>((*pos_cursor)->second);
}

template <typename ElemHash, typename ElemCompare>
void const* DebugHashTable<ElemHash, ElemCompare>::ConstRefer(
    void const* debug_ht, void const* pos_cursor) {
    return Refer(const_cast<void*>(debug_ht), pos_cursor);
}

template <typename ElemHash, typename ElemCompare>
void* DebugHashTable<ElemHash, ElemCompare>::Find(
    void* debug_ht_, void const* key, FnHash const& key_hash,
    FnCompare const& key_elem_compare, void* dst_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    auto dst_cursor{ static_cast<hash_table_t::iterator*>(dst_cursor_) };

    hash_table_t* hash_table{ debug_ht->hash_table };

    hash_table->hash_function().key_hash = key_hash;
    hash_table->key_eq().key_elem_compare = key_elem_compare;

    auto pos_cursor{ hash_table->find(ElemKeyWrapper{ key_tag, key }) };

    if (dst_cursor != nullptr) {
        new (dst_cursor) hash_table_t::iterator{ pos_cursor };
    }

    return pos_cursor == hash_table->end()
               ? nullptr
               : const_cast<void*>(pos_cursor->second);
}

template <typename ElemHash, typename ElemCompare>
void const* DebugHashTable<ElemHash, ElemCompare>::ConstFind(
    void const* debug_ht, void const* key, FnHash const& key_hash,
    FnCompare const& key_elem_compare, void* dst_cursor) {
    return Find(const_cast<void*>(debug_ht), key, key_hash, key_elem_compare,
                dst_cursor);
}

template <typename ElemHash, typename ElemCompare>
template <typename Key, typename KeyHash, typename KeyElemCompare>
void* DebugHashTable<ElemHash, ElemCompare>::TplFind(
    void* debug_ht_, Key const& key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, void* dst_cursor_) {
    FnHash lambda_key_hash{
        &key_hash,
        [](void const* context, void const* key, unsigned long long salt) {
            auto const& key_hash{ *static_cast<KeyHash const*>(context) };
            return key_hash(*static_cast<Key const*>(key), salt);
        },
    };

    FnCompare lambda_key_elem_compare{
        &key_elem_compare,
        [](void const* context, void const* a, void const* b) {
            auto const& key_elem_compare{ *static_cast<KeyElemCompare const*>(
                context) };
            return key_elem_compare(*static_cast<Key const*>(a),
                                    *static_cast<Key const*>(b));
        },
    };

    return Find(debug_ht_, &key, lambda_key_hash, lambda_key_elem_compare,
                dst_cursor_);
}

template <typename ElemHash, typename ElemCompare>
template <typename Key, typename KeyHash, typename KeyElemCompare>
void const* DebugHashTable<ElemHash, ElemCompare>::ConstTplFind(
    void const* debug_ht, Key const& key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, void* dst_cursor) {
    return FindTemp(const_cast<void*>(debug_ht), key, key_hash,
                    key_elem_compare, dst_cursor);
}

template <typename ElemHash, typename ElemCompare>
void* DebugHashTable<ElemHash, ElemCompare>::Insert(void* debug_ht_,
                                                    void const* elem,
                                                    void* dst_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    auto dst_cursor{ static_cast<hash_table_t::iterator*>(dst_cursor_) };

    hash_table_t* hash_table{ debug_ht->hash_table };

    ElemKeyWrapper wrapper{ elem_tag, std::malloc(debug_ht->width) };
    std::memcpy(const_cast<void*>(wrapper.second), elem, debug_ht->width);

    auto pos_cursor{ hash_table->insert(wrapper) };

    if (dst_cursor != nullptr) {
        new (dst_cursor) hash_table_t::iterator{ pos_cursor };
    }

    return const_cast<void*>(pos_cursor->second);
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::Erase(void* debug_ht_,
                                                  void* pos_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    auto pos_cursor{ static_cast<hash_table_t::iterator*>(pos_cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, pos_cursor));

    hash_table_t* hash_table{ debug_ht->hash_table };

    ZETA_Core_DebugAssert(*pos_cursor != hash_table->end());

    void* elem{ const_cast<void*>((*pos_cursor)->second) };

    *pos_cursor = hash_table->erase(*pos_cursor);

    std::free(elem);
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::EraseAll(void* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    hash_table_t* hash_table{ debug_ht->hash_table };

    hash_table->clear();
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::CopyCursor(void const* debug_ht_,
                                                       void const* cursor_,
                                                       void* dst_cursor_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    auto cursor{ static_cast<hash_table_t::iterator const*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, cursor));

    auto dst_cursor{ static_cast<hash_table_t::iterator*>(dst_cursor_) };

    *dst_cursor = *cursor;
}

template <typename ElemHash, typename ElemCompare>
bool DebugHashTable<ElemHash, ElemCompare>::AreEqualCursor(
    void const* debug_ht_, void const* cursor_a_, void const* cursor_b_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    auto cursor_a{ static_cast<hash_table_t::iterator const*>(cursor_b_) };
    auto cursor_b{ static_cast<hash_table_t::iterator const*>(cursor_a_) };

    ZETA_Core_DebugAssert(CheckCursor(debug_ht, cursor_a));
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, cursor_b));

    return *cursor_a == *cursor_b;
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::CursorStepL(void const* debug_ht_,
                                                        void* cursor_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    auto cursor{ static_cast<hash_table_t::iterator*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, cursor));

    --(*cursor);
}

template <typename ElemHash, typename ElemCompare>
void DebugHashTable<ElemHash, ElemCompare>::CursorStepR(void const* debug_ht_,
                                                        void* cursor_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    auto cursor{ static_cast<hash_table_t::iterator*>(cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(debug_ht, cursor));

    ++(*cursor);
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare>
assoc_cntr::AssocCntrRef
DebugHashTable<ElemHash, ElemCompare>::GetAsscocCntrRef(void* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    return {
        .inst = debug_ht,

        .cursor_size = sizeof(typename hash_table_t::iterator),
        .width = debug_ht->width,
        .capacity = ZETA_Core_max_capacity,

        .vtable = &assoc_cntr_vtable,
    };
}

template <typename ElemHash, typename ElemCompare>
assoc_cntr::ConstAssocCntrRef
DebugHashTable<ElemHash, ElemCompare>::GetAsscocCntrRef(void const* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    ZETA_Core_DebugAssert(CheckCntr(debug_ht));

    return {
        .inst = const_cast<DebugHashTable*>(debug_ht),

        .cursor_size = sizeof(hash_table_t::iterator),
        .width = debug_ht->width,
        .capacity = ZETA_Core_max_capacity,

        .vtable = &assoc_cntr_vtable,
    };
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare>
bool DebugHashTable<ElemHash, ElemCompare>::CheckCntr(void const* debug_ht_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    if (!(debug_ht != nullptr)) { return false; }

    hash_table_t* hash_table{ debug_ht->hash_table };
    if (!(hash_table != nullptr)) { return false; }

    return true;
}

template <typename ElemHash, typename ElemCompare>
bool DebugHashTable<ElemHash, ElemCompare>::CheckCursor(void const* debug_ht_,
                                                        void const* cursor_) {
    auto debug_ht{ static_cast<DebugHashTable const*>(debug_ht_) };
    if (!CheckCntr(debug_ht)) { return false; }

    auto cursor{ static_cast<hash_table_t::iterator const*>(cursor_) };
    if (!(cursor != nullptr)) { return false; }

    return true;
}

// -----------------------------------------------------------------------------

template <typename ElemHash, typename ElemCompare>
assoc_cntr::AssocCntrVTable const
    DebugHashTable<ElemHash, ElemCompare>::assoc_cntr_vtable{
        .Deinit = &DebugHashTable::Deinit,

        .GetSize = &DebugHashTable::GetSize,
        .GetCapacity = &DebugHashTable::GetCapacity,

        .GetLBCursor = nullptr,
        .GetRBCursor = &DebugHashTable::GetRBCursor,

        .PeekL = &DebugHashTable::PeekL,
        .ConstPeekL = &DebugHashTable::ConstPeekL,

        .PeekR = nullptr,
        .ConstPeekR = nullptr,

        .Refer = &DebugHashTable::Refer,
        .ConstRefer = &DebugHashTable::ConstRefer,

        .Find = &DebugHashTable::Find,
        .ConstFind = &DebugHashTable::ConstFind,

        .Insert = &DebugHashTable::Insert,
        .Erase = &DebugHashTable::Erase,
        .EraseAll = &DebugHashTable::EraseAll,

        .CopyCursor = &DebugHashTable::CopyCursor,
        .AreEqualCursor = &DebugHashTable::AreEqualCursor,
        .CompareCursor = nullptr,
        .GetCursorDist = nullptr,
        .GetCursorIdx = nullptr,
        .CursorStepL = &DebugHashTable::CursorStepL,
        .CursorStepR = &DebugHashTable::CursorStepR,
        .CursorAdvanceL = nullptr,
        .CursorAdvanceR = nullptr,
    };

}  // namespace zeta::core
