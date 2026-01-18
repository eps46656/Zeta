#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#define ZETA_Core_AssocCntr_AllocaCursor_(tmp_cntr, cntr)                 \
    ({                                                                    \
        auto tmp_cntr{ cntr };                                            \
        ZETA_Core_DebugAssert(tmp_cntr != nullptr);                       \
                                                                          \
        __builtin_alloca_with_align(tmp_cntr->cursor_size,                \
                                    __CHAR_BIT__ * alignof(max_align_t)); \
    })

#define ZETA_Core_AssocCntr_AllocaCursor(cntr) \
    ZETA_Core_AssocCntr_AllocaCursor_(ZETA_Core_TmpName, cntr)

namespace zeta::core::assoc_cntr {

using FnHash = FunctionRef<unsigned long long(void const*, unsigned long long)>;

using FnCompare = FunctionRef<int(void const*, void const*)>;

// -----------------------------------------------------------------------------

struct AssocCntrAbilityEnum {
    static constexpr size_t Never{ 0 };
    static constexpr size_t Always{ Never + 1 };

    static constexpr size_t GetLBCursor{ Always + 1 };
    static constexpr size_t GetRBCursor{ GetLBCursor + 1 };

    static constexpr size_t PeekL{ GetRBCursor + 1 };
    static constexpr size_t PeekR{ PeekL + 1 };

    static constexpr size_t CompareCursor{ PeekR + 1 };
    static constexpr size_t GetCursorDist{ CompareCursor + 1 };
    static constexpr size_t GetCursorIdx{ GetCursorDist + 1 };
    static constexpr size_t CursorStepL{ GetCursorIdx + 1 };
    static constexpr size_t CursorStepR{ CursorStepL + 1 };
    static constexpr size_t CursorAdvanceL{ CursorStepR + 1 };
    static constexpr size_t CursorAdvanceR{ CursorAdvanceL + 1 };

    static constexpr size_t Total{ CursorAdvanceR + 1 };
};

using AssocCntrAbilityFlagType = unsigned;

ZETA_Core_StaticAssert(AssocCntrAbilityEnum::Total <=
                       WidthOf<AssocCntrAbilityFlagType>);

struct AssocCntrAbilityFlagBuilder {
    bool const GetLBCursor;
    bool const GetRBCursor;

    bool const PeekL;
    bool const PeekR;

    bool const CompareCursor;
    bool const GetCursorDist;
    bool const GetCursorIdx;
    bool const CursorStepL;
    bool const CursorStepR;
    bool const CursorAdvanceL;
    bool const CursorAdvanceR;

    constexpr AssocCntrAbilityFlagType operator()() const {
#pragma push_macro("F")

#define F(name)                                        \
    (static_cast<AssocCntrAbilityFlagType>(this->name) \
     << AssocCntrAbilityEnum::name)

        return ((static_cast<AssocCntrAbilityFlagType>(1)
                 << AssocCntrAbilityEnum::Always) |  //

                F(GetLBCursor) |  //
                F(GetRBCursor) |  //

                F(PeekL) |  //
                F(PeekR) |  //

                F(CompareCursor) |   //
                F(GetCursorDist) |   //
                F(GetCursorIdx) |    //
                F(CursorStepL) |     //
                F(CursorStepR) |     //
                F(CursorAdvanceL) |  //
                F(CursorAdvanceR) |  //

                static_cast<AssocCntrAbilityFlagType>(0));

#pragma pop_macro("F")
    }
};

constexpr AssocCntrAbilityFlagType assoc_cntr_empty_ability_flag{
    AssocCntrAbilityFlagBuilder{
        .GetLBCursor = false,
        .GetRBCursor = false,

        .PeekL = false,
        .PeekR = false,

        .CompareCursor = false,
        .GetCursorDist = false,
        .GetCursorIdx = false,
        .CursorStepL = false,
        .CursorStepR = false,
        .CursorAdvanceL = false,
        .CursorAdvanceR = false,
    }()
};

constexpr AssocCntrAbilityFlagType assoc_cntr_full_ability_flag{
    AssocCntrAbilityFlagBuilder{
        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,
        .CursorStepL = true,
        .CursorStepR = true,
        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }()
};

bool CheckAssocCntrAbilityFlags(
    AssocCntrAbilityFlagType static_enabled_ability_flag,
    AssocCntrAbilityFlagType static_disabled_ability_flag,
    AssocCntrAbilityFlagType dynamic_enabled_ability_flag,
    AssocCntrAbilityFlagType dynamic_disabled_ability_flag);

// -----------------------------------------------------------------------------

struct AssocCntrMethodSig {
    using GetDynamicEnabledAbilityFlag =
        AssocCntrAbilityFlagType(void const* cntr);

