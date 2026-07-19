#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_AllocaCursor(cntr)             \
    ({                                                     \
        __builtin_alloca_with_align(                       \
            ::zeta::core::assoc_cntr::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));          \
    })

namespace zeta::core::assoc_cntr {

using FnHash =
    function_ref::Ref<unsigned long long(void const*, unsigned long long)>;

using FnCompare = function_ref::Ref<int(void const*, void const*)>;

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_KKKCntr_Capability_XMacro(func, sep)                            \
    func(GetCursorSize) sep                                                    \
                                                                               \
    func(GetElemSize) sep                                                      \
    func(GetElemCnt) sep                                                       \
    func(GetMaxElemCnt) sep                                                    \
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

struct CapabilityEnum {
    static constexpr size_t NumBase{ __COUNTER__ + 1 };

    static constexpr size_t Never{ __COUNTER__ - NumBase };
    static constexpr size_t Always{ __COUNTER__ - NumBase };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) static constexpr size_t name{ __COUNTER__ - NumBase };

    ZETA_Core_KKKCntr_Capability_XMacro(F, );

#pragma pop_macro("F")

    static constexpr size_t Total{ __COUNTER__ - NumBase };
};

using CapabilityFlag = unsigned;

ZETA_Core_StaticAssert(CapabilityEnum::Total <=
                       integral::WidthOf<CapabilityFlag>);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct CapabilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_KKKCntr_Capability_XMacro(F, );

#pragma pop_macro("F")

    constexpr CapabilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) \
    (static_cast<CapabilityFlag>(this->name) << CapabilityEnum::name)

        return (static_cast<CapabilityFlag>(1) << CapabilityEnum::Always) |
               ZETA_Core_KKKCntr_Capability_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr CapabilityFlag empty_capability_flag{ CapabilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = false,

    ZETA_Core_KKKCntr_Capability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr CapabilityFlag full_capability_flag{ CapabilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = true,

    ZETA_Core_KKKCntr_Capability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr CapabilityFlag non_const_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = false,
    .GetElemSize = false,
    .GetElemCnt = false,
    .GetMaxElemCnt = false,
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

constexpr CapabilityFlag const_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = true,
    .GetElemSize = true,
    .GetElemCnt = true,
    .GetMaxElemCnt = true,
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

ZETA_Core_StaticAssert((non_const_capability_flag & const_capability_flag) ==
                       empty_capability_flag);

ZETA_Core_StaticAssert((non_const_capability_flag | const_capability_flag) ==
                       full_capability_flag);

template <typename Cntr, typename = void>
struct CntrTraits;  // IWYU pragma: export

constexpr bool CheckCapabilityFlags(
    CapabilityFlag static_enabled_capability_flag,
    CapabilityFlag static_disabled_capability_flag);

bool CheckCapabilityFlags(CapabilityFlag static_enabled_capability_flag,
                          CapabilityFlag static_disabled_capability_flag,
                          CapabilityFlag dynamic_enabled_capability_flag,
                          CapabilityFlag dynamic_disabled_capability_flag);

template <typename Cntr>
auto* GetReferedInstPtr(Cntr& cntr);

template <typename Cntr>
constexpr CapabilityFlag GetStaticEnabledCapabilityFlag();

template <typename Cntr>
constexpr CapabilityFlag GetStaticDisabledCapabilityFlag();

template <typename Cntr>
CapabilityFlag GetDynamicEnabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
CapabilityFlag GetDynamicDisabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
size_t GetCursorSize(Cntr& cntr);

template <typename Cntr>
size_t GetElemSize(Cntr& cntr);

template <typename Cntr>
size_t GetElemCnt(Cntr& cntr);

template <typename Cntr>
size_t GetMaxElemCnt(Cntr& cntr);

template <typename Cntr>
void GetLBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
void GetRBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
void* PeekL(Cntr& cntr, bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

template <typename Cntr>
void* PeekR(Cntr& cntr, bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

template <typename Cntr>
void* Derefer(Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename Cntr, typename KeyHash, typename KeyElemCompare>
void* Find(Cntr& cntr, void const* key, KeyHash&& key_hash,
           KeyElemCompare&& key_elem_compare, bool lazy_copy_elem,
           void* dst_cursor, void* dst_elem);

template <typename Cntr>
void* Insert(Cntr& cntr, void const* elem, void* dst_cursor);

template <typename Cntr>
void PopL(Cntr& cntr, size_t cnt);

template <typename Cntr>
void PopR(Cntr& cntr, size_t cnt);

template <typename Cntr>
void Erase(Cntr& cntr, void* pos_cursor);

template <typename Cntr>
void EraseAll(Cntr& cntr);

template <typename Cntr>
void CopyCursor(Cntr& cntr, void* src_cursor, void* dst_cursor);

template <typename Cntr>
bool AreEqualCursor(Cntr& cntr, void const* cursor_a, void const* cursor_b);

template <typename Cntr>
int CompareCursor(Cntr& cntr, void const* cursor_a, void const* cursor_b);

template <typename Cntr>
size_t GetCursorDist(Cntr& cntr, void const* cursor_a, void const* cursor_b);

template <typename Cntr>
size_t GetCursorIdx(Cntr& cntr, void const* cursor);

template <typename Cntr>
void CursorStepL(Cntr& cntr, void* cursor);

template <typename Cntr>
void CursorStepR(Cntr& cntr, void* cursor);

template <typename Cntr>
void CursorAdvanceL(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr>
void CursorAdvanceR(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr>
void CheckContract(Cntr& cntr);

struct VTable {
    size_t (*GetElemCnt)(void* cntr);

    size_t (*GetMaxElemCnt)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void* (*PeekL)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*PeekR)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*Derefer)(void* cntr, void* pos_cursor, bool lazy_copy_elem,
                     void* dst_elem);

    void* (*FnFind)(void* cntr, void const* key, FnHash const& key_hash,
                    FnCompare const& key_elem_compare, bool lazy_copy_elem,
                    void* dst_cursor, void* dst_elem);

    void* (*FnInsert)(void* cntr, void const* elem, void* dst_cursor);

    void (*PopL)(void* cntr, size_t cnt);

    void (*PopR)(void* cntr, size_t cnt);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void* cntr, void* src_cursor, void* dst_cursor);

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
