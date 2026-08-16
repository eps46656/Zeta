#pragma once

#include <vcruntime_string.h>

#include <cstdlib>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/comparison_ref.ipp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename HasherLike, typename ComparatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList HasherLike, ComparatorLike

namespace zeta::core {

namespace debug_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    ZETA_Core_DebugAssert(cntr.hash_table != nullptr);
}

template <CntrTplParamList>
void CheckCursor_(Cntr<CntrTplArgList> const& cntr,
                  typename Cntr<CntrTplArgList>::Cursor const* cursor) {
    (CheckCntr_)(cntr);
    ZETA_Core_DebugAssert(cursor != nullptr);
}

}  // namespace debug_hash_table::detail

template <typename HasherLike>
unsigned long long debug_hash_table::HasherProxy<HasherLike>::operator()(
    ObjWrapper const& a_) const {
    void const* a{ a_.obj };

    return a == this->cur_key ? this->key_hash_func(this->key_hasher, a)
                              : this->elem_hash_func(&this->elem_hasher, a);
}

template <typename HasherLike>
bool debug_hash_table::EqProxy<HasherLike>::operator()(
    ObjWrapper const& a_, ObjWrapper const& b_) const {
    void const* a{ a_.obj };
    void const* b{ b_.obj };

    bool a_is_key{ a == this->cur_key };
    bool b_is_key{ b == this->cur_key };

    if (a_is_key && b_is_key) { return true; }

    if (a_is_key && !b_is_key) {
        return this->key_elem_eq_func(this->key_elem_cmptr, a, b);
    }

    if (!a_is_key && b_is_key) {
        return this->key_elem_eq_func(this->key_elem_cmptr, b, a);
    }

    return this->elem_eq_func(this->key_elem_cmptr, a, b);
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Init(this Cntr& cntr) {
    cntr.hasher_proxy.elem_hash_func = [](void const* elem_hasher,
                                          void const* elem) {
        return hash::Hash(
            *static_cast<meta::RemoveRef<decltype(meta::GetInstRef(
                cntr.hasher_proxy.elem_hasher))> const*>(elem_hasher),
            elem, 0);
    };

    cntr.eq_proxy.elem_eq_func = [](void const* elem_cmptr, void const* elem_a,
                                    void const* elem_b) {
        return comparison::Compare(
            *static_cast<meta::RemoveRef<decltype(meta::GetInstRef(
                cntr.eq_proxy.elem_cmptr))> const*>(elem_cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Equal>{}, elem_a,
            elem_b);
    };

    cntr.hash_table =
        new hash_table_t<HasherLike, ComparatorLike>{ 0, cntr.hasher_proxy,
                                                      cntr.eq_proxy };
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Deinit(this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    delete hash_table;

    cntr.hash_table = nullptr;
}

template <CntrTplParamList>
constexpr void* debug_hash_table::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);
    return const_cast<void*>(static_cast<void const*>(&cntr));
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetCursorSize(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    return hash_table->size();
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& cntr, Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor)
        hash_table_t<HasherLike, ComparatorLike>::iterator{ hash_table->end() };
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PeekL(
    this auto& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    auto pos_cursor{ hash_table->begin() };

    void* elem{ hash_table->empty() ? nullptr : pos_cursor->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<HasherLike, ComparatorLike>::iterator{ pos_cursor };
    }

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PeekR(
    this auto& cntr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    auto pos_cursor{ hash_table->end() };

    void* elem;

    if (hash_table->empty()) {
        elem = nullptr;
    } else {
        --pos_cursor;
        elem = pos_cursor->obj;
    }

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<HasherLike, ComparatorLike>::iterator{ pos_cursor };
    }

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Derefer(
    this auto& cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr.hash_table };

    void* elem{ *pos_cursor == hash_table->end() ? nullptr
                                                 : (*pos_cursor)->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& cntr, void const* elem, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    auto* hash_table{ cntr.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    cntr.Find(elem, hasher_wrapper.elem_hasher, eq_wrapper.elem_cmptr,
              lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    auto* hash_table{ cntr.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    hasher_wrapper.key_hasher = &key_hasher;

    hasher_wrapper.key_hash_func = [](void const* key_hasher, void const* key) {
        return hash::Hash(*static_cast<KeyHasher const*>(key_hasher), key, 0);
    };

    hasher_wrapper.cur_key = key;

    eq_wrapper.key_elem_cmptr = &key_elem_cmptr;

    eq_wrapper.key_elem_eq_func = [](void const* key_elem_cmptr,
                                     void const* key, void const* elem) {
        return comparison::Compare(
            *static_cast<KeyElemComparator const*>(key_elem_cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Equal>{}, key, elem);
    };

    eq_wrapper.cur_key = key;

    auto pos_cursor{ hash_table->find(ObjWrapper{ const_cast<void*>(key) }) };

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<HasherLike, ComparatorLike>::iterator{ pos_cursor };
    }

    void* elem{ pos_cursor == hash_table->end() ? nullptr : pos_cursor->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(cntr)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? assoc_cntr::ElemPtrView::AliasabilityEnum::Null
                    : assoc_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<HasherLike, ComparatorLike>::iterator{ pos_cursor };
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, cntr.elem_size);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsWriter Writer>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& cntr, void const* elem, Writer&& writer, Cursor* dst_cursor) {
    auto* hash_table{ cntr.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    cntr.Insert(elem, hasher_wrapper.elem_hasher, eq_wrapper.elem_cmptr, writer,
                dst_cursor);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Writer&& writer,
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    HashTable* hash_table{ cntr.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    hasher_wrapper.key_hasher = &key_hasher;

    hasher_wrapper.key_hash_func = [](void const* key_hasher, void const* key) {
        return hash::Hash(*static_cast<KeyHasher const*>(key_hasher), key, 0);
    };

    hasher_wrapper.cur_key = key;

    eq_wrapper.key_elem_cmptr = &key_elem_cmptr;

    eq_wrapper.key_elem_eq_func = [](void const* key_elem_cmptr,
                                     void const* key,
                                     void const* elem_wrapper) {
        return comparison::Compare(
            *static_cast<KeyElemComparator const*>(key_elem_cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Equal>{}, key,
            static_cast<ObjWrapper const*>(elem_wrapper)->obj);
    };

    eq_wrapper.cur_key = key;

    auto pos_cursor{ hash_table->insert(ObjWrapper{ const_cast<void*>(key) }) };

    pos_cursor->obj = std::malloc(cntr.elem_size);

    elem_stream::provider::Transfer(writer, pos_cursor->obj, cntr.elem_size,
                                    cntr.elem_size, 1);

    if (dst_cursor != nullptr) {
        new (dst_cursor)
            hash_table_t<HasherLike, ComparatorLike>::iterator{ pos_cursor };
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PopL(this Cntr& cntr,
                                                            size_t cnt,
                                                            Reader&& reader) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    ZETA_Core_DebugAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->begin() };

        void* elem{ pos_cursor->obj };

        elem_stream::acceptor::Transfer(reader, elem, cntr.elem_size,
                                        cntr.elem_size, 1);

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PopR(this Cntr& cntr,
                                                            size_t cnt,
                                                            Reader&& reader) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    ZETA_Core_DebugAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->end() };
        --pos_cursor;

        void* elem{ pos_cursor->obj };

        elem_stream::acceptor::Transfer(reader, elem, cntr.elem_size,
                                        cntr.elem_size, 1);

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader Reader>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Erase(this Cntr& cntr,
                                                             Cursor* pos_cursor,
                                                             size_t cnt,
                                                             Reader&& reader) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto* hash_table{ cntr.hash_table };

    for (; 0 < cnt; ++cnt) {
        ZETA_Core_DebugAssert(*pos_cursor != hash_table->end());

        void* elem{ (*pos_cursor)->obj };

        elem_stream::acceptor::Transfer(reader, elem, cntr.elem_size,
                                        cntr.elem_size, 1);

        *pos_cursor = hash_table->erase(*pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::EraseAll(
    this Cntr& cntr) {
    detail::CheckCntr_(cntr);

    auto* hash_table{ cntr.hash_table };

    hash_table->clear();
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& cntr, Cursor const* src_cursor, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool debug_hash_table::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& cntr, Cursor const* cursor_a, Cursor const* cursor_b) {
    detail::CheckCntr_(cntr);

    detail::CheckCursor_(cntr, cursor_a);
    detail::CheckCursor_(cntr, cursor_b);

    return *cursor_a == *cursor_b;
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& cntr, Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    --(*cursor);
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& cntr, Cursor* cursor) {
    detail::CheckCursor_(cntr, cursor);

    ++(*cursor);
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList>>::GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::capability::FlagBuilder{
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
constexpr assoc_cntr::capability::Flag assoc_cntr::CntrTraits<
    debug_hash_table::Cntr<CntrTplArgList>>::GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::capability::FlagBuilder{
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
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::
    GetDynamicEnabledCapabilityFlag(debug_hash_table::Cntr<CntrTplArgList>&) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>::
    GetDynamicDisabledCapabilityFlag(debug_hash_table::Cntr<CntrTplArgList>&) {
    return assoc_cntr::capability::empty_capability_flag;
}

//

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticEnabledCapabilityFlag() {
    return assoc_cntr::GetStaticEnabledCapabilityFlag<
               debug_hash_table::Cntr<CntrTplArgList>>() &
           assoc_cntr::capability::const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetStaticDisabledCapabilityFlag() {
    return assoc_cntr::GetStaticDisabledCapabilityFlag<
               debug_hash_table::Cntr<CntrTplArgList>>() |
           assoc_cntr::capability::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledCapabilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledCapabilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&) {
    return assoc_cntr::capability::empty_capability_flag;
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
