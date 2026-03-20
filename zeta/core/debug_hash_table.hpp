#pragma once

#include <unordered_set>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList typename ElemHashLike, typename ElemCompareLike

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename ElemHashLike, typename ElemCompareLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList ElemHashLike, ElemCompareLike

namespace zeta::core::debug_hash_table {

constexpr bool elem_tag{ false };
constexpr bool key_tag{ true };

using ElemKeyWrapper = utils::Pair<bool, void const*>;

template <typename ElemHashLike>
struct ElemKeyHashProxy {
    ElemHashLike elem_hash;

    assoc_cntr::FnHash
        key_hash;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    unsigned long long operator()(ElemKeyWrapper const& a) const;
};

template <typename ElemCompareLike>
struct ElemKeyEqProxy {
    ElemCompareLike elem_compare;

    assoc_cntr::FnCompare key_elem_compare;

    bool operator()(utils::Pair<bool, void const*> const& a,
                    utils::Pair<bool, void const*> const& b) const;
};

template <typename ElemHashLike, typename ElemCompareLike>
using hash_table_t =
    std::unordered_multiset<ElemKeyWrapper, ElemKeyHashProxy<ElemHashLike>&,
                            ElemKeyEqProxy<ElemCompareLike>&>;

template <CntrTplDeclParamList>
struct Cntr {
    size_t width;

    ElemKeyHashProxy<ElemHashLike> elem_key_hash_proxy;
    ElemKeyEqProxy<ElemCompareLike> elem_key_eq_proxy;

    hash_table_t<ElemHashLike, ElemCompareLike>* hash_table;
};

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
size_t GetCursorSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
void GetRBCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor);

template <CntrTplParamList>
void* PeekL(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void* PeekR(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void* Derefer(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(
    Cntr<CntrTplArgList> const* cntr, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void* Insert(
    Cntr<CntrTplArgList>* cntr, void const* elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void Erase(
    Cntr<CntrTplArgList>* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* pos_cursor);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void CopyCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        src_cursor,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor_a,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor_b);

template <CntrTplParamList>
void CursorStepL(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor);

template <CntrTplParamList>
void CursorStepR(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* cursor);

template <CntrTplParamList>
bool CheckCntr(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
bool CheckCursor(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        cursor);

}  // namespace zeta::core::debug_hash_table

namespace zeta::core {

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList> const,
                              void> {
    static void* GetReferedInst(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const*);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        debug_hash_table::Cntr<CntrTplArgList> const*);

    static constexpr size_t GetCursorSize(
        debug_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetWidth(debug_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetSize(debug_hash_table::Cntr<CntrTplArgList> const*);

    static size_t GetCapacity(debug_hash_table::Cntr<CntrTplArgList> const*);

    static void GetRBCursor(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                            void* dst_cursor);

    static void* PeekL(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                      void const* key, KeyHash const& key_hash,
                      KeyElemCompare const& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void CopyCursor(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(
        debug_hash_table::Cntr<CntrTplArgList> const* cntr,
        void const* cursor_a, void const* cursor_b);

    static void CursorStepL(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                            void* cursor);

    static void CursorStepR(debug_hash_table::Cntr<CntrTplArgList> const* cntr,
                            void* cursor);
};

template <CntrTplParamList>
struct assoc_cntr::CntrTraits<debug_hash_table::Cntr<CntrTplArgList>, void>
    : public assoc_cntr::CntrTraits<
          debug_hash_table::Cntr<CntrTplArgList> const, void> {
    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static void* Insert(debug_hash_table::Cntr<CntrTplArgList>* cntr,
                        void const* elem, void* dst_cursor);

    static void PopL(debug_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt);

    static void PopR(debug_hash_table::Cntr<CntrTplArgList>* cntr, size_t cnt);

    static void Erase(debug_hash_table::Cntr<CntrTplArgList>* cntr,
                      void* pos_cursor);

    static void EraseAll(debug_hash_table::Cntr<CntrTplArgList>* cntr);
};

}  // namespace zeta::core

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
