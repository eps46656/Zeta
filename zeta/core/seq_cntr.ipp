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

constexpr void seq_cntr::EmptyReader::operator()(void const*, size_t,
                                                 size_t) const {}

constexpr void seq_cntr::EmptyWriter::operator()(void*, size_t, size_t) const {}

inline void seq_cntr::MemReader::operator()(void const* src, size_t src_stride,
                                            size_t cnt) {
    utils::ElemCopy(this->data, src, this->elem_size, this->elem_stride,
                    src_stride, cnt);
    this->data = static_cast<char*>(this->data) + this->elem_stride * cnt;
}

inline void seq_cntr::MemWriter::operator()(void* dst, size_t dst_stride,
                                            size_t cnt) {
    utils::ElemCopy(dst, this->data, this->elem_size, dst_stride,
                    this->elem_stride, cnt);
    this->data = static_cast<char const*>(this->data) + this->elem_stride * cnt;
}

constexpr bool seq_cntr::CheckAbilityFlags(
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

inline bool seq_cntr::CheckAbilityFlags(
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
    using Cntr = meta::RemovePointer<decltype(utils::GetInstPtr(              \
        meta::Forward<CntrLike>(cntr)))>;                                     \
                                                                              \
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

template <typename CntrLike>
void* seq_cntr::GetReferedInstPtr(CntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<CntrLike>(cntr_)) };

    return CntrTraits<meta::RemovePointer<decltype(cntr)>>::GetReferedInstPtr(
        cntr);
}

template <typename Cntr>
constexpr seq_cntr::AbilityFlag seq_cntr::GetStaticEnabledAbilityFlag() {
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
constexpr seq_cntr::AbilityFlag seq_cntr::GetStaticDisabledAbilityFlag() {
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

template <typename CntrLike>
seq_cntr::AbilityFlag seq_cntr::GetDynamicEnabledAbilityFlag(CntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<CntrLike>(cntr_)) };

    using Cntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    CheckAbilityFlags(static_enabled_ability_flag,
                      static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    CheckAbilityFlags(static_enabled_ability_flag, static_disabled_ability_flag,
                      dynamic_enabled_ability_flag,
                      dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename CntrLike>
seq_cntr::AbilityFlag seq_cntr::GetDynamicDisabledAbilityFlag(
    CntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<CntrLike>(cntr_)) };

    using Cntr = meta::RemovePointer<decltype(cntr)>;

    constexpr AbilityFlag static_enabled_ability_flag{
        CntrTraits<Cntr>::GetStaticEnabledAbilityFlag()
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        CntrTraits<Cntr>::GetStaticDisabledAbilityFlag()
    };

    CheckAbilityFlags(static_enabled_ability_flag,
                      static_disabled_ability_flag);

    AbilityFlag dynamic_enabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledAbilityFlag(cntr)
    };

    AbilityFlag dynamic_disabled_ability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledAbilityFlag(cntr)
    };

    CheckAbilityFlags(static_enabled_ability_flag, static_disabled_ability_flag,
                      dynamic_enabled_ability_flag,
                      dynamic_disabled_ability_flag);

    return dynamic_enabled_ability_flag;
}

template <typename CntrLike>
size_t seq_cntr::GetCursorSize(CntrLike&& cntr) {
    CallMethod(true, GetCursorSize, GetCursorSize,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)));
}

template <typename CntrLike>
size_t seq_cntr::GetElemSize(CntrLike&& cntr) {
    CallMethod(true, GetElemSize, GetElemSize,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)));
}

template <typename CntrLike>
size_t seq_cntr::GetElemCnt(CntrLike&& cntr) {
    CallMethod(true, GetElemCnt, GetElemCnt,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)));
}

template <typename CntrLike>
size_t seq_cntr::GetMaxElemCnt(CntrLike&& cntr) {
    CallMethod(true, GetMaxElemCnt, GetMaxElemCnt,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)));
}

