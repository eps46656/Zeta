#pragma once

#include <vcruntime_string.h>

#include <cstdlib>
#include <cstring>
#include <new>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename ElemHasherLike, typename ElemComparatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ElemHasherLike, ElemComparatorLike

namespace zeta::core {

namespace debug_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    ZETA_Core_DebugAssert(cntr.hash_table != nullptr);
}

template <CntrTplParamList>
void CheckCursor_(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator const*
        cursor) {
    (CheckCntr_)(cntr);
    ZETA_Core_DebugAssert(cursor != nullptr);
}

}  // namespace debug_hash_table::detail

template <typename ElemHasherLike>
unsigned long long
debug_hash_table::ElemKeyHasherProxy<ElemHasherLike>::operator()(
    ElemKeyWrapper const& a) const {
    ZETA_Core_DebugAssert(a.first == elem_tag || a.first == key_tag);

    return a.first == key_tag ? this->key_hasher(a.second, 0)
                              : this->elem_hasher(a.second, 0);
}

template <typename ElemHasherLike>
bool debug_hash_table::ElemKeyEqProxy<ElemHasherLike>::operator()(
    ElemKeyWrapper const& a, ElemKeyWrapper const& b) const {
    ZETA_Core_DebugAssert((a.first == elem_tag && b.first == key_tag) ||
                          (a.first == key_tag && b.first == elem_tag) ||
                          (a.first == elem_tag && b.first == elem_tag));

    if (a.first == key_tag) {
        return this->key_elem_cmptr(a.second, b.second) == 0;
    }

    if (b.first == key_tag) {
        return this->key_elem_cmptr(a.second, b.second) == 0;
    }

    return this->elem_cmptr(a.second, b.second) == 0;
}

template <CntrTplParamList>
void debug_hash_table::Init(Cntr<CntrTplArgList>& cntr) {
    cntr.hash_table = new hash_table_t<ElemHasherLike, ElemComparatorLike>{
        0, cntr.elem_key_hasher_proxy, cntr.elem_key_eq_proxy
    };
}

template <CntrTplParamList>
void debug_hash_table::Deinit(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    delete hash_table;

    cntr.hash_table = nullptr;
}

template <CntrTplParamList>
size_t debug_hash_table::GetCursorSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(
        typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator);
}

template <CntrTplParamList>
size_t debug_hash_table::GetElemSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

template <CntrTplParamList>
size_t debug_hash_table::GetElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    return hash_table->size();
}

template <CntrTplParamList>
size_t debug_hash_table::GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void debug_hash_table::GetRBCursor(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor) hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator{
        hash_table->end()
    };
}

template <CntrTplParamList>
void* debug_hash_table::PeekL(
    Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    auto pos_cursor{ hash_table->begin() };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator{
                pos_cursor
            };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::PeekR(
    Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    auto pos_cursor{ hash_table->end() };
    --pos_cursor;

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator{
                pos_cursor
            };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::Derefer(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr.hash_table };

    void* elem{ *pos_cursor == hash_table->end()
                    ? nullptr
                    : const_cast<void*>((*pos_cursor)->second) };

    if (elem != nullptr && dst_elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList, typename KeyHash, typename KeyElemComparator>
void* debug_hash_table::Find(
    Cntr<CntrTplArgList> const& cntr, void const* key,
    KeyHash const& key_hasher, KeyElemComparator const& key_elem_cmptr,
    bool lazy_copy_elem,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor,
    void* dst_elem) {
    auto* hash_table{ cntr.hash_table };

    hash_table->hash_function().key_hasher = key_hasher;
    hash_table->key_eq().key_elem_cmptr = key_elem_cmptr;

    auto pos_cursor{ hash_table->find(ElemKeyWrapper{ key_tag, key }) };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator{
                pos_cursor
            };
    }

    void* elem{ pos_cursor == hash_table->end()
                    ? nullptr
                    : const_cast<void*>(pos_cursor->second) };

    if (elem != nullptr && dst_elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }

    return elem;
}

template <CntrTplParamList>
void* debug_hash_table::Insert(
    Cntr<CntrTplArgList>& cntr, void const* elem,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    ElemKeyWrapper wrapper{ elem_tag, std::malloc(cntr.elem_size) };
    std::memcpy(const_cast<void*>(wrapper.second), elem, cntr.elem_size);

    auto pos_cursor{ hash_table->insert(wrapper) };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator{
                pos_cursor
            };
    }

    return const_cast<void*>(pos_cursor->second);
}

template <CntrTplParamList>
void debug_hash_table::PopL(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    ZETA_Core_DebugAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->begin() };

        void* elem{ const_cast<void*>(pos_cursor->second) };

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
void debug_hash_table::PopR(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

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
    Cntr<CntrTplArgList>& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        pos_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr.hash_table };

    ZETA_Core_DebugAssert(*pos_cursor != hash_table->end());

    void* elem{ const_cast<void*>((*pos_cursor)->second) };

    *pos_cursor = hash_table->erase(*pos_cursor);

    std::free(elem);
}

