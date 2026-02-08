#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
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

constexpr bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                                 AbilityFlag static_disabled_ability_flag) {
    AbilityFlag ability_flags[]{ static_enabled_ability_flag,
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

namespace ops {

template <typename SeqCntrLike>
void CheckContract(SeqCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = RemovePointer<decltype(cntr)>;

    constexpr type_wrapper::TypeWrapper<SeqCntr*> cntr_ptr_type_wrapper;

    bool bool_val{ false };

    void* void_ptr{ nullptr };
    void const* const_void_ptr{ nullptr };

    size_t size_val{ 0 };

    auto reader{ [](void const*, size_t, size_t) {} };
    auto writer{ [](void*, size_t, size_t) {} };
    auto reader_writer{ [](void*, size_t, size_t) {} };

#pragma push_macro("CheckMethod")
#pragma push_macro("CheckMethodOp")

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, return_type, ...) \
    ZETA_Core_StaticAssert(                   \
        IsAnyOf<decltype(SeqCntr::method(__VA_ARGS__)), return_type>);

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(ability, method, return_type, ...)                 \
    if constexpr (!TestAbility(static_disabled_ability_flag, ability)) { \
        CheckMethod(method, return_type, __VA_ARGS__)                    \
    }

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

    CheckMethodOp(      //
        GetCursorSize,  // ability
        GetCursorSize,  // method

        size_t,  // return cursor size

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

    CheckMethodOp(  //
        Access,     // ability
        Access,     // method
                    //
        void*,      // elem(returned)
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
        void*,           // return, elem
                         //
        cntr,            // inst
        const_void_ptr,  // pos_cursor
        bool_val,        // lazy_copy_elem
        void_ptr         // mem, optional
    );

    CheckMethodOp(       //
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

template <typename SeqCntr>
constexpr VTable BasicVTableBuilder<SeqCntr>::Build() {
    constexpr AbilityFlag static_disabled_ability_flag{
        SeqCntr::GetStaticDisabledAbilityFlag(
            type_wrapper::TypeWrapper<SeqCntr*>{})
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
        .custom_tags{ 0 },

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

        .CustomMethods{ nullptr },
    };

#pragma pop_macro("F")

    return table;
}

template <typename SeqCntr>
size_t BasicVTableBuilder<SeqCntr>::GetSize(void* cntr) {
    return SeqCntr::GetSize(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
constexpr size_t BasicVTableBuilder<SeqCntr>::GetCapacity(void* cntr) {
    return SeqCntr::GetCapacity(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::GetLBCursor(void* cntr, void* dst_cursor) {
    return SeqCntr::GetLBCursor(static_cast<SeqCntr*>(cntr), dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::GetRBCursor(void* cntr, void* dst_cursor) {
    return SeqCntr::GetRBCursor(static_cast<SeqCntr*>(cntr), dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::PeekL(void* cntr, bool lazy_copy_elem,
                                         void* dst_cursor, void* dst_elem) {
    return SeqCntr::PeekL(static_cast<SeqCntr*>(cntr), lazy_copy_elem,
                          dst_cursor, dst_elem);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::PeekR(void* cntr, bool lazy_copy_elem,
                                         void* dst_cursor, void* dst_elem) {
    return SeqCntr::PeekR(static_cast<SeqCntr*>(cntr), lazy_copy_elem,
                          dst_cursor, dst_elem);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::Access(void* cntr, size_t idx,
                                          bool lazy_copy_elem, void* dst_cursor,
                                          void* dst_elem) {
    return SeqCntr::Access(static_cast<SeqCntr*>(cntr), idx, lazy_copy_elem,
                           dst_cursor, dst_elem);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::Derefer(void* cntr, void const* pos_cursor,
                                           bool lazy_copy_elem,
                                           void* dst_elem) {
    return SeqCntr::Derefer(static_cast<SeqCntr*>(cntr), pos_cursor,
                            lazy_copy_elem, dst_elem);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::FnRead(void* cntr, void const* pos_cursor,
                                         size_t cnt, FnReader reader,
                                         void* dst_cursor) {
    return SeqCntr::Read(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, reader,
                         dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::FnWrite(void* cntr, void* pos_cursor,
                                          size_t cnt, FnWriter writer,
                                          void* dst_cursor) {
    return SeqCntr::Write(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                          dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::FnReadWrite(void* cntr, void* pos_cursor,
                                              size_t cnt,
                                              FnReaderWriter reader_writer,
                                              void* dst_cursor) {
    return SeqCntr::ReadWrite(static_cast<SeqCntr*>(cntr), pos_cursor, cnt,
                              reader_writer, dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::MemRead(void* cntr, void const* pos_cursor,
                                          size_t cnt, MemReader reader,
                                          void* dst_cursor) {
    return SeqCntr::Read(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, reader,
                         dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::MemWrite(void* cntr, void* pos_cursor,
                                           size_t cnt, MemWriter writer,
                                           void* dst_cursor) {
    return SeqCntr::Write(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                          dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::FnPushL(void* cntr, size_t cnt,
                                           FnWriter writer, void* dst_cursor) {
    return SeqCntr::PushL(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::FnPushR(void* cntr, size_t cnt,
                                           FnWriter writer, void* dst_cursor) {
    return SeqCntr::PushR(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::FnInsert(void* cntr, void* pos_cursor,
                                            size_t cnt, FnWriter writer,
                                            void* dst_cursor) {
    return SeqCntr::Insert(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                           dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::MemPushL(void* cntr, size_t cnt,
                                            MemWriter writer,
                                            void* dst_cursor) {
    return SeqCntr::PushL(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::MemPushR(void* cntr, size_t cnt,
                                            MemWriter writer,
                                            void* dst_cursor) {
    return SeqCntr::PushR(static_cast<SeqCntr*>(cntr), cnt, writer, dst_cursor);
}

template <typename SeqCntr>
void* BasicVTableBuilder<SeqCntr>::MemInsert(void* cntr, void* pos_cursor,
                                             size_t cnt, MemWriter writer,
                                             void* dst_cursor) {
    return SeqCntr::Insert(static_cast<SeqCntr*>(cntr), pos_cursor, cnt, writer,
                           dst_cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::PopL(void* cntr, size_t cnt) {
    return SeqCntr::PopL(static_cast<SeqCntr*>(cntr), cnt);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::PopR(void* cntr, size_t cnt) {
    return SeqCntr::PopR(static_cast<SeqCntr*>(cntr), cnt);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::Erase(void* cntr, void* pos_cursor,
                                        size_t cnt) {
    return SeqCntr::Erase(static_cast<SeqCntr*>(cntr), pos_cursor, cnt);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::EraseAll(void* cntr) {
    return SeqCntr::EraseAll(static_cast<SeqCntr*>(cntr));
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::CopyCursor(void* cntr, void const* src_cursor,
                                             void* dst_cursor) {
    return SeqCntr::CopyCursor(static_cast<SeqCntr*>(cntr), src_cursor,
                               dst_cursor);
}

template <typename SeqCntr>
bool BasicVTableBuilder<SeqCntr>::AreEqualCursor(void* cntr,
                                                 void const* cursor_a,
                                                 void const* cursor_b) {
    return SeqCntr::AreEqualCursor(static_cast<SeqCntr*>(cntr), cursor_a,
                                   cursor_b);
}

template <typename SeqCntr>
int BasicVTableBuilder<SeqCntr>::CompareCursor(void* cntr, void const* cursor_a,
                                               void const* cursor_b) {
    return SeqCntr::CompareCursor(static_cast<SeqCntr*>(cntr), cursor_a,
                                  cursor_b);
}

template <typename SeqCntr>
size_t BasicVTableBuilder<SeqCntr>::GetCursorDist(void* cntr,
                                                  void const* cursor_a,
                                                  void const* cursor_b) {
    return SeqCntr::GetCursorDist(static_cast<SeqCntr*>(cntr), cursor_a,
                                  cursor_b);
}

template <typename SeqCntr>
size_t BasicVTableBuilder<SeqCntr>::GetCursorIdx(void* cntr,
                                                 void const* cursor) {
    return SeqCntr::GetCursorIdx(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::CursorStepL(void* cntr, void* cursor) {
    return SeqCntr::CursorStepL(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::CursorStepR(void* cntr, void* cursor) {
    return SeqCntr::CursorStepR(static_cast<SeqCntr*>(cntr), cursor);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::CursorAdvanceL(void* cntr, void* cursor,
                                                 size_t step) {
    return SeqCntr::CursorAdvanceL(static_cast<SeqCntr*>(cntr), cursor, step);
}

template <typename SeqCntr>
void BasicVTableBuilder<SeqCntr>::CursorAdvanceR(void* cntr, void* cursor,
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

inline AbilityFlag Ref::GetDynamicEnabledAbilityFlag(Ref* ref) {
    return ref->dynamic_enabled_ability_flag;
}

inline AbilityFlag Ref::GetDynamicEnabledAbilityFlag(Ref const* ref) {
    return non_const_ability_flag &
           GetDynamicEnabledAbilityFlag(const_cast<Ref*>(ref));
}

inline AbilityFlag Ref::GetDynamicDisabledAbilityFlag(Ref* ref) {
    return ref->dynamic_disabled_ability_flag;
}

inline AbilityFlag Ref::GetDynamicDisabledAbilityFlag(Ref const* ref) {
    return const_ability_flag &
           GetDynamicDisabledAbilityFlag(const_cast<Ref*>(ref));
}

inline size_t Ref::GetCursorSize(Ref const* ref) { return ref->cursor_size; }

inline size_t Ref::GetWidth(Ref const* ref) { return ref->width; }

inline size_t Ref::GetSize(Ref const* ref) { CallMethod(GetSize, GetSize); }

inline size_t Ref::GetCapacity(Ref const* ref) {
    CallMethod(GetCapacity, GetCapacity);
}

inline void Ref::GetLBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void Ref::GetRBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void* Ref::PeekL(Ref* ref, bool lazy_copy_elem, void* dst_cursor,
                        void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void const* Ref::PeekL(Ref const* ref, bool lazy_copy_elem,
                              void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* Ref::PeekR(Ref* ref, bool lazy_copy_elem, void* dst_cursor,
                        void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void const* Ref::PeekR(Ref const* ref, bool lazy_copy_elem,
                              void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* Ref::Access(Ref* ref, size_t idx, bool lazy_copy_elem,
                         void* dst_cursor, void* dst_elem) {
    CallMethod(Access, Access, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void const* Ref::Access(Ref const* ref, size_t idx, bool lazy_copy_elem,
                               void* dst_cursor, void* dst_elem) {
    CallMethod(Access, Access, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* Ref::Derefer(Ref* ref, void const* pos_cursor, bool lazy_copy_elem,
                          void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

inline void const* Ref::Derefer(Ref const* ref, void const* pos_cursor,
                                bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename Reader>
void Ref::Read(
    Ref const* ref, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Read, FnRead, pos_cursor, cnt, reader, dst_cursor);
}

template <typename Writer>
void Ref::Write(
    Ref* ref, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Write, FnWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void Ref::ReadWrite(
    Ref* ref, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(ReadWrite, FnReadWrite, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

inline void Ref::Read(Ref const* ref, void const* pos_cursor, size_t cnt,
                      MemReader reader, void* dst_cursor) {
    CallMethod(Read, MemRead, pos_cursor, cnt, reader, dst_cursor);
}

inline void Ref::Write(Ref* ref, void* pos_cursor, size_t cnt, MemWriter writer,
                       void* dst_cursor) {
    CallMethod(Write, MemWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename Writer>
void* Ref::PushL(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushL, FnPushL, cnt, writer, dst_cursor);
}

template <typename Writer>
void* Ref::PushR(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushR, FnPushR, cnt, writer, dst_cursor);
}

template <typename Writer>
void* Ref::Insert(
    Ref* ref, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Insert, FnInsert, pos_cursor, cnt, writer, dst_cursor);
}

inline void* Ref::PushL(Ref* ref, size_t cnt, MemWriter writer,
                        void* dst_cursor) {
    CallMethod(PushL, MemPushL, cnt, writer, dst_cursor);
}

inline void* Ref::PushR(Ref* ref, size_t cnt, MemWriter writer,
                        void* dst_cursor) {
    CallMethod(PushR, MemPushR, cnt, writer, dst_cursor);
}

inline void* Ref::Insert(Ref* ref, void* pos_cursor, size_t cnt,
                         MemWriter writer, void* dst_cursor) {
    CallMethod(Insert, MemInsert, pos_cursor, cnt, writer, dst_cursor);
}

inline void Ref::PopL(Ref* ref, size_t cnt) { CallMethod(PopL, PopL, cnt); }

inline void Ref::PopR(Ref* ref, size_t cnt) { CallMethod(PopR, PopR, cnt); }

inline void Ref::Erase(Ref* ref, void* pos_cursor, size_t cnt) {
    CallMethod(Erase, Erase, pos_cursor, cnt);
}

inline void Ref::EraseAll(Ref* ref) { CallMethod(EraseAll, EraseAll); }

inline void Ref::CopyCursor(Ref const* ref, void const* src_cursor,
                            void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool Ref::AreEqualCursor(Ref const* ref, void const* cursor_a,
                                void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int Ref::CompareCursor(Ref const* ref, void const* cursor_a,
                              void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t Ref::GetCursorDist(Ref const* ref, void const* cursor_a,
                                 void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t Ref::GetCursorIdx(Ref const* ref, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void Ref::CursorStepL(Ref const* ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void Ref::CursorStepR(Ref const* ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void Ref::CursorAdvanceL(Ref const* ref, void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void Ref::CursorAdvanceR(Ref const* ref, void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

inline void Ref::CheckCntr(Ref* ref) {
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

namespace ops {

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

#pragma push_macro("CallMethod")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...)                                    \
    {                                                                       \
        auto* cntr{ GetInstPtr(Forward<SeqCntrLike>(cntr_)) };              \
        using SeqCntr = RemovePointer<decltype(cntr)>;                      \
                                                                            \
        CheckContract(cntr);                                                \
                                                                            \
        constexpr AbilityFlag static_enabled_ability_flag{                  \
            GetStaticEnabledAbilityFlag(                                    \
                type_wrapper::TypeWrapper<SeqCntrLike&&>())                 \
        };                                                                  \
                                                                            \
        constexpr AbilityFlag static_disabled_ability_flag{                 \
            GetStaticDisabledAbilityFlag(                                   \
                type_wrapper::TypeWrapper<SeqCntrLike&&>())                 \
        };                                                                  \
                                                                            \
        if constexpr (TestAbility(static_enabled_ability_flag,              \
                                  AbilityEnum::ability)) {                  \
            return SeqCntr::method(cntr, __VA_ARGS__);                      \
        } else if constexpr (TestAbility(static_disabled_ability_flag,      \
                                         AbilityEnum::ability)) {           \
            ZETA_Core_StaticAssert(false);                                  \
        } else {                                                            \
            AbilityFlag dynamic_enabled_ability_flag{                       \
                GetDynamicEnabledAbilityFlag(Forward<SeqCntrLike>(cntr_))   \
            };                                                              \
                                                                            \
            ZETA_Core_DebugAssert(TestAbility(dynamic_enabled_ability_flag, \
                                              AbilityEnum::ability));       \
                                                                            \
            return SeqCntr::method(cntr, __VA_ARGS__);                      \
        }                                                                   \
    }                                                                       \
    ZETA_Core_StaticAssert(true);

template <typename SeqCntrLike>
constexpr AbilityFlag GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrLike>) {
    using SeqCntr = RemovePointer<decltype(GetInstPtr(Declval<SeqCntrLike>()))>;

    return SeqCntr::GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<SeqCntr*>());
}

template <typename SeqCntrLike>
constexpr AbilityFlag GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrLike>) {
    using SeqCntr = RemovePointer<decltype(GetInstPtr(Declval<SeqCntrLike>()))>;

    return SeqCntr::GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<SeqCntr*>());
}

template <typename SeqCntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(SeqCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = RemovePointer<decltype(cntr)>;

    return SeqCntr::GetDynamicEnabledAbilityFlag(cntr);
}

template <typename SeqCntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(SeqCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = RemovePointer<decltype(cntr)>;

    return SeqCntr::GetDynamicDisabledAbilityFlag(cntr);
}

template <typename SeqCntrLike>
decltype(auto) GetCursorSize(SeqCntrLike&& cntr_) {
    CallMethod(GetCursorSize, GetCursorSize);
}

template <typename SeqCntrLike>
size_t GetWidth(SeqCntrLike&& cntr_) {
    CallMethod(GetWidth, GetWidth);
}

template <typename SeqCntrLike>
size_t GetSize(SeqCntrLike&& cntr_) {
    CallMethod(GetSize, GetSize);
}

template <typename SeqCntrLike>
decltype(auto) GetCapacity(SeqCntrLike&& cntr_) {
    CallMethod(GetCapacity, GetCapacity);
}

template <typename SeqCntrLike>
void GetLBCursor(SeqCntrLike&& cntr_, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <typename SeqCntrLike>
void GetRBCursor(SeqCntrLike&& cntr_, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <typename SeqCntrLike>
auto PeekL(SeqCntrLike&& cntr_, bool lazy_copy_elem, void* dst_cursor,
           void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
auto PeekR(SeqCntrLike&& cntr_, bool lazy_copy_elem, void* dst_cursor,
           void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
auto Access(SeqCntrLike&& cntr_, size_t idx, bool lazy_copy_elem,
            void* dst_cursor, void* dst_elem) {
    CallMethod(Access, Access, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

template <typename SeqCntrLike>
auto Derefer(SeqCntrLike&& cntr_, void const* pos_cursor, bool lazy_copy_elem,
             void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename SeqCntrLike, typename Reader>
void Read(SeqCntrLike&& cntr_, void const* pos_cursor, size_t cnt,
          Reader&& reader, void* dst_cursor) {
    CallMethod(Read, Read, pos_cursor, cnt, Forward<Reader>(reader),
               dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void Write(SeqCntrLike&& cntr_, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor) {
    CallMethod(Write, Write, pos_cursor, cnt, Forward<Writer>(writer),
               dst_cursor);
}

template <typename SeqCntrLike, typename ReaderWriter>
void ReadWrite(SeqCntrLike&& cntr_, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor) {
    CallMethod(ReadWrite, ReadWrite, pos_cursor, cnt,
               Forward<ReaderWriter>(reader_writer), dst_cursor);
}

template <typename SeqCntrLike>
void* PushL(SeqCntrLike&& cntr_, size_t cnt, auto&& writer, void* dst_cursor) {
    CallMethod(PushL, PushL, cnt, Forward<decltype(writer)>(writer),
               dst_cursor);
}

template <typename SeqCntrLike>
void* PushR(SeqCntrLike&& cntr_, size_t cnt, auto&& writer, void* dst_cursor) {
    CallMethod(PushR, PushR, cnt, Forward<decltype(writer)>(writer),
               dst_cursor);
}

template <typename SeqCntrLike, typename Writer>
void Insert(SeqCntrLike&& cntr_, void* pos_cursor, size_t cnt, Writer&& writer,
            void* dst_cursor) {
    CallMethod(Insert, Insert, pos_cursor, cnt,
               Forward<decltype(writer)>(writer), dst_cursor);
}

template <typename SeqCntrLike>
void PopL(SeqCntrLike&& cntr_, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

template <typename SeqCntrLike>
void PopR(SeqCntrLike&& cntr_, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

template <typename SeqCntrLike>
void Erase(SeqCntrLike&& cntr_, void* pos_cursor, size_t cnt) {
    CallMethod(Erase, Erase, pos_cursor, cnt);
}

template <typename SeqCntrLike>
void EraseAll(SeqCntrLike&& cntr_) {
    CallMethod(EraseAll, EraseAll);
}

template <typename SeqCntrLike>
void CopyCursor(SeqCntrLike&& cntr_, void const* src_cursor, void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <typename SeqCntrLike>
bool AreEqualCursor(SeqCntrLike&& cntr_, void const* cursor_a,
                    void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <typename SeqCntrLike>
int CompareCursor(SeqCntrLike&& cntr_, void const* cursor_a,
                  void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <typename SeqCntrLike>
size_t GetCursorDist(SeqCntrLike&& cntr_, void const* cursor_a,
                     void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <typename SeqCntrLike>
size_t GetCursorIdx(SeqCntrLike&& cntr_, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <typename SeqCntrLike>
void CursorStepL(SeqCntrLike&& cntr_, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <typename SeqCntrLike>
void CursorStepR(SeqCntrLike&& cntr_, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <typename SeqCntrLike>
void CursorAdvanceL(SeqCntrLike&& cntr_, void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <typename SeqCntrLike>
void CursorAdvanceR(SeqCntrLike&& cntr_, void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void RangeAssign(DstSeqCntrLike&& dst_cntr_, SrcSeqCntrLike&& src_cntr_,
                 size_t dst_beg, size_t src_beg, size_t cnt) {
    auto* dst_cntr{ GetInstPtr(Forward<DstSeqCntrLike>(dst_cntr_)) };
    auto* src_cntr{ GetInstPtr(Forward<SrcSeqCntrLike>(src_cntr_)) };

    using DstSeqCntr = RemovePointer<decltype(dst_cntr)>;
    using SrcSeqCntr = RemovePointer<decltype(src_cntr)>;

    CheckContract(dst_cntr);
    CheckContract(src_cntr);

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

    if constexpr (IsAnyOf<RemoveCVRef<DstSeqCntr>, Ref>) {
        real_dst_cntr = dst_cntr->cntr;
    }

    if constexpr (IsAnyOf<RemoveCVRef<SrcSeqCntr>, Ref>) {
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

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void Assign(DstSeqCntrLike&& dst_cntr_, SrcSeqCntrLike&& src_cntr_) {
    auto* dst_cntr{ GetInstPtr(Forward<DstSeqCntrLike>(dst_cntr_)) };
    auto* src_cntr{ GetInstPtr(Forward<SrcSeqCntrLike>(src_cntr_)) };

    using DstSeqCntr = RemovePointer<decltype(dst_cntr)>;
    using SrcSeqCntr = RemovePointer<decltype(src_cntr)>;

    CheckContract(dst_cntr);
    CheckContract(src_cntr);

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

template <typename SeqCntrLike>
Ref MakeRef  // NOLINT(misc-use-internal-linkage)
    (SeqCntrLike&& cntr_) {
    auto* cntr{ GetInstPtr(Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = RemovePointer<decltype(cntr)>;

    CheckContract(cntr);

    size_t cursor_size{ SeqCntr::GetCursorSize(cntr) };
    ZETA_Core_DebugAssert(cursor_size <= ZETA_Core_ushrt_max);

    constexpr type_wrapper::TypeWrapper<SeqCntr*> cntr_ptr_type_wrapper;

    return {
        .cursor_size = static_cast<unsigned short>(cursor_size),

        .width = SeqCntr::GetWidth(cntr),
        .capacity = SeqCntr::GetCapacity(cntr),

        .dynamic_enabled_ability_flag =
            SeqCntr::GetStaticEnabledAbilityFlag(cntr_ptr_type_wrapper) |
            SeqCntr::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            SeqCntr::GetStaticDisabledAbilityFlag(cntr_ptr_type_wrapper) |
            SeqCntr::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &GetVTable<SeqCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(cntr)),
    };
}

}  // namespace ops

}  // namespace zeta::core::seq_cntr
