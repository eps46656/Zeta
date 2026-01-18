#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::assoc_cntr {

inline bool CheckAssocCntrAbilityFlags(
    AssocCntrAbilityFlagType static_enabled_ability_flag,
    AssocCntrAbilityFlagType static_disabled_ability_flag,
    AssocCntrAbilityFlagType dynamic_enabled_ability_flag,
    AssocCntrAbilityFlagType dynamic_disabled_ability_flag) {
    AssocCntrAbilityFlagType ability_flags[]{ static_enabled_ability_flag,
                                              static_disabled_ability_flag,
                                              dynamic_enabled_ability_flag,
                                              dynamic_disabled_ability_flag };

    for (AssocCntrAbilityFlagType ability_flag : ability_flags) {
        if ((ability_flag & assoc_cntr_empty_ability_flag) !=
            assoc_cntr_empty_ability_flag) {
            return false;
        }
        if ((ability_flag | assoc_cntr_full_ability_flag) !=
            assoc_cntr_full_ability_flag) {
            return false;
        }
    }

    for (int i{ 0 }; i < 4; ++i) {
        for (int j{ i + 1 }; j < 4; ++j) {
            if ((ability_flags[i] & ability_flags[j]) !=
                assoc_cntr_empty_ability_flag) {
                return false;
            }
        }
    }

    return (static_enabled_ability_flag | static_disabled_ability_flag |
            dynamic_enabled_ability_flag | dynamic_disabled_ability_flag) ==
           assoc_cntr_full_ability_flag;
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename AssocCntrType, bool HashInst>
struct CheckAssocCntr_ {
    static bool F(AssocCntrType const* cntr) {
        ZETA_Core_StaticAssert(
            IsAnyOf<decltype(AssocCntrType::static_enabled_ability_flag),
                    AssocCntrAbilityFlagType const>);

        ZETA_Core_StaticAssert(
            IsAnyOf<decltype(AssocCntrType::static_disabled_ability_flag),
                    AssocCntrAbilityFlagType const>);

        constexpr AssocCntrAbilityFlagType static_enabled_ability_flag{
            AssocCntrType::static_enabled_ability_flag
        };

        constexpr AssocCntrAbilityFlagType static_disabled_ability_flag{
            AssocCntrType::static_disabled_ability_flag
        };

        ZETA_Core_StaticAssert(
            (static_enabled_ability_flag & assoc_cntr_empty_ability_flag) ==
            assoc_cntr_empty_ability_flag);

        // -------------------------------------------------------------------------

#pragma push_macro("CheckMethodSig")

        // NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CheckMethodSig(method_name)                                          \
    if constexpr (!TestAssocCntrMethodWithAbilityFlag::method_name(          \
                      static_disabled_ability_flag, false)) {                \
        ZETA_Core_StaticAssert(IsAnyOf<decltype(AssocCntrType::method_name), \
                                       AssocCntrMethodSig::method_name>);    \
    }                                                                        \
    ZETA_Core_StaticAssert(true)

        // NOLINTEND(cppcoreguidelines-macro-usage)

        // -------------------------------------------------------------------------

        CheckMethodSig(GetDynamicEnabledAbilityFlag);
        CheckMethodSig(GetDynamicDisabledAbilityFlag);

        // -------------------------------------------------------------------------

        if constexpr (HashInst) {
            AssocCntrAbilityFlagType dynamic_enabled_ability_flag{
                AssocCntrType::GetDynamicEnabledAbilityFlag(cntr)
            };

            AssocCntrAbilityFlagType dynamic_disabled_ability_flag{
                AssocCntrType::GetDynamicDisabledAbilityFlag(cntr)
            };

            if (!CheckAssocCntrAbilityFlags(static_enabled_ability_flag,
                                            static_disabled_ability_flag,
                                            dynamic_enabled_ability_flag,
                                            dynamic_disabled_ability_flag)) {
                return false;
            }
        }

        // -------------------------------------------------------------------------

        CheckMethodSig(GetCursorSize);
        CheckMethodSig(GetWidth);
        CheckMethodSig(GetSize);
        CheckMethodSig(GetCapacity);

        CheckMethodSig(GetLBCursor);
        CheckMethodSig(GetRBCursor);
        CheckMethodSig(PeekL);
        CheckMethodSig(ConstPeekL);
        CheckMethodSig(PeekR);
        CheckMethodSig(ConstPeekR);
        CheckMethodSig(Refer);
        CheckMethodSig(ConstRefer);

        CheckMethodSig(FnFind);
        CheckMethodSig(ConstFnFind);

        CheckMethodSig(Erase);
        CheckMethodSig(EraseAll);

        CheckMethodSig(CopyCursor);
        CheckMethodSig(AreEqualCursor);
        CheckMethodSig(CompareCursor);
        CheckMethodSig(GetCursorDist);
        CheckMethodSig(GetCursorIdx);
        CheckMethodSig(CursorStepL);
        CheckMethodSig(CursorStepR);
        CheckMethodSig(CursorAdvanceL);
        CheckMethodSig(CursorAdvanceR);

#pragma pop_macro("CheckMethodSig")

        return true;
    }
};

}  // namespace detail