    using GetDynamicDisabledAbilityFlag =
        AssocCntrAbilityFlagType(void const* cntr);

    using GetCursorSize = size_t(void const* cntr);

    using GetWidth = size_t(void const* cntr);

    using GetSize = size_t(void const* cntr);

    using GetCapacity = size_t(void const* cntr);

    using GetLBCursor = void(void const* cntr, void* dst_cursor);

    using GetRBCursor = void(void const* cntr, void* dst_cursor);

    using PeekL = void*(void* cntr, void* dst_cursor, void* dst_elem);

    using ConstPeekL = void const*(void const* cntr, void* dst_cursor,
                                   void* dst_elem);

    using PeekR = void*(void* cntr, void* dst_cursor, void* dst_elem);

    using ConstPeekR = void const*(void const* cntr, void* dst_cursor,
                                   void* dst_elem);

    using Refer = void*(void* cntr, void const* pos_cursor);

    using ConstRefer = void const*(void const* cntr, void const* pos_cursor);

    using FnFind = void*(void* cntr, void const* key, FnHash const& key_hash,
                         FnCompare const& key_elem_compare, void* dst_cursor);

    using ConstFnFind = void const*(void const* cntr, void const* key,
                                    FnHash const& key_hash,
                                    FnCompare const& key_elem_compare,
                                    void* dst_cursor);

    using Insert = void*(void* cntr, void const* elem, void* dst_cursor);

    using Erase = void(void* cntr, void* pos_cursor);

    using EraseAll = void(void* cntr);

    using CopyCursor = void(void const* cntr, void const* cursor,
                            void* dst_cursor);

    using AreEqualCursor = bool(void const* cntr, void const* cursor_a,
                                void const* cursor_b);

    using CompareCursor = int(void const* cntr, void const* cursor_a,
                              void const* cursor_b);

    using GetCursorDist = size_t(void const* cntr, void const* cursor_a,
                                 void const* cursor_b);

    using GetCursorIdx = size_t(void const* cntr, void const* cursor);

    using CursorStepL = void(void const* cntr, void* cursor);

    using CursorStepR = void(void const* cntr, void* cursor);

    using CursorAdvanceL = void(void const* cntr, void* cursor, size_t step);

    using CursorAdvanceR = void(void const* cntr, void* cursor, size_t step);
};

// -----------------------------------------------------------------------------

template <typename AssocCntrType>
bool CheckAssocCntr();

template <typename AssocCntrType>
bool CheckAssocCntr(AssocCntrType const* cntr);

// -----------------------------------------------------------------------------

struct TestAssocCntrMethodWithAbilityFlag {
#pragma push_macro("F")

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define F(ability_name, method_name)                                           \
    static constexpr bool method_name(AssocCntrAbilityFlagType ability_flag,   \
                                      bool when_always) {                      \
        if constexpr (AssocCntrAbilityEnum::ability_name ==                    \
                      AssocCntrAbilityEnum::Always) {                          \
            return when_always;                                                \
        } else {                                                               \
            return (ability_flag & (static_cast<AssocCntrAbilityFlagType>(1)   \
                                    << AssocCntrAbilityEnum::ability_name)) != \
                   0;                                                          \
        }                                                                      \
    }

    // NOLINTEND(cppcoreguidelines-macro-usage)

    F(Always, GetDynamicEnabledAbilityFlag);
    F(Always, GetDynamicDisabledAbilityFlag);

    F(Always, GetCursorSize);
    F(Always, GetWidth);
    F(Always, GetSize);
    F(Always, GetCapacity);

    F(GetLBCursor, GetLBCursor);
    F(GetRBCursor, GetRBCursor);
    F(PeekL, PeekL);
    F(PeekL, ConstPeekL);
    F(PeekR, PeekR);
    F(PeekR, ConstPeekR);
    F(Always, Refer);
    F(Always, ConstRefer);

    F(Always, FnFind);
    F(Always, ConstFnFind);

    F(Always, Insert);
    F(Always, Erase);
    F(Always, EraseAll);

