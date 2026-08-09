#pragma once

#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

constexpr bool assoc_cntr::capability::CheckFlags(
    Flag static_enabled_capability_flag, Flag static_disabled_capability_flag) {
    Flag capability_flags[]{ static_enabled_capability_flag,
                             static_disabled_capability_flag };

    for (Flag capability_flag : capability_flags) {
        if ((capability_flag & empty_capability_flag) !=
            empty_capability_flag) {
            return false;
        }

        if ((capability_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    if ((static_enabled_capability_flag & static_disabled_capability_flag) !=
        empty_capability_flag) {
        return false;
    }

    return true;
}

constexpr bool assoc_cntr::capability::CheckFlags(
    Flag static_enabled_capability_flag, Flag static_disabled_capability_flag,
    Flag dynamic_enabled_capability_flag,
    Flag dynamic_disabled_capability_flag) {
    Flag capability_flags[]{ static_enabled_capability_flag,
                             static_disabled_capability_flag,
                             dynamic_enabled_capability_flag,
                             dynamic_disabled_capability_flag };

    for (Flag capability_flag : capability_flags) {
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

    if ((static_enabled_capability_flag | static_disabled_capability_flag |
         dynamic_enabled_capability_flag | dynamic_disabled_capability_flag) !=
        full_capability_flag) {
        return false;
    }

    return true;
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetReferedInstPtr(
    Cntr& cntr) {
    return cntr.GetReferedInstPtr();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) assoc_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetStaticEnabledCapabilityFlag() {
    return Cntr::GetStaticEnabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) assoc_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetStaticDisabledCapabilityFlag() {
    return Cntr::GetStaticDisabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) assoc_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetDynamicEnabledCapabilityFlag(Cntr& cntr) {
    return cntr.GetDynamicEnabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) assoc_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetDynamicDisabledCapabilityFlag(Cntr& cntr) {
    return cntr.GetDynamicDisabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorSize(
    Cntr& cntr) {
    return cntr.GetCursorSize();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetElemSize(Cntr& cntr) {
    return cntr.GetElemSize();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetElemCnt(Cntr& cntr) {
    return cntr.GetElemCnt();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetMaxElemCnt(
    Cntr& cntr) {
    return cntr.GetMaxElemCnt();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetLBCursor(
    Cntr& cntr, void* dst_cursor) {
    return cntr.GetLBCursor(static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetRBCursor(
    Cntr& cntr, void* dst_cursor) {
    return cntr.GetRBCursor(static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PeekL(
    Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    void* dst_cursor, void* dst_elem) {
    return cntr.PeekL(lazy_copy_elem, dst_elem_ptr_view,
                      static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PeekR(
    Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    void* dst_cursor, void* dst_elem) {
    return cntr.PeekR(lazy_copy_elem, dst_elem_ptr_view,
                      static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Derefer(
    Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
    ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    return cntr.Derefer(static_cast<Cursor*>(pos_cursor), lazy_copy_elem,
                        dst_elem_ptr_view, dst_elem);
}

template <typename Cntr, typename Cursor>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Find(
    Cntr& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
    ElemPtrView* dst_elem_ptr_view, void* dst_cursor, void* dst_elem) {
    return cntr.Find(key, key_hasher, key_elem_cmptr, lazy_copy_elem,
                     dst_elem_ptr_view, static_cast<Cursor*>(dst_cursor),
                     dst_elem);
}

template <typename Cntr, typename Cursor>
template <assoc_cntr::IsReader Reader>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Read(Cntr& cntr,
                                                            void* pos_cursor,
                                                            size_t cnt,
                                                            Reader&& reader,
                                                            void* dst_cursor) {
    return cntr.Read(static_cast<Cursor*>(pos_cursor), cnt, reader,
                     static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Insert(
    Cntr& cntr, void const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Writer&& writer,
    void* dst_cursor) {
    return cntr.Insert(key, key_hasher, key_elem_cmptr, writer,
                       static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <assoc_cntr::IsReader Reader>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PopL(Cntr& cntr,
                                                            size_t cnt,
                                                            Reader&& reader) {
    return cntr.PopL(cnt, reader);
}

template <typename Cntr, typename Cursor>
template <assoc_cntr::IsReader Reader>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PopR(Cntr& cntr,
                                                            size_t cnt,
                                                            Reader&& reader) {
    return cntr.PopR(cnt, reader);
}

template <typename Cntr, typename Cursor>
template <assoc_cntr::IsReader Reader>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Erase(Cntr& cntr,
                                                             void* pos_cursor,
                                                             size_t cnt,
                                                             Reader&& reader) {
    return cntr.Erase(static_cast<Cursor*>(pos_cursor), cnt, reader);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::EraseAll(Cntr& cntr) {
    return cntr.EraseAll();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CopyCursor(
    Cntr& cntr, void* src_cursor, void* dst_cursor) {
    return cntr.CopyCursor(static_cast<Cursor*>(src_cursor),
                           static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::AreEqualCursor(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.AreEqualCursor(static_cast<Cursor*>(cursor_a),
                               static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CompareCursor(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.CompareCursor(static_cast<Cursor*>(cursor_a),
                              static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorDist(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.GetCursorDist(static_cast<Cursor*>(cursor_a),
                              static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorIdx(
    Cntr& cntr, void* cursor) {
    return cntr.GetCursorIdx(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorStepL(
    Cntr& cntr, void* cursor) {
    return cntr.CursorStepL(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorStepR(
    Cntr& cntr, void* cursor) {
    return cntr.CursorStepR(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorAdvanceL(
    Cntr& cntr, void* cursor, size_t step) {
    return cntr.CursorAdvanceL(static_cast<Cursor*>(cursor), step);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
assoc_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorAdvanceR(
    Cntr& cntr, void* cursor, size_t step) {
    return cntr.CursorAdvanceR(static_cast<Cursor*>(cursor), step);
}

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)           \
    (((cap_flag) & (static_cast<capability::Flag>(1) \
                    << meta::ToUnderlying(capability::Kind::cap_name))) != 0)

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method_name, ...)                              \
    if constexpr (!TestCapability((GetStaticEnabledCapabilityFlag<Cntr>)(), \
                                  cap_name)) {                              \
        ZETA_Core_StaticAssert(!TestCapability(                             \
            (GetStaticDisabledCapabilityFlag<Cntr>)(), cap_name));          \
                                                                            \
        ZETA_Core_DebugAssert(TestCapability(                               \
            (GetDynamicEnabledCapabilityFlag)(cntr), cap_name));            \
    }                                                                       \
                                                                            \
    return CntrTraits<Cntr>::method_name(__VA_ARGS__);                      \
                                                                            \
    ZETA_Core_StaticAssert(true)

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetReferedInstPtr(Cntr& cntr) {
    return CntrTraits<Cntr>::GetReferedInstPtr(cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetStaticEnabledCapabilityFlag() {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_enabled_capability_flag;
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetStaticDisabledCapabilityFlag() {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_disabled_capability_flag;
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetDynamicEnabledCapabilityFlag(
    Cntr& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    capability::Flag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    capability::Flag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    capability::CheckFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    capability::Flag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    capability::Flag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    capability::CheckFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetCursorSize(Cntr& cntr) {
    CallMethod(GetCursorSize, GetCursorSize, cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetElemSize(Cntr& cntr) {
    CallMethod(GetElemSize, GetElemSize, cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetElemCnt(Cntr& cntr) {
    CallMethod(GetElemCnt, GetElemCnt, cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetMaxElemCnt(Cntr& cntr) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt, cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetLBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, cntr, dst_cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetRBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, cntr, dst_cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::PeekL(Cntr& cntr, bool lazy_copy_elem,
                                           ElemPtrView* dst_elem_ptr_view,
                                           void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, cntr, lazy_copy_elem, dst_cursor,
               dst_elem_ptr_view, dst_elem);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::PeekR(Cntr& cntr, bool lazy_copy_elem,
                                           ElemPtrView* dst_elem_ptr_view,
                                           void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, cntr, lazy_copy_elem, dst_cursor,
               dst_elem_ptr_view, dst_elem);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::Derefer(Cntr& cntr, void* pos_cursor,
                                             bool lazy_copy_elem,
                                             ElemPtrView* dst_elem_ptr_view,
                                             void* dst_elem) {
    CallMethod(Derefer, Derefer, cntr, pos_cursor, lazy_copy_elem,
               dst_elem_ptr_view, dst_elem);
}

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr decltype(auto) assoc_cntr::Find(Cntr& cntr, void const* key,
                                          KeyHasher key_hasher,
                                          KeyElemComparator key_elem_cmptr,
                                          bool lazy_copy_elem,
                                          ElemPtrView* dst_elem_ptr_view,
                                          void* dst_cursor, void* dst_elem) {
    CallMethod(Find, Find, cntr, key, key_hasher, key_elem_cmptr,
               lazy_copy_elem, dst_elem_ptr_view, dst_cursor, dst_elem);
}

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          assoc_cntr::IsWriter Writer>
constexpr decltype(auto) assoc_cntr::Insert(Cntr& cntr, void const* key,
                                            KeyHasher key_hasher,
                                            KeyElemComparator key_elem_cmptr,
                                            Writer&& writer, void* dst_cursor) {
    CallMethod(Insert, Insert, cntr, key, key_hasher, key_elem_cmptr, writer,
               dst_cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::PopL(Cntr& cntr, size_t cnt) {
    CallMethod(PopL, PopL, cntr, cnt);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::PopR(Cntr& cntr, size_t cnt) {
    CallMethod(PopR, PopR, cntr, cnt);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::Erase(Cntr& cntr, void* pos_cursor) {
    CallMethod(Erase, Erase, cntr, pos_cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::EraseAll(Cntr& cntr) {
    CallMethod(EraseAll, EraseAll, cntr);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CopyCursor(Cntr& cntr, void* src_cursor,
                                                void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, cntr, src_cursor, dst_cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::AreEqualCursor(Cntr& cntr,
                                                    void const* cursor_a,
                                                    void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CompareCursor(Cntr& cntr,
                                                   void const* cursor_a,
                                                   void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetCursorDist(Cntr& cntr,
                                                   void const* cursor_a,
                                                   void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cntr, cursor_a, cursor_b);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::GetCursorIdx(Cntr& cntr,
                                                  void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cntr, cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CursorStepL(Cntr& cntr, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cntr, cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CursorStepR(Cntr& cntr, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cntr, cursor);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CursorAdvanceL(Cntr& cntr, void* cursor,
                                                    size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cntr, cursor, step);
}

template <typename Cntr>
constexpr decltype(auto) assoc_cntr::CursorAdvanceR(Cntr& cntr, void* cursor,
                                                    size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cntr, cursor, step);
}

#pragma pop_macro("CallMethod")

template <typename Cntr>
constexpr assoc_cntr::VTable assoc_cntr::BuildVTableBasic() {
    constexpr capability::Flag static_disabled_capability_flag{
        GetStaticDisabledCapabilityFlag<Cntr>()
    };

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
                             (GetLBCursor)(*static_cast<Cntr*>(cntr),
                                           dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             (GetRBCursor)(*static_cast<Cntr*>(cntr),
                                           dst_cursor);
                         }),

        .PeekL = F(PeekL,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekL)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                                      dst_cursor, dst_elem);
                   }),

        .PeekR = F(PeekR,
                   [](void* cntr, bool lazy_copy_elem, void* dst_cursor,
                      void* dst_elem) {
                       return (PeekR)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                                      dst_cursor, dst_elem);
                   }),

        .Derefer = F(Derefer,
                     [](void* cntr, void* pos_cursor, bool lazy_copy_elem,
                        void* dst_elem) {
                         return (Derefer)(*static_cast<Cntr*>(cntr), pos_cursor,
                                          lazy_copy_elem, dst_elem);
                     }),

        .Find_ConstVoidPtr =
            F(Find,
              [](void* cntr, void const* key, bool lazy_copy_elem,
                 void* dst_cursor, void* dst_elem) {
                  return (Find)(*static_cast<Cntr*>(cntr), key, lazy_copy_elem,
                                dst_cursor, dst_elem);
              }),

        .FnInsert = F(Insert,
                      [](void* cntr, void const* elem, void* dst_cursor) {
                          return (Insert)(*static_cast<Cntr*>(cntr), elem,
                                          dst_cursor);
                      }),

        .PopL =
            F(PopL, [](void* cntr,
                       size_t cnt) { (PopL)(*static_cast<Cntr*>(cntr), cnt); }),

        .PopR =
            F(PopR, [](void* cntr,
                       size_t cnt) { (PopR)(*static_cast<Cntr*>(cntr), cnt); }),

        .Erase = F(Erase,
                   [](void* cntr, void* pos_cursor) {
                       (Erase)(*static_cast<Cntr*>(cntr), pos_cursor);
                   }),

        .EraseAll =
            F(EraseAll,
              [](void* cntr) { (EraseAll)(*static_cast<Cntr*>(cntr)); }),

        .CopyCursor = F(CopyCursor,
                        [](void* cntr, void* src_cursor, void* dst_cursor) {
                            (CopyCursor)(*static_cast<Cntr*>(cntr), src_cursor,
                                         dst_cursor);
                        }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (AreEqualCursor)(*static_cast<Cntr*>(cntr), cursor_a,
                                          cursor_b);
              }),

        .CompareCursor =
            F(CompareCursor,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (CompareCursor)(*static_cast<Cntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorDist =
            F(GetCursorDist,
              [](void* cntr, void const* cursor_a, void const* cursor_b) {
                  return (GetCursorDist)(*static_cast<Cntr*>(cntr), cursor_a,
                                         cursor_b);
              }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void const* cursor) {
                              return (GetCursorIdx)(*static_cast<Cntr*>(cntr),
                                                    cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             (CursorStepL)(*static_cast<Cntr*>(cntr), cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             (CursorStepR)(*static_cast<Cntr*>(cntr), cursor);
                         }),

        .CursorAdvanceL = F(CursorAdvanceL,
                            [](void* cntr, void* cursor, size_t step) {
                                (CursorAdvanceL)(*static_cast<Cntr*>(cntr),
                                                 cursor, step);
                            }),

        .CursorAdvanceR = F(CursorAdvanceR,
                            [](void* cntr, void* cursor, size_t step) {
                                (CursorAdvanceR)(*static_cast<Cntr*>(cntr),
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

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
