#pragma once

#include <cstdlib>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_hash_table.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/poly_comparison.ipp>
#include <zeta/core/utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename ElemHasherLike, typename ElemComparatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ElemHasherLike, ElemComparatorLike

namespace zeta::core {

namespace debug_hash_table::detail {

template <CntrTplParamList>
void CheckCntr_(Cntr<CntrTplArgList> const& cntr) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr.hash_table != nullptr);
}

template <CntrTplParamList>
void CheckCursor_(Cntr<CntrTplArgList> const& cntr,
                  typename Cntr<CntrTplArgList>::Cursor const* cursor) {
    (CheckCntr_)(cntr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);
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
constexpr debug_hash_table::Cntr<CntrTplArgList>::Cntr() {
    this->hash_table.hash()->elem_hash_func = [](void const* elem_hasher,
                                                 void const* elem) {
        return hash::Hash(*static_cast<ElemHasherLike const*>(elem_hasher),
                          elem, 0);
    };

    this->hash_table.eq()->elem_eq_func =
        [](void const* elem_cmptr, void const* elem_a, void const* elem_b) {
            return comparison::Compare(
                *static_cast<ElemComparatorLike const*>(elem_cmptr),
                comparison::OpTags::Equal{}, elem_a, elem_b);
        };

    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);
}

template <CntrTplParamList>
constexpr debug_hash_table::Cntr<CntrTplArgList>::~Cntr() {
    detail::CheckCntr_(*this);

    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr>) {
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
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(assoc_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           assoc_cntr::capability::const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    assoc_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return assoc_cntr::capability::non_const_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetDynamicEnabledCapabilityFlag(
    assoc_cntr::Tag) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr assoc_cntr::capability::Flag
debug_hash_table::Cntr<CntrTplArgList>::GetDynamicDisabledCapabilityFlag(
    assoc_cntr::Tag) {
    return assoc_cntr::capability::empty_capability_flag;
}

template <CntrTplParamList>
constexpr void* debug_hash_table::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);
    return const_cast<void*>(static_cast<void const*>(&self));
}

template <CntrTplParamList>
constexpr meta::TypeWrapper<
    typename debug_hash_table::Cntr<CntrTplArgList>::Cursor>
