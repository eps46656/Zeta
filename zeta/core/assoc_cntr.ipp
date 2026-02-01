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

// -----------------------------------------------------------------------------

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
#pragma push_macro("CheckConstMethodOp")

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(AssocCntr::method(__VA_ARGS__)), return_type>);

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, return_type, ...)                 \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        ZETA_Core_StaticAssert(!is_const);                               \
        CheckMethod(method, return_type, __VA_ARGS__)                    \
    }

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckConstMethodOp(ability, method, return_type, ...)            \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, return_type, __VA_ARGS__)                    \
    }

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(AssocCntr::IsConst(cntr_ptr_type_wrapper)), bool>);

    constexpr bool is_const{ AssocCntr::IsConst(cntr_ptr_type_wrapper) };

    using elem_ptr_t = Conditional<is_const, void const*, void*>;

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

    CheckConstMethodOp(  //
        GetCursorSize,   // ability
        GetCursorSize,   // method

        size_t,  // return cursor size

        cntr  // inst
    );

    CheckConstMethodOp(  //
        GetWidth,        // ability
        GetWidth,        // method

        size_t,  // return number of elments

        cntr  // inst
    );

    CheckConstMethodOp(  //
        GetSize,         // ability
        GetSize,         // method

        size_t,  // return number of elments

        cntr  // inst
    );

    CheckConstMethodOp(  //
        GetCapacity,     // ability
        GetCapacity,     // method

        size_t,  // return maximum number of elements can be stored

        cntr  // inst
    );

    CheckConstMethodOp(  //
        GetLBCursor,     // ability
        GetLBCursor,     // method

        TypeAny,  // return void

        cntr,     // inst
        void_ptr  // cursor
    );

    CheckConstMethodOp(  //
        GetRBCursor,     // ability
        GetRBCursor,     // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr         // cursor
    );

    CheckConstMethodOp(  //
        PeekL,           // ability
        PeekL,           // method
                         //
        elem_ptr_t,      // elem(returned)
                         //
        cntr,            // inst
        bool_val,        // lazy_copy_elem
        void_ptr,        // dst_cursor(optional)
        void_ptr         // mem(optional)
    );

    CheckConstMethodOp(  //
        PeekR,           // ability
        PeekR,           // method
                         //
        void*,           // return, elem
                         //
        cntr,            // inst
        bool_val,        // lazy_copy_elem
        void_ptr,        // dst_cursor, optional
        void_ptr         // mem, optional
    );

    CheckConstMethodOp(  //
        Derefer,         // ability
        Derefer,         // method
                         //
        elem_ptr_t,      // return, elem
                         //
        cntr,            // inst
        const_void_ptr,  // pos_cursor
        bool_val,        // lazy_copy_elem
        void_ptr         // mem, optional
    );

    CheckConstMethodOp(    //
        Find,              // ability
        Find,              // method
                           //
        elem_ptr_t,        // return, elem
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
        cntr,     // inst
        void_ptr  // pos_cursor
    );

    CheckMethodOp(  //
        EraseAll,   // ability
        EraseAll,   // method
                    //
        TypeAny,    // return void
                    //
        cntr        // inst
    );

    CheckConstMethodOp(  //
        CopyCursor,      // ability
        CopyCursor,      // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        const_void_ptr,  // src_cursor
        void_ptr         // dst_cursor
    );

    CheckConstMethodOp(  //
        AreEqualCursor,  // ability
        AreEqualCursor,  // method
                         //
        bool,            // return bool
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckConstMethodOp(  //
        CompareCursor,   // ability
        CompareCursor,   // method
                         //
        int,             // return int
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckConstMethodOp(  //
        GetCursorDist,   // ability
        GetCursorDist,   // method
                         //
        size_t,          // return size_t
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckConstMethodOp(  //
        GetCursorIdx,    // ability
        GetCursorIdx,    // method
                         //
        size_t,          // return size_t
                         //
        cntr,            // inst
        const_void_ptr   // cursor
    );

    CheckConstMethodOp(  //
        CursorStepL,     // ability
        CursorStepL,     // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr         // cursor
    );

    CheckConstMethodOp(  //
        CursorStepR,     // ability
        CursorStepR,     // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr         // cursor
    );

    CheckConstMethodOp(  //
        CursorAdvanceL,  // ability
        CursorAdvanceL,  // method
                         //
        TypeAny,         // return void
                         //
        cntr,            // inst
        void_ptr,        // cursor
        size_val         // step
    );

    CheckConstMethodOp(  //
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
#pragma pop_macro("CheckConstMethodOp")
}

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
#define CallMethod(ability, method, ...)                                    \
    {                                                                       \
        auto ref{ reinterpret_cast<                                         \
            Conditional<core::IsConst<RemovePointer<decltype(ref_view)>>,   \
                        Ref<ConstTag> const*, Ref<ConstTag>*>>(ref_view) }; \
                                                                            \
        ZETA_Core_DebugAssert(                                              \
            TestAbility(ref->dynamic_enabled_ability_flag, ability));       \
                                                                            \
        auto method_ptr{ ref->vtable->method };                             \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                       \
                                                                            \
        return method_ptr(ref->cntr, __VA_ARGS__);                          \
    }                                                                       \
    ZETA_Core_StaticAssert(true);

template <typename ConstTag>
constexpr bool RefView<ConstTag>::IsConst(type_wrapper::TypeWrapper<RefView*>) {
    return ConstTag::value;
}

template <typename ConstTag>
constexpr bool RefView<ConstTag>::IsConst(
    type_wrapper::TypeWrapper<RefView const*>) {
    return false;
}

template <typename ConstTag>
constexpr AbilityFlag RefView<ConstTag>::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<RefView const*>) {
    return empty_ability_flag;
}

template <typename ConstTag>
constexpr AbilityFlag RefView<ConstTag>::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<RefView*>) {
    return empty_ability_flag;
}

template <typename ConstTag>
constexpr AbilityFlag RefView<ConstTag>::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<RefView const*>) {
    return non_const_ability_flag;
}

