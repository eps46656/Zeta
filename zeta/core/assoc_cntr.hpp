#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_AllocaCursor_(tmp_cntr, cntr)                    \
    ({                                                                       \
        auto tmp_cntr{ cntr };                                               \
        ZETA_Core_StaticAssert(::zeta::core::IsPointer<decltype(tmp_cntr)>); \
                                                                             \
        ZETA_Core_DebugAssert(tmp_cntr != nullptr);                          \
        ::zeta::core::assoc_cntr::CheckContract(tmp_cntr);                   \
                                                                             \
        __builtin_alloca_with_align(                                         \
            ::zeta::core::RemovePointer<decltype(tmp_cntr)>::GetCursorSize(  \
                tmp_cntr),                                                   \
            __CHAR_BIT__ * alignof(max_align_t));                            \
    })

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_AllocaCursor(cntr) \
    ZETA_Core_AssocCntr_AllocaCursor_(ZETA_Core_TmpName, cntr)

namespace zeta::core::assoc_cntr {

using FnHash = FunctionRef<unsigned long long(void const*, unsigned long long)>;

using FnCompare = FunctionRef<int(void const*, void const*)>;

// -----------------------------------------------------------------------------

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_Ability_XMacro(func, sep)                          \
    func(GetCursorSize) sep                                                    \
                                                                               \
    func(GetWidth) sep                                                         \
    func(GetSize) sep                                                          \
    func(GetCapacity) sep                                                      \
                                                                               \
    func(GetLBCursor) sep                                                      \
    func(GetRBCursor) sep                                                      \
                                                                               \
    func(PeekL) sep                                                            \
    func(PeekR) sep                                                            \
                                                                               \
    func(Derefer) sep                                                          \
                                                                               \
    func(Find) sep                                                             \
                                                                               \
    func(Insert) sep                                                           \
                                                                               \
    func(PopL) sep                                                             \
    func(PopR) sep                                                             \
    func(Erase) sep                                                            \
    func(EraseAll) sep                                                         \
                                                                               \
    func(CopyCursor) sep                                                       \
                                                                               \
    func(AreEqualCursor) sep                                                   \
    func(CompareCursor) sep                                                    \
    func(GetCursorDist) sep                                                    \
    func(GetCursorIdx) sep                                                     \
                                                                               \
    func(CursorStepL) sep                                                      \
    func(CursorStepR) sep                                                      \
                                                                               \
    func(CursorAdvanceL) sep                                                   \
    func(CursorAdvanceR)
// clang-format on

struct AbilityEnum {
    static constexpr size_t NumBase{ __COUNTER__ + 1 };

    static constexpr size_t Never{ __COUNTER__ - NumBase };
    static constexpr size_t Always{ __COUNTER__ - NumBase };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) static constexpr size_t name{ __COUNTER__ - NumBase };

    ZETA_Core_AssocCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    static constexpr size_t Total{ __COUNTER__ - NumBase };
};

using AbilityFlag = unsigned;

ZETA_Core_StaticAssert(AbilityEnum::Total <= WidthOf<AbilityFlag>);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct AbilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_AssocCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    constexpr AbilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) (static_cast<AbilityFlag>(this->name) << AbilityEnum::name)

        return (static_cast<AbilityFlag>(1) << AbilityEnum::Always) |
               ZETA_Core_AssocCntr_Ability_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr AbilityFlag empty_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = false,

    ZETA_Core_AssocCntr_Ability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr AbilityFlag full_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = true,

    ZETA_Core_AssocCntr_Ability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr AbilityFlag non_const_ability_flag{ AbilityFlagBuilder{
    .GetCursorSize = false,
    .GetWidth = false,
    .GetSize = false,
    .GetCapacity = false,
    .GetLBCursor = false,
    .GetRBCursor = false,
    .PeekL = false,
    .PeekR = false,
    .Derefer = false,
    .Find = false,
    .Insert = true,
    .PopL = true,
    .PopR = true,
    .Erase = true,
    .EraseAll = true,
    .CopyCursor = false,
    .AreEqualCursor = false,
    .CompareCursor = false,
    .GetCursorDist = false,
    .GetCursorIdx = false,
    .CursorStepL = false,
    .CursorStepR = false,
    .CursorAdvanceL = false,
    .CursorAdvanceR = false,
}() };

constexpr AbilityFlag const_ability_flag{ AbilityFlagBuilder{
    .GetCursorSize = true,
    .GetWidth = true,
    .GetSize = true,
    .GetCapacity = true,
    .GetLBCursor = true,
    .GetRBCursor = true,
    .PeekL = true,
    .PeekR = true,
    .Derefer = true,
    .Find = true,
    .Insert = false,
    .PopL = false,
    .PopR = false,
    .Erase = false,
    .EraseAll = false,
    .CopyCursor = true,
    .AreEqualCursor = true,
    .CompareCursor = true,
    .GetCursorDist = true,
    .GetCursorIdx = true,
    .CursorStepL = true,
    .CursorStepR = true,
    .CursorAdvanceL = true,
    .CursorAdvanceR = true,
}() };

ZETA_Core_StaticAssert((non_const_ability_flag & const_ability_flag) ==
                       empty_ability_flag);

ZETA_Core_StaticAssert((non_const_ability_flag | const_ability_flag) ==
                       full_ability_flag);

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

template <typename AssocCntr>
void CheckContract(AssocCntr* cntr);

// -----------------------------------------------------------------------------

struct VTable {
    template <typename AssocCntr>
    static constexpr VTable const& Make();

    size_t (*GetSize)(void* cntr);