template <typename CntrLike>
void seq_cntr::GetLBCursor(CntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetLBCursor, GetLBCursor,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), dst_cursor);
}

template <typename CntrLike>
void seq_cntr::GetRBCursor(CntrLike&& cntr, void* dst_cursor) {
    CallMethod(false, GetRBCursor, GetRBCursor,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), dst_cursor);
}

template <typename CntrLike>
void* seq_cntr::PeekL(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
    CallMethod(true, PeekL, PeekL,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename CntrLike>
void* seq_cntr::PeekR(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
    CallMethod(true, PeekR, PeekR,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), lazy_copy_elem,
               dst_cursor, dst_elem);
}

template <typename CntrLike>
void* seq_cntr::Access(CntrLike&& cntr, size_t idx, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
    CallMethod(true, Access, Access,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), idx,
               lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename CntrLike>
void* seq_cntr::Derefer(CntrLike&& cntr, void const* pos_cursor,
                        bool lazy_copy_elem, void* dst_elem) {
    CallMethod(true, Derefer, Derefer,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               lazy_copy_elem, dst_elem);
}

template <typename CntrLike, typename Reader>
void seq_cntr::Read(CntrLike&& cntr, void const* pos_cursor, size_t cnt,
                    Reader&& reader, void* dst_cursor) {
    CallMethod(false, Read, Read,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Reader>(reader), dst_cursor);
}

template <typename CntrLike, typename Writer>
void seq_cntr::Write(CntrLike&& cntr, void* pos_cursor, size_t cnt,
                     Writer&& writer, void* dst_cursor) {
    CallMethod(false, Write, Write,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Writer>(writer), dst_cursor);
}

template <typename CntrLike, typename ReaderWriter>
void seq_cntr::ReadWrite(CntrLike&& cntr, void* pos_cursor, size_t cnt,
                         ReaderWriter&& reader_writer, void* dst_cursor) {
    CallMethod(false, ReadWrite, ReadWrite,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<ReaderWriter>(reader_writer), dst_cursor);
}

template <typename CntrLike, typename Writer>
void* seq_cntr::PushL(CntrLike&& cntr, size_t cnt, Writer&& writer,
                      void* dst_cursor) {
    CallMethod(true, PushL, PushL,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cnt,
               meta::Forward<Writer>(writer), dst_cursor);
}

template <typename CntrLike, typename Writer>
void* seq_cntr::PushR(CntrLike&& cntr, size_t cnt, Writer&& writer,
                      void* dst_cursor) {
    CallMethod(true, PushR, PushR,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cnt,
               meta::Forward<Writer>(writer), dst_cursor);
}

template <typename CntrLike, typename Writer>
void* seq_cntr::Insert(CntrLike&& cntr, void* pos_cursor, size_t cnt,
                       Writer&& writer, void* dst_cursor) {
    CallMethod(true, Insert, Insert,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               cnt, meta::Forward<Writer>(writer), dst_cursor);
}

template <typename CntrLike>
void seq_cntr::PopL(CntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopL, PopL,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cnt);
}

template <typename CntrLike>
void seq_cntr::PopR(CntrLike&& cntr, size_t cnt) {
    CallMethod(false, PopR, PopR,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cnt);
}

template <typename CntrLike>
void seq_cntr::Erase(CntrLike&& cntr, void* pos_cursor, size_t cnt) {
    CallMethod(false, Erase, Erase,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), pos_cursor,
               cnt);
}

template <typename CntrLike>
void seq_cntr::EraseAll(CntrLike&& cntr) {
    CallMethod(false, EraseAll, EraseAll,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)));
}

template <typename CntrLike>
void seq_cntr::CopyCursor(CntrLike&& cntr, void const* src_cursor,
                          void* dst_cursor) {
    CallMethod(false, CopyCursor, CopyCursor,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), src_cursor,
               dst_cursor);
}