template <typename AssocCntrType>
bool CheckAssocCntr() {
    return detail::CheckAssocCntr_<AssocCntrType, false>::F(nullptr);
}

template <typename AssocCntrType>
bool CheckAssocCntr(AssocCntrType const* cntr) {
    return detail::CheckAssocCntr_<AssocCntrType, true>::F(cntr);
}

// -----------------------------------------------------------------------------

#pragma push_macro("GetMethodPtr")

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define GetMethodPtr(method_name)                                          \
    template <typename AssocCntrType>                                      \
    constexpr AssocCntrMethodSig::method_name*                             \
    GetAssocCntrMethodPtr<AssocCntrType>::method_name() {                  \
        if constexpr (TestAssocCntrMethodWithAbilityFlag::method_name(     \
                          AssocCntrType::static_disabled_ability_flag,     \
                          false)) {                                        \
            return static_cast<AssocCntrMethodSig::method_name*>(nullptr); \
        } else {                                                           \
            return &AssocCntrType::method_name;                            \
        }                                                                  \
    }                                                                      \
    ZETA_Core_StaticAssert(true)

// NOLINTEND(cppcoreguidelines-macro-usage)

GetMethodPtr(GetDynamicEnabledAbilityFlag);
GetMethodPtr(GetDynamicDisabledAbilityFlag);

GetMethodPtr(GetCursorSize);
GetMethodPtr(GetWidth);
GetMethodPtr(GetSize);
GetMethodPtr(GetCapacity);

GetMethodPtr(GetLBCursor);
GetMethodPtr(GetRBCursor);

GetMethodPtr(PeekL);
GetMethodPtr(ConstPeekL);
GetMethodPtr(PeekR);
GetMethodPtr(ConstPeekR);

GetMethodPtr(Refer);
GetMethodPtr(ConstRefer);

GetMethodPtr(FnFind);
GetMethodPtr(ConstFnFind);

GetMethodPtr(Insert);

GetMethodPtr(Erase);
GetMethodPtr(EraseAll);

GetMethodPtr(CopyCursor);
GetMethodPtr(AreEqualCursor);
GetMethodPtr(CompareCursor);
GetMethodPtr(GetCursorDist);
GetMethodPtr(GetCursorIdx);
GetMethodPtr(CursorStepL);
GetMethodPtr(CursorStepR);
GetMethodPtr(CursorAdvanceL);
GetMethodPtr(CursorAdvanceR);

#pragma pop_macro("GetMethodPtr")

// -----------------------------------------------------------------------------

namespace detail {

template <typename AssocCntrType>
struct MakeAssocCntrVTable_ {
    using GetMethodPtr = GetAssocCntrMethodPtr<AssocCntrType>;

