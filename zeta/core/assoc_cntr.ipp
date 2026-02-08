#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("TestAbility")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                                     \
    (((ability_flag) & (static_cast<::zeta::core::assoc_cntr::AbilityFlag>(1)  \
                        << ::zeta::core::assoc_cntr::AbilityEnum::ability)) != \
     0)

namespace zeta::core::assoc_cntr {

namespace ops {

inline bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                              AbilityFlag static_disabled_ability_flag,
                              AbilityFlag dynamic_enabled_ability_flag,
                              AbilityFlag dynamic_disabled_ability_flag) {
    AbilityFlag ability_flags[]{ static_enabled_ability_flag,
                                 static_disabled_ability_flag,
                                 dynamic_enabled_ability_flag,
                                 dynamic_disabled_ability_flag };

    for (AbilityFlag ability_flag : ability_flags) {
        if ((ability_flag & empty_ability_flag) != empty_ability_flag) {
            return false;
        }
        if ((ability_flag | full_ability_flag) != full_ability_flag) {
            return false;
        }
    }

    for (int i{ 0 }; i < 4; ++i) {
        for (int j{ i + 1 }; j < 4; ++j) {
            if ((ability_flags[i] & ability_flags[j]) != empty_ability_flag) {
                return false;
            }
        }
    }

    return (static_enabled_ability_flag | static_disabled_ability_flag |
            dynamic_enabled_ability_flag | dynamic_disabled_ability_flag) ==
           full_ability_flag;
}

template <typename AssocCntr>
void CheckContract(AssocCntr* cntr) {
    constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper;

    bool bool_val{ false };

    void* void_ptr{ nullptr };
    void const* const_void_ptr{ nullptr };

    size_t size_val{ 0 };

    auto key_hash{ [](void const*, unsigned long long) -> unsigned long long {
        return 0;
    } };

    auto key_elem_compare{ [](void const*, void const*) -> int { return 0; } };

#pragma push_macro("CheckMethod")
#pragma push_macro("CheckMethodOp")

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(AssocCntr::method(__VA_ARGS__)), return_type>);

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, return_type, ...)                 \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, return_type, __VA_ARGS__)                    \
    }

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AssocCntr::GetStaticEnabledAbilityFlag(
                    cntr_ptr_type_wrapper)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AssocCntr::GetStaticDisabledAbilityFlag(
                    cntr_ptr_type_wrapper)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AssocCntr::GetDynamicEnabledAbilityFlag(cntr)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AssocCntr::GetDynamicDisabledAbilityFlag(cntr)),
                AbilityFlag>);

    constexpr AbilityFlag static_disabled_ability_flag{
        AssocCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper)
    };

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method

        size_t,  // return cursor size

        cntr  // inst
    );

    CheckMethodOp(  //
        GetWidth,   // ability
        GetWidth,   // method

        size_t,  // return number of elments

        cntr  // inst
    );

    CheckMethodOp(  //
        GetSize,    // ability
        GetSize,    // method

        size_t,  // return number of elments

        cntr  // inst
    );

    CheckMethodOp(    //
        GetCapacity,  // ability
        GetCapacity,  // method

        size_t,  // return maximum number of elements can be stored

        cntr  // inst
    );

    CheckMethodOp(    //
        GetLBCursor,  // ability
        GetLBCursor,  // method

        TypeAny,  // return void

        cntr,     // inst
        void_ptr  // cursor
    );

    CheckMethodOp(    //
        GetRBCursor,  // ability
        GetRBCursor,  // method
                      //
        TypeAny,      // return void
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(  //
        PeekL,      // ability
        PeekL,      // method
                    //
        void*,      // elem(returned)
                    //
        cntr,       // inst
        bool_val,   // lazy_copy_elem
        void_ptr,   // dst_cursor(optional)
        void_ptr    // mem(optional)
    );

    CheckMethodOp(  //
        PeekR,      // ability
        PeekR,      // method
                    //
        void*,      // return, elem
                    //
        cntr,       // inst
        bool_val,   // lazy_copy_elem
        void_ptr,   // dst_cursor, optional
        void_ptr    // mem, optional
    );

    CheckMethodOp(       //
        Derefer,         // ability
        Derefer,         // method
                         //
        void*,           // return, elem
                         //
        cntr,            // inst
        const_void_ptr,  // pos_cursor
        bool_val,        // lazy_copy_elem
        void_ptr         // mem, optional
    );

    CheckMethodOp(         //
        Find,              // ability
        Find,              // method
                           //
        void*,             // return, elem
                           //
        cntr,              // inst
        const_void_ptr,    // key
        key_hash,          // key_hash
        key_elem_compare,  // key_elem_compare
        bool_val,          // lazy_copy_elem
        void_ptr,          // dst_cursor
        void_ptr           // mem(optional)
    );

    CheckMethodOp(       //
        Insert,          // ability
        Insert,          // method
                         //
        void*,           // return, elem
                         //
        cntr,            // inst
        const_void_ptr,  // elem
        void_ptr         // dst_cursor
    );

    CheckMethodOp(  //
        PopL,       // ability
        PopL,       // method
                    //
        TypeAny,    // return void
                    //
        cntr,       // inst
        size_val    // cnt
    );

    CheckMethodOp(  //
        PopR,       // ability
        PopR,       // method
                    //
        TypeAny,    // return void
                    //
        cntr,       // inst
        size_val    // cnt
    );

    CheckMethodOp(  //
        Erase,      // ability
        Erase,      // method
                    //
        TypeAny,    // return void
                    //
        cntr,       // inst
        void_ptr    // pos_cursor
    );

    CheckMethodOp(  //
        EraseAll,   // ability
        EraseAll,   // method
                    //
        TypeAny,    // return void
                    //
        cntr        // inst
    );

    CheckMethodOp(       //
        CopyCursor,      // ability
        CopyCursor,      // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        const_void_ptr,  // src_cursor
        void_ptr         // dst_cursor
    );

    CheckMethodOp(       //
        AreEqualCursor,  // ability
        AreEqualCursor,  // method
                         //
        bool,            // return bool
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(       //
        CompareCursor,   // ability
        CompareCursor,   // method
                         //
        int,             // return int
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(       //
        GetCursorDist,   // ability
        GetCursorDist,   // method
                         //
        size_t,          // return size_t
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(      //
        GetCursorIdx,   // ability
        GetCursorIdx,   // method
                        //
        size_t,         // return size_t
                        //
        cntr,           // inst
        const_void_ptr  // cursor
    );

    CheckMethodOp(    //
        CursorStepL,  // ability
        CursorStepL,  // method
                      //
        TypeAny,      // return void
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(    //
        CursorStepR,  // ability
        CursorStepR,  // method
                      //
        TypeAny,      // return void
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(       //
        CursorAdvanceL,  // ability
        CursorAdvanceL,  // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr,        // cursor
        size_val         // step
    );

    CheckMethodOp(       //
        CursorAdvanceR,  // ability
        CursorAdvanceR,  // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr,        // cursor
        size_val         // step
    );

#pragma pop_macro("CheckMethod")
#pragma pop_macro("CheckMethodOp")
}

}  // namespace ops

template <typename AssocCntr>
constexpr VTable BasicVTableBuilder<AssocCntr>::Build() {
    constexpr AbilityFlag static_disabled_ability_flag{
        AssocCntr::GetStaticDisabledAbilityFlag(
            type_wrapper::TypeWrapper<AssocCntr*>{})
    };

#pragma push_macro("F")

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(ability, method)                                                  \
    []() constexpr {                                                        \
        if constexpr (TestAbility(static_disabled_ability_flag, ability)) { \
            return static_cast<decltype(method)*>(nullptr);                 \
        } else {                                                            \
            return method;                                                  \
        }                                                                   \
    }()

    constexpr VTable table{
        .GetSize = F(GetSize, GetSize),
        .GetCapacity = F(GetCapacity, GetCapacity),

        .GetLBCursor = F(GetLBCursor, GetLBCursor),
        .GetRBCursor = F(GetRBCursor, GetRBCursor),

        .PeekL = F(PeekL, PeekL),
        .PeekR = F(PeekR, PeekR),

        .Derefer = F(Derefer, Derefer),

        .FnFind = F(Insert, FnFind),

        .FnInsert = F(Insert, FnInsert),

        .PopL = F(PopL, PopL),
        .PopR = F(PopR, PopR),
        .Erase = F(Erase, Erase),
        .EraseAll = F(EraseAll, EraseAll),

        .CopyCursor = F(CopyCursor, CopyCursor),
        .AreEqualCursor = F(AreEqualCursor, AreEqualCursor),
        .CompareCursor = F(CompareCursor, CompareCursor),
        .GetCursorDist = F(GetCursorDist, GetCursorDist),
        .GetCursorIdx = F(GetCursorIdx, GetCursorIdx),
        .CursorStepL = F(CursorStepL, CursorStepL),
        .CursorStepR = F(CursorStepR, CursorStepR),
        .CursorAdvanceL = F(CursorAdvanceL, CursorAdvanceL),
        .CursorAdvanceR = F(CursorAdvanceR, CursorAdvanceR),
    };

#pragma pop_macro("F")

    return table;
}

template <typename AssocCntr>
size_t BasicVTableBuilder<AssocCntr>::GetSize(void* cntr) {
    return AssocCntr::GetSize(static_cast<AssocCntr*>(cntr));
}

template <typename AssocCntr>
constexpr size_t BasicVTableBuilder<AssocCntr>::GetCapacity(void* cntr) {
    return AssocCntr::GetCapacity(static_cast<AssocCntr*>(cntr));
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::GetLBCursor(void* cntr, void* dst_cursor) {
    return AssocCntr::GetLBCursor(static_cast<AssocCntr*>(cntr), dst_cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::GetRBCursor(void* cntr, void* dst_cursor) {
    return AssocCntr::GetRBCursor(static_cast<AssocCntr*>(cntr), dst_cursor);
}

template <typename AssocCntr>
void* BasicVTableBuilder<AssocCntr>::PeekL(void* cntr, bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return AssocCntr::PeekL(static_cast<AssocCntr*>(cntr), lazy_copy_elem,
                            dst_cursor, dst_elem);
}

template <typename AssocCntr>
void* BasicVTableBuilder<AssocCntr>::PeekR(void* cntr, bool lazy_copy_elem,
                                           void* dst_cursor, void* dst_elem) {
    return AssocCntr::PeekR(static_cast<AssocCntr*>(cntr), lazy_copy_elem,
                            dst_cursor, dst_elem);
}

template <typename AssocCntr>
void* BasicVTableBuilder<AssocCntr>::Derefer(void* cntr, void const* pos_cursor,
                                             bool lazy_copy_elem,
                                             void* dst_elem) {
    return AssocCntr::Derefer(static_cast<AssocCntr*>(cntr), pos_cursor,
                              lazy_copy_elem, dst_elem);
}

template <typename AssocCntr>
void* BasicVTableBuilder<AssocCntr>::FnFind(void* cntr, void const* key,
                                            FnHash const& key_hash,
                                            FnCompare const& key_elem_compare,
                                            bool lazy_copy_elem,
                                            void* dst_cursor, void* dst_elem) {
    return AssocCntr::Find(static_cast<AssocCntr*>(cntr), key, key_hash,
                           key_elem_compare, lazy_copy_elem, dst_cursor,
                           dst_elem);
}

template <typename AssocCntr>
void* BasicVTableBuilder<AssocCntr>::FnInsert(void* cntr, void const* elem,
                                              void* dst_cursor) {
    return AssocCntr::Insert(static_cast<AssocCntr*>(cntr), elem, dst_cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::PopL(void* cntr, size_t cnt) {
    return AssocCntr::PopL(static_cast<AssocCntr*>(cntr), cnt);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::PopR(void* cntr, size_t cnt) {
    return AssocCntr::PopR(static_cast<AssocCntr*>(cntr), cnt);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::Erase(void* cntr, void* pos_cursor) {
    return AssocCntr::Erase(static_cast<AssocCntr*>(cntr), pos_cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::EraseAll(void* cntr) {
    return AssocCntr::EraseAll(static_cast<AssocCntr*>(cntr));
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::CopyCursor(void* cntr,
                                               void const* src_cursor,
                                               void* dst_cursor) {
    return AssocCntr::CopyCursor(static_cast<AssocCntr*>(cntr), src_cursor,
                                 dst_cursor);
}

template <typename AssocCntr>
bool BasicVTableBuilder<AssocCntr>::AreEqualCursor(void* cntr,
                                                   void const* cursor_a,
                                                   void const* cursor_b) {
    return AssocCntr::AreEqualCursor(static_cast<AssocCntr*>(cntr), cursor_a,
                                     cursor_b);
}

template <typename AssocCntr>
int BasicVTableBuilder<AssocCntr>::CompareCursor(void* cntr,
                                                 void const* cursor_a,
                                                 void const* cursor_b) {
    return AssocCntr::CompareCursor(static_cast<AssocCntr*>(cntr), cursor_a,
                                    cursor_b);
}

template <typename AssocCntr>
size_t BasicVTableBuilder<AssocCntr>::GetCursorDist(void* cntr,
                                                    void const* cursor_a,
                                                    void const* cursor_b) {
    return AssocCntr::GetCursorDist(static_cast<AssocCntr*>(cntr), cursor_a,
                                    cursor_b);
}

template <typename AssocCntr>
size_t BasicVTableBuilder<AssocCntr>::GetCursorIdx(void* cntr,
                                                   void const* cursor) {
    return AssocCntr::GetCursorIdx(static_cast<AssocCntr*>(cntr), cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::CursorStepL(void* cntr, void* cursor) {
    return AssocCntr::CursorStepL(static_cast<AssocCntr*>(cntr), cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::CursorStepR(void* cntr, void* cursor) {
    return AssocCntr::CursorStepR(static_cast<AssocCntr*>(cntr), cursor);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::CursorAdvanceL(void* cntr, void* cursor,
                                                   size_t step) {
    return AssocCntr::CursorAdvanceL(static_cast<AssocCntr*>(cntr), cursor,
                                     step);
}

template <typename AssocCntr>
void BasicVTableBuilder<AssocCntr>::CursorAdvanceR(void* cntr, void* cursor,
                                                   size_t step) {
    return AssocCntr::CursorAdvanceR(static_cast<AssocCntr*>(cntr), cursor,
                                     step);
}

template <typename AssocCntr>
constexpr VTable VTableBuilder<AssocCntr>::Build() {
    return BasicVTableBuilder<AssocCntr>::Build();
};

namespace detail {

template <typename AssocCntr>
struct VTableHolder_ {
    static constexpr VTable vtable{ VTableBuilder<AssocCntr>::Build() };
};

}  // namespace detail

template <typename AssocCntr>
constexpr VTable const& GetVTable() {
    return detail::VTableHolder_<AssocCntr>::vtable;
}

#pragma push_macro("CallMethod_")
#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...)                              \
    {                                                                 \
        ZETA_Core_DebugAssert(                                        \
            TestAbility(ref->dynamic_enabled_ability_flag, ability)); \
                                                                      \
        auto method_ptr{ ref->vtable->method };                       \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                 \
                                                                      \
        return method_ptr(ref->cntr, __VA_ARGS__);                    \
    }                                                                 \
    ZETA_Core_StaticAssert(true);

constexpr AbilityFlag Ref::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref const*>) {
    return empty_ability_flag;
}

constexpr AbilityFlag Ref::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref*>) {
    return empty_ability_flag;
}

constexpr AbilityFlag Ref::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref const*>) {
    return non_const_ability_flag;
}

AbilityFlag Ref::GetDynamicEnabledAbilityFlag(Ref* ref) {
    return ref->dynamic_enabled_ability_flag;
}

AbilityFlag Ref::GetDynamicEnabledAbilityFlag(Ref const* ref) {
    return non_const_ability_flag &
           GetDynamicEnabledAbilityFlag(const_cast<Ref*>(ref));
}

AbilityFlag Ref::GetDynamicDisabledAbilityFlag(Ref* ref) {
    return ref->dynamic_disabled_ability_flag;
}

AbilityFlag Ref::GetDynamicDisabledAbilityFlag(Ref const* ref) {
    return const_ability_flag &
           GetDynamicDisabledAbilityFlag(const_cast<Ref*>(ref));
}

size_t Ref::GetCursorSize(Ref const* ref) { return ref->cursor_size; }

size_t Ref::GetWidth(Ref const* ref) { return ref->width; }

size_t Ref::GetSize(Ref const* ref) { CallMethod(GetSize, GetSize); }

size_t Ref::GetCapacity(Ref const* ref) {
    CallMethod(GetCapacity, GetCapacity);
}

void Ref::GetLBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

void Ref::GetRBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

void* Ref::PeekL(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
                 void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

void* Ref::PeekR(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
                 void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

void* Ref::Derefer(Ref const* ref, void const* pos_cursor, bool lazy_copy_elem,
                   void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename KeyHash, typename KeyElemCompare>
void* Ref::Find(Ref const* ref, void const* key, KeyHash const& key_hash,
                KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
                void* dst_cursor, void* dst_elem) {
    CallMethod(Find, FnFind, key, key_hash, key_elem_compare, lazy_copy_elem,
               dst_cursor, dst_elem);
}

void* Ref::Insert(Ref* ref, void const* elem, void* dst_cursor) {
    CallMethod(Insert, FnInsert, elem, dst_cursor);
}

void Ref::PopL(Ref* ref, size_t cnt) { CallMethod(PopL, PopL, cnt); }

void Ref::PopR(Ref* ref, size_t cnt) { CallMethod(PopR, PopR, cnt); }

void Ref::Erase(Ref* ref, void* pos_cursor) {
    CallMethod(Erase, Erase, pos_cursor);
}

void Ref::EraseAll(Ref* ref) { CallMethod(EraseAll, EraseAll); }

void Ref::CopyCursor(Ref const* ref, void const* src_cursor, void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

bool Ref::AreEqualCursor(Ref const* ref, void const* cursor_a,
                         void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

int Ref::CompareCursor(Ref const* ref, void const* cursor_a,
                       void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

size_t Ref::GetCursorDist(Ref const* ref, void const* cursor_a,
                          void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

size_t Ref::GetCursorIdx(Ref const* ref, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

void Ref::CursorStepL(Ref const* ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

void Ref::CursorStepR(Ref const* ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

void Ref::CursorAdvanceL(Ref const* ref, void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

void Ref::CursorAdvanceR(Ref const* ref, void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

void Ref::CheckCntr(Ref* ref) {
    ZETA_Core_DebugAssert(ref != nullptr);
    ZETA_Core_DebugAssert(0 < ref->width);
    ZETA_Core_DebugAssert(ref->vtable != nullptr);
    ZETA_Core_DebugAssert(ref->cntr != nullptr);

    AbilityFlag enabled_ability_flag{ ref->dynamic_enabled_ability_flag };

#pragma push_macro("CheckMethod")

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CheckMethod(ability, method)                                     \
    ZETA_Core_DebugAssert(!TestAbility(enabled_ability_flag, ability) || \
                          ref->vtable->method != nullptr);

    // NOLINTEND(cppcoreguidelines-macro-usage)

    CheckMethod(GetSize, GetSize);
    CheckMethod(GetCapacity, GetCapacity);

    CheckMethod(GetLBCursor, GetLBCursor);
    CheckMethod(GetRBCursor, GetRBCursor);
    CheckMethod(PeekL, PeekL);
    CheckMethod(PeekR, PeekR);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Find, FnFind);
    CheckMethod(Insert, FnInsert);

    CheckMethod(PopL, PopL);
    CheckMethod(PopR, PopR);
    CheckMethod(Erase, Erase);
    CheckMethod(EraseAll, EraseAll);

    CheckMethod(CopyCursor, CopyCursor);
    CheckMethod(AreEqualCursor, AreEqualCursor);
    CheckMethod(CompareCursor, CompareCursor);
    CheckMethod(GetCursorDist, GetCursorDist);
    CheckMethod(GetCursorIdx, GetCursorIdx);
    CheckMethod(CursorStepL, CursorStepL);
    CheckMethod(CursorStepR, CursorStepR);
    CheckMethod(CursorAdvanceL, CursorAdvanceL);
    CheckMethod(CursorAdvanceR, CursorAdvanceR);

#pragma pop_macro("CheckMethod")
}

namespace ops {

#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...)                                       \
    {                                                                          \
        auto* cntr{ GetInstPtr(Forward<AssocCntrLike>(cntr_)) };               \
        using AssocCntr = RemovePointer<decltype(cntr)>;                       \
                                                                               \
        CheckContract(cntr);                                                   \
                                                                               \
        constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper; \
                                                                               \
        constexpr AbilityFlag static_enabled_ability_flag{                     \
            AssocCntr::GetStaticEnabledAbilityFlag(cntr_ptr_type_wrapper)      \
        };                                                                     \
                                                                               \
        constexpr AbilityFlag static_disabled_ability_flag{                    \
            AssocCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper)     \
        };                                                                     \
                                                                               \
        if constexpr (TestAbility(static_enabled_ability_flag,                 \
                                  AbilityEnum::ability)) {                     \
            return AssocCntr::method(cntr, __VA_ARGS__);                       \
        } else if constexpr (TestAbility(static_disabled_ability_flag,         \
                                         AbilityEnum::ability)) {              \
            ZETA_Core_StaticAssert(false);                                     \
        } else {                                                               \
            AbilityFlag dynamic_enabled_ability_flag{                          \
                AssocCntr::GetDynamicEnabledAbilityFlag(cntr)                  \
            };                                                                 \
                                                                               \
            ZETA_Core_DebugAssert(TestAbility(dynamic_enabled_ability_flag,    \
                                              AbilityEnum::ability));          \
                                                                               \
            return AssocCntr::method(cntr, __VA_ARGS__);                       \
        }                                                                      \
    }                                                                          \
    ZETA_Core_StaticAssert(true);

template <typename AssocCntrLike>
constexpr AbilityFlag GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrLike>) {
    using AssocCntr =
        RemovePointer<decltype(GetInstPtr(Declval<AssocCntrLike>()))>;

    constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper;

    return AssocCntr::GetStaticEnabledAbilityFlag(cntr_ptr_type_wrapper);
}

template <typename AssocCntrLike>
constexpr AbilityFlag GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<AssocCntrLike>) {
    using AssocCntr =
        RemovePointer<decltype(GetInstPtr(Declval<AssocCntrLike>()))>;

    constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper;

    return AssocCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper);
}

template <typename AssocCntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(AssocCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<AssocCntrLike>(cntr_)) };
    using AssocCntr = RemovePointer<decltype(cntr)>;

    return AssocCntr::GetDynamicEnabledAbilityFlag(cntr);
}

template <typename AssocCntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(AssocCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<AssocCntrLike>(cntr_)) };
    using AssocCntr = RemovePointer<decltype(cntr)>;

    return AssocCntr::GetDynamicDisabledAbilityFlag(cntr);
}

template <typename AssocCntrLike>
decltype(auto) GetCursorSize(AssocCntrLike&& cntr_) {
    CallMethod(GetCursorSize, GetCursorSize);
}

template <typename AssocCntrLike>
size_t GetWidth(AssocCntrLike&& cntr_) {
    CallMethod(GetWidth, GetWidth);
}

template <typename AssocCntrLike>
size_t GetSize(AssocCntrLike&& cntr_) {
    CallMethod(GetSize, GetSize);
}

template <typename AssocCntrLike>
decltype(auto) GetCapacity(AssocCntrLike&& cntr_) {
    CallMethod(GetCapacity, GetCapacity);
}

template <typename AssocCntrLike>
void GetLBCursor(AssocCntrLike&& cntr_, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <typename AssocCntrLike>
void GetRBCursor(AssocCntrLike&& cntr_, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <typename AssocCntrLike>
void* PeekL(AssocCntrLike&& cntr_, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* PeekR(AssocCntrLike&& cntr_, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* Derefer(AssocCntrLike&& cntr_, void const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename AssocCntrLike, typename KeyHash, typename KeyElemCompare>
void* Find(AssocCntrLike&& cntr_, void const* key, KeyHash const& key_hash,
           KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
           void* dst_cursor, void* dst_elem) {
    CallMethod(Find, FnFind, key, key_hash, key_elem_compare, lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* Insert(AssocCntrLike&& cntr_, void const* elem, void* dst_cursor) {
    CallMethod(Insert, FnInsert, elem, dst_cursor);
}

template <typename AssocCntrLike>
void PopL(AssocCntrLike&& cntr_, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

template <typename AssocCntrLike>
void PopR(AssocCntrLike&& cntr_, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

template <typename AssocCntrLike>
void Erase(AssocCntrLike&& cntr_, void* pos_cursor) {
    CallMethod(Erase, Erase, pos_cursor);
}

template <typename AssocCntrLike>
void EraseAll(AssocCntrLike&& cntr_) {
    CallMethod(EraseAll, EraseAll);
}

template <typename AssocCntrLike>
void CopyCursor(AssocCntrLike&& cntr_, void const* src_cursor,
                void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <typename AssocCntrLike>
bool AreEqualCursor(AssocCntrLike&& cntr_, void const* cursor_a,
                    void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <typename AssocCntrLike>
int CompareCursor(AssocCntrLike&& cntr_, void const* cursor_a,
                  void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <typename AssocCntrLike>
size_t GetCursorDist(AssocCntrLike&& cntr_, void const* cursor_a,
                     void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <typename AssocCntrLike>
size_t GetCursorIdx(AssocCntrLike&& cntr_, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <typename AssocCntrLike>
void CursorStepL(AssocCntrLike&& cntr_, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <typename AssocCntrLike>
void CursorStepR(AssocCntrLike&& cntr_, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <typename AssocCntrLike>
void CursorAdvanceL(AssocCntrLike&& cntr_, void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <typename AssocCntrLike>
void CursorAdvanceR(AssocCntrLike&& cntr_, void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename AssocCntrLike>
Ref MakeRef(AssocCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<AssocCntrLike>(cntr_)) };
    using AssocCntr = RemovePointer<AssocCntrLike>;

    CheckContract(cntr);

    size_t cursor_size{ AssocCntr::GetCursorSize(cntr) };
    ZETA_Core_DebugAssert(cursor_size <= ZETA_Core_ushrt_max);

    constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper;

    return {
        .cursor_size = static_cast<unsigned short>(cursor_size),

        .width = AssocCntr::GetWidth(cntr),
        .capacity = AssocCntr::GetCapacity(cntr),

        .dynamic_enabled_ability_flag =
            AssocCntr::GetStaticEnabledAbilityFlag(cntr_ptr_type_wrapper) |
            AssocCntr::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            AssocCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper) |
            AssocCntr::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &GetVTable<AssocCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(cntr)),
    };
}

}  // namespace ops

}  // namespace zeta::core::assoc_cntr
