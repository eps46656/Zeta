#pragma once

#include <unordered_set>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/pair.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename HasherLike, typename ComparatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList HasherLike, ComparatorLike

namespace zeta::core::debug_hash_table {

struct ObjWrapper {
    mutable void* obj;
};

template <typename ElemHasherLike>
struct HasherProxy {
    ElemHasherLike elem_hasher;
    void const* key_hasher;

    void const* cur_key;

    unsigned long long (*elem_hash_func)(void const* elem_hasher,
                                         void const* elem);

    unsigned long long (*key_hash_func)(void const* key_hasher,
                                        void const* key);

    unsigned long long operator()(ObjWrapper const& a) const;
};

template <typename ElemComparatorLike>
struct EqProxy {
    ElemComparatorLike elem_cmptr;
    void const* key_elem_cmptr;

    void const* cur_key;

    bool (*elem_eq_func)(void const* elem_cmptr, void const* elem_a,
                         void const* elem_b);

    bool (*key_elem_eq_func)(void const* key_elem_cmptr, void const* elem,
                             void const* key);

    bool operator()(ObjWrapper const& a, ObjWrapper const& b) const;
};

template <CntrTplParamList>
struct Cntr {
    using HashTable =
        std::unordered_multiset<ObjWrapper, HasherProxy<HasherLike>,
                                EqProxy<ComparatorLike>>;
    using Cursor = HashTable::iterator;

    size_t elem_size;

    HashTable hash_table;

    constexpr Cntr();

    constexpr ~Cntr();

    static constexpr assoc_cntr::capability::Flag
        GetStaticEnabledCapabilityFlag(assoc_cntr::Tag,
                                       meta::TypeWrapper<Cntr>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticEnabledCapabilityFlag(assoc_cntr::Tag,
                                       meta::TypeWrapper<Cntr const>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticDisabledCapabilityFlag(assoc_cntr::Tag,
                                        meta::TypeWrapper<Cntr>);

    static constexpr assoc_cntr::capability::Flag
        GetStaticDisabledCapabilityFlag(assoc_cntr::Tag,
                                        meta::TypeWrapper<Cntr const>);

    static constexpr assoc_cntr::capability::Flag
        GetDynamicEnabledCapabilityFlag(assoc_cntr::Tag);

    static constexpr assoc_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(assoc_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, assoc_cntr::Tag);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        assoc_cntr::Tag, meta::TypeWrapper<Cntr>);

    constexpr size_t GetCursorSize(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, assoc_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, assoc_cntr::Tag);

    constexpr void GetRBCursor(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& self, assoc_cntr::Tag, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& self, assoc_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    constexpr void Find(this auto& self, assoc_cntr::Tag, void const* elem,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this auto& self, assoc_cntr::Tag, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <assoc_cntr::IsWriter Provider>
    constexpr void Insert(this Cntr& self, assoc_cntr::Tag, void const* elem,
                          Provider&& writer, Cursor* dst_cursor);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Provider>
    constexpr void Insert(this Cntr& self, assoc_cntr::Tag, void const* key,
                          KeyHasher const& key_hasher,
                          KeyElemComparator const& key_elem_cmptr,
                          Provider&& writer, Cursor* dst_cursor);

    template <assoc_cntr::IsReader acceptor>
    constexpr void PopL(this Cntr& self, assoc_cntr::Tag, size_t cnt,
                        acceptor&& reader);

    template <assoc_cntr::IsReader acceptor>
    constexpr void PopR(this Cntr& self, assoc_cntr::Tag, size_t cnt,
                        acceptor&& reader);

    template <assoc_cntr::IsReader acceptor>
    constexpr void Erase(this Cntr& self, assoc_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, acceptor&& reader);

    constexpr void EraseAll(this Cntr& self, assoc_cntr::Tag);

    constexpr void CopyCursor(this Cntr const& self, assoc_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, assoc_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr void CursorStepL(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, assoc_cntr::Tag,
                               Cursor* cursor);
};

}  // namespace zeta::core::debug_hash_table

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
