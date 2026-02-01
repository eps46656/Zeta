#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")

#define CntrTplParamList typename ElemHashLike, typename ElemCompareLike

#define CntrTplArgList ElemHashLike, ElemCompareLike

namespace zeta::core::debug_hash_table {

template <typename ElemHashLike>
unsigned long long ElemKeyHashProxy<ElemHashLike>::operator()(
    ElemKeyWrapper const& a) const {
    ZETA_Core_DebugAssert(a.first == elem_tag || a.first == key_tag);

    return a.first == key_tag ? this->key_hash(a.second, 0)
                              : this->elem_hash(a.second, 0);
}

template <typename ElemHashLike>
bool ElemKeyEqProxy<ElemHashLike>::operator()(ElemKeyWrapper const& a,
                                              ElemKeyWrapper const& b) const {
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

namespace ops {

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    cntr->hash_table = new hash_table_t<ElemHashLike, ElemCompareLike>{
        0, cntr->elem_key_hash_proxy, cntr->elem_key_eq_proxy
    };
}

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    delete hash_table;

    cntr->hash_table = nullptr;
}

template <CntrTplParamList>
size_t GetCursorSize(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return sizeof(
        typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator);
}

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return cntr->width;
}

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    return hash_table->size();
}

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr,
                 typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
                     dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor) hash_table_t<ElemHashLike, ElemCompareLike>::iterator{
        hash_table->end()
    };
}

template <CntrTplParamList>
void* PeekL(
    Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    auto pos_cursor{ hash_table->begin() };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* PeekL(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    return PeekL(const_cast<Cntr<CntrTplArgList>*>(cntr), lazy_copy_elem,
                 dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* PeekR(
    Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    auto pos_cursor{ hash_table->end() };
    --pos_cursor;

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<ElemHashLike, ElemCompareLike>::iterator{ pos_cursor };
    }

    void* elem{ const_cast<void*>(pos_cursor->second) };

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* PeekR(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    return PeekR(const_cast<Cntr<CntrTplArgList>*>(cntr), lazy_copy_elem,
                 dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* Derefer(
    Cntr<CntrTplArgList>* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    auto* hash_table{ cntr->hash_table };

    void* elem{ *pos_cursor == hash_table->end()
                    ? nullptr
                    : const_cast<void*>((*pos_cursor)->second) };

    if (elem != nullptr && dst_elem != nullptr && !lazy_copy_elem) {
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList>
void const* Derefer(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem) {
    return Derefer(const_cast<void*>(cntr), pos_cursor, lazy_copy_elem,
                   dst_elem);
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(
    Cntr<CntrTplArgList>* cntr, void const* key, KeyHash const& key_hash,
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
        MemCopy(dst_elem, elem, cntr->width);
    }

    return elem;
}

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void const* Find(
    Cntr<CntrTplArgList> const* cntr, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem) {
    return Find(const_cast<Cntr<CntrTplArgList>*>(cntr), key, key_hash,
                key_elem_compare, lazy_copy_elem, dst_cursor, dst_elem);
}

template <CntrTplParamList>
void* Insert(Cntr<CntrTplArgList>* cntr, void const* elem,
             typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
                 dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

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
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

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
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

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
void Erase(Cntr<CntrTplArgList>* cntr,
           typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
               pos_cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    auto* hash_table{ cntr->hash_table };

    ZETA_Core_DebugAssert(*pos_cursor != hash_table->end());

    void* elem{ const_cast<void*>((*pos_cursor)->second) };

    *pos_cursor = hash_table->erase(*pos_cursor);

    std::free(elem);
}

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* hash_table{ cntr->hash_table };

    hash_table->clear();
}

// -----------------------------------------------------------------------------

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor_,
                typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*
                    dst_cursor) {
    auto cursor{ static_cast<
        hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*>(
        cursor_) };
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    *dst_cursor = *cursor;
}

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor_a_,
                    void const* cursor_b_) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto cursor_a{ static_cast<
        hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*>(
        cursor_b_) };
    auto cursor_b{ static_cast<
        hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*>(
        cursor_a_) };

    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor_a));
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor_b));

    return *cursor_a == *cursor_b;
}

template <CntrTplParamList>
void CursorStepL(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    --(*cursor);
}

template <CntrTplParamList>
void CursorStepR(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    ++(*cursor);
}

// -----------------------------------------------------------------------------

template <CntrTplParamList>
bool CheckCntr(Cntr<CntrTplArgList> const* cntr) {
    if (!(cntr != nullptr)) { return false; }

    auto* hash_table{ cntr->hash_table };
    if (!(hash_table != nullptr)) { return false; }

    return true;
}