    F(Always, CopyCursor);
    F(Always, AreEqualCursor);
    F(CompareCursor, CompareCursor);
    F(GetCursorDist, GetCursorDist);
    F(GetCursorIdx, GetCursorIdx);
    F(CursorStepL, CursorStepL);
    F(CursorStepR, CursorStepR);
    F(CursorAdvanceL, CursorAdvanceL);
    F(CursorAdvanceR, CursorAdvanceR);

#pragma pop_macro("F")
};

// -----------------------------------------------------------------------------

template <typename AssocCntrType>
struct GetAssocCntrMethodPtr {
    static constexpr AssocCntrMethodSig::GetDynamicEnabledAbilityFlag*
    GetDynamicEnabledAbilityFlag();

    static constexpr AssocCntrMethodSig::GetDynamicDisabledAbilityFlag*
    GetDynamicDisabledAbilityFlag();

    static constexpr AssocCntrMethodSig::GetCursorSize* GetCursorSize();

    static constexpr AssocCntrMethodSig::GetWidth* GetWidth();

    static constexpr AssocCntrMethodSig::GetSize* GetSize();

    static constexpr AssocCntrMethodSig::GetCapacity* GetCapacity();

    static constexpr AssocCntrMethodSig::GetLBCursor* GetLBCursor();

    static constexpr AssocCntrMethodSig::GetRBCursor* GetRBCursor();

    static constexpr AssocCntrMethodSig::PeekL* PeekL();

    static constexpr AssocCntrMethodSig::ConstPeekL* ConstPeekL();

    static constexpr AssocCntrMethodSig::PeekR* PeekR();

    static constexpr AssocCntrMethodSig::ConstPeekR* ConstPeekR();

    static constexpr AssocCntrMethodSig::Refer* Refer();

    static constexpr AssocCntrMethodSig::ConstRefer* ConstRefer();

    static constexpr AssocCntrMethodSig::FnFind* FnFind();

    static constexpr AssocCntrMethodSig::ConstFnFind* ConstFnFind();

    static constexpr AssocCntrMethodSig::Insert* Insert();

    static constexpr AssocCntrMethodSig::Erase* Erase();

    static constexpr AssocCntrMethodSig::EraseAll* EraseAll();

    static constexpr AssocCntrMethodSig::CopyCursor* CopyCursor();

    static constexpr AssocCntrMethodSig::AreEqualCursor* AreEqualCursor();

    static constexpr AssocCntrMethodSig::CompareCursor* CompareCursor();

    static constexpr AssocCntrMethodSig::GetCursorDist* GetCursorDist();

    static constexpr AssocCntrMethodSig::GetCursorIdx* GetCursorIdx();

    static constexpr AssocCntrMethodSig::CursorStepL* CursorStepL();

    static constexpr AssocCntrMethodSig::CursorStepR* CursorStepR();

    static constexpr AssocCntrMethodSig::CursorAdvanceL* CursorAdvanceL();

    static constexpr AssocCntrMethodSig::CursorAdvanceR* CursorAdvanceR();
};

// -----------------------------------------------------------------------------

struct AssocCntrVTable {
    template <typename AssocCntrImpl>
    static constexpr AssocCntrVTable const& Make();

    // -------------------------------------------------------------------------

    size_t (*GetSize)(void const* cntr);

    size_t (*GetCapacity)(void const* cntr);

    void (*GetLBCursor)(void const* cntr, void* dst_cursor);

    void (*GetRBCursor)(void const* cntr, void* dst_cursor);

    void* (*PeekL)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekL)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*PeekR)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekR)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*Refer)(void* cntr, void const* pos_cursor);

    void const* (*ConstRefer)(void const* cntr, void const* pos_cursor);

    void* (*FnFind)(void* cntr, void const* key, FnHash const& key_hash,
                    FnCompare const& key_elem_compare, void* dst_cursor);

    void const* (*ConstFnFind)(void const* cntr, void const* key,
                               FnHash const& key_hash,
                               FnCompare const& key_elem_compare,
                               void* dst_cursor);

    void* (*Insert)(void* cntr, void const* elem, void* dst_cursor);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void const* cntr, void const* cursor, void* dst_cursor);

    bool (*AreEqualCursor)(void const* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void const* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void const* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void const* cntr, void const* cursor);

    void (*CursorStepL)(void const* cntr, void* cursor);

    void (*CursorStepR)(void const* cntr, void* cursor);

    void (*CursorAdvanceL)(void const* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void const* cntr, void* cursor, size_t step);
};

// -----------------------------------------------------------------------------

template <typename IsConstInst>
struct AssocCntrRefTpl {
#pragma push_macro("IfIsNotConstInst")

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define IfIsNotConstInst \
    template <typename _ = void, typename = EnableIf<!IsConstInst::value, _>>

    // NOLINTEND(cppcoreguidelines-macro-usage)

