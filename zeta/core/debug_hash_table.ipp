#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename ElemHashLike, typename ElemCompareLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ElemHashLike, ElemCompareLike

namespace zeta::core {

namespace debug_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);
    ZETA_Core_DebugAssert(cntr->hash_table != nullptr);
}

template <CntrTplParamList>
void CheckCursor_(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor) {
    (CheckCntr_)(cntr);
    ZETA_Core_DebugAssert(cursor != nullptr);
}

}  // namespace debug_hash_table::detail

template <typename ElemHashLike>
unsigned long long debug_hash_table::ElemKeyHashProxy<ElemHashLike>::operator()(
    ElemKeyWrapper const& a) const {
    ZETA_Core_DebugAssert(a.first == elem_tag || a.first == key_tag);

    return a.first == key_tag ? this->key_hash(a.second, 0)
                              : this->elem_hash(a.second, 0);
}

template <typename ElemHashLike>
bool debug_hash_table::ElemKeyEqProxy<ElemHashLike>::operator()(
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

template <CntrTplParamList>
void debug_hash_table::Init(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    cntr->hash_table = new hash_table_t<ElemHashLike, ElemCompareLike>{
        0, cntr->elem_key_hash_proxy, cntr->elem_key_eq_proxy
    };
}

template <CntrTplParamList>
void debug_hash_table::Deinit(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    delete hash_table;

    cntr->hash_table = nullptr;
}

template <CntrTplParamList>
size_t debug_hash_table::GetCursorSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(
        typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator);
}

template <CntrTplParamList>
size_t debug_hash_table::GetWidth(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->width;
}

template <CntrTplParamList>
size_t debug_hash_table::GetSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    return hash_table->size();
}

template <CntrTplParamList>
size_t debug_hash_table::GetCapacity(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void debug_hash_table::GetRBCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
        dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor) hash_table_t<ElemHashLike, ElemCompareLike>::iterator{
        hash_table->end()
    };
}