    size_t (*GetCapacity)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void* (*PeekL)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*PeekR)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*Derefer)(void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                     void* dst_elem);

    void* (*FnFind)(void* cntr, void const* key, FnHash const& key_hash,
                    FnCompare const& key_elem_compare, bool lazy_copy_elem,
                    void* dst_cursor, void* dst_elem);

    void* (*FnInsert)(void* cntr, void const* elem, void* dst_cursor);

    void (*PopL)(void* cntr, size_t cnt);

    void (*PopR)(void* cntr, size_t cnt);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void* cntr, void const* src_cursor, void* dst_cursor);

    bool (*AreEqualCursor)(void* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void* cntr, void const* cursor);

    void (*CursorStepL)(void* cntr, void* cursor);

    void (*CursorStepR)(void* cntr, void* cursor);

    void (*CursorAdvanceL)(void* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void* cntr, void* cursor, size_t step);
};

template <typename AssocCntr>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static size_t GetSize(void* cntr);

    static constexpr size_t GetCapacity(void* cntr);

    static void GetLBCursor(void* cntr, void* dst_cursor);

    static void GetRBCursor(void* cntr, void* dst_cursor);

    static void* PeekL(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* PeekR(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* Derefer(void* cntr, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    static void* FnFind(void* cntr, void const* key, FnHash const& key_hash,
                        FnCompare const& key_elem_compare, bool lazy_copy_elem,
                        void* dst_cursor, void* dst_elem);

    static void* FnInsert(void* cntr, void const* elem, void* dst_cursor);

    static void PopL(void* cntr, size_t cnt);

    static void PopR(void* cntr, size_t cnt);

    static void Erase(void* cntr, void* pos_cursor);

    static void EraseAll(void* cntr);

    static void CopyCursor(void* cntr, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void* cntr, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void* cntr, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void* cntr, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void* cntr, void const* cursor);

    static void CursorStepL(void* cntr, void* cursor);

    static void CursorStepR(void* cntr, void* cursor);

    static void CursorAdvanceL(void* cntr, void* cursor, size_t step);

    static void CursorAdvanceR(void* cntr, void* cursor, size_t step);
};

template <typename AssocCntr>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename SeqCntr>
constexpr VTable const& GetVTable();

// -----------------------------------------------------------------------------

template <typename ConstTag>
struct Ref {
    ZETA_Core_StaticAssert(
        IsAnyOf<ConstTag, value_wrapper::FalseType, value_wrapper::TrueType>);

    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AbilityFlag dynamic_enabled_ability_flag;
    AbilityFlag dynamic_disabled_ability_flag;

    VTable const* vtable;

    void* cntr;
};

template <typename ConstTag>
struct RefView {
    ZETA_Core_StaticAssert(
        IsAnyOf<ConstTag, value_wrapper::FalseType, value_wrapper::TrueType>);

    static constexpr bool IsConst(type_wrapper::TypeWrapper<RefView*>);

    static constexpr bool IsConst(type_wrapper::TypeWrapper<RefView const*>);

    static constexpr AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<RefView*>);

    static constexpr AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<RefView const*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<RefView*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<RefView const*>);

    static AbilityFlag GetDynamicEnabledAbilityFlag(RefView*);

    static AbilityFlag GetDynamicEnabledAbilityFlag(RefView const*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(RefView*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(RefView const*);

    static size_t GetCursorSize(RefView const*);

    static size_t GetWidth(RefView const* ref_view);

    static size_t GetSride(RefView const* ref_view);

    static size_t GetOffset(RefView const* ref_view);

    static size_t GetSize(RefView const* ref_view);

    static size_t GetCapacity(RefView const* ref_view);

    static void GetLBCursor(RefView const* ref_view, void* dst_cursor);

    static void GetRBCursor(RefView const* ref_view, void* dst_cursor);

    static Conditional<ConstTag::value, void const*, void*> PeekL(
        RefView* ref_view, bool lazy_copy_elem, void* dst_cursor,
        void* dst_elem);

    static void const* PeekL(RefView const* ref_view, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static Conditional<ConstTag::value, void const*, void*> PeekR(
        RefView* ref_view, bool lazy_copy_elem, void* dst_cursor,
        void* dst_elem);

    static void const* PeekR(RefView const* ref_view, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static Conditional<ConstTag::value, void const*, void*> Derefer(
        RefView* ref_view, void const* pos_cursor, bool lazy_copy_elem,
        void* dst_elem);

    static void const* Derefer(RefView const* ref_view, void const* pos_cursor,
                               bool lazy_copy_elem, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static Conditional<ConstTag::value, void const*, void*> Find(
        RefView* ref_view, void const* key, KeyHash const& key_hash,
        KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
        void* dst_cursor, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void const* Find(RefView const* ref_view, void const* key,
                            KeyHash const& key_hash,
                            KeyElemCompare const& key_elem_compare,
                            bool lazy_copy_elem, void* dst_cursor,
                            void* dst_elem);

    static void* Insert(RefView* ref_view, void const* elem, void* dst_cursor);

    static void PopL(RefView* ref_view, size_t cnt);

    static void PopR(RefView* ref_view, size_t cnt);

    static void Erase(RefView* ref_view, void* pos_cursor);

    static void EraseAll(RefView* ref_view);

    static void CopyCursor(RefView const* ref_view, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(RefView const* ref_view, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(RefView const* ref_view, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(RefView const* ref_view, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(RefView const* ref_view, void const* cursor);

    static void CursorStepL(RefView const* ref_view, void* cursor);

    static void CursorStepR(RefView const* ref_view, void* cursor);

    static void CursorAdvanceL(RefView const* ref_view, void* cursor,
                               size_t step);

    static void CursorAdvanceR(RefView const* ref_view, void* cursor,
                               size_t step);

    static void CheckCntr(RefView* ref);
};

template <typename AssocCntr>
auto MakeRef(AssocCntr* cntr);

}  // namespace zeta::core::assoc_cntr