    ZETA_Core_StaticAssert(
        IsAnyOf<IsConstInst, value_wrapper::StaticValueWrapper<false>,
                value_wrapper::StaticValueWrapper<true>>);

    // -------------------------------------------------------------------------

    static constexpr AssocCntrAbilityFlagType static_enabled_ability_flag{
        assoc_cntr_empty_ability_flag
    };

    static constexpr AssocCntrAbilityFlagType static_disabled_ability_flag{
        assoc_cntr_empty_ability_flag
    };

    // -------------------------------------------------------------------------

    template <typename _, typename = EnableIf<!IsConstInst::value, _>>
    operator AssocCntrRefTpl<
        value_wrapper::StaticValueWrapper<!IsConstInst::value>>();

    static AssocCntrAbilityFlagType GetDynamicEnabledAbilityFlag(
        void const* assoc_cntr_ref);

    static AssocCntrAbilityFlagType GetDynamicDisabledAbilityFlag(
        void const* assoc_cntr_ref);

    static size_t GetCursorSize(void const* assoc_cntr_ref);

    static size_t GetSize(void const* assoc_cntr_ref);

    static size_t GetCapacity(void const* assoc_cntr_ref);

    static void GetLBCursor(void const* assoc_cntr_ref, void* dst_cursor);

    static void GetRBCursor(void const* assoc_cntr_ref, void* dst_cursor);

    IfIsNotConstInst static void* PeekL(void* assoc_cntr_ref, void* dst_cursor,
                                        void* dst_elem);

    static void const* ConstPeekL(void const* assoc_cntr_ref, void* dst_cursor,
                                  void* dst_elem);

    IfIsNotConstInst static void* PeekR(void* assoc_cntr_ref, void* dst_cursor,
                                        void* dst_elem);

    static void const* ConstPeekR(void const* assoc_cntr_ref, void* dst_cursor,
                                  void* dst_elem);

    IfIsNotConstInst static void* Refer(void* assoc_cntr_ref,
                                        void const* pos_cursor);

    static void const* ConstRefer(void const* assoc_cntr_ref,
                                  void const* pos_cursor);

    IfIsNotConstInst static void* FnFind(void* assoc_cntr_ref, void const* key,
                                         FnHash const& key_hash,
                                         FnCompare const& key_elem_compare,
                                         void* dst_cursor);

    static void const* ConstFnFind(void const* assoc_cntr_ref, void const* key,
                                   FnHash const& key_hash,
                                   FnCompare const& key_elem_compare,
                                   void* dst_cursor);

    IfIsNotConstInst static void* Insert(void* assoc_cntr_ref, void const* elem,
                                         void* dst_cursor);

    IfIsNotConstInst static void Erase(void* assoc_cntr_ref, void* pos_cursor);

    IfIsNotConstInst static void EraseAll(void* assoc_cntr_ref);

    static void CopyCursor(void const* assoc_cntr_ref, void const* cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void const* assoc_cntr_ref, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void const* assoc_cntr_ref, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void const* assoc_cntr_ref,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(void const* assoc_cntr_ref, void const* cursor);

    static void CursorStepL(void const* assoc_cntr_ref, void* cursor);

    static void CursorStepR(void const* assoc_cntr_ref, void* cursor);

    static void CursorAdvanceL(void const* assoc_cntr_ref, void* cursor,
                               size_t step);

    static void CursorAdvanceR(void const* assoc_cntr_ref, void* cursor,
                               size_t step);

    static bool CheckCntr(void const* assoc_cntr_ref);

    // -------------------------------------------------------------------------

    Conditional<IsConstInst::value, void const*, void*> inst;

    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AssocCntrAbilityFlagType dynamic_enabled_ability_flag;
    AssocCntrAbilityFlagType dynamic_disabled_ability_flag;

    AssocCntrVTable const* vtable;
};

using AssocCntrRef = AssocCntrRefTpl<value_wrapper::StaticValueWrapper<false>>;

using ConstAssocCntrRef =
    AssocCntrRefTpl<value_wrapper::StaticValueWrapper<true>>;

template <typename AssocCntrType>
AssocCntrRef MakeAssocCntrRef(AssocCntrType* cntr);

template <typename AssocCntrType>
ConstAssocCntrRef MakeAssocCntrRef(AssocCntrType const* cntr);

template <typename AssocCntrType>
ConstAssocCntrRef MakeConstAssocCntrRef(AssocCntrType* cntr);

template <typename AssocCntrType>
ConstAssocCntrRef MakeConstAssocCntrRef(AssocCntrType const* cntr);

}  // namespace zeta::core::assoc_cntr