    static constexpr AssocCntrVTable table{
        .GetSize = GetMethodPtr::GetSize(),
        .GetCapacity = GetMethodPtr::GetCapacity(),

        .GetLBCursor = GetMethodPtr::GetLBCursor(),
        .GetRBCursor = GetMethodPtr::GetRBCursor(),

        .PeekL = GetMethodPtr::PeekL(),
        .ConstPeekL = GetMethodPtr::ConstPeekL(),
        .PeekR = GetMethodPtr::PeekR(),
        .ConstPeekR = GetMethodPtr::ConstPeekR(),
        .Refer = GetMethodPtr::Refer(),
        .ConstRefer = GetMethodPtr::ConstRefer(),

        .FnFind = GetMethodPtr::FnFind(),
        .ConstFnFind = GetMethodPtr::ConstFnFind(),

        .Insert = GetMethodPtr::Insert(),

        .Erase = GetMethodPtr::Erase(),
        .EraseAll = GetMethodPtr::EraseAll(),

        .CopyCursor = GetMethodPtr::CopyCursor(),
        .AreEqualCursor = GetMethodPtr::AreEqualCursor(),
        .CompareCursor = GetMethodPtr::CompareCursor(),
        .GetCursorDist = GetMethodPtr::GetCursorDist(),
        .GetCursorIdx = GetMethodPtr::GetCursorIdx(),
        .CursorStepL = GetMethodPtr::CursorStepL(),
        .CursorStepR = GetMethodPtr::CursorStepR(),
        .CursorAdvanceL = GetMethodPtr::CursorAdvanceL(),
        .CursorAdvanceR = GetMethodPtr::CursorAdvanceR(),
    };
};

}  // namespace detail

template <typename AssocCntrType>
constexpr AssocCntrVTable const& AssocCntrVTable::Make() {
    return detail::MakeAssocCntrVTable_<AssocCntrType>::table;
}

// -----------------------------------------------------------------------------

#pragma push_macro("CallMethod")

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CallMethod(method_name, ...)                                           \
    {                                                                          \
        auto assoc_cntr_ref{ static_cast<                                      \
            Conditional<IsAnyOf<decltype(assoc_cntr_ref_), void const*>,       \
                        AssocCntrRefTpl const*, AssocCntrRefTpl*>>(            \
            assoc_cntr_ref_) };                                                \
                                                                               \
        ZETA_Core_DebugAssert(TestAssocCntrMethodWithAbilityFlag::method_name( \
            assoc_cntr_ref->dynamic_enabled_ability_flag, true));              \
                                                                               \
        auto method{ assoc_cntr_ref->vtable->method_name };                    \
        ZETA_Core_DebugAssert(method != nullptr);                              \
                                                                               \
        return method(assoc_cntr_ref->inst, __VA_ARGS__);                      \
    }                                                                          \
    ZETA_Core_StaticAssert(true);

// NOLINTEND(cppcoreguidelines-macro-usage)

template <typename IsConstInst>
template <typename, typename>
AssocCntrRefTpl<IsConstInst>::operator AssocCntrRefTpl<
    value_wrapper::StaticValueWrapper<!IsConstInst::value>>() {
    return *this;
}

