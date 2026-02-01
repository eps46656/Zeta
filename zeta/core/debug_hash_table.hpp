#pragma once

#include <unordered_set>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>

#pragma push_macro("CntrTplDeclParamList")
#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")

#define CntrTplDeclParamList typename ElemHashLike, typename ElemCompareLike

#define CntrTplParamList typename ElemHashLike, typename ElemCompareLike

#define CntrTplArgList ElemHashLike, ElemCompareLike

namespace zeta::core::debug_hash_table {

template <CntrTplDeclParamList>
struct Cntr;

// -----------------------------------------------------------------------------

constexpr bool elem_tag{ false };
constexpr bool key_tag{ true };

using ElemKeyWrapper = Pair<bool, void const*>;

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

    bool operator()(Pair<bool, void const*> const& a,
                    Pair<bool, void const*> const& b) const;
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

namespace ops {

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
    Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void const* PeekL(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void* PeekR(
    Cntr<CntrTplArgList>* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void const* PeekR(
    Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList>
void* Derefer(
    Cntr<CntrTplArgList>* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList>
void const* Derefer(
    Cntr<CntrTplArgList> const* cntr,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator const*
        pos_cursor,
    bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void* Find(
    Cntr<CntrTplArgList>* cntr, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor,
    void* dst_elem);

template <CntrTplParamList, typename KeyHash, typename KeyElemCompare>
void const* Find(
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
    Cntr<CntrTplArgList> const* cntr, void const* cursor,
    typename hash_table_t<ElemHashLike, ElemCompareLike>::iterator* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor_a,
                    void const* cursor_b);

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

}  // namespace ops

template <CntrTplDeclParamList>
struct AssocCntrView {
    static constexpr bool IsConst(type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView*>);

    static constexpr assoc_cntr::AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<AssocCntrView const*>);

    static constexpr assoc_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        AssocCntrView const*);

    static constexpr assoc_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        AssocCntrView const*);

    static constexpr size_t GetCursorSize(AssocCntrView const* cntr);

    static size_t GetWidth(AssocCntrView const* cntr);

    static size_t GetSize(AssocCntrView const* cntr);

    static size_t GetCapacity(AssocCntrView const* cntr);

    static void GetRBCursor(AssocCntrView const* cntr, void* dst_cursor);

    static Conditional<false, void*, void*> PeekL(AssocCntrView* cntr,
                                                  bool lazy_copy_elem,
                                                  void* dst_cursor,
                                                  void* dst_elem);

    static void const* PeekL(AssocCntrView const* cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static Conditional<false, void*, void*> PeekR(AssocCntrView* cntr,
                                                  bool lazy_copy_elem,
                                                  void* dst_cursor,
                                                  void* dst_elem);

    static void const* PeekR(AssocCntrView const* cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static void* Derefer(AssocCntrView* cntr, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    static void const* Derefer(AssocCntrView const* cntr,
                               void const* pos_cursor, bool lazy_copy_elem,
                               void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(AssocCntrView* cntr, void const* key,
                      KeyHash const& key_hash,
                      KeyElemCompare const& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void const* Find(AssocCntrView const* cntr, void const* key,
                            KeyHash const& key_hash,
                            KeyElemCompare const& key_elem_compare,
                            bool lazy_copy_elem, void* dst_cursor,
                            void* dst_elem);

    static void* Insert(AssocCntrView* cntr, void const* elem,
                        void* dst_cursor);

    static void PopL(AssocCntrView* cntr, size_t cnt);

    static void PopR(AssocCntrView* cntr, size_t cnt);

    static void Erase(AssocCntrView* cntr, void* pos_cursor);

    static void EraseAll(AssocCntrView* cntr);

    static void CopyCursor(AssocCntrView const* cntr, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(AssocCntrView const* cntr, void const* cursor_a,
                               void const* cursor_b);

    static void CursorStepL(AssocCntrView const* cntr, void* cursor);

    static void CursorStepR(AssocCntrView const* cntr, void* cursor);
};

}  // namespace zeta::core::debug_hash_table