template <CntrTplParamList>
bool CheckCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor) {
    if (!CheckCntr(cntr)) { return false; }

    if (!(cursor != nullptr)) { return false; }

    return true;
}

}  // namespace ops

template <CntrTplParamList>
constexpr bool AssocCntrView<CntrTplArgList>::IsConst(
    type_wrapper::TypeWrapper<AssocCntrView*>) {
    return false;
}

template <CntrTplParamList>
constexpr bool AssocCntrView<CntrTplArgList>::IsConst(
    type_wrapper::TypeWrapper<AssocCntrView const*>) {
    return true;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView*>) {
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
AssocCntrView<CntrTplArgList>::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView const*>) {
    return GetStaticEnabledAbilityFlag(
               type_wrapper::TypeWrapper<AssocCntrView*>()) &
           assoc_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::AbilityFlag
AssocCntrView<CntrTplArgList>::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrView*>) {
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
void AssocCntrView<CntrTplArgList>::GetRBCursor(AssocCntrView const* cntr_view,
                                                void* dst_cursor) {
    return ops::GetRBCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor));
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::PeekL(AssocCntrView* cntr_view,
                                           bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return ops::PeekL(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::PeekL(AssocCntrView const* cntr_view,
                                                 bool lazy_copy_elem,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    return ops::PeekL(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::PeekR(AssocCntrView* cntr_view,
                                           bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return ops::PeekR(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::PeekR(AssocCntrView const* cntr_view,
                                                 bool lazy_copy_elem,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    return ops::PeekR(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::Derefer(AssocCntrView* cntr_view,
                                             void const* pos_cursor,
                                             bool lazy_copy_elem,
                                             void* dst_elem) {
    return ops::Derefer(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
        static_cast<typename hash_table_t<ElemHashLike,
                                          ElemCompareLike>::iterator const*>(
            pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
void const* AssocCntrView<CntrTplArgList>::Derefer(
    AssocCntrView const* cntr_view, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return ops::Derefer(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<typename hash_table_t<ElemHashLike,
                                          ElemCompareLike>::iterator const*>(
            pos_cursor),
        lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void* AssocCntrView<CntrTplArgList>::Find(
    AssocCntrView* cntr_view, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return ops::Find(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), key, key_hash,
        key_elem_compare, lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
template <typename KeyHash, typename KeyElemCompare>
void const* AssocCntrView<CntrTplArgList>::Find(
    AssocCntrView const* cntr_view, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return ops::Find(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view), key, key_hash,
        key_elem_compare, lazy_copy_elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor),
        dst_elem);
}

template <CntrTplParamList>
void* AssocCntrView<CntrTplArgList>::Insert(AssocCntrView* cntr_view,
                                            void const* elem,
                                            void* dst_cursor) {
    return ops::Insert(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), elem,
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::PopL(AssocCntrView* cntr_view, size_t cnt) {
    ops::PopL(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), cnt);
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::PopR(AssocCntrView* cntr_view, size_t cnt) {
    ops::PopR(reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view), cnt);
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::Erase(AssocCntrView* cntr_view,
                                          void* pos_cursor) {
    return ops::Erase(
        reinterpret_cast<Cntr<CntrTplArgList>*>(cntr_view),
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            pos_cursor));
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
        static_cast<typename hash_table_t<ElemHashLike,
                                          ElemCompareLike>::iterator const*>(
            cursor),
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            dst_cursor));
}

template <CntrTplParamList>
bool AssocCntrView<CntrTplArgList>::AreEqualCursor(
    AssocCntrView const* cntr_view, void const* cursor_a,
    void const* cursor_b) {
    return ops::AreEqualCursor(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<typename hash_table_t<ElemHashLike,
                                          ElemCompareLike>::iterator const*>(
            cursor_a),
        static_cast<typename hash_table_t<ElemHashLike,
                                          ElemCompareLike>::iterator const*>(
            cursor_b));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::CursorStepL(AssocCntrView const* cntr_view,
                                                void* cursor) {
    return ops::CursorStepL(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            cursor));
}

template <CntrTplParamList>
void AssocCntrView<CntrTplArgList>::CursorStepR(AssocCntrView const* cntr_view,
                                                void* cursor) {
    return ops::CursorStepR(
        reinterpret_cast<Cntr<CntrTplArgList> const*>(cntr_view),
        static_cast<
            typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator*>(
            cursor));
}

}  // namespace zeta::core::debug_hash_table
