#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_AllocaCursor(cntr)                  \
    ({                                                          \
        __builtin_alloca_with_align(                            \
            ::zeta::core::assoc_cntr::ops::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));               \
    })

namespace zeta::core::assoc_cntr {

using FnHash = FunctionRef<unsigned long long(void const*, unsigned long long)>;

using FnCompare = FunctionRef<int(void const*, void const*)>;

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

namespace ops {

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

template <typename AssocCntr>
void CheckContract(AssocCntr* cntr);

}  // namespace ops

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

struct Ref {
    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AbilityFlag dynamic_enabled_ability_flag;
    AbilityFlag dynamic_disabled_ability_flag;

    VTable const* vtable;

    void* cntr;

    static constexpr AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref const*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref const*>);

    static AbilityFlag GetDynamicEnabledAbilityFlag(Ref*);

    static AbilityFlag GetDynamicEnabledAbilityFlag(Ref const*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(Ref*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(Ref const*);

    static size_t GetCursorSize(Ref const*);

    static size_t GetWidth(Ref const* ref);

    static size_t GetSride(Ref const* ref);

    static size_t GetOffset(Ref const* ref);

    static size_t GetSize(Ref const* ref);

    static size_t GetCapacity(Ref const* ref);

    static void GetLBCursor(Ref const* ref, void* dst_cursor);

    static void GetRBCursor(Ref const* ref, void* dst_cursor);

    static void* PeekL(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* PeekR(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* Derefer(Ref const* ref, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    template <typename KeyHash, typename KeyElemCompare>
    static void* Find(Ref const* ref, void const* key, KeyHash const& key_hash,
                      KeyElemCompare const& key_elem_compare,
                      bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Insert(Ref* ref, void const* elem, void* dst_cursor);

    static void PopL(Ref* ref, size_t cnt);

    static void PopR(Ref* ref, size_t cnt);

    static void Erase(Ref* ref, void* pos_cursor);

    static void EraseAll(Ref* ref);

    static void CopyCursor(Ref const* ref, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(Ref const* ref, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(Ref const* ref, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(Ref const* ref, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(Ref const* ref, void const* cursor);

    static void CursorStepL(Ref const* ref, void* cursor);

    static void CursorStepR(Ref const* ref, void* cursor);

    static void CursorAdvanceL(Ref const* ref, void* cursor, size_t step);

    static void CursorAdvanceR(Ref const* ref, void* cursor, size_t step);

    static void CheckCntr(Ref* ref);
};

namespace ops {

template <typename AssocCntrLike>
constexpr AbilityFlag GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrLike>);

template <typename AssocCntrLike>
constexpr AbilityFlag GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrLike>);

template <typename AssocCntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
decltype(auto) GetCursorSize(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
size_t GetWidth(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
size_t GetSize(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
decltype(auto) GetCapacity(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
void GetLBCursor(AssocCntrLike&& cntr, void* dst_cursor);

template <typename AssocCntrLike>
void GetRBCursor(AssocCntrLike&& cntr, void* dst_cursor);

template <typename AssocCntrLike>
void* PeekL(AssocCntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename AssocCntrLike>
void* PeekR(AssocCntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename AssocCntrLike>
void* Derefer(AssocCntrLike&& cntr, void const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename AssocCntrLike, typename KeyHash, typename KeyElemCompare>
void* Find(AssocCntrLike&& cntr, void const* key, KeyHash const& key_hash,
           KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
           void* dst_cursor, void* dst_elem);

template <typename AssocCntrLike>
void* Insert(AssocCntrLike&& cntr, void const* elem, void* dst_cursor);

template <typename AssocCntrLike>
void PopL(AssocCntrLike&& cntr, size_t cnt);

template <typename AssocCntrLike>
void PopR(AssocCntrLike&& cntr, size_t cnt);

template <typename AssocCntrLike>
void Erase(AssocCntrLike&& cntr, void* pos_cursor);

template <typename AssocCntrLike>
void EraseAll(AssocCntrLike&& cntr);

template <typename AssocCntrLike>
void CopyCursor(AssocCntrLike&& cntr, void const* src_cursor, void* dst_cursor);

template <typename AssocCntrLike>
bool AreEqualCursor(AssocCntrLike&& cntr, void const* cursor_a,
                    void const* cursor_b);

template <typename AssocCntrLike>
int CompareCursor(AssocCntrLike&& cntr, void const* cursor_a,
                  void const* cursor_b);

template <typename AssocCntrLike>
size_t GetCursorDist(AssocCntrLike&& cntr, void const* cursor_a,
                     void const* cursor_b);

template <typename AssocCntrLike>
size_t GetCursorIdx(AssocCntrLike&& cntr, void const* cursor);

template <typename AssocCntrLike>
void CursorStepL(AssocCntrLike&& cntr, void* cursor);

template <typename AssocCntrLike>
void CursorStepR(AssocCntrLike&& cntr, void* cursor);

template <typename AssocCntrLike>
void CursorAdvanceL(AssocCntrLike&& cntr, void* cursor, size_t step);

template <typename AssocCntrLike>
void CursorAdvanceR(AssocCntrLike&& cntr, void* cursor, size_t step);

template <typename AssocCntrLike>
Ref MakeRef(AssocCntrLike&& cntr);

}  // namespace ops

}  // namespace zeta::core::assoc_cntr