template <typename CntrLike>
bool seq_cntr::AreEqualCursor(CntrLike&& cntr, void const* cursor_a,
                              void const* cursor_b) {
    CallMethod(true, AreEqualCursor, AreEqualCursor,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename CntrLike>
int seq_cntr::CompareCursor(CntrLike&& cntr, void const* cursor_a,
                            void const* cursor_b) {
    CallMethod(true, CompareCursor, CompareCursor,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename CntrLike>
size_t seq_cntr::GetCursorDist(CntrLike&& cntr, void const* cursor_a,
                               void const* cursor_b) {
    CallMethod(true, GetCursorDist, GetCursorDist,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor_a,
               cursor_b);
}

template <typename CntrLike>
size_t seq_cntr::GetCursorIdx(CntrLike&& cntr, void const* cursor) {
    CallMethod(true, GetCursorIdx, GetCursorIdx,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor);
}

template <typename CntrLike>
void seq_cntr::CursorStepL(CntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepL, CursorStepL,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor);
}

template <typename CntrLike>
void seq_cntr::CursorStepR(CntrLike&& cntr, void* cursor) {
    CallMethod(false, CursorStepR, CursorStepR,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor);
}

template <typename CntrLike>
void seq_cntr::CursorAdvanceL(CntrLike&& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceL, CursorAdvanceL,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor, step);
}

template <typename CntrLike>
void seq_cntr::CursorAdvanceR(CntrLike&& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceR, CursorAdvanceR,
               utils::GetInstPtr(meta::Forward<CntrLike>(cntr)), cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename CntrLike>
void seq_cntr::CheckContract(CntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<CntrLike>(cntr_)) };
    using Cntr = meta::RemovePointer<decltype(cntr)>;

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

    CheckMethod(                            //
        GetStaticEnabledAbilityFlag<Cntr>,  // method
    );

    CheckMethod(                             //
        GetStaticDisabledAbilityFlag<Cntr>,  // method
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
        GetStaticDisabledAbilityFlag<Cntr>)() };

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method
                        //
        cntr            // inst
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

template <typename Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTableBasic() {
    constexpr AbilityFlag static_disabled_ability_flag{ (
        GetStaticDisabledAbilityFlag<Cntr>)() };

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

        .GetElemCnt = F(
            GetElemCnt,
            [](void* cntr) { return (GetElemCnt)(static_cast<Cntr*>(cntr)); }),

        .GetMaxElemCnt = F(GetMaxElemCnt,
                           [](void* cntr) {
                               return (GetMaxElemCnt)(static_cast<Cntr*>(cntr));
                           }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetLBCursor)(static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetRBCursor)(static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .PeekL = F(PeekL,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekL)(static_cast<Cntr*>(cntr), lazy_copy_elem,
                                      dst_cursor, dst_elem);
                   }),

        .PeekR = F(PeekR,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekR)(static_cast<Cntr*>(cntr), lazy_copy_elem,
                                      dst_cursor, dst_elem);
                   }),

        .Access = F(Access,
                    [](void* cntr, size_t idx, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem) {
                        return (Access)(static_cast<Cntr*>(cntr), idx,
                                        lazy_copy_elem, dst_cursor, dst_elem);
                    }),

        .Derefer = F(Derefer,
                     [](void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                        void* dst_elem) {
                         return (Derefer)(static_cast<Cntr*>(cntr), pos_cursor,
                                          lazy_copy_elem, dst_elem);
                     }),

        .FnRead = F(Read,
                    [](void* cntr, void const* pos_cursor, size_t cnt,
                       FnReader reader, void* dst_cursor) {
                        return (Read)(static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                      reader, dst_cursor);
                    }),

        .FnWrite = F(Write,
                     [](void* cntr, void* pos_cursor, size_t cnt,
                        FnWriter writer, void* dst_cursor) {
                         return (Write)(static_cast<Cntr*>(cntr), pos_cursor,
                                        cnt, writer, dst_cursor);
                     }),

        .FnReadWrite = F(ReadWrite,
                         [](void* cntr, void* pos_cursor, size_t cnt,
                            FnReaderWriter reader_writer, void* dst_cursor) {
                             return (ReadWrite)(static_cast<Cntr*>(cntr),
                                                pos_cursor, cnt, reader_writer,
                                                dst_cursor);
                         }),

        .MemRead = F(Read,
                     [](void* cntr, void const* pos_cursor, size_t cnt,
                        MemReader reader, void* dst_cursor) {
                         return (Read)(static_cast<Cntr*>(cntr), pos_cursor,
                                       cnt, reader, dst_cursor);
                     }),

        .MemWrite = F(Write,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         MemWriter writer, void* dst_cursor) {
                          return (Write)(static_cast<Cntr*>(cntr), pos_cursor,
                                         cnt, writer, dst_cursor);
                      }),

        .FnPushL =
            F(PushL,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushL)(static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnPushR =
            F(PushR,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushR)(static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnInsert = F(Insert,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         FnWriter writer, void* dst_cursor) {
                          return (Insert)(static_cast<Cntr*>(cntr), pos_cursor,
                                          cnt, writer, dst_cursor);
                      }),

        .MemPushL =
            F(PushL,
              [](void* cntr, size_t cnt, MemWriter writer, void* dst_cursor) {
                  return (PushL)(static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemPushR =
            F(PushR,
              [](void* cntr, size_t cnt, MemWriter writer, void* dst_cursor) {
                  return (PushR)(static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemInsert = F(Insert,
                       [](void* cntr, void* pos_cursor, size_t cnt,
                          MemWriter writer, void* dst_cursor) {
                           return (Insert)(static_cast<Cntr*>(cntr), pos_cursor,
                                           cnt, writer, dst_cursor);
                       }),

        .PopL = F(PopL,
                  [](void* cntr, size_t cnt) {
                      return (PopL)(static_cast<Cntr*>(cntr), cnt);
                  }),

        .PopR = F(PopR,
                  [](void* cntr, size_t cnt) {
                      return (PopR)(static_cast<Cntr*>(cntr), cnt);
                  }),

        .Erase = F(Erase,
                   [](void* cntr, void* pos_cursor, size_t cnt) {
                       return (Erase)(static_cast<Cntr*>(cntr), pos_cursor,
                                      cnt);
                   }),

        .EraseAll =
            F(EraseAll,
              [](void* cntr) { return (EraseAll)(static_cast<Cntr*>(cntr)); }),

        .CopyCursor =
            F(CopyCursor,
              [](void* cntr, void const* src_cursor, void* dst_cursor) {
                  return (CopyCursor)(static_cast<Cntr*>(cntr), src_cursor,
                                      dst_cursor);
              }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (AreEqualCursor)(static_cast<Cntr*>(cntr), cursor_a,
                                          cursor_b);
              }),

        .CompareCursor =
            F(CompareCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (CompareCursor)(static_cast<Cntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorDist =
            F(GetCursorDist,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (GetCursorDist)(static_cast<Cntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void const* cursor) {
                              return (GetCursorIdx)(static_cast<Cntr*>(cntr),
                                                    cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             return (CursorStepL)(static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             return (CursorStepR)(static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorAdvanceL = F(
            CursorAdvanceL,
            [](void* cntr, void* cursor, size_t step) {
                return (CursorAdvanceL)(static_cast<Cntr*>(cntr), cursor, step);
            }),

        .CursorAdvanceR = F(
            CursorAdvanceR,
            [](void* cntr, void* cursor, size_t step) {
                return (CursorAdvanceR)(static_cast<Cntr*>(cntr), cursor, step);
            }),

        .CustomMethods{ nullptr },
    };

#pragma pop_macro("F")

    return table;
}

template <typename Cntr, typename En>
constexpr seq_cntr::VTable seq_cntr::BuildVTableImpl<Cntr, En>::Call() {
    return (BuildVTableBasic<Cntr>)();
}

template <typename Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTable() {
    return BuildVTableImpl<Cntr>::Call();
};

namespace seq_cntr::detail {

template <typename Cntr>
struct VTableHolder_ {
    static constexpr seq_cntr::VTable vtable{ (BuildVTable<Cntr>)() };
};

}  // namespace seq_cntr::detail

template <typename Cntr>
constexpr seq_cntr::VTable const& seq_cntr::GetVTable() {
    return detail::VTableHolder_<Cntr>::vtable;
}

constexpr bool seq_cntr::IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

constexpr bool seq_cntr::IsDereferable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

constexpr bool seq_cntr::IsInsertable(size_t idx, size_t cnt, size_t size,
                                      size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

constexpr bool seq_cntr::IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

template <typename DstCntrLike, typename SrcCntrLike>
void seq_cntr::RangeAssign(DstCntrLike&& dst_cntr_, SrcCntrLike&& src_cntr_,
                           size_t dst_beg, size_t src_beg, size_t cnt) {
    auto* dst_cntr{ utils::GetInstPtr(meta::Forward<DstCntrLike>(dst_cntr_)) };
    auto* src_cntr{ utils::GetInstPtr(meta::Forward<SrcCntrLike>(src_cntr_)) };

    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    ZETA_Core_DebugAssert((GetElemSize)(dst_cntr) == (GetElemSize)(src_cntr));

    size_t elem_size{ (GetElemSize)(dst_cntr) };

    size_t buffer_capacity{ utils::Max(1ULL,
                                       sizeof(void*) * 1024 / elem_size) };

    size_t dst_elem_cnt{ (GetElemCnt)(dst_cntr) };
    size_t src_elem_cnt{ (GetElemCnt)(src_cntr) };

    ZETA_Core_DebugAssert((IsDereferable)(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugAssert((IsDereferable)(src_beg, cnt, src_elem_cnt));

    void const* real_dst_cntr{ (GetReferedInstPtr)(dst_cntr) };
    void const* real_src_cntr{ (GetReferedInstPtr)(src_cntr) };

    if (cnt == 0 || real_dst_cntr == real_src_cntr) { return; }

    size_t dst_end{ dst_beg + cnt };
    size_t src_end{ src_beg + cnt };

    void* buffer{ __builtin_alloca_with_align(
        elem_size * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t)) };

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
                       .data = buffer,
                       .elem_size = elem_size,
                       .elem_stride = elem_size,
                   },
                   src_cursor);
            (Write)(dst_cntr, dst_cursor, cur_cnt,
                    MemWriter{
                        .data = buffer,
                        .elem_size = elem_size,
                        .elem_stride = elem_size,
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
                   .data = buffer,
                   .elem_size = elem_size,
                   .elem_stride = elem_size,
               },
               nullptr);

        (Write)(dst_cntr, dst_cursor, cur_cnt,
                MemWriter{
                    .data = buffer,
                    .elem_size = elem_size,
                    .elem_stride = elem_size,
                },
                nullptr);

        cnt -= cur_cnt;
    }
}

template <typename DstCntrLike, typename SrcCntrLike>
void seq_cntr::Assign(DstCntrLike&& dst_cntr_, SrcCntrLike&& src_cntr_) {
    auto* dst_cntr{ utils::GetInstPtr(meta::Forward<DstCntrLike>(dst_cntr_)) };
    auto* src_cntr{ utils::GetInstPtr(meta::Forward<SrcCntrLike>(src_cntr_)) };

    using DstCntr = meta::RemovePointer<decltype(dst_cntr)>;

    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    size_t dst_size{ (GetElemCnt)(dst_cntr) };
    size_t src_size{ (GetElemCnt)(src_cntr) };

    constexpr AbilityFlag static_enabled_ability_flag{ (
        GetStaticEnabledAbilityFlag<DstCntr>)() };

    constexpr AbilityFlag static_disabled_ability_flag{ (
        GetStaticDisabledAbilityFlag<DstCntr>)() };

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
