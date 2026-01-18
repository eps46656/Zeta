#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("TestAbility")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                                   \
    (((ability_flag) & (static_cast<::zeta::core::seq_cntr::AbilityFlag>(1)  \
                        << ::zeta::core::seq_cntr::AbilityEnum::ability)) != \
     0)

namespace zeta::core::seq_cntr {

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

inline void MemReader::operator()(void const* src, size_t src_stride,
                                  size_t cnt) {
    ElemCopy(this->dst, src, this->dst_width, this->dst_stride, src_stride,
             cnt);
    this->dst = static_cast<char*>(this->dst) + this->dst_stride * cnt;
}

inline void MemWriter::operator()(void* dst, size_t dst_stride, size_t cnt) {
    ElemCopy(dst, this->src, this->src_width, dst_stride, this->src_stride,
             cnt);
    this->src = static_cast<char const*>(this->src) + this->src_stride * cnt;
}

template <typename SeqCntr>
void CheckContract(SeqCntr* cntr) {
    constexpr type_wrapper::TypeWrapper<SeqCntr*> cntr_ptr_type_wrapper;

    void* void_ptr{ nullptr };
    void const* const_void_ptr{ nullptr };

    size_t size_val{ 0 };

    auto reader{ [](void const*, size_t, size_t) {} };
    auto writer{ [](void*, size_t, size_t) {} };
    auto reader_writer{ [](void*, size_t, size_t) {} };

#pragma push_macro("CheckMethod")
#pragma push_macro("CheckMethodOp")
#pragma push_macro("CheckConstMethodOp")

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(SeqCntr::method(__VA_ARGS__)), return_type>);

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

    ZETA_Core_StaticAssert(IsAnyOf<decltype(SeqCntr::IsConst()), bool>);

    constexpr bool is_const{ SeqCntr::IsConst() };

    using elem_ptr_t = Conditional<is_const, void const*, void*>;

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(SeqCntr::GetStaticEnabledAbilityFlag(
                    cntr_ptr_type_wrapper)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(SeqCntr::GetStaticDisabledAbilityFlag(
                    cntr_ptr_type_wrapper)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(SeqCntr::GetDynamicEnabledAbilityFlag(cntr)),
                AbilityFlag>);

    ZETA_Core_StaticAssert(
        IsAnyOf<decltype(SeqCntr::GetDynamicDisabledAbilityFlag(cntr)),
                AbilityFlag>);

    constexpr AbilityFlag static_disabled_ability_flag{
        SeqCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper)
    };

    CheckConstMethodOp(  //
        GetCursorSize,   // ability
        GetCursorSize,   // method

        size_t,  // return cursor size

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
        GetLBCursor,     // ability
        GetLBCursor,     // method
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
        void_ptr,        // dst_cursor, optional
        void_ptr         // mem, optional
    );

    CheckConstMethodOp(  //
        Access,          // ability
        Access,          // method
                         //
        void*,           // elem(returned)
                         //
        cntr,            // inst
        size_val,        // idx
        void_ptr,        // dst_cursor, optional
        void_ptr         // mem, optional
    );

    CheckConstMethodOp(  //
        Derefer,         // ability
        Derefer,         // method
                         //
        void*,           // return, elem
                         //
        cntr,            // inst
        const_void_ptr,  // pos_cursor
        void_ptr         // mem, optional
    );

    CheckConstMethodOp(  //
        Read,            // ability
        Read,            // method
                         //
        TypeAny,         // return, none
                         //
        cntr,            // inst
        const_void_ptr,  // pos_cursor
        size_val,        // cnt
        Move(reader),    // reader
        void_ptr         // dst_cursor
    );

    CheckMethodOp(     //
        Write,         // ability
        Write,         // method
                       //
        TypeAny,       // return: none
                       //
        cntr,          // inst
        void_ptr,      // pos_cursor, point to original position
        size_val,      // cnt
        Move(writer),  // writer
        void_ptr       // dst_cursor, optional, point to final position
                       // after write
    );