template <CntrTplParamList>
void debug_hash_table::EraseAll(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    hash_table->clear();
}

template <CntrTplParamList>
void debug_hash_table::CopyCursor(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator const*
        src_cursor,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool debug_hash_table::AreEqualCursor(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator const*
        cursor_a,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator const*
        cursor_b) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return *cursor_a == *cursor_b;
}

template <CntrTplParamList>
void debug_hash_table::CursorStepL(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        cursor) {
    detail::CheckCursor_(cntr, cursor);

    --(*cursor);
}

template <CntrTplParamList>
void debug_hash_table::CursorStepR(
    Cntr<CntrTplArgList> const& cntr,
    typename hash_table_t<ElemHasherLike, ElemComparatorLike>::iterator*
        cursor) {
    detail::CheckCursor_(cntr, cursor);

    ++(*cursor);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetReferedInstPtr(debug_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return const_cast<debug_hash_table::Cntr<CntrTplArgList>::Cntr*>(cntr);
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList>>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,
        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,
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
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<debug_hash_table::Cntr<
    CntrTplArgList> const>::GetStaticEnabledAbilityFlag() {
    return assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::
               GetStaticEnabledAbilityFlag() &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList>>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::AbilityFlagBuilder{
        .GetCursorSize = false,
        .GetElemSize = false,
        .GetElemCnt = false,
        .GetMaxElemCnt = false,
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
constexpr assoc_cntr::AbilityFlag assoc_cntr::CntrTraits<debug_hash_table::Cntr<
    CntrTplArgList> const>::GetStaticDisabledAbilityFlag() {
    return assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::
               GetStaticDisabledAbilityFlag() |
           assoc_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr size_t assoc_cntr::
    CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::GetCursorSize(
        debug_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return debug_hash_table::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetElemSize(debug_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return debug_hash_table::GetElemSize(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetElemCnt(debug_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return debug_hash_table::GetElemCnt(cntr);
}

template <CntrTplParamList>
size_t assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetMaxElemCnt(debug_hash_table::Cntr<CntrTplArgList> const& cntr) {
    return debug_hash_table::GetMaxElemCnt(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetRBCursor(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* dst_cursor) {
    debug_hash_table::GetRBCursor(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    PeekL(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
          bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::PeekL(
        cntr, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    PeekR(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
          bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::PeekR(
        cntr, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    Derefer(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
            void const* pos_cursor, bool lazy_copy_elem, void* dst_elem) {
    return debug_hash_table::Derefer(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator const*>(pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    Find(debug_hash_table::Cntr<CntrTplArgList> const& cntr, void const* key,
         KeyHash const& key_hash, KeyElemCompare const& key_elem_compare,
         bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return debug_hash_table::Find(
        cntr, key, key_hash, key_elem_compare, lazy_copy_elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::Insert(
    debug_hash_table::Cntr<CntrTplArgList>& cntr, void const* elem,
    void* dst_cursor) {
    return debug_hash_table::Insert(
        cntr, elem,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::PopL(
    debug_hash_table::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    debug_hash_table::PopL(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::PopR(
    debug_hash_table::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    debug_hash_table::PopR(cntr, cnt);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::Erase(
    debug_hash_table::Cntr<CntrTplArgList>& cntr, void* pos_cursor) {
    debug_hash_table::Erase(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHasherLike, ElemComparatorLike>::iterator*>(pos_cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::EraseAll(
    debug_hash_table::Cntr<CntrTplArgList>& cntr) {
    debug_hash_table::EraseAll(cntr);
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    CopyCursor(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
               void const* src_cursor, void* dst_cursor) {
    debug_hash_table::CopyCursor(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator const*>(src_cursor),
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator*>(dst_cursor));
}

template <CntrTplParamList>
bool assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    AreEqualCursor(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
                   void const* cursor_a, void const* cursor_b) {
    return debug_hash_table::AreEqualCursor(
        cntr,
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator const*>(cursor_a),
        static_cast<typename debug_hash_table::hash_table_t<
            ElemHasherLike, ElemComparatorLike>::iterator const*>(cursor_b));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    CursorStepL(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    debug_hash_table::CursorStepL(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHasherLike, ElemComparatorLike>::iterator*>(cursor));
}

template <CntrTplParamList>
void assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    CursorStepR(debug_hash_table::Cntr<CntrTplArgList> const& cntr,
                void* cursor) {
    debug_hash_table::CursorStepR(
        cntr, static_cast<typename debug_hash_table::hash_table_t<
                  ElemHasherLike, ElemComparatorLike>::iterator*>(cursor));
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
