#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

#pragma push_macro("TestAbility")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                                     \
    (((ability_flag) & (static_cast<::zeta::core::assoc_cntr::AbilityFlag>(1)  \
                        << ::zeta::core::assoc_cntr::AbilityEnum::ability)) != \
     0)

constexpr bool assoc_cntr::CheckAbilityFlags(
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

inline bool assoc_cntr::CheckAbilityFlags(
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
    if constexpr (!TestAbility((GetStaticEnabledAbilityFlag<Cntr>)(),         \
                               ability_name)) {                               \
        ZETA_Core_StaticAssert(!TestAbility(                                  \
            (GetStaticDisabledAbilityFlag<Cntr>)(), ability_name));           \
                                                                              \
        ZETA_Core_DebugAssert(                                                \
            TestAbility((GetDynamicEnabledAbilityFlag)(cntr), ability_name)); \
    }                                                                         \
                                                                              \
    if constexpr (ret) {                                                      \
        return CntrTraits<Cntr>::method_name(__VA_ARGS__);                    \
    } else {                                                                  \
        CntrTraits<Cntr>::method_name(__VA_ARGS__);                           \
    };                                                                        \
                                                                              \
    ZETA_Core_StaticAssert(true)

template <typename Cntr>
auto* assoc_cntr::GetReferedInstPtr(Cntr& cntr) {
    return CntrTraits<Cntr>::GetReferedInstPtr(cntr);
}

template <typename Cntr>
constexpr assoc_cntr::AbilityFlag assoc_cntr::GetStaticEnabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_enabled_ability_flag;
}

template <typename Cntr>
constexpr assoc_cntr::AbilityFlag assoc_cntr::GetStaticDisabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_disabled_ability_flag;
}

