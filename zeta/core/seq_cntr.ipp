/**
@file zeta/core/seq_cntr.ipp
*/

#pragma once

#include <zeta/core/comparison_utils.ipp>
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

#pragma push_macro("TestCapability")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(capability_flag, capability)           \
    (((capability_flag) &                                     \
      (static_cast<::zeta::core::seq_cntr::CapabilityFlag>(1) \
       << ::zeta::core::seq_cntr::CapabilityEnum::capability::value)) != 0)

constexpr void seq_cntr::EmptyReader::operator()(void const*, size_t,
                                                 size_t) const {}

constexpr void seq_cntr::EmptyWriter::operator()(void*, size_t, size_t) const {}

inline void seq_cntr::MemReader::operator()(void const* src,
                                            size_t src_elem_stride,
                                            size_t cnt) {
    utils::ElemCopy(this->data, src, this->elem_size, this->elem_stride,
                    src_elem_stride, cnt);
    this->data = static_cast<char*>(this->data) + this->elem_stride * cnt;
}

inline void seq_cntr::MemWriter::operator()(void* dst, size_t dst_elem_stride,
                                            size_t cnt) {
    utils::ElemCopy(dst, this->data, this->elem_size, dst_elem_stride,
                    this->elem_stride, cnt);
    this->data = static_cast<char const*>(this->data) + this->elem_stride * cnt;
}

