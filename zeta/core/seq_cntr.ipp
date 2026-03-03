#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

#pragma push_macro("TestAbility")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                                   \
    (((ability_flag) & (static_cast<::zeta::core::seq_cntr::AbilityFlag>(1)  \
                        << ::zeta::core::seq_cntr::AbilityEnum::ability)) != \
     0)

inline void seq_cntr::MemReader::operator()(void const* src, size_t src_stride,
                                            size_t cnt) {
    utils::ElemCopy(this->dst, src, this->dst_width, this->dst_stride,
                    src_stride, cnt);
    this->dst = static_cast<char*>(this->dst) + this->dst_stride * cnt;
}

inline void seq_cntr::MemWriter::operator()(void* dst, size_t dst_stride,
                                            size_t cnt) {
    utils::ElemCopy(dst, this->src, this->src_width, dst_stride,
                    this->src_stride, cnt);
    this->src = static_cast<char const*>(this->src) + this->src_stride * cnt;
}

constexpr bool seq_cntr::ops::CheckAbilityFlags(
    seq_cntr::AbilityFlag static_enabled_ability_flag,
    seq_cntr::AbilityFlag static_disabled_ability_flag) {
    seq_cntr::AbilityFlag ability_flags[]{ static_enabled_ability_flag,
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

inline bool seq_cntr::ops::CheckAbilityFlags(
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
    using SeqCntr = meta::RemovePointer<decltype(utils::GetInstPtr(           \
        meta::Forward<SeqCntrLike>(cntr)))>;                                  \
                                                                              \
    if constexpr (!TestAbility((GetStaticEnabledAbilityFlag<SeqCntr>)(),      \
                               ability_name)) {                               \
        ZETA_Core_StaticAssert(!TestAbility(                                  \
            (GetStaticDisabledAbilityFlag<SeqCntr>)(), ability_name));        \
                                                                              \
        ZETA_Core_DebugAssert(                                                \
            TestAbility((GetDynamicEnabledAbilityFlag)(cntr), ability_name)); \
    }                                                                         \
                                                                              \
    if constexpr (ret) {                                                      \
        return Traits<SeqCntr>::method_name(__VA_ARGS__);                     \
    } else {                                                                  \
        Traits<SeqCntr>::method_name(__VA_ARGS__);                            \
    };                                                                        \
                                                                              \
    ZETA_Core_StaticAssert(true)

template <typename SeqCntrLike>
void* seq_cntr::ops::GetReferedInstPtr(SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };

    return Traits<meta::RemovePointer<decltype(cntr)>>::GetReferedInstPtr(cntr);
}

template <typename SeqCntr>
constexpr seq_cntr::AbilityFlag seq_cntr::ops::GetStaticEnabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<SeqCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<SeqCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_enabled_ability_flag;
}

template <typename SeqCntr>
constexpr seq_cntr::AbilityFlag seq_cntr::ops::GetStaticDisabledAbilityFlag() {
    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<SeqCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<SeqCntr>::GetStaticDisabledAbilityFlag()
    };

    (CheckAbilityFlags)(static_enabled_ability_flag,
                        static_disabled_ability_flag);

    return static_disabled_ability_flag;
}

template <typename SeqCntrLike>
seq_cntr::AbilityFlag seq_cntr::ops::GetDynamicEnabledAbilityFlag(
    SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };

    using SeqCntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<SeqCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<SeqCntr>::GetStaticDisabledAbilityFlag()
    };

    CheckAbilityFlags(static_enabled_ability_flag,
                      static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        Traits<SeqCntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        Traits<SeqCntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    CheckAbilityFlags(static_enabled_ability_flag, static_disabled_ability_flag,
                      dynamic_enabled_ability_flag,
                      dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename SeqCntrLike>
seq_cntr::AbilityFlag seq_cntr::ops::GetDynamicDisabledAbilityFlag(
    SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };

    using SeqCntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        Traits<SeqCntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        Traits<SeqCntr>::GetStaticDisabledAbilityFlag()
    };

    CheckAbilityFlags(static_enabled_ability_flag,
                      static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        Traits<SeqCntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        Traits<SeqCntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    CheckAbilityFlags(static_enabled_ability_flag, static_disabled_ability_flag,
                      dynamic_enabled_ability_flag,
                      dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetCursorSize(SeqCntrLike&& cntr) {
    CallMethod(true, GetCursorSize, GetCursorSize,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)));
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetWidth(SeqCntrLike&& cntr) {
    CallMethod(true, GetWidth, GetWidth,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)));
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetSize(SeqCntrLike&& cntr) {
    CallMethod(true, GetSize, GetSize,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)));
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetCapacity(SeqCntrLike&& cntr) {
    CallMethod(true, GetCapacity, GetCapacity,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)));
}