template <typename Cntr>
assoc_cntr::AbilityFlag assoc_cntr::GetDynamicEnabledAbilityFlag(Cntr& cntr) {
    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    (CheckAbilityFlags)(
        static_enabled_ability_flag, static_disabled_ability_flag,
        dynamic_enabled_ability_flag, dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename Cntr>
assoc_cntr::AbilityFlag assoc_cntr::GetDynamicDisabledAbilityFlag(Cntr& cntr) {
    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    (CheckAbilityFlags)(
        static_enabled_ability_flag, static_disabled_ability_flag,
        dynamic_enabled_ability_flag, dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename Cntr>
size_t assoc_cntr::GetCursorSize(Cntr& cntr) {
    CallMethod(true, GetCursorSize, GetCursorSize, cntr);
}

template <typename Cntr>
size_t assoc_cntr::GetElemSize(Cntr& cntr) {
    CallMethod(true, GetElemSize, GetElemSize, cntr);
}

template <typename Cntr>
size_t assoc_cntr::GetElemCnt(Cntr& cntr) {
    CallMethod(true, GetElemCnt, GetElemCnt, cntr);
}

template <typename Cntr>
size_t assoc_cntr::GetMaxElemCnt(Cntr& cntr) {
    CallMethod(true, GetMaxElemCnt, GetMaxElemCnt, cntr);
}

template <typename Cntr>
void assoc_cntr::GetLBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(false, GetLBCursor, GetLBCursor, cntr, dst_cursor);
}

template <typename Cntr>
void assoc_cntr::GetRBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(false, GetRBCursor, GetRBCursor, cntr, dst_cursor);
}

template <typename Cntr>
void* assoc_cntr::PeekL(Cntr& cntr, bool lazy_copy_elem, void* dst_cursor,
                        void* dst_elem) {
    CallMethod(true, PeekL, PeekL, cntr, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename Cntr>
void* assoc_cntr::PeekR(Cntr& cntr, bool lazy_copy_elem, void* dst_cursor,
                        void* dst_elem) {
    CallMethod(true, PeekR, PeekR, cntr, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename Cntr>
void* assoc_cntr::Derefer(Cntr& cntr, void const* pos_cursor,
                          bool lazy_copy_elem, void* dst_elem) {
    CallMethod(true, Derefer, Derefer, cntr, pos_cursor, lazy_copy_elem,
               dst_elem);
}

template <typename Cntr, typename KeyHash, typename KeyElemCompare>
void* assoc_cntr::Find(Cntr& cntr, void const* key, KeyHash&& key_hash,
                       KeyElemCompare&& key_elem_compare, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
    CallMethod(true, Find, Find, cntr, key, meta::Forward<KeyHash>(key_hash),
               meta::Forward<KeyElemCompare>(key_elem_compare), lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename Cntr>
void* assoc_cntr::Insert(Cntr& cntr, void const* elem, void* dst_cursor) {
    CallMethod(true, Insert, Insert, cntr, elem, dst_cursor);
}

template <typename Cntr>
void assoc_cntr::PopL(Cntr& cntr, size_t cnt) {
    CallMethod(false, PopL, PopL, cntr, cnt);
}

template <typename Cntr>
void assoc_cntr::PopR(Cntr& cntr, size_t cnt) {
    CallMethod(false, PopR, PopR, cntr, cnt);
}

template <typename Cntr>
void assoc_cntr::Erase(Cntr& cntr, void* pos_cursor) {
    CallMethod(false, Erase, Erase, cntr, pos_cursor);
}

template <typename Cntr>
void assoc_cntr::EraseAll(Cntr& cntr) {
    CallMethod(false, EraseAll, EraseAll, cntr);
}

template <typename Cntr>
void assoc_cntr::CopyCursor(Cntr& cntr, void const* src_cursor,
                            void* dst_cursor) {
    CallMethod(false, CopyCursor, CopyCursor, cntr, src_cursor, dst_cursor);
}

template <typename Cntr>
bool assoc_cntr::AreEqualCursor(Cntr& cntr, void const* cursor_a,
                                void const* cursor_b) {
    CallMethod(true, AreEqualCursor, AreEqualCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
int assoc_cntr::CompareCursor(Cntr& cntr, void const* cursor_a,
                              void const* cursor_b) {
    CallMethod(true, CompareCursor, CompareCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
size_t assoc_cntr::GetCursorDist(Cntr& cntr, void const* cursor_a,
                                 void const* cursor_b) {
    CallMethod(true, GetCursorDist, GetCursorDist, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
size_t assoc_cntr::GetCursorIdx(Cntr& cntr, void const* cursor) {
    CallMethod(true, GetCursorIdx, GetCursorIdx, cntr, cursor);
}

template <typename Cntr>
void assoc_cntr::CursorStepL(Cntr& cntr, void* cursor) {
    CallMethod(false, CursorStepL, CursorStepL, cntr, cursor);
}

template <typename Cntr>
void assoc_cntr::CursorStepR(Cntr& cntr, void* cursor) {
    CallMethod(false, CursorStepR, CursorStepR, cntr, cursor);
}

template <typename Cntr>
void assoc_cntr::CursorAdvanceL(Cntr& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceL, CursorAdvanceL, cntr, cursor, step);
}

template <typename Cntr>
void assoc_cntr::CursorAdvanceR(Cntr& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceR, CursorAdvanceR, cntr, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename Cntr>
void assoc_cntr::CheckContract(Cntr& cntr) {
    struct KeyHash {
        unsigned long long operator()(void const*, unsigned long long) const {
            return 0;
        }
    };

    struct KeyElemCompare {
        int operator()(void const*, void const*) const { return 0; }
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                                              \
    ZETA_Core_Unused([&](bool bool_val, void* void_ptr,                       \
                         void const* const_void_ptr, size_t size_val,         \
                         KeyHash key_hash, KeyElemCompare key_elem_compare) { \
        ZETA_Core_Unused(bool_val);                                           \
        ZETA_Core_Unused(void_ptr);                                           \
        ZETA_Core_Unused(const_void_ptr);                                     \
        ZETA_Core_Unused(size_val);                                           \
        ZETA_Core_Unused(key_hash);                                           \
        ZETA_Core_Unused(key_elem_compare);                                   \
        (method)(__VA_ARGS__);                                                \
    })

#pragma push_macro("CheckMethodOp")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, ...)                              \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, __VA_ARGS__);                                \
    }                                                                    \
                                                                         \
    ZETA_Core_StaticAssert(true);

    CheckMethod(                            //
        GetStaticEnabledAbilityFlag<Cntr>,  // method
    );

    CheckMethod(                             //
        GetStaticDisabledAbilityFlag<Cntr>,  // method
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
        GetStaticDisabledAbilityFlag<Cntr>()
    };

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method
                        //
        cntr            // inst
    );

    CheckMethodOp(    //
        GetElemSize,  // ability
        GetElemSize,  // method
                      //
        cntr          // inst
    );

    CheckMethodOp(   //
        GetElemCnt,  // ability
        GetElemCnt,  // method
                     //
        cntr         // inst
    );

    CheckMethodOp(      //
        GetMaxElemCnt,  // ability
        GetMaxElemCnt,  // method
                        //
        cntr            // inst
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

template <typename Cntr>
constexpr assoc_cntr::VTable assoc_cntr::BuildVTableBasic() {
    constexpr AbilityFlag static_disabled_ability_flag{
        GetStaticDisabledAbilityFlag<Cntr>()
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
        .GetElemCnt =
            F(GetElemCnt,
              [](void* cntr) { return GetElemCnt(*static_cast<Cntr*>(cntr)); }),

        .GetMaxElemCnt = F(GetMaxElemCnt,
                           [](void* cntr) {
                               return GetMaxElemCnt(*static_cast<Cntr*>(cntr));
                           }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             GetLBCursor(*static_cast<Cntr*>(cntr), dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             GetRBCursor(*static_cast<Cntr*>(cntr), dst_cursor);
                         }),

        .PeekL = F(PeekL,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return PeekL(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                                    dst_cursor, dst_elem);
                   }),

        .PeekR = F(PeekR,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return PeekR(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                                    dst_cursor, dst_elem);
                   }),

        .Derefer = F(Derefer,
                     [](void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                        void* dst_elem) {
                         return Derefer(*static_cast<Cntr*>(cntr), pos_cursor,
                                        lazy_copy_elem, dst_elem);
                     }),

        .FnFind = F(Find,
                    [](void* cntr, void const* key, FnHash const& key_hash,
                       FnCompare const& key_elem_compare, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
                        return Find(*static_cast<Cntr*>(cntr), key, key_hash,
                                    key_elem_compare, lazy_copy_elem,
                                    dst_cursor, dst_elem);
                    }),

        .FnInsert = F(Insert,
                      [](void* cntr, void const* elem, void* dst_cursor) {
                          return Insert(*static_cast<Cntr*>(cntr), elem,
                                        dst_cursor);
                      }),

        .PopL =
            F(PopL, [](void* cntr,
                       size_t cnt) { PopL(*static_cast<Cntr*>(cntr), cnt); }),

        .PopR =
            F(PopR, [](void* cntr,
                       size_t cnt) { PopR(*static_cast<Cntr*>(cntr), cnt); }),

        .Erase = F(Erase,
                   [](void* cntr, void* pos_cursor) {
                       Erase(*static_cast<Cntr*>(cntr), pos_cursor);
                   }),

        .EraseAll = F(EraseAll,
                      [](void* cntr) { EraseAll(*static_cast<Cntr*>(cntr)); }),

        .CopyCursor =
            F(CopyCursor,
              [](void* cntr, void const* src_cursor, void* dst_cursor) {
                  CopyCursor(*static_cast<Cntr*>(cntr), src_cursor, dst_cursor);
              }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return AreEqualCursor(*static_cast<Cntr*>(cntr), cursor_a,
                                        cursor_b);
              }),

        .CompareCursor =
            F(CompareCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return CompareCursor(*static_cast<Cntr*>(cntr), cursor_a,
                                       cursor_b);
              }),

        .GetCursorDist =
            F(GetCursorDist,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return GetCursorDist(*static_cast<Cntr*>(cntr), cursor_a,
                                       cursor_b);
              }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void const* cursor) {
                              return GetCursorIdx(*static_cast<Cntr*>(cntr),
                                                  cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             CursorStepL(*static_cast<Cntr*>(cntr), cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             CursorStepR(*static_cast<Cntr*>(cntr), cursor);
                         }),

        .CursorAdvanceL = F(CursorAdvanceL,
                            [](void* cntr, void* cursor, size_t step) {
                                CursorAdvanceL(*static_cast<Cntr*>(cntr),
                                               cursor, step);
                            }),

        .CursorAdvanceR = F(CursorAdvanceR,
                            [](void* cntr, void* cursor, size_t step) {
                                CursorAdvanceR(*static_cast<Cntr*>(cntr),
                                               cursor, step);
                            }),

    };

#pragma pop_macro("F")

    return table;
}

template <typename Cntr, typename En>
constexpr assoc_cntr::VTable assoc_cntr::BuildVTableImpl<Cntr, En>::Call() {
    return (BuildVTableBasic<Cntr>)();
}

template <typename Cntr>
constexpr assoc_cntr::VTable assoc_cntr::BuildVTable() {
    return BuildVTableImpl<Cntr>::Call();
};

namespace assoc_cntr::detail {

template <typename Cntr>
struct VTableHolder_ {
    static constexpr assoc_cntr::VTable vtable{ (BuildVTable<Cntr>)() };
};

}  // namespace assoc_cntr::detail

template <typename Cntr>
constexpr assoc_cntr::VTable const& assoc_cntr::GetVTable() {
    return detail::VTableHolder_<Cntr>::vtable;
}

#pragma pop_macro("TestAbility")

}  // namespace zeta::core