constexpr bool seq_cntr::CheckCapabilityFlags(
    seq_cntr::CapabilityFlag static_enabled_capability_flag,
    seq_cntr::CapabilityFlag static_disabled_capability_flag) {
    seq_cntr::CapabilityFlag capability_flags[]{
        static_enabled_capability_flag, static_disabled_capability_flag
    };

    for (CapabilityFlag capability_flag : capability_flags) {
        if ((capability_flag & empty_capability_flag) !=
            empty_capability_flag) {
            return false;
        }

        if ((capability_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    return (static_enabled_capability_flag & static_disabled_capability_flag) ==
           empty_capability_flag;
}

inline bool seq_cntr::CheckCapabilityFlags(
    CapabilityFlag static_enabled_capability_flag,
    CapabilityFlag static_disabled_capability_flag,
    CapabilityFlag dynamic_enabled_capability_flag,
    CapabilityFlag dynamic_disabled_capability_flag) {
    CapabilityFlag capability_flags[]{ static_enabled_capability_flag,
                                       static_disabled_capability_flag,
                                       dynamic_enabled_capability_flag,
                                       dynamic_disabled_capability_flag };

    for (CapabilityFlag capability_flag : capability_flags) {
        if ((capability_flag & empty_capability_flag) !=
            empty_capability_flag) {
            return false;
        }
        if ((capability_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    for (int i{ 0 }; i < 4; ++i) {
        for (int j{ i + 1 }; j < 4; ++j) {
            if ((capability_flags[i] & capability_flags[j]) !=
                empty_capability_flag) {
                return false;
            }
        }
    }

    return (static_enabled_capability_flag | static_disabled_capability_flag |
            dynamic_enabled_capability_flag |
            dynamic_disabled_capability_flag) == full_capability_flag;
}

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ret, capability_name, method_name, ...)                  \
    if constexpr (!TestCapability((GetStaticEnabledCapabilityFlag<Cntr>)(), \
                                  capability_name)) {                       \
        ZETA_Core_StaticAssert(!TestCapability(                             \
            (GetStaticDisabledCapabilityFlag<Cntr>)(), capability_name));   \
                                                                            \
        ZETA_Core_DebugAssert(TestCapability(                               \
            (GetDynamicEnabledCapabilityFlag)(cntr), capability_name));     \
    }                                                                       \
                                                                            \
    if constexpr (ret) {                                                    \
        return CntrTraits<Cntr>::method_name(__VA_ARGS__);                  \
    } else {                                                                \
        CntrTraits<Cntr>::method_name(__VA_ARGS__);                         \
    };                                                                      \
                                                                            \
    ZETA_Core_StaticAssert(true)

template <typename Cntr>
void* seq_cntr::GetReferedInstPtr(Cntr& cntr) {
    return CntrTraits<Cntr>::GetReferedInstPtr(cntr);
}

template <typename Cntr>
constexpr seq_cntr::CapabilityFlag seq_cntr::GetStaticEnabledCapabilityFlag() {
    constexpr CapabilityFlag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr CapabilityFlag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    (CheckCapabilityFlags)(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_enabled_capability_flag;
}

template <typename Cntr>
constexpr seq_cntr::CapabilityFlag seq_cntr::GetStaticDisabledCapabilityFlag() {
    constexpr CapabilityFlag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr CapabilityFlag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    (CheckCapabilityFlags)(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_disabled_capability_flag;
}

template <typename Cntr>
seq_cntr::CapabilityFlag seq_cntr::GetDynamicEnabledCapabilityFlag(Cntr& cntr) {
    constexpr CapabilityFlag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr CapabilityFlag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    CheckCapabilityFlags(static_enabled_capability_flag,
                         static_disabled_capability_flag);

    CapabilityFlag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    CapabilityFlag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    CheckCapabilityFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <typename Cntr>
seq_cntr::CapabilityFlag seq_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr& cntr) {
    constexpr CapabilityFlag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr CapabilityFlag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    CheckCapabilityFlags(static_enabled_capability_flag,
                         static_disabled_capability_flag);

    CapabilityFlag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    CapabilityFlag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    CheckCapabilityFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <typename Cntr>
size_t seq_cntr::GetCursorSize(Cntr& cntr) {
    CallMethod(true, GetCursorSize, GetCursorSize, cntr);
}

template <typename Cntr>
size_t seq_cntr::GetElemSize(Cntr& cntr) {
    CallMethod(true, GetElemSize, GetElemSize, cntr);
}

template <typename Cntr>
size_t seq_cntr::GetElemCnt(Cntr& cntr) {
    CallMethod(true, GetElemCnt, GetElemCnt, cntr);
}

template <typename Cntr>
size_t seq_cntr::GetMaxElemCnt(Cntr& cntr) {
    CallMethod(true, GetMaxElemCnt, GetMaxElemCnt, cntr);
}

template <typename Cntr>
void seq_cntr::GetLBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(false, GetLBCursor, GetLBCursor, cntr, dst_cursor);
}

template <typename Cntr>
void seq_cntr::GetRBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(false, GetRBCursor, GetRBCursor, cntr, dst_cursor);
}

template <typename Cntr>
void seq_cntr::PeekL(Cntr& cntr, bool lazy_copy_elem,
                     ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                     void* dst_elem) {
    CallMethod(true, PeekL, PeekL, cntr, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <typename Cntr>
void seq_cntr::PeekR(Cntr& cntr, bool lazy_copy_elem,
                     ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                     void* dst_elem) {
    CallMethod(true, PeekR, PeekR, cntr, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <typename Cntr>
void seq_cntr::Refer(Cntr& cntr, size_t idx, bool lazy_copy_elem,
                     ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                     void* dst_elem) {
    CallMethod(true, Refer, Refer, cntr, idx, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <typename Cntr>
void seq_cntr::Derefer(Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
                       ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(true, Derefer, Derefer, cntr, pos_cursor, lazy_copy_elem,
               dst_elem_ptr_view, dst_elem);
}

template <typename Cntr, typename Reader>
void seq_cntr::Read(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, Read, Read, cntr, pos_cursor, cnt, reader, dst_cursor);
}

template <typename Cntr, typename Writer>
void seq_cntr::Write(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, Write, Write, cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <typename Cntr, typename ReaderWriter>
void seq_cntr::ReadWrite(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, ReadWrite, ReadWrite, cntr, pos_cursor, cnt,
               reader_writer, dst_cursor);
}

template <typename Cntr, typename Writer>
void seq_cntr::PushL(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, PushL, PushL, cntr, cnt, writer, dst_cursor);
}

template <typename Cntr, typename Writer>
void seq_cntr::PushR(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, PushR, PushR, cntr, cnt, writer, dst_cursor);
}

template <typename Cntr, typename Writer>
void seq_cntr::Insert(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(false, Insert, Insert, cntr, pos_cursor, cnt, writer,
               dst_cursor);
}

template <typename Cntr, typename Reader>
void seq_cntr::PopL(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(false, PopL, PopL, cntr, cnt, reader);
}

template <typename Cntr, typename Reader>
void seq_cntr::PopR(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(false, PopR, PopR, cntr, cnt, reader);
}

template <typename Cntr, typename Reader>
void seq_cntr::Erase(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(false, Erase, Erase, cntr, pos_cursor, cnt, reader);
}

template <typename Cntr>
void seq_cntr::EraseAll(Cntr& cntr) {
    CallMethod(false, EraseAll, EraseAll, cntr);
}

template <typename Cntr>
void seq_cntr::CopyCursor(Cntr& cntr, void* src_cursor, void* dst_cursor) {
    CallMethod(false, CopyCursor, CopyCursor, cntr, src_cursor, dst_cursor);
}

template <typename Cntr>
bool seq_cntr::AreEqualCursor(Cntr& cntr, void* cursor_a, void* cursor_b) {
    CallMethod(true, AreEqualCursor, AreEqualCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
int seq_cntr::CompareCursor(Cntr& cntr, void* cursor_a, void* cursor_b) {
    CallMethod(true, CompareCursor, CompareCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
size_t seq_cntr::GetCursorDist(Cntr& cntr, void* cursor_a, void* cursor_b) {
    CallMethod(true, GetCursorDist, GetCursorDist, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
size_t seq_cntr::GetCursorIdx(Cntr& cntr, void* cursor) {
    CallMethod(true, GetCursorIdx, GetCursorIdx, cntr, cursor);
}

template <typename Cntr>
void seq_cntr::CursorStepL(Cntr& cntr, void* cursor) {
    CallMethod(false, CursorStepL, CursorStepL, cntr, cursor);
}

template <typename Cntr>
void seq_cntr::CursorStepR(Cntr& cntr, void* cursor) {
    CallMethod(false, CursorStepR, CursorStepR, cntr, cursor);
}

template <typename Cntr>
void seq_cntr::CursorAdvanceL(Cntr& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceL, CursorAdvanceL, cntr, cursor, step);
}

template <typename Cntr>
void seq_cntr::CursorAdvanceR(Cntr& cntr, void* cursor, size_t step) {
    CallMethod(false, CursorAdvanceR, CursorAdvanceR, cntr, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename Cntr>
void seq_cntr::CheckContract(Cntr& cntr) {
    struct Reader {
        void operator()(void const*, size_t, size_t) const {}
    };

    struct Writer {
        void operator()(void*, size_t, size_t) const {}
    };

    struct ReaderWriter {
        void operator()(void*, size_t, size_t) const {}
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                                        \
    ZETA_Core_Unused(                                                   \
        [&](bool bool_val, void* void_ptr, void const* const_void_ptr,  \
            size_t size_val, seq_cntr::ElemPtrView* elem_ptr_view_ptr,  \
            Reader reader, Writer writer, ReaderWriter reader_writer) { \
            ZETA_Core_Unused(bool_val);                                 \
            ZETA_Core_Unused(void_ptr);                                 \
            ZETA_Core_Unused(const_void_ptr);                           \
            ZETA_Core_Unused(elem_ptr_view_ptr);                        \
            ZETA_Core_Unused(size_val);                                 \
            ZETA_Core_Unused(reader);                                   \
            ZETA_Core_Unused(writer);                                   \
            ZETA_Core_Unused(reader_writer);                            \
            (method)(__VA_ARGS__);                                      \
        })

#pragma push_macro("CheckMethodOp")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethodOp(capability, method, ...)                     \
    if constexpr (!TestCapability(static_disabled_capability_flag, \
                                  capability)) {                   \
        CheckMethod(method, __VA_ARGS__);                          \
    }                                                              \
                                                                   \
    ZETA_Core_StaticAssert(true);

    CheckMethod(                               //
        GetStaticEnabledCapabilityFlag<Cntr>,  // method
    );

    CheckMethod(                                //
        GetStaticDisabledCapabilityFlag<Cntr>,  // method
    );

    CheckMethod(                          //
        GetDynamicEnabledCapabilityFlag,  // method

        cntr  //
    );

    CheckMethod(                           //
        GetDynamicDisabledCapabilityFlag,  // method

        cntr  //
    );

    constexpr CapabilityFlag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<Cntr>)() };

    CheckMethodOp(      //
        GetCursorSize,  // capability
        GetCursorSize,  // method
                        //
        cntr            //
    );

    CheckMethodOp(   //
        GetElemCnt,  // capability
        GetElemCnt,  // method
                     //
        cntr         //
    );

    CheckMethodOp(      //
        GetMaxElemCnt,  // capability
        GetMaxElemCnt,  // method
                        //
        cntr            //
    );

    CheckMethodOp(    //
        GetLBCursor,  // capability
        GetLBCursor,  // method
                      //
        cntr,         //
        void_ptr      // cursor
    );

    CheckMethodOp(    //
        GetRBCursor,  // capability
        GetRBCursor,  // method
                      //
        cntr,         //
        void_ptr      // cursor
    );

    CheckMethodOp(          //
        PeekL,              // capability
        PeekL,              // method
                            //
        cntr,               //
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor(optional)
        void_ptr            // mem(optional)
    );

    CheckMethodOp(          //
        PeekR,              // capability
        PeekR,              // method
                            //
        cntr,               //
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor, optional
        void_ptr            // mem, optional
    );

    CheckMethodOp(          //
        Refer,              // capability
        Refer,              // method
                            //
        cntr,               //
        size_val,           // idx
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor, optional
        void_ptr            // mem, optional
    );

    CheckMethodOp(          //
        Derefer,            // capability
        Derefer,            // method
                            //
        cntr,               //
        void_ptr,           // pos_cursor
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr            // mem, optional
    );

    CheckMethodOp(  //
        Read,       // capability
        Read,       // method
                    //
        cntr,       //
        void_ptr,   // pos_cursor
        size_val,   // cnt
        reader,     // reader
        void_ptr    // dst_cursor
    );

    CheckMethodOp(  //
        Write,      // capability
        Write,      // method
                    //
        cntr,       //
        void_ptr,   // pos_cursor, point to original position
        size_val,   // cnt
        writer,     // writer
        void_ptr    // dst_cursor, optional, point to final position
                    // after write
    );

    CheckMethodOp(      //
        ReadWrite,      // capability
        ReadWrite,      // method
                        //
        cntr,           //
        void_ptr,       // pos_cursor
        size_val,       // cnt
        reader_writer,  // reader_writer
        void_ptr        // dst_cursor
    );

    CheckMethodOp(  //
        PushL,      // capability
        PushL,      // method
                    //
        cntr,       //
        size_val,   // cnt
        writer,     // writer
        void_ptr    // dst_cursor
    );

    CheckMethodOp(  //
        PushR,      // capability
        PushR,      // method
                    //
        cntr,       //
        size_val,   // cnt
        writer,     //
        void_ptr    // dst_cursor
    );

    CheckMethodOp(  //
        Insert,     // capability
        Insert,     // method
                    //
        cntr,       //
        void_ptr,   // pos_cursor
        size_val,   // cnt
        writer,     // writer
        void_ptr    // dst_cursor
    );

    CheckMethodOp(  //
        PopL,       // capability
        PopL,       // method
                    //
        cntr,       //
        size_val,   // cnt
        reader      // reader
    );

    CheckMethodOp(  //
        PopR,       // capability
        PopR,       // method
                    //
        cntr,       //
        size_val,   // cnt
        reader      // reader
    );

    CheckMethodOp(  //
        Erase,      // capability
        Erase,      // method
                    //
        cntr,       //
        void_ptr,   // pos_cursor
        size_val,   // cnt
        reader      // reader
    );

    CheckMethodOp(  //
        EraseAll,   // capability
        EraseAll,   // method
                    //
        cntr        //
    );

    CheckMethodOp(   //
        CopyCursor,  // capability
        CopyCursor,  // method
                     //
        cntr,        //
        void_ptr,    // src_cursor
        void_ptr     // dst_cursor
    );

    CheckMethodOp(       //
        AreEqualCursor,  // capability
        AreEqualCursor,  // method
                         //
        cntr,            //
        void_ptr,        // cursor_a
        void_ptr         // cursor_b
    );

    CheckMethodOp(      //
        CompareCursor,  // capability
        CompareCursor,  // method
                        //
        cntr,           //
        void_ptr,       // cursor_a
        void_ptr        // cursor_b
    );

    CheckMethodOp(      //
        GetCursorDist,  // capability
        GetCursorDist,  // method
                        //
        cntr,           //
        void_ptr,       // cursor_a
        void_ptr        // cursor_b
    );

    CheckMethodOp(     //
        GetCursorIdx,  // capability
        GetCursorIdx,  // method
                       //
        cntr,          //
        void_ptr       // cursor
    );

    CheckMethodOp(    //
        CursorStepL,  // capability
        CursorStepL,  // method
                      //
        cntr,         //
        void_ptr      // cursor
    );

    CheckMethodOp(    //
        CursorStepR,  // capability
        CursorStepR,  // method
                      //
        cntr,         //
        void_ptr      // cursor
    );

    CheckMethodOp(       //
        CursorAdvanceL,  // capability
        CursorAdvanceL,  // method
                         //
        cntr,            //
        void_ptr,        // cursor
        size_val         // step
    );

    CheckMethodOp(       //
        CursorAdvanceR,  // capability
        CursorAdvanceR,  // method
                         //
        cntr,            //
        void_ptr,        // cursor
        size_val         // step
    );

#pragma pop_macro("CheckMethod")
#pragma pop_macro("CheckMethodOp")
}

template <typename Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTableBasic() {
    constexpr CapabilityFlag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<Cntr>)() };

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(capability, method)                                         \
    []() constexpr {                                                  \
        if constexpr (TestCapability(static_disabled_capability_flag, \
                                     capability)) {                   \
            return nullptr;                                           \
        } else {                                                      \
            return method;                                            \
        }                                                             \
    }()

    constexpr VTable table{
        .custom_tags{ 0 },

        .GetElemCnt = F(
            GetElemCnt,
            [](void* cntr) { return (GetElemCnt)(*static_cast<Cntr*>(cntr)); }),

        .GetMaxElemCnt =
            F(GetMaxElemCnt,
              [](void* cntr) {
                  return (GetMaxElemCnt)(*static_cast<Cntr*>(cntr));
              }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetLBCursor)(*static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetRBCursor)(*static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .PeekL = F(
            PeekL,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekL)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view, dst_cursor, dst_elem);
            }),

        .PeekR = F(
            PeekR,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekR)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view, dst_cursor, dst_elem);
            }),

        .Refer = F(Refer,
                   [](void* cntr, size_t idx, bool lazy_copy_elem,
                      ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                      void* dst_elem) {
                       return (Refer)(*static_cast<Cntr*>(cntr), idx,
                                      lazy_copy_elem, dst_elem_ptr_view,
                                      dst_cursor, dst_elem);
                   }),

        .Derefer = F(Derefer,
                     [](void* cntr, void* pos_cursor, bool lazy_copy_elem,
                        ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
                         return (Derefer)(*static_cast<Cntr*>(cntr), pos_cursor,
                                          lazy_copy_elem, dst_elem_ptr_view,
                                          dst_elem);
                     }),

        .EmptyRead = F(Read,
                       [](void* cntr, void* pos_cursor, size_t cnt,
                          EmptyReader reader, void* dst_cursor) {
                           return (Read)(*static_cast<Cntr*>(cntr), pos_cursor,
                                         cnt, reader, dst_cursor);
                       }),

        .MemRead = F(Read,
                     [](void* cntr, void* pos_cursor, size_t cnt,
                        MemReader& reader, void* dst_cursor) {
                         return (Read)(*static_cast<Cntr*>(cntr), pos_cursor,
                                       cnt, reader, dst_cursor);
                     }),

        .FnRead = F(Read,
                    [](void* cntr, void* pos_cursor, size_t cnt,
                       FnReader reader, void* dst_cursor) {
                        return (Read)(*static_cast<Cntr*>(cntr), pos_cursor,
                                      cnt, reader, dst_cursor);
                    }),

        .EmptyWrite = F(Write,
                        [](void* cntr, void* pos_cursor, size_t cnt,
                           EmptyWriter writer, void* dst_cursor) {
                            return (Write)(*static_cast<Cntr*>(cntr),
                                           pos_cursor, cnt, writer, dst_cursor);
                        }),

        .MemWrite = F(Write,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         MemWriter& writer, void* dst_cursor) {
                          return (Write)(*static_cast<Cntr*>(cntr), pos_cursor,
                                         cnt, writer, dst_cursor);
                      }),

        .FnWrite = F(Write,
                     [](void* cntr, void* pos_cursor, size_t cnt,
                        FnWriter writer, void* dst_cursor) {
                         return (Write)(*static_cast<Cntr*>(cntr), pos_cursor,
                                        cnt, writer, dst_cursor);
                     }),

        .FnReadWrite = F(ReadWrite,
                         [](void* cntr, void* pos_cursor, size_t cnt,
                            FnReaderWriter reader_writer, void* dst_cursor) {
                             return (ReadWrite)(*static_cast<Cntr*>(cntr),
                                                pos_cursor, cnt, reader_writer,
                                                dst_cursor);
                         }),

        .EmptyPushL =
            F(PushL,
              [](void* cntr, size_t cnt, EmptyWriter writer, void* dst_cursor) {
                  return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemPushL =
            F(PushL,
              [](void* cntr, size_t cnt, MemWriter& writer, void* dst_cursor) {
                  return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnPushL =
            F(PushL,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .EmptyPushR =
            F(PushR,
              [](void* cntr, size_t cnt, EmptyWriter writer, void* dst_cursor) {
                  return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .MemPushR =
            F(PushR,
              [](void* cntr, size_t cnt, MemWriter& writer, void* dst_cursor) {
                  return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .FnPushR =
            F(PushR,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .EmptyInsert = F(Insert,
                         [](void* cntr, void* pos_cursor, size_t cnt,
                            EmptyWriter writer, void* dst_cursor) {
                             return (Insert)(*static_cast<Cntr*>(cntr),
                                             pos_cursor, cnt, writer,
                                             dst_cursor);
                         }),

        .MemInsert = F(Insert,
                       [](void* cntr, void* pos_cursor, size_t cnt,
                          MemWriter& writer, void* dst_cursor) {
                           return (Insert)(*static_cast<Cntr*>(cntr),
                                           pos_cursor, cnt, writer, dst_cursor);
                       }),

        .FnInsert = F(Insert,
                      [](void* cntr, void* pos_cursor, size_t cnt,
                         FnWriter writer, void* dst_cursor) {
                          return (Insert)(*static_cast<Cntr*>(cntr), pos_cursor,
                                          cnt, writer, dst_cursor);
                      }),

        .EmptyPopL = F(PopL,
                       [](void* cntr, size_t cnt, EmptyReader reader) {
                           return (PopL)(*static_cast<Cntr*>(cntr), cnt,
                                         reader);
                       }),

        .MemPopL = F(PopL,
                     [](void* cntr, size_t cnt, MemReader& reader) {
                         return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
                     }),

        .FnPopL = F(PopL,
                    [](void* cntr, size_t cnt, FnReader reader) {
                        return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
                    }),

        .EmptyPopR = F(PopR,
                       [](void* cntr, size_t cnt, EmptyReader reader) {
                           return (PopR)(*static_cast<Cntr*>(cntr), cnt,
                                         reader);
                       }),

        .MemPopR = F(PopR,
                     [](void* cntr, size_t cnt, MemReader& reader) {
                         return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
                     }),

        .FnPopR = F(PopR,
                    [](void* cntr, size_t cnt, FnReader reader) {
                        return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
                    }),
        .EmptyErase =
            F(Erase,
              [](void* cntr, void* pos_cursor, size_t cnt, EmptyReader reader) {
                  return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                 reader);
              }),

        .MemErase =
            F(Erase,
              [](void* cntr, void* pos_cursor, size_t cnt, MemReader& reader) {
                  return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                 reader);
              }),

        .FnErase =
            F(Erase,
              [](void* cntr, void* pos_cursor, size_t cnt, FnReader reader) {
                  return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                 reader);
              }),

        .EraseAll =
            F(EraseAll,
              [](void* cntr) { return (EraseAll)(*static_cast<Cntr*>(cntr)); }),

        .CopyCursor = F(CopyCursor,
                        [](void* cntr, void* src_cursor, void* dst_cursor) {
                            return (CopyCursor)(*static_cast<Cntr*>(cntr),
                                                src_cursor, dst_cursor);
                        }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void* cursor_a, void* cursor_b) {
                  return (AreEqualCursor)(*static_cast<Cntr*>(cntr), cursor_a,
                                          cursor_b);
              }),

        .CompareCursor = F(CompareCursor,
                           [](void* cntr, void* cursor_a, void* cursor_b) {
                               return (CompareCursor)(*static_cast<Cntr*>(cntr),
                                                      cursor_a, cursor_b);
                           }),

        .GetCursorDist = F(GetCursorDist,
                           [](void* cntr, void* cursor_a, void* cursor_b) {
                               return (GetCursorDist)(*static_cast<Cntr*>(cntr),
                                                      cursor_a, cursor_b);
                           }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void* cursor) {
                              return (GetCursorIdx)(*static_cast<Cntr*>(cntr),
                                                    cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             return (CursorStepL)(*static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             return (CursorStepR)(*static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorAdvanceL =
            F(CursorAdvanceL,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceL)(*static_cast<Cntr*>(cntr), cursor,
                                          step);
              }),

        .CursorAdvanceR =
            F(CursorAdvanceR,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceR)(*static_cast<Cntr*>(cntr), cursor,
                                          step);
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

constexpr bool seq_cntr::check_operation::CanRefer(size_t idx, size_t cnt,
                                                   size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           cnt <= elem_cnt - idx + 1;
}

constexpr bool seq_cntr::check_operation::CanDerefer(size_t idx, size_t cnt,
                                                     size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx <= elem_cnt &&
           cnt <= elem_cnt - idx;
}

constexpr bool seq_cntr::check_operation::CanPushL(size_t cnt, size_t elem_cnt,
                                                   size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPushR(size_t cnt, size_t elem_cnt,
                                                   size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanInsert(size_t idx, size_t cnt,
                                                    size_t elem_cnt,
                                                    size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           idx <= elem_cnt && elem_cnt <= max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPopL(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPopR(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanErase(size_t idx, size_t cnt,
                                                   size_t elem_cnt) {
    return (CanDerefer)(idx, cnt, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanStepL(size_t idx,
                                                   size_t elem_cnt) {
    return (CanAdvancableL)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanStepR(size_t idx,
                                                   size_t elem_cnt) {
    return (CanAdvancableR)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanAdvancableL(size_t idx,
                                                         size_t step,
                                                         size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= idx + 1;
}

constexpr bool seq_cntr::check_operation::CanAdvancableR(size_t idx,
                                                         size_t step,
                                                         size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= elem_cnt - idx;
}

template <typename DstCntr, typename SrcCntr>
void seq_cntr::RangeAssign(DstCntr& dst_cntr, SrcCntr& src_cntr, size_t dst_beg,
                           size_t src_beg, size_t cnt) {
    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    ZETA_Core_DebugAssert((GetElemSize)(dst_cntr) == (GetElemSize)(src_cntr));

    size_t elem_size{ (GetElemSize)(dst_cntr) };

    size_t buffer_capacity{ comparison_utils::BasicMax(
        1U, sizeof(void*) * 1024 / elem_size) };

    size_t dst_elem_cnt{ (GetElemCnt)(dst_cntr) };
    size_t src_elem_cnt{ (GetElemCnt)(src_cntr) };

    ZETA_Core_DebugAssert(
        check_operation::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugAssert(
        check_operation::CanDerefer(src_beg, cnt, src_elem_cnt));

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
        (Refer)(dst_cntr, dst_beg, true, nullptr, dst_cursor, nullptr);
        (Refer)(src_cntr, src_beg, true, nullptr, src_cursor, nullptr);

        while (0 < cnt) {
            size_t cur_cnt{ comparison_utils::BasicMin(buffer_capacity, cnt) };

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

    (Refer)(dst_cntr, dst_end, true, nullptr, dst_cursor, nullptr);
    (Refer)(src_cntr, src_end, true, nullptr, src_cursor, nullptr);

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(buffer_capacity, cnt) };

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

template <typename DstCntr, typename SrcCntr>
void seq_cntr::Assign(DstCntr& dst_cntr, SrcCntr& src_cntr) {
    (CheckContract)(dst_cntr);
    (CheckContract)(src_cntr);

    size_t dst_size{ (GetElemCnt)(dst_cntr) };
    size_t src_size{ (GetElemCnt)(src_cntr) };

    constexpr CapabilityFlag static_enabled_capability_flag{ (
        GetStaticEnabledCapabilityFlag<DstCntr>)() };

    constexpr CapabilityFlag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<DstCntr>)() };

    CapabilityFlag dynamic_enabled_capability_flag{ (
        GetDynamicEnabledCapabilityFlag)(dst_cntr) };

    constexpr bool static_enabled_push_l{ TestCapability(
        static_enabled_capability_flag, CapabilityEnum::PushL) };

    constexpr bool static_disabled_push_l{ TestCapability(
        static_disabled_capability_flag, CapabilityEnum::PushL) };

    bool dynamic_enabled_push_l{ TestCapability(dynamic_enabled_capability_flag,
                                                CapabilityEnum::PushL) };

    constexpr bool static_enabled_push_r{ TestCapability(
        static_enabled_capability_flag, CapabilityEnum::PushR) };

    constexpr bool static_disabled_push_r{ TestCapability(
        static_disabled_capability_flag, CapabilityEnum::PushR) };

    bool dynamic_enabled_push_r{ TestCapability(dynamic_enabled_capability_flag,
                                                CapabilityEnum::PushR) };

    constexpr bool static_enabled_pop_l{ TestCapability(
        static_enabled_capability_flag, CapabilityEnum::PopL) };

    constexpr bool static_disabled_pop_l{ TestCapability(
        static_disabled_capability_flag, CapabilityEnum::PopL) };

    bool dynamic_enabled_pop_l{ TestCapability(dynamic_enabled_capability_flag,
                                               CapabilityEnum::PopL) };

    constexpr bool static_enabled_pop_r{ TestCapability(
        static_enabled_capability_flag, CapabilityEnum::PopR) };

    constexpr bool static_disabled_pop_r{ TestCapability(
        static_disabled_capability_flag, CapabilityEnum::PopR) };

    bool dynamic_enabled_pop_r{ TestCapability(dynamic_enabled_capability_flag,
                                               CapabilityEnum::PopR) };
    /*

    se sd de dd
    se R  L  L  L
    sd R  X  R  X
    de R  L  R  L
    dd R  X  R  X

    */

#pragma push_macro("FPushL")
#define FPushL (PushL)(dst_cntr, src_size - dst_size, empty_writer, nullptr);

#pragma push_macro("FPushR")
#define FPushR (PushR)(dst_cntr, src_size - dst_size, empty_writer, nullptr);

#pragma push_macro("FPopL")
#define FPopL (PopL)(dst_cntr, dst_size - src_size, empty_reader);

#pragma push_macro("FPopR")
#define FPopR (PopR)(dst_cntr, dst_size - src_size, empty_reader);

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

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