template <typename IsConst>
AssocCntrAbilityFlagType AssocCntrRefTpl<IsConst>::GetDynamicEnabledAbilityFlag(
    void const* assoc_cntr_ref_) {
    CallMethod(GetDynamicEnabledAbilityFlag);
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetCursorSize(void const* assoc_cntr_ref_) {
    auto assoc_cntr_ref{ static_cast<AssocCntrRefTpl const*>(assoc_cntr_ref_) };
    return assoc_cntr_ref->cursor_size;
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetSize(void const* assoc_cntr_ref_) {
    CallMethod(GetSize);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::GetLBCursor(void const* assoc_cntr_ref_,
                                           void* dst_cursor) {
    CallMethod(GetLBCursor, dst_cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::GetRBCursor(void const* assoc_cntr_ref_,
                                           void* dst_cursor) {
    CallMethod(GetRBCursor, dst_cursor);
}

template <typename IsConst>
template <typename _, typename>
void* AssocCntrRefTpl<IsConst>::PeekL(void* assoc_cntr_ref_, void* dst_cursor,
                                      void* dst_elem) {
    CallMethod(PeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstPeekL(void const* assoc_cntr_ref_,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    CallMethod(ConstPeekL, dst_cursor, dst_elem);
}

template <typename IsConst>
template <typename _, typename>
void* AssocCntrRefTpl<IsConst>::PeekR(void* assoc_cntr_ref_, void* dst_cursor,
                                      void* dst_elem) {
    CallMethod(PeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstPeekR(void const* assoc_cntr_ref_,
                                                 void* dst_cursor,
                                                 void* dst_elem) {
    CallMethod(ConstPeekR, dst_cursor, dst_elem);
}

template <typename IsConst>
template <typename _, typename>
void* AssocCntrRefTpl<IsConst>::Refer(void* assoc_cntr_ref_,
                                      void const* pos_cursor) {
    CallMethod(Refer, pos_cursor);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstRefer(void const* assoc_cntr_ref_,
                                                 void const* pos_cursor) {
    CallMethod(ConstRefer, pos_cursor);
}

template <typename IsConst>
template <typename _, typename>
void* AssocCntrRefTpl<IsConst>::FnFind(void* assoc_cntr_ref_, void const* key,
                                       FnHash const& key_hash,
                                       FnCompare const& key_elem_compare,
                                       void* dst_cursor) {
    CallMethod(FnFind, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename IsConst>
void const* AssocCntrRefTpl<IsConst>::ConstFnFind(
    void const* assoc_cntr_ref_, void const* key, FnHash const& key_hash,
    FnCompare const& key_elem_compare, void* dst_cursor) {
    CallMethod(ConstFnFind, key, key_hash, key_elem_compare, dst_cursor);
}

template <typename IsConst>
template <typename _, typename>
void* AssocCntrRefTpl<IsConst>::Insert(void* assoc_cntr_ref_, void const* elem,
                                       void* dst_cursor) {
    CallMethod(Insert, elem, dst_cursor);
}

template <typename IsConst>
template <typename _, typename>
void AssocCntrRefTpl<IsConst>::Erase(void* assoc_cntr_ref_, void* pos_cursor) {
    CallMethod(Erase, pos_cursor);
}

template <typename IsConst>
template <typename _, typename>
void AssocCntrRefTpl<IsConst>::EraseAll(void* assoc_cntr_ref_) {
    CallMethod(EraseAll);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CopyCursor(void const* assoc_cntr_ref_,
                                          void const* cursor,
                                          void* dst_cursor) {
    CallMethod(CopyCursor, cursor, dst_cursor);
}

template <typename IsConst>
bool AssocCntrRefTpl<IsConst>::AreEqualCursor(void const* assoc_cntr_ref_,
                                              void const* cursor_a,
                                              void const* cursor_b) {
    CallMethod(AreEqualCursor, cursor_a, cursor_b);
}

template <typename IsConst>
int AssocCntrRefTpl<IsConst>::CompareCursor(void const* assoc_cntr_ref_,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    CallMethod(CompareCursor, cursor_a, cursor_b);
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetCursorDist(void const* assoc_cntr_ref_,
                                               void const* cursor_a,
                                               void const* cursor_b) {
    CallMethod(GetCursorDist, cursor_a, cursor_b);
}

template <typename IsConst>
size_t AssocCntrRefTpl<IsConst>::GetCursorIdx(void const* assoc_cntr_ref_,
                                              void const* cursor) {
    CallMethod(GetCursorIdx, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorStepL(void const* assoc_cntr_ref_,
                                           void* cursor) {
    CallMethod(CursorStepL, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorStepR(void const* assoc_cntr_ref_,
                                           void* cursor) {
    CallMethod(CursorStepR, cursor);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorAdvanceL(void const* assoc_cntr_ref_,
                                              void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, cursor, step);
}

template <typename IsConst>
void AssocCntrRefTpl<IsConst>::CursorAdvanceR(void const* assoc_cntr_ref_,
                                              void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename IsConst>
bool AssocCntrRefTpl<IsConst>::CheckCntr(void const* assoc_cntr_ref_) {
    auto assoc_cntr_ref{ static_cast<AssocCntrRefTpl const*>(assoc_cntr_ref_) };

    if (!(assoc_cntr_ref != nullptr)) { return false; }
    if (!(assoc_cntr_ref->inst != nullptr)) { return false; }
    if (!(assoc_cntr_ref->vtable != nullptr)) { return false; }

    CheckAssocCntrAbilityFlags(static_enabled_ability_flag,
                               static_disabled_ability_flag,
                               assoc_cntr_ref->dynamic_enabled_ability_flag,
                               assoc_cntr_ref->dynamic_disabled_ability_flag);

    AssocCntrAbilityFlagType enabled_ability_flag{
        static_enabled_ability_flag |
        assoc_cntr_ref->dynamic_enabled_ability_flag
    };

#pragma push_macro("CheckVTableMethodPtr")

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CheckVTableMethodPtr(method_name)                                     \
    if (TestAssocCntrMethodWithAbilityFlag::method_name(enabled_ability_flag, \
                                                        true) &&              \
        assoc_cntr_ref->vtable->method_name == nullptr) {                     \
        return false;                                                         \
    }

    // NOLINTEND(cppcoreguidelines-macro-usage)

    CheckVTableMethodPtr(GetSize);
    CheckVTableMethodPtr(GetCapacity);

    CheckVTableMethodPtr(GetLBCursor);
    CheckVTableMethodPtr(GetRBCursor);
    CheckVTableMethodPtr(PeekL);
    CheckVTableMethodPtr(ConstPeekL);
    CheckVTableMethodPtr(PeekR);
    CheckVTableMethodPtr(ConstPeekR);
    CheckVTableMethodPtr(Refer);
    CheckVTableMethodPtr(ConstRefer);

    CheckVTableMethodPtr(FnFind);
    CheckVTableMethodPtr(ConstFnFind);

    CheckVTableMethodPtr(Insert);

    CheckVTableMethodPtr(Erase);
    CheckVTableMethodPtr(EraseAll);

    CheckVTableMethodPtr(CopyCursor);
    CheckVTableMethodPtr(AreEqualCursor);
    CheckVTableMethodPtr(CompareCursor);
    CheckVTableMethodPtr(GetCursorDist);
    CheckVTableMethodPtr(GetCursorIdx);
    CheckVTableMethodPtr(CursorStepL);
    CheckVTableMethodPtr(CursorStepR);
    CheckVTableMethodPtr(CursorAdvanceL);
    CheckVTableMethodPtr(CursorAdvanceR);

#pragma pop_macro("CheckVTableMethodPtr")

    return true;
}

namespace detail {

template <typename AssocCntrType, bool IsConstInst>
struct MakeAssocCntrRef_ {
    static AssocCntrRefTpl<value_wrapper::StaticValueWrapper<IsConstInst>> F(
        Conditional<IsConstInst, AssocCntrType const*, AssocCntrType*> cntr) {
        ZETA_Core_DebugAssert(CheckAssocCntr(cntr));

        size_t cursor_size{ AssocCntrType::GetCursorSize(cntr) };
        ZETA_Core_DebugAssert(cursor_size <= ZETA_Core_ushrt_max);

        return {
            .inst = cntr,
            .cursor_size = static_cast<unsigned short>(cursor_size),
            .width = AssocCntrType::GetWidth(cntr),
            .capacity = AssocCntrType::GetCapacity(cntr),
            .dynamic_enabled_ability_flag =
                AssocCntrType::static_enabled_ability_flag |
                AssocCntrType::GetDynamicEnabledAbilityFlag(cntr),
            .dynamic_disabled_ability_flag =
                AssocCntrType::static_disabled_ability_flag |
                AssocCntrType::GetDynamicDisabledAbilityFlag(cntr),
            .vtable = &AssocCntrVTable::Make<AssocCntrType>(),
        };
    }
};

}  // namespace detail

template <typename AssocCntrType>
AssocCntrRef MakeAssocCntrRef(AssocCntrType* cntr) {
    return detail::MakeAssocCntrRef_<AssocCntrType, false>::F(cntr);
}

template <typename AssocCntrType>
ConstAssocCntrRef MakeAssocCntrRef(AssocCntrType const* cntr) {
    return detail::MakeAssocCntrRef_<AssocCntrType, true>::F(cntr);
}

template <typename AssocCntrType>
ConstAssocCntrRef MakeConstAssocCntrRef(AssocCntrType* cntr) {
    return detail::MakeAssocCntrRef_<AssocCntrType, true>::F(cntr);
}

template <typename AssocCntrType>
ConstAssocCntrRef MakeConstAssocCntrRef(AssocCntrType const* cntr) {
    return detail::MakeAssocCntrRef_<AssocCntrType, true>::F(cntr);
}

}  // namespace zeta::core::assoc_cntr
