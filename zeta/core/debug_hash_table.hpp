#pragma once

#include <unordered_set>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/pair.hpp>

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

template <typename HasherLike, typename ComparatorLike>
using hash_table_t =
    std::unordered_multiset<ObjWrapper, HasherProxy<HasherLike>&,
                            EqProxy<ComparatorLike>&>;

template <typename HasherLike, typename ComparatorLike>
using hash_table_cursor_t = hash_table_t<HasherLike, ComparatorLike>::iterator;

template <CntrTplParamList>
struct Cntr {
    using HashTable = hash_table_t<HasherLike, ComparatorLike>;
    using Cursor = hash_table_cursor_t<HasherLike, ComparatorLike>;

    size_t elem_size;

    HasherProxy<HasherLike> hasher_proxy;
    EqProxy<ComparatorLike> eq_proxy;

    HashTable* hash_table;

    constexpr void Init(this Cntr& cntr);

    constexpr void Deinit(this Cntr& cntr);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    constexpr size_t GetElemSize(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr);

    constexpr void GetRBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void PeekL(this auto& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& cntr, bool lazy_copy_elem,
                         assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& cntr, Cursor const* pos_cursor,
                           bool lazy_copy_elem,
                           assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    constexpr void Find(this auto& cntr, void const* elem, bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    constexpr void Find(this auto& cntr, void const* key,
                        KeyHasher const& key_hasher,
                        KeyElemComparator const& key_elem_cmptr,
                        bool lazy_copy_elem,
                        assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                        Cursor* dst_cursor, void* dst_elem);

    template <assoc_cntr::IsWriter Writer>
    constexpr void Insert(this Cntr& cntr, void const* elem, Writer&& writer,
                          Cursor* dst_cursor);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        assoc_cntr::IsWriter Writer>
    constexpr void Insert(this Cntr& cntr, void const* key,
                          KeyHasher const& key_hasher,
                          KeyElemComparator const& key_elem_cmptr,
                          Writer&& writer, Cursor* dst_cursor);

    template <assoc_cntr::IsReader Reader>
    constexpr void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <assoc_cntr::IsReader Reader>
    constexpr void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <assoc_cntr::IsReader Reader>
    constexpr void Erase(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                         Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr);

    constexpr void CopyCursor(this Cntr const& cntr, Cursor const* src_cursor,
                              Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr void CursorStepL(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, Cursor* cursor);
};

}  // namespace zeta::core::debug_hash_table

namespace zeta::core {

template <CntrTplParamList>
struct lifecycle::Traits<debug_hash_table::Cntr<CntrTplArgList>>
    : public lifecycle::MemberFuncTraitsAdapter<
          debug_hash_table::Cntr<CntrTplArgList>> {};

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<
          debug_hash_table::Cntr<CntrTplArgList>,
          typename debug_hash_table::Cntr<CntrTplArgList>::Cursor> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(debug_hash_table::Cntr<CntrTplArgList>&);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(debug_hash_table::Cntr<CntrTplArgList>&);
};

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const>
    : public assoc_cntr::MemberFuncCntrTraitsAdapter<
          debug_hash_table::Cntr<CntrTplArgList> const,
          typename debug_hash_table::Cntr<CntrTplArgList>::Cursor> {
    static constexpr assoc_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr assoc_cntr::capability::Flag
    GetDynamicEnabledCapabilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&);

    static constexpr assoc_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const&);
};

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