debug_hash_table::Cntr<CntrTplArgList>::GetCursorType(assoc_cntr::Tag,
                                                      meta::TypeWrapper<Cntr>) {
    return {};
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetCursorSize(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return sizeof(Cursor);
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_size;
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    return hash_table->size();
}

template <CntrTplParamList>
constexpr size_t debug_hash_table::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    if (dst_cursor == nullptr) { return; }

    new (dst_cursor) Cursor{ hash_table->end() };
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PeekL(
    this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    auto pos_cursor{ hash_table->begin() };

    void* elem{ hash_table->empty() ? nullptr : pos_cursor->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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

    if (dst_cursor != nullptr) { new (dst_cursor) Cursor{ pos_cursor }; }

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PeekR(
    this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

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

        if constexpr (meta::IsConst<decltype(self)>) {
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

    if (dst_cursor != nullptr) { new (dst_cursor) Cursor{ pos_cursor }; }

    if (dst_elem != nullptr && elem != nullptr && !lazy_copy_elem) {
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Derefer(
    this auto& self, assoc_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, assoc_cntr::ElemPtrView* dst_elem_ptr_view,
    void* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    auto* hash_table{ self.hash_table };

    void* elem{ *pos_cursor == hash_table->end() ? nullptr
                                                 : (*pos_cursor)->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& self, assoc_cntr::Tag, void const* elem, bool lazy_copy_elem,
    assoc_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    auto* hash_table{ self.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    self.Find(elem, hasher_wrapper.elem_hasher, eq_wrapper.elem_cmptr,
              lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Find(
    this auto& self, assoc_cntr::Tag, void const* key,
    KeyHasher const& key_hasher, KeyElemComparator const& key_elem_cmptr,
    bool lazy_copy_elem, assoc_cntr::ElemPtrView* dst_elem_ptr_view,
    Cursor* dst_cursor, void* dst_elem) {
    auto* hash_table{ self.hash_table };

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
            comparison::OpTags::Equal{}, key, elem);
    };

    eq_wrapper.cur_key = key;

    auto pos_cursor{ hash_table->find(ObjWrapper{ const_cast<void*>(key) }) };

    if (dst_cursor != nullptr) { new (dst_cursor) Cursor{ pos_cursor }; }

    void* elem{ pos_cursor == hash_table->end() ? nullptr : pos_cursor->obj };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
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

    if (dst_cursor != nullptr) { new (dst_cursor) Cursor{ pos_cursor }; }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsWriter Provider>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& self, assoc_cntr::Tag, void const* elem, Provider&& writer,
    Cursor* dst_cursor) {
    auto* hash_table{ self.hash_table };

    auto& hasher_wrapper{ hash_table->hash_function() };
    auto& eq_wrapper{ hash_table->key_eq() };

    self.Insert(elem, hasher_wrapper.elem_hasher, eq_wrapper.elem_cmptr, writer,
                dst_cursor);
}

template <CntrTplParamList>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Provider>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& self, assoc_cntr::Tag, void const* key,
    KeyHasher const& key_hasher, KeyElemComparator const& key_elem_cmptr,
    Provider&& writer, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    HashTable* hash_table{ self.hash_table };

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
            comparison::OpTags::Equal{}, key,
            static_cast<ObjWrapper const*>(elem_wrapper)->obj);
    };

    eq_wrapper.cur_key = key;

    auto pos_cursor{ hash_table->insert(ObjWrapper{ const_cast<void*>(key) }) };

    pos_cursor->obj = std::malloc(self.elem_size);

    seq_endpoint::provider::Transfer(writer, pos_cursor->obj, self.elem_size,
                                     self.elem_size, 1);

    if (dst_cursor != nullptr) { new (dst_cursor) Cursor{ pos_cursor }; }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PopL(this Cntr& self,
                                                            assoc_cntr::Tag,
                                                            size_t cnt,
                                                            acceptor&& reader) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->begin() };

        void* elem{ pos_cursor->obj };

        seq_endpoint::acceptor::Transfer(reader, elem, self.elem_size,
                                         self.elem_size, 1);

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::PopR(this Cntr& self,
                                                            assoc_cntr::Tag,
                                                            size_t cnt,
                                                            acceptor&& reader) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= hash_table->size());

    for (size_t i{ 0 }; i < cnt; ++i) {
        auto pos_cursor{ hash_table->end() };
        --pos_cursor;

        void* elem{ pos_cursor->obj };

        seq_endpoint::acceptor::Transfer(reader, elem, self.elem_size,
                                         self.elem_size, 1);

        hash_table->erase(pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
template <assoc_cntr::IsReader acceptor>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::Erase(
    this Cntr& self, assoc_cntr::Tag, Cursor* pos_cursor, size_t cnt,
    acceptor&& reader) {
    detail::CheckCursor_(self, pos_cursor);

    auto* hash_table{ self.hash_table };

    for (; 0 < cnt; ++cnt) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(*pos_cursor !=
                                                hash_table->end());

        void* elem{ (*pos_cursor)->obj };

        seq_endpoint::acceptor::Transfer(reader, elem, self.elem_size,
                                         self.elem_size, 1);

        *pos_cursor = hash_table->erase(*pos_cursor);

        std::free(elem);
    }
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::EraseAll(
    this Cntr& self, assoc_cntr::Tag) {
    detail::CheckCntr_(self);

    auto* hash_table{ self.hash_table };

    hash_table->clear();
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor const* src_cursor,
    Cursor* dst_cursor) {
    detail::CheckCursor_(self, src_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool debug_hash_table::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& self, assoc_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    detail::CheckCntr_(self);

    detail::CheckCursor_(self, cursor_a);
    detail::CheckCursor_(self, cursor_b);

    return *cursor_a == *cursor_b;
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& self, assoc_cntr::Tag, Cursor* cursor) {
    detail::CheckCursor_(self, cursor);

    --(*cursor);
}

template <CntrTplParamList>
constexpr void debug_hash_table::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& self, assoc_cntr::Tag, Cursor* cursor) {
    detail::CheckCursor_(self, cursor);

    ++(*cursor);
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
