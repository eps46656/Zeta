#pragma once

#include <zeta/core/debug_utils/revision.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/resolving.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_endpoint.hpp>

ZETA_Core_ClangdPreambleBarrier;

#if ZETA_Core_Editor
#pragma push_macro("ZETA_Core_DebugEnable")
#define ZETA_Core_DebugEnable 1
#endif

namespace zeta::core::circular_array {

struct Cursor {
    size_t idx;

#if ZETA_Core_DebugEnable
    mutable debug_utils::revision::Stamp stamp;
#endif

    constexpr Cursor() = default;

    constexpr Cursor(Cursor const& src_cursor) = default;

    constexpr Cursor(seq_cntr::CursorLimit const& src_cursor);

    constexpr operator seq_cntr::CursorLimit(this Cursor const& self);

    constexpr Cursor& operator=(Cursor const& src_cursor) = default;

    constexpr Cursor& operator=(seq_cntr::CursorLimit const& src_cursor);

    constexpr bool operator==(Cursor const&) const = default;
    constexpr bool operator!=(Cursor const&) const = default;
};

template <meta::IsObjectType Elem>
struct Cntr;

namespace detail {

template <typename T>
struct IsCntrImpl_ {
    static constexpr bool value{ false };
};

template <meta::IsObjectType Elem>
struct IsCntrImpl_<Cntr<Elem>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsCntr = detail::IsCntrImpl_<T>::value;

template <meta::IsObjectType Elem>
constexpr Elem* ReferElem(Elem* data, size_t elem_stride, size_t elem_capacity,
                          size_t slot_offset, size_t idx);

constexpr size_t GetLongestContSucr(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

constexpr size_t GetLongestContPred(size_t elem_cnt, size_t elem_capacity,
                                    size_t slot_offset, size_t idx);

template <typename DstCntr, typename SrcCntr>
constexpr void SegTransfer(DstCntr&& dst_cntr, SrcCntr&& src_cntr,
                           size_t dst_beg, size_t src_beg, size_t cnt)
    requires requires {
        requires IsCntr<meta::RemoveConstVolatileReference<DstCntr>>;
        requires IsCntr<meta::RemoveConstVolatileReference<SrcCntr>>;
    };

template <meta::IsObjectType Elem_>
struct Cntr {
    using Elem = Elem_;

    Elem* data;
    size_t elem_stride;
    size_t elem_cnt;
    size_t slot_cnt;
    size_t rot;

#if ZETA_Core_DebugEnable
    debug_utils::revision::Stamp stamp;
#endif

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemStride(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetIdxOffset(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(
        this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void PeekR(
        this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void Refer(
        this auto&& self, seq_cntr::Tag, size_t idx, Cursor* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void Derefer(
        this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this auto&& self, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
                         Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this meta::IsNonConstWithoutReference auto&& self,
                             seq_cntr::Tag, Cursor const* pos_cursor,
                             size_t cnt, Acceptor&& acceptor,
                             Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxRead(this auto&& self, size_t idx, size_t cnt,
                           Acceptor&& acceptor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void IdxWrite(this meta::IsNonConstWithoutReference auto&& self,
                            size_t idx, size_t cnt, Provider&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxReadWrite(
        this meta::IsNonConstWithoutReference auto&& self, size_t idx,
        size_t cnt, Acceptor&& acceptor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this meta::IsNonConstWithoutReference auto&& self,
                          seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                          Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void IdxInsert(this meta::IsNonConstWithoutReference auto&& self,
                             size_t idx, size_t cnt, Provider&& provider);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this meta::IsNonConstWithoutReference auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this meta::IsNonConstWithoutReference auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void IdxErase(this meta::IsNonConstWithoutReference auto&& self,
                            size_t idx, size_t cnt, Acceptor&& acceptor,
                            Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this meta::IsNonConstWithoutReference auto&& self,
                            seq_cntr::Tag, Acceptor&& acceptor);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);
};

}  // namespace zeta::core::circular_array

#if ZETA_Core_Editor
#pragma pop_macro("ZETA_Core_DebugEnable")
#endif