template <typename ConstTag>
AbilityFlag RefView<ConstTag>::GetDynamicEnabledAbilityFlag(RefView* ref_view) {
    auto ref{ reinterpret_cast<Ref<ConstTag>*>(ref_view) };

    return ref->dynamic_enabled_ability_flag;
}

template <typename ConstTag>
AbilityFlag RefView<ConstTag>::GetDynamicEnabledAbilityFlag(
    RefView const* ref_view) {
    return non_const_ability_flag &
           GetDynamicEnabledAbilityFlag(const_cast<RefView*>(ref_view));
}

template <typename ConstTag>
AbilityFlag RefView<ConstTag>::GetDynamicDisabledAbilityFlag(
    RefView* ref_view) {
    auto ref{ reinterpret_cast<Ref<ConstTag>*>(ref_view) };

    return ref->dynamic_disabled_ability_flag;
}

template <typename ConstTag>
AbilityFlag RefView<ConstTag>::GetDynamicDisabledAbilityFlag(
    RefView const* ref_view) {
    return const_ability_flag &
           GetDynamicDisabledAbilityFlag(const_cast<RefView*>(ref_view));
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetCursorSize(RefView const* ref_view) {
    auto ref{ reinterpret_cast<Ref<ConstTag> const*>(ref_view) };

    return ref->cursor_size;
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetWidth(RefView const* ref_view) {
    auto ref{ reinterpret_cast<Ref<ConstTag> const*>(ref_view) };

    return ref->width;
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetSize(RefView const* ref_view) {
    CallMethod(GetSize, GetSize);
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetCapacity(RefView const* ref_view) {
    CallMethod(GetCapacity, GetCapacity);
}

template <typename ConstTag>
void RefView<ConstTag>::GetLBCursor(RefView const* ref_view, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::GetRBCursor(RefView const* ref_view, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <typename ConstTag>
Conditional<ConstTag::value, void const*, void*> RefView<ConstTag>::PeekL(
    RefView* ref_view, bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename ConstTag>
void const* RefView<ConstTag>::PeekL(RefView const* ref_view,
                                     bool lazy_copy_elem, void* dst_cursor,
                                     void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename ConstTag>
Conditional<ConstTag::value, void const*, void*> RefView<ConstTag>::PeekR(
    RefView* ref_view, bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename ConstTag>
void const* RefView<ConstTag>::PeekR(RefView const* ref_view,
                                     bool lazy_copy_elem, void* dst_cursor,
                                     void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename ConstTag>
Conditional<ConstTag::value, void const*, void*> RefView<ConstTag>::Derefer(
    RefView* ref_view, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename ConstTag>
void const* RefView<ConstTag>::Derefer(RefView const* ref_view,
                                       void const* pos_cursor,
                                       bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename ConstTag>
template <typename KeyHash, typename KeyElemCompare>
Conditional<ConstTag::value, void const*, void*> RefView<ConstTag>::Find(
    RefView* ref_view, void const* key, KeyHash const& key_hash,
    KeyElemCompare const& key_elem_compare, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    CallMethod(Find, FnFind, key, key_hash, key_elem_compare, lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename ConstTag>
template <typename KeyHash, typename KeyElemCompare>
void const* RefView<ConstTag>::Find(RefView const* ref_view, void const* key,
                                    KeyHash const& key_hash,
                                    KeyElemCompare const& key_elem_compare,
                                    bool lazy_copy_elem, void* dst_cursor,
                                    void* dst_elem) {
    CallMethod(Find, FnFind, key, key_hash, key_elem_compare, lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename ConstTag>
void* RefView<ConstTag>::Insert(RefView* ref_view, void const* elem,
                                void* dst_cursor) {
    CallMethod(Insert, FnInsert, elem, dst_cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::PopL(RefView* ref_view, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

template <typename ConstTag>
void RefView<ConstTag>::PopR(RefView* ref_view, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

template <typename ConstTag>
void RefView<ConstTag>::Erase(RefView* ref_view, void* pos_cursor) {
    CallMethod(Erase, Erase, pos_cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::EraseAll(RefView* ref_view) {
    CallMethod(EraseAll, EraseAll);
}

template <typename ConstTag>
void RefView<ConstTag>::CopyCursor(RefView const* ref_view,
                                   void const* src_cursor, void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <typename ConstTag>
bool RefView<ConstTag>::AreEqualCursor(RefView const* ref_view,
                                       void const* cursor_a,
                                       void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <typename ConstTag>
int RefView<ConstTag>::CompareCursor(RefView const* ref_view,
                                     void const* cursor_a,
                                     void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetCursorDist(RefView const* ref_view,
                                        void const* cursor_a,
                                        void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <typename ConstTag>
size_t RefView<ConstTag>::GetCursorIdx(RefView const* ref_view,
                                       void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::CursorStepL(RefView const* ref_view, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::CursorStepR(RefView const* ref_view, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <typename ConstTag>
void RefView<ConstTag>::CursorAdvanceL(RefView const* ref_view, void* cursor,
                                       size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <typename ConstTag>
void RefView<ConstTag>::CursorAdvanceR(RefView const* ref_view, void* cursor,
                                       size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename ConstTag>
void RefView<ConstTag>::CheckCntr(RefView* ref_view) {
    auto ref{ reinterpret_cast<Ref<ConstTag>>(ref_view) };

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

    CheckMethod(Find, Find);
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

template <typename AssocCntr>
auto MakeRef(AssocCntr* cntr) {
    CheckContract(cntr);

    size_t cursor_size{ AssocCntr::GetCursorSize(cntr) };
    ZETA_Core_DebugAssert(cursor_size <= ZETA_Core_ushrt_max);

    constexpr type_wrapper::TypeWrapper<AssocCntr*> cntr_ptr_type_wrapper;

    return Ref<value_wrapper::StaticValueWrapper<AssocCntr::IsConst(
        cntr_ptr_type_wrapper)>>{
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

}  // namespace zeta::core::assoc_cntr