template <CntrTplParamList>
void* debug_hash_table::PeekL(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    auto pos_cursor{ hash_table->begin() };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::PeekR(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    auto pos_cursor{ hash_table->end() };
    --pos_cursor;

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::Derefer(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr->hash_table };

    void* elem{ *pos_cursor == hash_table->end()
                    ? nullptr
                    : const_cast<void*>((*pos_cursor)->second) };

    if (elem != nullptr && dst_elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* debug_hash_table::Find(
    Cntr<CntrTplArgList> const* cntr, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    auto* hash_table{ cntr->hash_table };

    hash_table->hash_function().key_hash = key_hash;
    hash_table->key_eq().key_elem_compare = key_elem_compare;

    auto pos_cursor{ hash_table->find(ElemKeyWrapper{ key_tag, key }) };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    void* elem{ pos_cursor == hash_table->end()
                    ? nullptr
                    : const_cast<void*>(pos_cursor->second) };

    if (elem != nullptr && dst_elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::Insert(
    Cntr<CntrTplArgList>* cntr, void const* elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
        dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    ElemKeyWrapper wrapper{ elem_tag, std::malloc(cntr->width) };
    std::memcpy(const_cast<void*>(wrapper.second), elem, cntr->width);

    auto pos_cursor{ hash_table->insert(wrapper) };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    return const_cast<void*>(pos_cursor->second);
}

template <CntrTplParamList>
void debug_hash_table::PopL(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    ZETA_Core_DebugAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->begin() };

        void* elem{ const_cast<void*>(pos_cursor->second) };

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
void debug_hash_table::PopR(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    ZETA_Core_DebugAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->end() };
        --pos_cursor;

        void* elem{ const_cast<void*>(pos_cursor->second) };

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
void debug_hash_table::Erase(
    Cntr<CntrTplArgList>* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
        pos_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr->hash_table };

    ZETA_Core_DebugAssert(*pos_cursor != hash_table->end());

    void* elem{ const_cast<void*>((*pos_cursor)->second) };

    *pos_cursor = hash_table->erase(*pos_cursor);

    std::free(elem);
}

template <CntrTplParamList>
void debug_hash_table::EraseAll(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr->hash_table };

    hash_table->clear();
}

template <CntrTplParamList>
void debug_hash_table::CopyCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        src_cursor,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
        dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool debug_hash_table::AreEqualCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor_a,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor_b) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return *cursor_a == *cursor_b;
}

template <CntrTplParamList>
void debug_hash_table::CursorStepL(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor) {
    detail::CheckCursor_(cntr, cursor);

    --(*cursor);
}

template <CntrTplParamList>
void debug_hash_table::CursorStepR(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor) {
    detail::CheckCursor_(cntr, cursor);

    ++(*cursor);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::GetReferedInst(debug_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return const_cast<debug_hash_table::Cntr<CntrTplArgList>::Cntr*>(cntr);
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>,
                       void>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,
        .GetWidth = true,
        .GetSize = true,
        .GetCapacity = true,
        .GetLBCursor = false,
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
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const,
                       void>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>,
                                  void>::GetStaticEnabledAbilityFlag() &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>,
                       void>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = false,
        .GetWidth = false,
        .GetSize = false,
        .GetCapacity = false,
        .GetLBCursor = true,
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
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const,
                       void>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>,
                                  void>::GetStaticDisabledAbilityFlag() |
           assoc_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::
    GetDynamicEnabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::
    GetDynamicDisabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const*) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr size_t assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::GetCursorSize(debug_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return debug_hash_table::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::GetWidth(debug_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return debug_hash_table::GetWidth(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::GetSize(debug_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return debug_hash_table::GetSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::GetCapacity(debug_hash_table::Cntr<CntrTplArgList> const* cntr) {
    return debug_hash_table::GetCapacity(cntr);
}

template <CntrTplParamList>
void assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::GetRBCursor(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr, void* dst_cursor) {
    debug_hash_table::GetRBCursor(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::PeekL(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                 bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::PeekL(
        cntr, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::PeekR(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                 bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::PeekR(
        cntr, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::Derefer(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr,
        void const* pos_cursor, bool lazy_copy_elem, void* dst_elem) {
    return debug_hash_table::Derefer(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::Find(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr, void const* key,
        KeyHash const& key_hash, KeyElemCompare const& key_elem_compare,
        bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::Find(
        cntr, key, key_hash, key_elem_compare, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>::
    Insert(debug_hash_table::Cntr<CntrTplArgList>* cntr, void const* elem,
           void* dst_cursor) {
    return debug_hash_table::Insert(
        cntr, elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>::PopL(
    debug_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    debug_hash_table::PopL(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>::PopR(
    debug_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    debug_hash_table::PopR(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>::
    Erase(debug_hash_table::Cntr<CntrTplArgList>* cntr, void* pos_cursor) {
    debug_hash_table::Erase(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHashLike, ElemCompareLike>::iterator*>(pos_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>::
    EraseAll(debug_hash_table::Cntr<CntrTplArgList>* cntr) {
    debug_hash_table::EraseAll(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::CopyCursor(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                      void const* src_cursor, void* dst_cursor) {
    debug_hash_table::CopyCursor(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator const*>(src_cursor),
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
bool assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList> const,
    void>::AreEqualCursor(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                          void const* cursor_a, void const* cursor_b) {
    return debug_hash_table::AreEqualCursor(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator const*>(cursor_a),
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHashLike, ElemCompareLike>::iterator const*>(cursor_b));
}

template <CntrTplParamList>
void assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::CursorStepL(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr, void* cursor) {
    debug_hash_table::CursorStepL(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHashLike, ElemCompareLike>::iterator*>(cursor));
}

template <CntrTplParamList>
void assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const, void>::CursorStepR(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr, void* cursor) {
    debug_hash_table::CursorStepR(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHashLike, ElemCompareLike>::iterator*>(cursor));
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
