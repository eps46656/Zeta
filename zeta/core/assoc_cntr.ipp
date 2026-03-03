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

namespace zeta::core {

constexpr bool assoc_cntr::ops::CheckAbilityFlags(
    assoc_cntr::AbilityFlag static_enabled_ability_flag,
    assoc_cntr::AbilityFlag static_disabled_ability_flag) {
    assoc_cntr::AbilityFlag ability_flags[]{ static_enabled_ability_flag,
                                             static_disabled_ability_flag };

    for (AbilityFlag ability_flag : ability_flags) {
        if ((ability_flag & empty_ability_flag) != empty_ability_flag) {
            return false;
        }

        if ((ability_flag | full_ability_flag) != full_ability_flag) {
            return false;
        }
    }

    return (static_enabled_ability_flag & static_disabled_ability_flag) ==
           empty_ability_flag;
}

inline bool assoc_cntr::ops::CheckAbilityFlags(
    AbilityFlag static_enabled_ability_flag,
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

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ret, ability_name, method_name, ...)                       \
    using AssocCntr = meta::RemovePointer<decltype(utils::GetInstPtr(         \
        meta::Forward<AssocCntrLike>(cntr)))>;                                \
                                                                              \
    if constexpr (!TestAbility((GetStaticEnabledAbilityFlag<AssocCntr>)(),    \
                               ability_name)) {                               \
        ZETA_Core_StaticAssert(!TestAbility(                                  \
            (GetStaticDisabledAbilityFlag<AssocCntr>)(), ability_name));      \
                                                                              \
        ZETA_Core_DebugAssert(                                                \
            TestAbility((GetDynamicEnabledAbilityFlag)(cntr), ability_name)); \
    }                                                                         \
                                                                              \
    if constexpr (ret) {                                                      \
        return Traits<AssocCntr>::method_name(__VA_ARGS__);                   \
    } else {                                                                  \
        Traits<AssocCntr>::method_name(__VA_ARGS__);                          \
    };                                                                        \
                                                                              \
    ZETA_Core_StaticAssert(true)

template <typename AssocCntrLike>
auto* assoc_cntr::ops::GetReferedInstPtr(AssocCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr_)) };

    return Traits<meta::RemovePointer<decltype(cntr)>>::GetReferedInstPtr(cntr);
}

template <typename AssocCntr>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::ops::GetStaticEnabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<AssocCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<AssocCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_enabled_ability_flag;
}

template <typename AssocCntr>
constexpr assoc_cntr::AbilityFlag
assoc_cntr::ops::GetStaticDisabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<AssocCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<AssocCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_disabled_ability_flag;
}

template <typename AssocCntrLike>
assoc_cntr::AbilityFlag assoc_cntr::ops::GetDynamicEnabledAbilityFlag(
    AssocCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr_)) };

    using AssocCntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<AssocCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<AssocCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        Traits<AssocCntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        Traits<AssocCntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    (CheckAbilityFlags)(
        static_enabled_ability_flag, static_disabled_ability_flag,
        dynamic_enabled_ability_flag, dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename AssocCntrLike>
assoc_cntr::AbilityFlag assoc_cntr::ops::GetDynamicDisabledAbilityFlag(
    AssocCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr_)) };

    using AssocCntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<AssocCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<AssocCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        Traits<AssocCntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        Traits<AssocCntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    (CheckAbilityFlags)(
        static_enabled_ability_flag, static_disabled_ability_flag,
        dynamic_enabled_ability_flag, dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetCursorSize(AssocCntrLike&& cntr) {
    CallMethod(true, GetCursorSize, GetCursorSize,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)));
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetWidth(AssocCntrLike&& cntr) {
    CallMethod(true, GetWidth, GetWidth,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)));
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetSize(AssocCntrLike&& cntr) {
    CallMethod(true, GetSize, GetSize,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)));
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetCapacity(AssocCntrLike&& cntr) {
    CallMethod(true, GetCapacity, GetCapacity,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)));
}

template <typename AssocCntrLike>
void assoc_cntr::ops::GetLBCursor(AssocCntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetLBCursor, GetLBCursor,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               dst_cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::GetRBCursor(AssocCntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetRBCursor, GetRBCursor,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               dst_cursor);
}

