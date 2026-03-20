#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_AllocaCursor(cntr)             \
    ({                                                     \
        __builtin_alloca_with_align(                       \
            ::zeta::core::assoc_cntr::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));          \
    })

namespace zeta::core::assoc_cntr {

using FnHash = FunctionRef<unsigned long long(void const*, unsigned long long)>;

using FnCompare = FunctionRef<int(void const*, void const*)>;

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_KKKCntr_Ability_XMacro(func, sep)                          \
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

    ZETA_Core_KKKCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    static constexpr size_t Total{ __COUNTER__ - NumBase };
};

using AbilityFlag = unsigned;

ZETA_Core_StaticAssert(AbilityEnum::Total <= integral::WidthOf<AbilityFlag>);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct AbilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_KKKCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    constexpr AbilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) (static_cast<AbilityFlag>(this->name) << AbilityEnum::name)

        return (static_cast<AbilityFlag>(1) << AbilityEnum::Always) |
               ZETA_Core_KKKCntr_Ability_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr AbilityFlag empty_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = false,

    ZETA_Core_KKKCntr_Ability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr AbilityFlag full_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = true,

    ZETA_Core_KKKCntr_Ability_XMacro(F, )

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

template <typename Cntr, typename = void>
struct CntrTraits;

constexpr bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                                 AbilityFlag static_disabled_ability_flag);

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

template <typename CntrLike>
auto* GetReferedInstPtr(CntrLike&& cntr);

template <typename Cntr>
constexpr AbilityFlag GetStaticEnabledAbilityFlag();

template <typename Cntr>
constexpr AbilityFlag GetStaticDisabledAbilityFlag();

template <typename CntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(CntrLike&& cntr);

template <typename CntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(CntrLike&& cntr);

template <typename CntrLike>
size_t GetCursorSize(CntrLike&& cntr);

template <typename CntrLike>
size_t GetWidth(CntrLike&& cntr);

template <typename CntrLike>
size_t GetSize(CntrLike&& cntr);

template <typename CntrLike>
size_t GetCapacity(CntrLike&& cntr);

template <typename CntrLike>
void GetLBCursor(CntrLike&& cntr, void* dst_cursor);

template <typename CntrLike>
void GetRBCursor(CntrLike&& cntr, void* dst_cursor);

template <typename CntrLike>
void* PeekL(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename CntrLike>
void* PeekR(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename CntrLike>
void* Derefer(CntrLike&& cntr, void const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename CntrLike, typename KeyHash, typename KeyElemCompare>
void* Find(CntrLike&& cntr, void const* key, KeyHash&& key_hash,
           KeyElemCompare&& key_elem_compare, bool lazy_copy_elem,
           void* dst_cursor, void* dst_elem);

template <typename CntrLike>
void* Insert(CntrLike&& cntr, void const* elem, void* dst_cursor);

template <typename CntrLike>
void PopL(CntrLike&& cntr, size_t cnt);

template <typename CntrLike>
void PopR(CntrLike&& cntr, size_t cnt);

template <typename CntrLike>
void Erase(CntrLike&& cntr, void* pos_cursor);

template <typename CntrLike>
void EraseAll(CntrLike&& cntr);

template <typename CntrLike>
void CopyCursor(CntrLike&& cntr, void const* src_cursor, void* dst_cursor);

template <typename CntrLike>
bool AreEqualCursor(CntrLike&& cntr, void const* cursor_a,
                    void const* cursor_b);

template <typename CntrLike>
int CompareCursor(CntrLike&& cntr, void const* cursor_a, void const* cursor_b);

template <typename CntrLike>
size_t GetCursorDist(CntrLike&& cntr, void const* cursor_a,
                     void const* cursor_b);

template <typename CntrLike>
size_t GetCursorIdx(CntrLike&& cntr, void const* cursor);

template <typename CntrLike>
void CursorStepL(CntrLike&& cntr, void* cursor);

template <typename CntrLike>
void CursorStepR(CntrLike&& cntr, void* cursor);

template <typename CntrLike>
void CursorAdvanceL(CntrLike&& cntr, void* cursor, size_t step);

template <typename CntrLike>
void CursorAdvanceR(CntrLike&& cntr, void* cursor, size_t step);

template <typename CntrLike>
void CheckContract(CntrLike&& cntr);

struct VTable {
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

template <typename Cntr>
constexpr VTable BuildVTableBasic();

template <typename Cntr, typename = void>
struct BuildVTableImpl {
    static constexpr VTable Call();
};

template <typename Cntr>
constexpr VTable BuildVTable();

template <typename Cntr>
constexpr VTable const& GetVTable();

}  // namespace zeta::core::assoc_cntr