template <typename SeqCntrLike>
void seq_cntr::ops::GetLBCursor(SeqCntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetLBCursor, GetLBCursor,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), dst_cursor);
}

template <typename SeqCntrLike>
void seq_cntr::ops::GetRBCursor(SeqCntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetRBCursor, GetRBCursor,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), dst_cursor);
}

template <typename SeqCntrLike>
void* seq_cntr::ops::PeekL(SeqCntrLike&& cntr, bool lazy_copy_elem,
                           void* dst_cursor, void* dst_elem) {
    CallMethod(true, PeekL, PeekL,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)),
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
void* seq_cntr::ops::PeekR(SeqCntrLike&& cntr, bool lazy_copy_elem,
                           void* dst_cursor, void* dst_elem) {
    CallMethod(true, PeekR, PeekR,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)),
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
void* seq_cntr::ops::Access(SeqCntrLike&& cntr, size_t idx, bool lazy_copy_elem,
                            void* dst_cursor, void* dst_elem) {
    CallMethod(true, Access, Access,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), idx,
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
void* seq_cntr::ops::Derefer(SeqCntrLike&& cntr, void const* pos_cursor,
                             bool lazy_copy_elem, void* dst_elem) {
    CallMethod(true, Derefer, Derefer,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               lazy_copy_elem, dst_elem);
}

template <typename SeqCntrLike, typename Reader>
void seq_cntr::ops::Read(SeqCntrLike&& cntr, void const* pos_cursor, size_t cnt,
                         Reader&& reader, void* dst_cursor) {
    CallMethod(false, Read, Read,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Reader>(reader), dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void seq_cntr::ops::Write(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt,
                          Writer&& writer, void* dst_cursor) {
    CallMethod(false, Write, Write,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Writer>(writer), dst_cursor);
}

template <typename SeqCntrLike, typename ReaderWriter>
void seq_cntr::ops::ReadWrite(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt,
                              ReaderWriter&& reader_writer, void* dst_cursor) {
    CallMethod(false, ReadWrite, ReadWrite,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<ReaderWriter>(reader_writer), dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void* seq_cntr::ops::PushL(SeqCntrLike&& cntr, size_t cnt, Writer&& writer,
                           void* dst_cursor) {
    CallMethod(true, PushL, PushL,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cnt,
               meta::Forward<Writer>(writer), dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void* seq_cntr::ops::PushR(SeqCntrLike&& cntr, size_t cnt, Writer&& writer,
                           void* dst_cursor) {
    CallMethod(true, PushR, PushR,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cnt,
               meta::Forward<Writer>(writer), dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void* seq_cntr::ops::Insert(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt,
                            Writer&& writer, void* dst_cursor) {
    CallMethod(true, Insert, Insert,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Writer>(writer), dst_cursor);
}

template <typename SeqCntrLike>
void seq_cntr::ops::PopL(SeqCntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopL, PopL,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cnt);
}

template <typename SeqCntrLike>
void seq_cntr::ops::PopR(SeqCntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopR, PopR,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cnt);
}

template <typename SeqCntrLike>
void seq_cntr::ops::Erase(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt) {
    CallMethod(false, Erase, Erase,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), pos_cursor,
               cnt);
}

template <typename SeqCntrLike>
void seq_cntr::ops::EraseAll(SeqCntrLike&& cntr) {
    CallMethod(false, EraseAll, EraseAll,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)));
}

template <typename SeqCntrLike>
void seq_cntr::ops::CopyCursor(SeqCntrLike&& cntr, void const* src_cursor,
                               void* dst_cursor) {
    CallMethod(false, CopyCursor, CopyCursor,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), src_cursor,
               dst_cursor);
}

template <typename SeqCntrLike>
bool seq_cntr::ops::AreEqualCursor(SeqCntrLike&& cntr, void const* cursor_a,
                                   void const* cursor_b) {
    CallMethod(true, AreEqualCursor, AreEqualCursor,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename SeqCntrLike>
int seq_cntr::ops::CompareCursor(SeqCntrLike&& cntr, void const* cursor_a,
                                 void const* cursor_b) {
    CallMethod(true, CompareCursor, CompareCursor,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetCursorDist(SeqCntrLike&& cntr, void const* cursor_a,
                                    void const* cursor_b) {
    CallMethod(true, GetCursorDist, GetCursorDist,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename SeqCntrLike>
size_t seq_cntr::ops::GetCursorIdx(SeqCntrLike&& cntr, void const* cursor) {
    CallMethod(true, GetCursorIdx, GetCursorIdx,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor);
}

template <typename SeqCntrLike>
void seq_cntr::ops::CursorStepL(SeqCntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepL, CursorStepL,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor);
}

template <typename SeqCntrLike>
void seq_cntr::ops::CursorStepR(SeqCntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepR, CursorStepR,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor);
}

template <typename SeqCntrLike>
void seq_cntr::ops::CursorAdvanceL(SeqCntrLike&& cntr, void* cursor,
                                   size_t step) {
    CallMethod(false, CursorAdvanceL, CursorAdvanceL,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor,
               step);
}

template <typename SeqCntrLike>
void seq_cntr::ops::CursorAdvanceR(SeqCntrLike&& cntr, void* cursor,
                                   size_t step) {
    CallMethod(false, CursorAdvanceR, CursorAdvanceR,
               utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr)), cursor,
               step);
}

#pragma pop_macro("CallMethod")

template <typename SeqCntrLike>
void seq_cntr::ops::CheckContract(SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = meta::RemovePointer<decltype(cntr)>;

    bool bool_val{ false };

    void* void_ptr{ nullptr };
    void const* const_void_ptr{ nullptr };

    size_t size_val{ 0 };

    auto reader{ [](void const*, size_t, size_t) {} };
    auto writer{ [](void*, size_t, size_t) {} };
    auto reader_writer{ [](void*, size_t, size_t) {} };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                            \
    {                                                       \
        ZETA_Core_Unused([=]() { (method)(__VA_ARGS__); }); \
    }                                                       \
                                                            \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CheckMethodOp")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, ...)                              \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, __VA_ARGS__);                                \
    }                                                                    \
                                                                         \
    ZETA_Core_StaticAssert(true);

    CheckMethod(                               //
        GetStaticEnabledAbilityFlag<SeqCntr>,  // method
    );

    CheckMethod(                                //
        GetStaticDisabledAbilityFlag<SeqCntr>,  // method
    );

    CheckMethod(                       //
        GetDynamicEnabledAbilityFlag,  // method

        cntr  // inst
    );

    CheckMethod(                        //
        GetDynamicDisabledAbilityFlag,  // method

        cntr  // inst
    );

    constexpr AbilityFlag static_disabled_ability_flag{ (
        GetStaticDisabledAbilityFlag<SeqCntr>)() };

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method
                        //
        cntr            // inst
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

    CheckMethodOp(  //
        Access,     // ability
        Access,     // method
                    //
        cntr,       // inst
        size_val,   // idx
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

    CheckMethodOp(           //
        Read,                // ability
        Read,                // method
                             //
        cntr,                // inst
        const_void_ptr,      // pos_cursor
        size_val,            // cnt
        meta::Move(reader),  // reader
        void_ptr             // dst_cursor
    );

    CheckMethodOp(           //
        Write,               // ability
        Write,               // method
                             //
        cntr,                // inst
        void_ptr,            // pos_cursor, point to original position
        size_val,            // cnt
        meta::Move(writer),  // writer
        void_ptr             // dst_cursor, optional, point to final position
                             // after write
    );

    CheckMethodOp(                  //
        ReadWrite,                  // ability
        ReadWrite,                  // method
                                    //
        cntr,                       // inst
        void_ptr,                   // pos_cursor
        size_val,                   // cnt
        meta::Move(reader_writer),  // reader_writer
        void_ptr                    // dst_cursor
    );

    CheckMethodOp(           //
        PushL,               // ability
        PushL,               // method
                             //
        cntr,                // inst
        size_val,            // cnt
        meta::Move(writer),  // writer
        void_ptr             // dst_cursor
    );

    CheckMethodOp(           //
        PushR,               // ability
        PushR,               // method
                             //
        cntr,                // inst
        size_val,            // cnt
        meta::Move(writer),  //
        void_ptr             // dst_cursor
    );

    CheckMethodOp(           //
        Insert,              // ability
        Insert,              // method
                             //
        cntr,                // inst
        void_ptr,            // pos_cursor
        size_val,            // cnt
        meta::Move(writer),  // writer
        void_ptr             // dst_cursor
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
        void_ptr,   // pos_cursor
        size_val    // cnt
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

template <typename SeqCntr>
constexpr seq_cntr::VTable seq_cntr::ops::BuildVTableBasic() {
    constexpr AbilityFlag static_disabled_ability_flag{ (
        GetStaticDisabledAbilityFlag<SeqCntr>)() };

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
        .custom_tags{ 0 },

        .GetSize = F(
            GetSize,
            [](void* cntr) { return (GetSize)(static_cast<SeqCntr*>(cntr)); }),

        .GetCapacity = F(GetCapacity,
                         [](void* cntr) {
                             return (GetCapacity)(static_cast<SeqCntr*>(cntr));
                         }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetLBCursor)(static_cast<SeqCntr*>(cntr),
                                                  dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetRBCursor)(static_cast<SeqCntr*>(cntr),
                                                  dst_cursor);
                         }),

        .PeekL = F(PeekL,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekL)(static_cast<SeqCntr*>(cntr),
                                      lazy_copy_elem, dst_cursor, dst_elem);
                   }),

        .PeekR = F(PeekR,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekR)(static_cast<SeqCntr*>(cntr),
                                      lazy_copy_elem, dst_cursor, dst_elem);
                   }),

        .Access = F(Access,
                    [](void* cntr, size_t idx, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
                        return (Access)(static_cast<SeqCntr*>(cntr), idx,
                                        lazy_copy_elem, dst_cursor, dst_elem);
                    }),

        .Derefer = F(Derefer,
                     [](void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                        void* dst_elem) {
                         return (Derefer)(static_cast<SeqCntr*>(cntr),
                                          pos_cursor, lazy_copy_elem, dst_elem);
                     }),

        .FnRead = F(Read,
                    [](void* cntr, void const* pos_cursor, size_t cnt,
                       FnReader reader, void* dst_cursor) {
                        return (Read)(static_cast<SeqCntr*>(cntr), pos_cursor,
                                      cnt, reader, dst_cursor);
                    }),

        .FnWrite = F(Write,
                     [](void* cntr, void* pos_cursor, size_t cnt,
                        FnWriter writer, void* dst_cursor) {
                         return (Write)(static_cast<SeqCntr*>(cntr), pos_cursor,
                                        cnt, writer, dst_cursor);
                     }),

        .FnReadWrite = F(ReadWrite,
                         [](void* cntr, void* pos_cursor, size_t cnt,
                            FnReaderWriter reader_writer, void* dst_cursor) {
                             return (ReadWrite)(static_cast<SeqCntr*>(cntr),
                                                pos_cursor, cnt, reader_writer,
                                                dst_cursor);
                         }),

        .MemRead = F(Read,
                     [](void* cntr, void const* pos_cursor, size_t cnt,
                        MemReader reader, void* dst_cursor) {
                         return (Read)(static_cast<SeqCntr*>(cntr), pos_cursor,
                                       cnt, reader, dst_cursor);
                     }),

        .MemWrite = F(Write,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         MemWriter writer, void* dst_cursor) {
                          return (Write)(static_cast<SeqCntr*>(cntr),
                                         pos_cursor, cnt, writer, dst_cursor);
                      }),

        .FnPushL =
            F(PushL,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushL)(static_cast<SeqCntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnPushR =
            F(PushR,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushR)(static_cast<SeqCntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnInsert = F(Insert,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         FnWriter writer, void* dst_cursor) {
                          return (Insert)(static_cast<SeqCntr*>(cntr),
                                          pos_cursor, cnt, writer, dst_cursor);
                      }),

        .MemPushL =
            F(PushL,
              [](void* cntr, size_t cnt, MemWriter writer, void* dst_cursor) {
                  return (PushL)(static_cast<SeqCntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemPushR =
            F(PushR,
              [](void* cntr, size_t cnt, MemWriter writer, void* dst_cursor) {
                  return (PushR)(static_cast<SeqCntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemInsert = F(Insert,
                       [](void* cntr, void* pos_cursor, size_t cnt,
                          MemWriter writer, void* dst_cursor) {
                           return (Insert)(static_cast<SeqCntr*>(cntr),
                                           pos_cursor, cnt, writer, dst_cursor);
                       }),

        .PopL = F(PopL,
                  [](void* cntr, size_t cnt) {
                      return (PopL)(static_cast<SeqCntr*>(cntr), cnt);
                  }),

        .PopR = F(PopR,
                  [](void* cntr, size_t cnt) {
                      return (PopR)(static_cast<SeqCntr*>(cntr), cnt);
                  }),

        .Erase = F(Erase,
                   [](void* cntr, void* pos_cursor, size_t cnt) {
                       return (Erase)(static_cast<SeqCntr*>(cntr), pos_cursor,
                                      cnt);
                   }),

        .EraseAll = F(
            EraseAll,
            [](void* cntr) { return (EraseAll)(static_cast<SeqCntr*>(cntr)); }),

        .CopyCursor =
            F(CopyCursor,
              [](void* cntr, void const* src_cursor, void* dst_cursor) {
                  return (CopyCursor)(static_cast<SeqCntr*>(cntr), src_cursor,
                                      dst_cursor);
              }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (AreEqualCursor)(static_cast<SeqCntr*>(cntr), cursor_a,
                                          cursor_b);
              }),

        .CompareCursor =
            F(CompareCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (CompareCursor)(static_cast<SeqCntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorDist =
            F(GetCursorDist,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (GetCursorDist)(static_cast<SeqCntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void const* cursor) {
                              return (GetCursorIdx)(static_cast<SeqCntr*>(cntr),
                                                    cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             return (CursorStepL)(static_cast<SeqCntr*>(cntr),
                                                  cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             return (CursorStepR)(static_cast<SeqCntr*>(cntr),
                                                  cursor);
                         }),

        .CursorAdvanceL =
            F(CursorAdvanceL,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceL)(static_cast<SeqCntr*>(cntr), cursor,
                                          step);
              }),

        .CursorAdvanceR =
            F(CursorAdvanceR,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceR)(static_cast<SeqCntr*>(cntr), cursor,
                                          step);
              }),

        .CustomMethods{ nullptr },
    };

#pragma pop_macro("F")

    return table;
}

template <typename SeqCntr, typename En>
constexpr seq_cntr::VTable seq_cntr::ops::BuildVTableImpl<SeqCntr, En>::Call() {
    return (BuildVTableBasic<SeqCntr>)();
}

template <typename SeqCntr>
constexpr seq_cntr::VTable seq_cntr::ops::BuildVTable() {
    return BuildVTableImpl<SeqCntr>::Call();
};

namespace seq_cntr::ops::detail {

template <typename SeqCntr>
struct VTableHolder_ {
    static constexpr seq_cntr::VTable vtable{ (BuildVTable<SeqCntr>)() };
};

}  // namespace seq_cntr::ops::detail

template <typename SeqCntr>
constexpr seq_cntr::VTable const& seq_cntr::ops::GetVTable() {
    return detail::VTableHolder_<SeqCntr>::vtable;
}

constexpr bool seq_cntr::ops::IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

constexpr bool seq_cntr::ops::IsDereferable(size_t idx, size_t cnt,
                                            size_t size) {
    return idx <= size && cnt <= size - idx;
}

constexpr bool seq_cntr::ops::IsInsertable(size_t idx, size_t cnt, size_t size,
                                           size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

constexpr bool seq_cntr::ops::IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void seq_cntr::ops::RangeAssign(DstSeqCntrLike&& dst_cntr_,
                                SrcSeqCntrLike&& src_cntr_, size_t dst_beg,
                                size_t src_beg, size_t cnt) {
    auto* dst_cntr{ utils::GetInstPtr(
        meta::Forward<DstSeqCntrLike>(dst_cntr_)) };
    auto* src_cntr{ utils::GetInstPtr(
        meta::Forward<SrcSeqCntrLike>(src_cntr_)) };

    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    ZETA_Core_DebugAssert((GetWidth)(dst_cntr) == (GetWidth)(src_cntr));

    size_t width{ (GetWidth)(dst_cntr) };

    size_t buffer_capacity{ utils::Max(1ULL, sizeof(void*) * 1024 / width) };

    size_t dst_size{ (GetSize)(dst_cntr) };
    size_t src_size{ (GetSize)(src_cntr) };

    ZETA_Core_DebugAssert((IsDereferable)(dst_beg, cnt, dst_size));
    ZETA_Core_DebugAssert((IsDereferable)(src_beg, cnt, src_size));

    void const* real_dst_cntr{ (GetReferedInstPtr)(dst_cntr) };
    void const* real_src_cntr{ (GetReferedInstPtr)(src_cntr) };

    if (cnt == 0 || real_dst_cntr == real_src_cntr) { return; }

    size_t dst_end{ dst_beg + cnt };
    size_t src_end{ src_beg + cnt };

    void* buffer{ __builtin_alloca_with_align(
        width * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t)) };

    void* dst_cursor{ ZETA_Core_SeqCntr_AllocaCursor(dst_cntr) };
    void* src_cursor{ ZETA_Core_SeqCntr_AllocaCursor(src_cntr) };

    if (real_dst_cntr != real_src_cntr || dst_beg <= src_beg ||
        src_end <= dst_beg) {
        (Access)(dst_cntr, dst_beg, true, dst_cursor, nullptr);
        (Access)(src_cntr, src_beg, true, src_cursor, nullptr);

        while (0 < cnt) {
            size_t cur_cnt{ utils::Min(buffer_capacity, cnt) };

            (Read)(src_cntr, src_cursor, cur_cnt,
                   MemReader{
                       .dst = buffer,
                       .dst_width = width,
                       .dst_stride = width,
                   },
                   src_cursor);
            (Write)(dst_cntr, dst_cursor, cur_cnt,
                    MemWriter{
                        .src = buffer,
                        .src_width = width,
                        .src_stride = width,
                    },
                    dst_cursor);

            cnt -= cur_cnt;
        }

        return;
    }

    (Access)(dst_cntr, dst_end, true, dst_cursor, nullptr);
    (Access)(src_cntr, src_end, true, src_cursor, nullptr);

    while (0 < cnt) {
        size_t cur_cnt{ utils::Min(buffer_capacity, cnt) };

        (CursorAdvanceL)(src_cntr, src_cursor, cur_cnt);
        (CursorAdvanceL)(dst_cntr, dst_cursor, cur_cnt);

        (Read)(src_cntr, src_cursor, cur_cnt,
               MemReader{
                   .dst = buffer,
                   .dst_width = width,
                   .dst_stride = width,
               },
               nullptr);

        (Write)(dst_cntr, dst_cursor, cur_cnt,
                MemWriter{
                    .src = buffer,
                    .src_width = width,
                    .src_stride = width,
                },
                nullptr);

        cnt -= cur_cnt;
    }
}

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void seq_cntr::ops::Assign(DstSeqCntrLike&& dst_cntr_,
                           SrcSeqCntrLike&& src_cntr_) {
    auto* dst_cntr{ utils::GetInstPtr(
        meta::Forward<DstSeqCntrLike>(dst_cntr_)) };
    auto* src_cntr{ utils::GetInstPtr(
        meta::Forward<SrcSeqCntrLike>(src_cntr_)) };

    using DstSeqCntr = meta::RemovePointer<decltype(dst_cntr)>;

    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    size_t dst_size{ (GetSize)(dst_cntr) };
    size_t src_size{ (GetSize)(src_cntr) };

    constexpr AbilityFlag static_enabled_ability_flag{ (
        GetStaticEnabledAbilityFlag<DstSeqCntr>)() };

    constexpr AbilityFlag static_disabled_ability_flag{ (
        GetStaticDisabledAbilityFlag<DstSeqCntr>)() };

    AbilityFlag dynamic_enabled_ability_flag{ (
        GetDynamicEnabledAbilityFlag)(dst_cntr) };

    constexpr bool static_enabled_push_l{ TestAbility(
        static_enabled_ability_flag, AbilityEnum::PushL) };

    constexpr bool static_disabled_push_l{ TestAbility(
        static_disabled_ability_flag, AbilityEnum::PushL) };

    bool dynamic_enabled_push_l{ TestAbility(dynamic_enabled_ability_flag,
                                             AbilityEnum::PushL) };

    constexpr bool static_enabled_push_r{ TestAbility(
        static_enabled_ability_flag, AbilityEnum::PushR) };

    constexpr bool static_disabled_push_r{ TestAbility(
        static_disabled_ability_flag, AbilityEnum::PushR) };

    bool dynamic_enabled_push_r{ TestAbility(dynamic_enabled_ability_flag,
                                             AbilityEnum::PushR) };

    constexpr bool static_enabled_pop_l{ TestAbility(
        static_enabled_ability_flag, AbilityEnum::PopL) };

    constexpr bool static_disabled_pop_l{ TestAbility(
        static_disabled_ability_flag, AbilityEnum::PopL) };

    bool dynamic_enabled_pop_l{ TestAbility(dynamic_enabled_ability_flag,
                                            AbilityEnum::PopL) };

    constexpr bool static_enabled_pop_r{ TestAbility(
        static_enabled_ability_flag, AbilityEnum::PopR) };

    constexpr bool static_disabled_pop_r{ TestAbility(
        static_disabled_ability_flag, AbilityEnum::PopR) };

    bool dynamic_enabled_pop_r{ TestAbility(dynamic_enabled_ability_flag,
                                            AbilityEnum::PopR) };
    /*

    se sd de dd
    se R  L  L  L
    sd R  X  R  X
    de R  L  R  L
    dd R  X  R  X

    */

#pragma push_macro("FPushL")
#define FPushL \
    (PushL)(   \
        dst_cntr, src_size - dst_size, [](void*, size_t, size_t) {}, nullptr);

#pragma push_macro("FPushR")
#define FPushR \
    (PushR)(   \
        dst_cntr, src_size - dst_size, [](void*, size_t, size_t) {}, nullptr);

#pragma push_macro("FPopL")
#define FPopL (PopL)(dst_cntr, dst_size - src_size);

#pragma push_macro("FPopR")
#define FPopR (PopR)(dst_cntr, dst_size - src_size);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (dst_size < src_size) {
        if constexpr (static_enabled_push_r) {
            FPushR;
        } else if constexpr (static_enabled_push_l) {
            FPushL;
        } else if constexpr (static_disabled_push_r) {
            ZETA_Core_DebugAssert(dynamic_enabled_push_l);
            FPushL;
        } else if constexpr (static_disabled_push_l) {
            ZETA_Core_DebugAssert(dynamic_enabled_push_r);
            FPushR;
        } else if (dynamic_enabled_push_r) {
            FPushR;
        } else {
            ZETA_Core_DebugAssert(dynamic_enabled_push_l);
            FPushL;
        }
    } else if (src_size < dst_size) {
        if constexpr (static_enabled_pop_r) {
            FPopR;
        } else if constexpr (static_enabled_pop_l) {
            FPopL;
        } else if constexpr (static_disabled_pop_r) {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_l);
            FPopL;
        } else if constexpr (static_disabled_pop_l) {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_r);
            FPopR;
        } else if (dynamic_enabled_pop_r) {
            FPopR;
        } else {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_l);
            FPopL;
        }
    }
    // NOLINTEND(bugprone-branch-clone)

#pragma pop_macro("FPopR")
#pragma pop_macro("FPopL")
#pragma pop_macro("FPushR")
#pragma pop_macro("FPushL")

    (RangeAssign)(dst_cntr, src_cntr, 0, 0, src_size);
}

#pragma pop_macro("TestAbility")

}  // namespace zeta::core