    CheckMethodOp(            //
        ReadWrite,            // ability
        ReadWrite,            // method
                              //
        TypeAny,              // return void
                              //
        cntr,                 // inst
        void_ptr,             // pos_cursor
        size_val,             // cnt
        Move(reader_writer),  // reader_writer
        void_ptr              // dst_cursor
    );

    CheckMethodOp(     //
        PushL,         // ability
        PushL,         // method
                       //
        void*,         // return, elem
                       //
        cntr,          // inst
        size_val,      // cnt
        Move(writer),  // writer
        void_ptr       // dst_cursor
    );

    CheckMethodOp(     //
        PushR,         // ability
        PushR,         // method
                       //
        void*,         //
                       //
        cntr,          // inst
        size_val,      // cnt
        Move(writer),  //
        void_ptr       // dst_cursor
    );

    CheckMethodOp(     //
        Insert,        // ability
        Insert,        // method
                       //
        void*,         // return, elem
                       //
        cntr,          // inst
        void_ptr,      // pos_cursor
        size_val,      // cnt
        Move(writer),  // writer
        void_ptr       // dst_cursor
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
        void_ptr,   // pos_cursor
        size_val    // cnt
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
        void_ptr,        // dst_cursor
        const_void_ptr   // src_cursor
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

template <typename SeqCntr>
constexpr VTable BasicVTableBuilder<SeqCntr>::Build() {
    constexpr AbilityFlag static_disabled_ability_flag{
        SeqCntr::GetStaticDisabledAbilityFlag()
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
        .Access = F(Access, Access),
        .Derefer = F(Derefer, Derefer),

        .FnRead = F(Read, FnRead),
        .FnWrite = F(Read, FnWrite),
        .FnReadWrite = F(ReadWrite, FnReadWrite),

        .MemRead = F(Read, MemRead),
        .MemWrite = F(Write, MemWrite),

        .FnPushL = F(PushL, FnPushL),
        .FnPushR = F(PushR, FnPushR),
        .FnInsert = F(Insert, FnInsert),

        .MemPushL = F(PushL, MemPushL),
        .MemPushR = F(PushR, MemPushR),
        .MemInsert = F(Insert, MemInsert),

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

template <typename SeqCntr>
constexpr size_t BasicVTableBuilder<SeqCntr>::GetSize(void* cntr) {
    return SeqCntr::GetSize(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
constexpr size_t BasicVTableBuilder<SeqCntr>::GetCapacity(void* cntr) {
    return SeqCntr::GetCapacity(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::GetLBCursor(void* cntr,
                                                        void* dst_cursor) {
    return SeqCntr::GetLBCursor(static_cast<SeqCntr*>(cntr), dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::GetRBCursor(void* cntr,
                                                        void* dst_cursor) {
    return SeqCntr::GetRBCursor(static_cast<SeqCntr*>(cntr), dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::PeekL(void* cntr, void* dst_cursor,
                                                   void* dst_elem) {
    return SeqCntr::PeekL(static_cast<SeqCntr*>(cntr), dst_cursor, dst_elem);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::PeekR(void* cntr, void* dst_cursor,
                                                   void* dst_elem) {
    return SeqCntr::PeekR(static_cast<SeqCntr*>(cntr), dst_cursor, dst_elem);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::Access(void* cntr, size_t idx,
                                                    void* dst_cursor,
                                                    void* dst_elem) {
    return SeqCntr::Access(static_cast<SeqCntr*>(cntr), idx, dst_cursor,
                           dst_elem);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::Derefer(void* cntr,
                                                     void const* pos_cursor,
                                                     void* dst_elem) {
    return SeqCntr::Derefer(static_cast<SeqCntr*>(cntr), pos_cursor, dst_elem);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::FnRead(void* cntr,
                                                   void const* pos_cursor,
                                                   size_t cnt, FnReader reader,
                                                   void* dst_cursor) {
    return SeqCntr::Read(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, reader,
                         dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::FnWrite(void* cntr,
                                                    void* pos_cursor,
                                                    size_t cnt, FnWriter writer,
                                                    void* dst_cursor) {
    return SeqCntr::Write(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                          dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::FnReadWrite(
    void* cntr, void* pos_cursor, size_t cnt, FnReaderWriter reader_writer,
    void* dst_cursor) {
    return SeqCntr::ReadWrite(static_cast<SeqCntr*>(cntr), pos_cursor, cnt,
                              reader_writer, dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::MemRead(void* cntr,
                                                    void const* pos_cursor,
                                                    size_t cnt,
                                                    MemReader reader,
                                                    void* dst_cursor) {
    return SeqCntr::Read(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, reader,
                         dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::MemWrite(void* cntr,
                                                     void* pos_cursor,
                                                     size_t cnt,
                                                     MemWriter writer,
                                                     void* dst_cursor) {
    return SeqCntr::Write(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                          dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::FnPushL(void* cntr, size_t cnt,
                                                     FnWriter writer,
                                                     void* dst_cursor) {
    return SeqCntr::PushL(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::FnPushR(void* cntr, size_t cnt,
                                                     FnWriter writer,
                                                     void* dst_cursor) {
    return SeqCntr::PushR(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::FnInsert(void* cntr,
                                                      void* pos_cursor,
                                                      size_t cnt,
                                                      FnWriter writer,
                                                      void* dst_cursor) {
    return SeqCntr::Insert(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                           dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::MemPushL(void* cntr, size_t cnt,
                                                      MemWriter writer,
                                                      void* dst_cursor) {
    return SeqCntr::PushL(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::MemPushR(void* cntr, size_t cnt,
                                                      MemWriter writer,
                                                      void* dst_cursor) {
    return SeqCntr::PushR(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
constexpr void* BasicVTableBuilder<SeqCntr>::MemInsert(void* cntr,
                                                       void* pos_cursor,
                                                       size_t cnt,
                                                       MemWriter writer,
                                                       void* dst_cursor) {
    return SeqCntr::Insert(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                           dst_cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::PopL(void* cntr, size_t cnt) {
    return SeqCntr::PopL(static_cast<SeqCntr*>(cntr), cnt);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::PopR(void* cntr, size_t cnt) {
    return SeqCntr::PopR(static_cast<SeqCntr*>(cntr), cnt);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::Erase(void* cntr, void* pos_cursor,
                                                  size_t cnt) {
    return SeqCntr::Erase(static_cast<SeqCntr*>(cntr), pos_cursor, cnt);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::EraseAll(void* cntr) {
    return SeqCntr::EraseAll(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::CopyCursor(void* cntr,
                                                       void* dst_cursor,
                                                       void const* src_cursor) {
    return SeqCntr::CopyCursor(static_cast<SeqCntr*>(cntr), dst_cursor,
                               src_cursor);
}

template <typename SeqCntr>
constexpr bool BasicVTableBuilder<SeqCntr>::AreEqualCursor(
    void* cntr, void const* cursor_a, void const* cursor_b) {
    return SeqCntr::AreEqualCursor(static_cast<SeqCntr*>(cntr), cursor_a,
                                   cursor_b);
}

template <typename SeqCntr>
constexpr int BasicVTableBuilder<SeqCntr>::CompareCursor(void* cntr,
                                                         void const* cursor_a,
                                                         void const* cursor_b) {
    return SeqCntr::CompareCursor(static_cast<SeqCntr*>(cntr), cursor_a,
                                  cursor_b);
}

template <typename SeqCntr>
constexpr size_t BasicVTableBuilder<SeqCntr>::GetCursorDist(
    void* cntr, void const* cursor_a, void const* cursor_b) {
    return SeqCntr::GetCursorDist(static_cast<SeqCntr*>(cntr), cursor_a,
                                  cursor_b);
}

template <typename SeqCntr>
constexpr size_t BasicVTableBuilder<SeqCntr>::GetCursorIdx(void* cntr,
                                                           void const* cursor) {
    return SeqCntr::GetCursorIdx(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::CursorStepL(void* cntr,
                                                        void* cursor) {
    return SeqCntr::CursorStepL(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::CursorStepR(void* cntr,
                                                        void* cursor) {
    return SeqCntr::CursorStepR(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::CursorAdvanceL(void* cntr,
                                                           void* cursor,
                                                           size_t step) {
    return SeqCntr::CursorAdvanceL(static_cast<SeqCntr*>(cntr), cursor, step);
}

template <typename SeqCntr>
constexpr void BasicVTableBuilder<SeqCntr>::CursorAdvanceR(void* cntr,
                                                           void* cursor,
                                                           size_t step) {
    return SeqCntr::CursorAdvanceR(static_cast<SeqCntr*>(cntr), cursor, step);
}

template <typename SeqCntr>
constexpr VTable VTableBuilder<SeqCntr>::Build() {
    return BasicVTableBuilder<SeqCntr>::Build();
};

namespace detail {

template <typename SeqCntr>
struct VTableHolder_ {
    static constexpr VTable vtable{ VTableBuilder<SeqCntr>::Build() };
};

}  // namespace detail

template <typename SeqCntr>
constexpr VTable const& GetVTable() {
    return detail::VTableHolder_<SeqCntr>::vtable;
}

#pragma push_macro("CallMethod_")
#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(tmp_method_ptr, ability, method, ...)             \
    {                                                                 \
        ZETA_Core_DebugAssert(                                        \
            TestAbility(ref->dynamic_enabled_ability_flag, ability)); \
                                                                      \
        auto tmp_method_ptr{ ref->vtable->method };                   \
        ZETA_Core_DebugAssert(tmp_method_ptr != nullptr);             \
                                                                      \
        return tmp_method_ptr(ref->cntr, __VA_ARGS__);                \
    }                                                                 \
    ZETA_Core_StaticAssert(true);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...) \
    CallMethod_(ZETA_Core_TmpName, ability, method, __VA_ARGS__)

template <typename IsConst_, typename>
constexpr bool RefOperator::IsConst(type_wrapper::TypeWrapper<Ref<IsConst_>*>) {
    return IsConst_::value;
}

template <typename IsConst_, typename>
constexpr bool RefOperator::IsConst(
    type_wrapper::TypeWrapper<Ref<IsConst_> const*>) {
    return IsConst_::value;
}

template <typename IsConst, typename>
constexpr AbilityFlag RefOperator::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref<IsConst>*>) {
    return empty_ability_flag;
}

template <typename IsConst, typename>
constexpr AbilityFlag RefOperator::GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref<IsConst> const*>) {
    return empty_ability_flag;
}

template <typename IsConst, typename>
constexpr AbilityFlag RefOperator::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref<IsConst>*>) {
    return empty_ability_flag;
}

template <typename IsConst, typename>
constexpr AbilityFlag RefOperator::GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<Ref<IsConst> const*>) {
    return non_const_ability_flag;
}

template <typename IsConst, typename>
AbilityFlag RefOperator::GetDynamicEnabledAbilityFlag(Ref<IsConst>* ref) {
    if constexpr (IsConst::value) {
        return ref->dynamic_enabled_ability_flag;
    } else {
        constexpr AbilityFlag static_ability_flag{
            GetStaticEnabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{}) |
            GetStaticDisabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{})
        };

        return ~static_ability_flag & ref->dynamic_enabled_ability_flag;
    }
}

template <typename IsConst, typename>
AbilityFlag RefOperator::GetDynamicEnabledAbilityFlag(Ref<IsConst> const* ref) {
    if constexpr (IsConst::value) {
        return ref->dynamic_enabled_ability_flag;
    } else {
        constexpr AbilityFlag static_ability_flag{
            GetStaticEnabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{}) |
            GetStaticDisabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{})
        };

        return ~static_ability_flag & ref->dynamic_enabled_ability_flag;
    }
}

template <typename IsConst, typename>
AbilityFlag RefOperator::GetDynamicDisabledAbilityFlag(Ref<IsConst>* ref) {
    if constexpr (IsConst::value) {
        return ref->dynamic_disabled_ability_flag;
    } else {
        constexpr AbilityFlag static_ability_flag{
            GetStaticEnabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{}) |
            GetStaticDisabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{})
        };

        return ~static_ability_flag & ref->dynamic_disabled_ability_flag;
    }
}

template <typename IsConst, typename>
AbilityFlag RefOperator::GetDynamicDisabledAbilityFlag(
    Ref<IsConst> const* ref) {
    if constexpr (IsConst::value) {
        return ref->dynamic_disabled_ability_flag;
    } else {
        constexpr AbilityFlag static_ability_flag{
            GetStaticEnabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{}) |
            GetStaticDisabledAbilityFlag(
                type_wrapper::TypeWrapper<Ref<IsConst> const*>{})
        };

        return ~static_ability_flag & ref->dynamic_disabled_ability_flag;
    }
}

template <typename IsConst, typename>
size_t RefOperator::GetCursorSize(Ref<IsConst> const* ref) {
    return ref->cursor_size;
}

template <typename IsConst, typename>
size_t RefOperator::GetWidth(Ref<IsConst> const* ref) {
    return ref->width;
}

template <typename IsConst, typename>
size_t RefOperator::GetSize(Ref<IsConst> const* ref) {
    CallMethod(GetSize, GetSize);
}

template <typename IsConst, typename>
size_t RefOperator::GetCapacity(Ref<IsConst> const* ref) {
    CallMethod(GetCapacity, GetCapacity);
}

template <typename IsConst, typename>
void RefOperator::GetLBCursor(Ref<IsConst> const* ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <typename IsConst, typename>
void RefOperator::GetRBCursor(Ref<IsConst> const* ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <typename IsConst, typename>
Conditional<IsConst::value, void const*, void*> RefOperator::PeekL(
    Ref<IsConst>* ref, void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
void const* RefOperator::PeekL(Ref<IsConst> const* ref, void* dst_cursor,
                               void* dst_elem) {
    CallMethod(PeekL, PeekL, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
Conditional<IsConst::value, void const*, void*> RefOperator::PeekR(
    Ref<IsConst>* ref, void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
void const* RefOperator::PeekR(Ref<IsConst> const* ref, void* dst_cursor,
                               void* dst_elem) {
    CallMethod(PeekR, PeekR, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
Conditional<IsConst::value, void const*, void*> RefOperator::Access(
    Ref<IsConst>* ref, size_t idx, void* dst_cursor, void* dst_elem) {
    CallMethod(Access, Access, idx, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
void const* RefOperator::Access(Ref<IsConst> const* ref, size_t idx,
                                void* dst_cursor, void* dst_elem) {
    CallMethod(Access, Access, idx, dst_cursor, dst_elem);
}

template <typename IsConst, typename>
Conditional<IsConst::value, void const*, void*> RefOperator::Derefer(
    Ref<IsConst>* ref, void const* pos_cursor, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, dst_elem);
}

template <typename IsConst, typename>
void const* RefOperator::Derefer(Ref<IsConst> const* ref,
                                 void const* pos_cursor, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, dst_elem);
}

template <typename IsConst, typename Reader, typename>
void RefOperator::Read(Ref<IsConst> const* ref, void const* pos_cursor,
                       size_t cnt, Reader&& reader, void* dst_cursor) {
    CallMethod(Read, FnRead, pos_cursor, cnt, Forward<Reader>(reader),
               dst_cursor);
}

template <typename Writer>
void RefOperator::Write(Ref<value_wrapper::FalseType>* ref, void* pos_cursor,
                        size_t cnt, Writer&& writer, void* dst_cursor) {
    CallMethod(Write, FnWrite, pos_cursor, cnt, Forward<Writer>(writer),
               dst_cursor);
}

template <typename ReaderWriter>
void RefOperator::ReadWrite(Ref<value_wrapper::FalseType>* ref,
                            void* pos_cursor, size_t cnt,
                            ReaderWriter&& reader_writer, void* dst_cursor) {
    CallMethod(ReadWrite, FnReadWrite, pos_cursor, cnt,
               Forward<ReaderWriter>(reader_writer), dst_cursor);
}

template <typename IsConst, typename>
void RefOperator::Read(Ref<IsConst> const* ref, void const* pos_cursor,
                       size_t cnt, MemReader reader, void* dst_cursor) {
    CallMethod(Read, MemRead, pos_cursor, cnt, reader, dst_cursor);
}

void RefOperator::Write(Ref<value_wrapper::FalseType>* ref, void* pos_cursor,
                        size_t cnt, MemWriter writer, void* dst_cursor) {
    CallMethod(Write, MemWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename Writer>
void* RefOperator::PushL(Ref<value_wrapper::FalseType>* ref, size_t cnt,
                         Writer&& writer, void* dst_cursor) {
    CallMethod(PushL, FnPushL, cnt, Forward<Writer>(writer), dst_cursor);
}

template <typename Writer>
void* RefOperator::PushR(Ref<value_wrapper::FalseType>* ref, size_t cnt,
                         Writer&& writer, void* dst_cursor) {
    CallMethod(PushR, FnPushR, cnt, Forward<Writer>(writer), dst_cursor);
}

template <typename Writer>
void* RefOperator::Insert(Ref<value_wrapper::FalseType>* ref, void* pos_cursor,
                          size_t cnt, Writer&& writer, void* dst_cursor) {
    CallMethod(Insert, FnInsert, pos_cursor, cnt, Forward<Writer>(writer),
               dst_cursor);
}

void* RefOperator::PushL(Ref<value_wrapper::FalseType>* ref, size_t cnt,
                         MemWriter writer, void* dst_cursor) {
    CallMethod(PushL, MemPushL, cnt, writer, dst_cursor);
}

void* RefOperator::PushR(Ref<value_wrapper::FalseType>* ref, size_t cnt,
                         MemWriter writer, void* dst_cursor) {
    CallMethod(PushR, MemPushR, cnt, writer, dst_cursor);
}

void* RefOperator::Insert(Ref<value_wrapper::FalseType>* ref, void* pos_cursor,
                          size_t cnt, MemWriter writer, void* dst_cursor) {
    CallMethod(Insert, MemInsert, pos_cursor, cnt, writer, dst_cursor);
}

void RefOperator::PopL(Ref<value_wrapper::FalseType>* ref, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

void RefOperator::PopR(Ref<value_wrapper::FalseType>* ref, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

void RefOperator::Erase(Ref<value_wrapper::FalseType>* ref, void* pos_cursor,
                        size_t cnt) {
    CallMethod(Erase, Erase, pos_cursor, cnt);
}

void RefOperator::EraseAll(Ref<value_wrapper::FalseType>* ref) {
    CallMethod(EraseAll, EraseAll);
}

template <typename IsConst, typename>
void RefOperator::CopyCursor(Ref<IsConst> const* ref, void* dst_cursor,
                             void const* src_cursor) {
    CallMethod(CopyCursor, CopyCursor, dst_cursor, src_cursor);
}

template <typename IsConst, typename>
bool RefOperator::AreEqualCursor(Ref<IsConst> const* ref, void const* cursor_a,
                                 void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <typename IsConst, typename>
int RefOperator::CompareCursor(Ref<IsConst> const* ref, void const* cursor_a,
                               void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <typename IsConst, typename>
size_t RefOperator::GetCursorDist(Ref<IsConst> const* ref, void const* cursor_a,
                                  void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <typename IsConst, typename>
size_t RefOperator::GetCursorIdx(Ref<IsConst> const* ref, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <typename IsConst, typename>
void RefOperator::CursorStepL(Ref<IsConst> const* ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <typename IsConst, typename>
void RefOperator::CursorStepR(Ref<IsConst> const* ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <typename IsConst, typename>
void RefOperator::CursorAdvanceL(Ref<IsConst> const* ref, void* cursor,
                                 size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <typename IsConst, typename>
void RefOperator::CursorAdvanceR(Ref<IsConst> const* ref, void* cursor,
                                 size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename RefType, typename>
void RefOperator::CheckCntr(RefType* ref) {
    ZETA_Core_DebugAssert(ref != nullptr);
    ZETA_Core_DebugAssert(0 < ref->width);
    ZETA_Core_DebugAssert(ref->vtable != nullptr);
    ZETA_Core_DebugAssert(ref->cntr != nullptr);

    constexpr AbilityFlag static_enabled_ability_flag{
        GetStaticEnabledAbilityFlag(type_wrapper::TypeWrapper<RefType*>{})
    };

    AbilityFlag static_disabled_ability_flag{ GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<RefType*>{}) };

    CheckAbilityFlags(static_enabled_ability_flag, static_disabled_ability_flag,
                      ref->dynamic_enabled_ability_flag,
                      ref->dynamic_disabled_ability_flag);

    AbilityFlag enabled_ability_flag{ static_enabled_ability_flag |
                                      ref->dynamic_enabled_ability_flag };

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
    CheckMethod(Access, Access);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Read, FnRead);
    CheckMethod(Write, FnWrite);
    CheckMethod(ReadWrite, FnReadWrite);

    CheckMethod(Read, MemRead);
    CheckMethod(Write, MemWrite);

    CheckMethod(PushL, FnPushL);
    CheckMethod(PushR, FnPushR);
    CheckMethod(Insert, FnInsert);

    CheckMethod(PushL, MemPushL);
    CheckMethod(PushR, MemPushR);
    CheckMethod(Insert, MemInsert);

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

template <typename SC>
auto MakeRef(SC&& cntr_) {
    auto cntr{ GetInstPtr(Forward<SC>(cntr_)) };

    using SeqCntr = RemovePointer<decltype(cntr)>;

    CheckContract(cntr);

    size_t cursor_size{ SeqCntr::GetCursorSize(cntr) };
    ZETA_Core_DebugAssert(cursor_size <= ZETA_Core_ushrt_max);

    return Ref<value_wrapper::StaticValueWrapper<SeqCntr::IsConst()>>{
        .cursor_size = static_cast<unsigned short>(cursor_size),

        .width = SeqCntr::GetWidth(cntr),
        .capacity = SeqCntr::GetCapacity(cntr),

        .dynamic_enabled_ability_flag =
            SeqCntr::GetStaticEnabledAbilityFlag() |
            SeqCntr::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            SeqCntr::GetStaticDisabledAbilityFlag() |
            SeqCntr::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &GetVTable<SeqCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(cntr)),
    };
}

constexpr bool IsReferable(size_t idx, size_t cnt, size_t size) {
    return idx + 1 < size + 2 && cnt <= size - idx + 1;
}

constexpr bool IsDereferable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

constexpr bool IsInsertable(size_t idx, size_t cnt, size_t size,
                            size_t capacity) {
    return idx <= size && size <= capacity && cnt <= capacity - size;
}

constexpr bool IsErasable(size_t idx, size_t cnt, size_t size) {
    return idx <= size && cnt <= size - idx;
}

template <typename DstSeqCntr, typename SrcSeqCntr>
void RangeAssign(DstSeqCntr* dst_cntr, SrcSeqCntr* src_cntr, size_t dst_beg,
                 size_t src_beg, size_t cnt) {
    ZETA_Core_DebugAssert(CheckContract(dst_cntr));
    ZETA_Core_DebugAssert(CheckContract(src_cntr));

    ZETA_Core_DebugAssert(DstSeqCntr::GetWidth(dst_cntr) ==
                          SrcSeqCntr::GetWidth(src_cntr));

    size_t width{ DstSeqCntr::GetWidth(dst_cntr) };

    size_t buffer_capacity{ Max(1ULL, sizeof(void*) * 1024 / width) };

    size_t dst_size{ DstSeqCntr::GetSize(dst_cntr) };
    size_t src_size{ SrcSeqCntr::GetSize(src_cntr) };

    ZETA_Core_DebugAssert(IsDereferable(dst_beg, cnt, dst_size));
    ZETA_Core_DebugAssert(IsDereferable(src_beg, cnt, src_size));

    void const* real_dst_cntr{ dst_cntr };
    void const* real_src_cntr{ src_cntr };

    if constexpr (IsAnyOf<RemoveCVRef<DstSeqCntr>,
                          Ref<value_wrapper::FalseType>,
                          Ref<value_wrapper::TrueType>>) {
        real_dst_cntr = dst_cntr->cntr;
    }

    if constexpr (IsAnyOf<RemoveCVRef<SrcSeqCntr>,
                          Ref<value_wrapper::FalseType>,
                          Ref<value_wrapper::TrueType>>) {
        real_src_cntr = src_cntr->cntr;
    }

    if (cnt == 0 || real_dst_cntr == real_src_cntr) { return; }

    size_t dst_end{ dst_beg + cnt };
    size_t src_end{ src_beg + cnt };

    void* buffer{ __builtin_alloca_with_align(
        width * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t)) };

    void* dst_cursor{ ZETA_Core_SeqCntr_AllocaCursor(dst_cntr) };
    void* src_cursor{ ZETA_Core_SeqCntr_AllocaCursor(src_cntr) };

    if (real_dst_cntr != real_src_cntr || dst_beg <= src_beg ||
        src_end <= dst_beg) {
        DstSeqCntr::Access(dst_cntr, dst_beg, dst_cursor, nullptr);
        SrcSeqCntr::Access(src_cntr, src_beg, src_cursor, nullptr);

        while (0 < cnt) {
            size_t cur_cnt{ Min(buffer_capacity, cnt) };

            SrcSeqCntr::MemRead(src_cntr, src_cursor, cur_cnt, buffer, width,
                                src_cursor);
            DstSeqCntr::MemWrite(dst_cntr, dst_cursor, cur_cnt, buffer, width,
                                 dst_cursor);

            cnt -= cur_cnt;
        }

        return;
    }

    DstSeqCntr::Access(dst_cntr, dst_end, dst_cursor, nullptr);
    SrcSeqCntr::Access(src_cntr, src_end, src_cursor, nullptr);

    while (0 < cnt) {
        size_t cur_cnt{ Min(buffer_capacity, cnt) };

        SrcSeqCntr::CursorAdvanceL(src_cntr, src_cursor, cur_cnt);
        DstSeqCntr::CursorAdvanceL(dst_cntr, dst_cursor, cur_cnt);

        SrcSeqCntr::MemRead(src_cntr, src_cursor, cur_cnt, buffer, width,
                            nullptr);

        DstSeqCntr::MemWrite(dst_cntr, dst_cursor, cur_cnt, buffer, width,
                             nullptr);

        cnt -= cur_cnt;
    }
}

template <typename DstSeqCntr, typename SrcSeqCntr>
void Assign(DstSeqCntr* dst_cntr, SrcSeqCntr* src_cntr) {
    size_t dst_size{ DstSeqCntr::GetSize(dst_cntr) };
    size_t src_size{ SrcSeqCntr::GetSize(src_cntr) };

    constexpr AbilityFlag static_enabled_ability_flag{
        DstSeqCntr::GetStaticEnabledAbilityFlag(dst_cntr)
    };

    constexpr AbilityFlag static_disabled_ability_flag{
        DstSeqCntr::GetStaticDisabledAbilityFlag(dst_cntr)
    };

    AbilityFlag dynamic_enabled_ability_flag{
        DstSeqCntr::GetDynamicEnabledAbilityFlag(dst_cntr)
    };

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
#pragma push_macro("FPushR")
#pragma push_macro("FPopL")
#pragma push_macro("FPopR")

#define FPushL \
    DstSeqCntr::MemPushL(dst_cntr, src_size - dst_size, nullptr, 0, nullptr);

#define FPushR \
    DstSeqCntr::MemPushR(dst_cntr, src_size - dst_size, nullptr, 0, nullptr);

#define FPopL DstSeqCntr::PopL(dst_cntr, dst_size - src_size);

#define FPopR DstSeqCntr::PopR(dst_cntr, dst_size - src_size);

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

#pragma pop_macro("FPushL")
#pragma pop_macro("FPushR")
#pragma pop_macro("FPopL")
#pragma pop_macro("FPopR")

    RangeAssign(dst_cntr, src_cntr, 0, 0, src_size);
}

}  // namespace zeta::core::seq_cntr