template <typename AssocCntrLike>
void* assoc_cntr::ops::PeekL(AssocCntrLike&& cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem) {
    CallMethod(true, PeekL, PeekL,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* assoc_cntr::ops::PeekR(AssocCntrLike&& cntr, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem) {
    CallMethod(true, PeekR, PeekR,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* assoc_cntr::ops::Derefer(AssocCntrLike&& cntr, void const* pos_cursor,
                               bool lazy_copy_elem, void* dst_elem) {
    CallMethod(true, Derefer, Derefer,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename AssocCntrLike, typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::ops::Find(AssocCntrLike&& cntr, void const* key,
                            KeyHash&& key_hash,
                            KeyElemCompare&& key_elem_compare,
                            bool lazy_copy_elem, void* dst_cursor,
                            void* dst_elem) {
    CallMethod(true, Find, Find,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), key,
               meta::Forward<KeyHash>(key_hash),
               meta::Forward<KeyElemCompare>(key_elem_compare), lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename AssocCntrLike>
void* assoc_cntr::ops::Insert(AssocCntrLike&& cntr, void const* elem,
                              void* dst_cursor) {
    CallMethod(true, Insert, Insert,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), elem,
               dst_cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::PopL(AssocCntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopL, PopL,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cnt);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::PopR(AssocCntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopR, PopR,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cnt);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::Erase(AssocCntrLike&& cntr, void* pos_cursor) {
    CallMethod(false, Erase, Erase,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               pos_cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::EraseAll(AssocCntrLike&& cntr) {
    CallMethod(false, EraseAll, EraseAll,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)));
}

template <typename AssocCntrLike>
void assoc_cntr::ops::CopyCursor(AssocCntrLike&& cntr, void const* src_cursor,
                                 void* dst_cursor) {
    CallMethod(false, CopyCursor, CopyCursor,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)),
               src_cursor, dst_cursor);
}

template <typename AssocCntrLike>
bool assoc_cntr::ops::AreEqualCursor(AssocCntrLike&& cntr, void const* cursor_a,
                                     void const* cursor_b) {
    CallMethod(true, AreEqualCursor, AreEqualCursor,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename AssocCntrLike>
int assoc_cntr::ops::CompareCursor(AssocCntrLike&& cntr, void const* cursor_a,
                                   void const* cursor_b) {
    CallMethod(true, CompareCursor, CompareCursor,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetCursorDist(AssocCntrLike&& cntr,
                                      void const* cursor_a,
                                      void const* cursor_b) {
    CallMethod(true, GetCursorDist, GetCursorDist,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename AssocCntrLike>
size_t assoc_cntr::ops::GetCursorIdx(AssocCntrLike&& cntr, void const* cursor) {
    CallMethod(true, GetCursorIdx, GetCursorIdx,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::CursorStepL(AssocCntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepL, CursorStepL,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::CursorStepR(AssocCntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepR, CursorStepR,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::CursorAdvanceL(AssocCntrLike&& cntr, void* cursor,
                                     size_t step) {
    CallMethod(false, CursorAdvanceL, CursorAdvanceL,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor,
               step);
}

template <typename AssocCntrLike>
void assoc_cntr::ops::CursorAdvanceR(AssocCntrLike&& cntr, void* cursor,
                                     size_t step) {
    CallMethod(false, CursorAdvanceR, CursorAdvanceR,
               utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr)), cursor,
               step);
}

#pragma pop_macro("CallMethod")

template <typename AssocCntrLike>
void assoc_cntr::ops::CheckContract(AssocCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<AssocCntrLike>(cntr_)) };
    using AssocCntr = meta::RemovePointer<decltype(cntr)>;

    bool bool_val{ false };

    void* void_ptr{ nullptr };
    void const* const_void_ptr{ nullptr };

    size_t size_val{ 0 };

    auto key_hash{ [](void const*, unsigned long long) -> unsigned long long {
        return 0;
    } };

    auto key_elem_compare{ [](void const*, void const*) -> int { return 0; } };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                            \
    {                                                       \
        ZETA_Core_Unused([=]() { (method)(__VA_ARGS__); }); \
    }

#pragma push_macro("CheckMethodOp")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, ...)                              \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, __VA_ARGS__);                                \
    }                                                                    \
                                                                         \
    ZETA_Core_StaticAssert(true);

    CheckMethod(                                 //
        GetStaticEnabledAbilityFlag<AssocCntr>,  // method
    );

    CheckMethod(                                  //
        GetStaticDisabledAbilityFlag<AssocCntr>,  // method
    );

    CheckMethod(                       //
        GetDynamicEnabledAbilityFlag,  // method
                                       //
        cntr                           // inst
    );

    CheckMethod(                        //
        GetDynamicDisabledAbilityFlag,  // method
                                        //
        cntr                            // inst
    );

    constexpr AbilityFlag static_disabled_ability_flag{
        ops::GetStaticDisabledAbilityFlag<AssocCntr>()
    };

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method
                        //
        cntr            // inst
    );

    CheckMethodOp(  //
        GetWidth,   // ability
        GetWidth,   // method
                    //
        cntr        // inst
    );

    CheckMethodOp(  //
        GetSize,    // ability
        GetSize,    // method
                    //
        cntr        // inst
    );

    CheckMethodOp(    //
        GetCapacity,  // ability
        GetCapacity,  // method
                      //
        cntr          // inst
    );

    CheckMethodOp(    //
        GetLBCursor,  // ability
        GetLBCursor,  // method
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(    //
        GetRBCursor,  // ability
        GetRBCursor,  // method
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(  //
        PeekL,      // ability
        PeekL,      // method
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
        cntr,       // inst
        bool_val,   // lazy_copy_elem
        void_ptr,   // dst_cursor, optional
        void_ptr    // mem, optional
    );

    CheckMethodOp(       //
        Derefer,         // ability
        Derefer,         // method
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
        cntr,            // inst
        const_void_ptr,  // elem
        void_ptr         // dst_cursor
    );

    CheckMethodOp(  //
        PopL,       // ability
        PopL,       // method
                    //
        cntr,       // inst
        size_val    // cnt
    );

    CheckMethodOp(  //
        PopR,       // ability
        PopR,       // method
                    //
        cntr,       // inst
        size_val    // cnt
    );

    CheckMethodOp(  //
        Erase,      // ability
        Erase,      // method
                    //
        cntr,       // inst
        void_ptr    // pos_cursor
    );

    CheckMethodOp(  //
        EraseAll,   // ability
        EraseAll,   // method
                    //
        cntr        // inst
    );

    CheckMethodOp(       //
        CopyCursor,      // ability
        CopyCursor,      // method
                         //
        cntr,            // inst
        const_void_ptr,  // src_cursor
        void_ptr         // dst_cursor
    );

    CheckMethodOp(       //
        AreEqualCursor,  // ability
        AreEqualCursor,  // method
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(       //
        CompareCursor,   // ability
        CompareCursor,   // method
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(       //
        GetCursorDist,   // ability
        GetCursorDist,   // method
                         //
        cntr,            // inst
        const_void_ptr,  // cursor_a
        const_void_ptr   // cursor_b
    );

    CheckMethodOp(      //
        GetCursorIdx,   // ability
        GetCursorIdx,   // method
                        //
        cntr,           // inst
        const_void_ptr  // cursor
    );

    CheckMethodOp(    //
        CursorStepL,  // ability
        CursorStepL,  // method
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(    //
        CursorStepR,  // ability
        CursorStepR,  // method
                      //
        cntr,         // inst
        void_ptr      // cursor
    );

    CheckMethodOp(       //
        CursorAdvanceL,  // ability
        CursorAdvanceL,  // method
                         //
        cntr,            // inst
        void_ptr,        // cursor
        size_val         // step
    );

    CheckMethodOp(       //
        CursorAdvanceR,  // ability
        CursorAdvanceR,  // method
                         //
        cntr,            // inst
        void_ptr,        // cursor
        size_val         // step
    );

#pragma pop_macro("CheckMethod")
#pragma pop_macro("CheckMethodOp")
}

template <typename AssocCntr>
constexpr assoc_cntr::VTable assoc_cntr::ops::BuildVTableBasic() {
    constexpr AbilityFlag static_disabled_ability_flag{
        GetStaticDisabledAbilityFlag<AssocCntr>()
    };

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(ability, method)                                                  \
    []() constexpr {                                                        \
        if constexpr (TestAbility(static_disabled_ability_flag, ability)) { \
            return nullptr;                                                 \
        } else {                                                            \
            return method;                                                  \
        }                                                                   \
    }()

    constexpr VTable table{
        .GetSize = F(GetSize,
                     [](void* cntr) {
                         return ops::GetSize(static_cast<AssocCntr*>(cntr));
                     }),

        .GetCapacity =
            F(GetCapacity,
              [](void* cntr) {
                  return ops::GetCapacity(static_cast<AssocCntr*>(cntr));
              }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             ops::GetLBCursor(static_cast<AssocCntr*>(cntr),
                                              dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             ops::GetRBCursor(static_cast<AssocCntr*>(cntr),
                                              dst_cursor);
                         }),

        .PeekL = F(PeekL,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return ops::PeekL(static_cast<AssocCntr*>(cntr),
                                         lazy_copy_elem, dst_cursor, dst_elem);
                   }),

        .PeekR = F(PeekR,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return ops::PeekR(static_cast<AssocCntr*>(cntr),
                                         lazy_copy_elem, dst_cursor, dst_elem);
                   }),

        .Derefer = F(Derefer,
                     [](void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                        void* dst_elem) {
                         return ops::Derefer(static_cast<AssocCntr*>(cntr),
                                             pos_cursor, lazy_copy_elem,
                                             dst_elem);
                     }),

        .FnFind = F(Find,
                    [](void* cntr, void const* key, FnHash const& key_hash,
                       FnCompare const& key_elem_compare, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
                        return ops::Find(static_cast<AssocCntr*>(cntr), key,
                                         key_hash, key_elem_compare,
                                         lazy_copy_elem, dst_cursor, dst_elem);
                    }),

        .FnInsert = F(Insert,
                      [](void* cntr, void const* elem, void* dst_cursor) {
                          return ops::Insert(static_cast<AssocCntr*>(cntr),
                                             elem, dst_cursor);
                      }),

        .PopL = F(PopL,
                  [](void* cntr, size_t cnt) {
                      ops::PopL(static_cast<AssocCntr*>(cntr), cnt);
                  }),

        .PopR = F(PopR,
                  [](void* cntr, size_t cnt) {
                      ops::PopR(static_cast<AssocCntr*>(cntr), cnt);
                  }),

        .Erase = F(Erase,
                   [](void* cntr, void* pos_cursor) {
                       ops::Erase(static_cast<AssocCntr*>(cntr), pos_cursor);
                   }),

        .EraseAll =
            F(EraseAll,
              [](void* cntr) { ops::EraseAll(static_cast<AssocCntr*>(cntr)); }),

        .CopyCursor =
            F(CopyCursor,
              [](void* cntr, void const* src_cursor, void* dst_cursor) {
                  ops::CopyCursor(static_cast<AssocCntr*>(cntr), src_cursor,
                                  dst_cursor);
              }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return ops::AreEqualCursor(static_cast<AssocCntr*>(cntr),
                                             cursor_a, cursor_b);
              }),

        .CompareCursor =
            F(CompareCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return ops::CompareCursor(static_cast<AssocCntr*>(cntr),
                                            cursor_a, cursor_b);
              }),

        .GetCursorDist =
            F(GetCursorDist,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return ops::GetCursorDist(static_cast<AssocCntr*>(cntr),
                                            cursor_a, cursor_b);
              }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void const* cursor) {
                              return ops::GetCursorIdx(
                                  static_cast<AssocCntr*>(cntr), cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             ops::CursorStepL(static_cast<AssocCntr*>(cntr),
                                              cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             ops::CursorStepR(static_cast<AssocCntr*>(cntr),
                                              cursor);
                         }),

        .CursorAdvanceL = F(CursorAdvanceL,
                            [](void* cntr, void* cursor, size_t step) {
                                ops::CursorAdvanceL(
                                    static_cast<AssocCntr*>(cntr), cursor,
                                    step);
                            }),

        .CursorAdvanceR = F(CursorAdvanceR,
                            [](void* cntr, void* cursor, size_t step) {
                                ops::CursorAdvanceR(
                                    static_cast<AssocCntr*>(cntr), cursor,
                                    step);
                            }),

    };

#pragma pop_macro("F")

    return table;
}

template <typename AssocCntr, typename En>
constexpr assoc_cntr::VTable
assoc_cntr::ops::BuildVTableImpl<AssocCntr, En>::Call() {
    return (BuildVTableBasic<AssocCntr>)();
}

template <typename AssocCntr>
constexpr assoc_cntr::VTable assoc_cntr::ops::BuildVTable() {
    return BuildVTableImpl<AssocCntr>::Call();
};

namespace assoc_cntr::ops::detail {

template <typename AssocCntr>
struct VTableHolder_ {
    static constexpr assoc_cntr::VTable vtable{ (BuildVTable<AssocCntr>)() };
};

}  // namespace assoc_cntr::ops::detail

template <typename AssocCntr>
constexpr assoc_cntr::VTable const& assoc_cntr::ops::GetVTable() {
    return detail::VTableHolder_<AssocCntr>::vtable;
}

}  // namespace zeta::core
