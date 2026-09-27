#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

constexpr void* circular_array::ReferElem(void* data, size_t elem_stride,
                                          size_t slot_cnt, size_t rot,
                                          size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(slot_cnt <= ZETA_Core_max_capacity);

    size_t k{ rot + idx };

    return static_cast<char*>(data) +
           elem_stride * (k < slot_cnt ? k : k - slot_cnt);
}

constexpr size_t circular_array::GetLongestContPred(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx <= elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ idx <= k ? idx : idx - k };

    return ret;
}

constexpr size_t circular_array::GetLongestContSucr(size_t elem_cnt,
                                                    size_t slot_cnt, size_t rot,
                                                    size_t idx) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot == 0 || rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(idx <= elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);

    size_t k{ slot_cnt - rot };

    size_t ret{ (elem_cnt <= k || k <= idx) ? elem_cnt - idx : k - idx };

    return ret;
}

namespace circular_array::detail {

constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& self) {
    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_size);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_size <= elem_stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(slot_cnt <= ZETA_Core_max_capacity);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rot < slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr || slot_cnt == 0);
}

constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr const& self, Cursor const* cursor) {
    (CheckCntr_)(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(&self == cursor->cntr);

    void* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanRefer(cursor->idx, 1, elem_cnt));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_cnt <= cursor->idx ||
        cursor->elem ==
            (ReferElem)(data, elem_stride, slot_cnt, rot, cursor->idx));
}

}  // namespace circular_array::detail

constexpr void circular_array::Cntr::AssignFromCircularArray(
    this Cntr& dst_cntr, size_t dst_beg, Cntr const& src_cntr, size_t src_beg,
    size_t cnt) {
    detail::CheckCntr_(dst_cntr);
    detail::CheckCntr_(src_cntr);

    char* dst_data{ static_cast<char*>(dst_cntr.data) };
    size_t dst_elem_size{ dst_cntr.elem_size };
    size_t dst_elem_stride{ dst_cntr.elem_stride };
    size_t dst_elem_cnt{ dst_cntr.elem_cnt };
    size_t dst_capacity{ dst_cntr.slot_cnt };
    size_t dst_rot{ dst_cntr.rot };

    char* src_data{ static_cast<char*>(src_cntr.data) };
    size_t src_elem_size{ src_cntr.elem_size };
    size_t src_elem_stride{ src_cntr.elem_stride };
    size_t src_elem_cnt{ src_cntr.elem_cnt };
    size_t src_capacity{ src_cntr.slot_cnt };
    size_t src_rot{ src_cntr.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanDerefer(src_beg, cnt, src_elem_cnt));

    if (cnt == 0) { return; }

    size_t elem_size{ comparison_utils::BasicMin(dst_elem_size,
                                                 src_elem_size) };

    if (&dst_cntr != &src_cntr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !ZETA_Core_AreOverlapped(dst_data, dst_data + dst_capacity,
                                     src_data, src_data + src_capacity));

        goto VEC_FW_COPY;
    }

    if (dst_beg < src_beg) { goto VEC_FW_COPY; }

    if (src_beg < dst_beg) { goto VEC_BW_MOVE; }

    return;

VEC_FW_COPY:
    {
        while (0 < cnt) {
            size_t cur_cnt{ comparison_utils::BasicMin(
                cnt,
                (GetLongestContSucr)(dst_elem_cnt, dst_capacity, dst_rot,
                                     dst_beg),
                (GetLongestContSucr)(src_elem_cnt, src_capacity, src_rot,
                                     src_beg)) };

            utils::LinSeqCopy((ReferElem)(dst_data, dst_elem_stride,
                                          dst_capacity, dst_rot, dst_beg),
                              (ReferElem)(src_data, src_elem_stride,
                                          src_capacity, src_rot, src_beg),
                              elem_size, dst_elem_stride, src_elem_stride,
                              cur_cnt);

            dst_beg += cur_cnt;
            src_beg += cur_cnt;

            cnt -= cur_cnt;
        }

        return;
    }

VEC_BW_MOVE:
    {
        for (size_t dst_end{ dst_beg + cnt }, src_end{ src_beg + cnt };
             0 < cnt;) {
            size_t cur_cnt{ comparison_utils::BasicMin(
                cnt,
                (GetLongestContPred)(dst_elem_cnt, dst_capacity, dst_rot,
                                     dst_end),
                (GetLongestContPred)(src_elem_cnt, src_capacity, src_rot,
                                     src_end)) };

            dst_end -= cur_cnt;
            src_end -= cur_cnt;

            utils::LinSeqMove((ReferElem)(dst_data, dst_elem_stride,
                                          dst_capacity, dst_rot, dst_end),
                              (ReferElem)(src_data, src_elem_stride,
                                          src_capacity, src_rot, src_end),
                              elem_size, dst_elem_stride, src_elem_stride,
                              cur_cnt);

            cnt -= cur_cnt;
        }

        return;
    }
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetStaticEnabledCapabilityFlag(seq_cntr::Tag,
                                                     meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer = true,
        .Derefer = true,

        .Read = true,
        .Write = true,
        .ReadWrite = true,

        .PushL = true,
        .PushR = true,
        .Insert = true,

        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr>{}) &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetStaticDisabledCapabilityFlag(seq_cntr::Tag,
                                                      meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetDynamicEnabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
circular_array::Cntr::GetDynamicDisabledCapabilityFlag(seq_cntr::Tag) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr void* circular_array::Cntr::GetReferedInstPtr(this Cntr const& self,
                                                        seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return const_cast<void*>(static_cast<void const*>(&self));
}

constexpr meta::TypeWrapper<circular_array::Cursor>
circular_array::Cntr::GetCursorType(seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

constexpr meta::TypeWrapper<circular_array::Cursor>
circular_array::Cntr::GetCursorType(seq_cntr::Tag,
                                    meta::TypeWrapper<Cntr const>) {
    return {};
}

constexpr size_t circular_array::Cntr::GetCursorSize(seq_cntr::Tag) {
    return sizeof(Cursor);
}

constexpr size_t circular_array::Cntr::GetElemSize(this Cntr const& self,
                                                   seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_size;
}

constexpr size_t circular_array::Cntr::GetElemStride(this Cntr const& self,
                                                     seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_stride;
}

constexpr size_t circular_array::Cntr::GetIdxOffset(this Cntr const& self,
                                                    seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.rot;
}

constexpr size_t circular_array::Cntr::GetElemCnt(this Cntr const& self,
                                                  seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.elem_cnt;
}

constexpr size_t circular_array::Cntr::GetMaxElemCnt(this Cntr const& self,
                                                     seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return self.slot_cnt;
}

constexpr void circular_array::Cntr::GetLBCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->elem = nullptr;
}

constexpr void circular_array::Cntr::GetRBCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &self;
    dst_cursor->idx = self.elem_cnt;
    dst_cursor->elem = nullptr;
}

constexpr void circular_array::Cntr::PeekL(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, 0)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::PeekR(
    this auto& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot,
                                           elem_cnt - 1)
                             : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = elem_cnt - 1;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::Refer(
    this auto& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
    void* dst_elem) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_cursor != nullptr ||
                                            dst_elem != nullptr);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanRefer(idx, 1, elem_cnt));

    void* elem{ idx < elem_cnt
                    ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                    : nullptr };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
        dst_cursor->elem = elem;
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, elem_size);
    }
}

constexpr void circular_array::Cntr::Derefer(
    this auto& self, seq_cntr::Tag, Cursor const* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    void* dst_elem) {
    detail::CheckCursor_(self, pos_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_ptr_view != nullptr ||
                                            dst_elem != nullptr);

    void* elem{ pos_cursor->elem };

    if (dst_elem_ptr_view != nullptr) {
        dst_elem_ptr_view->ptr = elem;

        if constexpr (meta::IsConst<decltype(self)>) {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadOnly;
        } else {
            dst_elem_ptr_view->aliasability =
                elem == nullptr
                    ? seq_cntr::ElemPtrView::AliasabilityEnum::Null
                    : seq_cntr::ElemPtrView::AliasabilityEnum::ReadWrite;
        }
    }

    if (elem != nullptr && !lazy_copy_elem && dst_elem != nullptr) {
        utils::MemCopy(dst_elem, elem, self.elem_size);
    }
}

namespace circular_array::detail {

template <seq_endpoint::Type type, typename Endpoint>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr& self, size_t idx, size_t cnt, Endpoint&& endpoint,
     Cursor* dst_cursor) {
    static_assert(type == seq_endpoint::Type::Acceptor ||
                  type == seq_endpoint::Type::Provider ||
                  type == seq_endpoint::Type::AcceptorProvider);

    (CheckCntr_)(self);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t rot{ self.rot };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanDerefer(idx, cnt, elem_cnt));

    if constexpr (seq_endpoint::acceptor::IsEmptyAcceptor<Endpoint> ||
                  seq_endpoint::provider::IsEmptyProvider<Endpoint> ||
                  seq_endpoint::acceptor_provider::IsEmptyAcceptorProvider<
                      Endpoint>) {
        idx += cnt;
        cnt = 0;
    } else {
        while (0 < cnt) {
            size_t cur_cnt{ comparison_utils::BasicMin(
                cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

            void* elem{ (ReferElem)(data, elem_stride, slot_cnt, rot, idx) };

            if constexpr (type == seq_endpoint::Type::Acceptor) {
                seq_endpoint::acceptor::Transfer(
                    endpoint, elem, elem_size,
                    static_cast<ptrdiff_t>(elem_stride), cur_cnt);
            } else if constexpr (type == seq_endpoint::Type::Provider) {
                seq_endpoint::provider::Transfer(
                    endpoint, elem, elem_size,
                    static_cast<ptrdiff_t>(elem_stride), cur_cnt);
            } else if constexpr (type == seq_endpoint::Type::AcceptorProvider) {
                seq_endpoint::acceptor_provider::Transfer(
                    endpoint, elem, elem_size,
                    static_cast<ptrdiff_t>(elem_stride), cur_cnt);
            } else {
                ZETA_Core_Unreachable();
            }

            idx += cur_cnt;
            cnt -= cur_cnt;
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = idx;
        dst_cursor->elem =
            idx < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                           : nullptr;
    }
}

}  // namespace circular_array::detail

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::Read(this Cntr const& self, seq_cntr::Tag,
                                          Cursor const* pos_cursor, size_t cnt,
                                          Reader&& reader, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(
        const_cast<Cntr&>(self), pos_cursor->idx, cnt, reader, dst_cursor);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::Write(this Cntr& self, seq_cntr::Tag,
                                           Cursor const* pos_cursor, size_t cnt,
                                           Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, pos_cursor->idx, cnt,
                                                     writer, dst_cursor);
}

template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr void circular_array::Cntr::ReadWrite(this Cntr& self, seq_cntr::Tag,
                                               Cursor const* pos_cursor,
                                               size_t cnt,
                                               ReaderWriter&& reader_writer,
                                               Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    detail::ReadWrite_<seq_endpoint::Type::AcceptorProvider>(
        self, pos_cursor->idx, cnt, reader_writer, dst_cursor);
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::IdxRead(this Cntr const& self, size_t idx,
                                             size_t cnt, Reader&& reader) {
    detail::ReadWrite_<seq_endpoint::Type::Acceptor>(const_cast<Cntr&>(self),
                                                     idx, cnt, reader, nullptr);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::IdxWrite(this Cntr& self, size_t idx,
                                              size_t cnt, Writer&& writer) {
    detail::ReadWrite_<seq_endpoint::Type::Provider>(self, idx, cnt, writer,
                                                     nullptr);
}

template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr void circular_array::Cntr::IdxReadWrite(
    this Cntr& self, size_t idx, size_t cnt, ReaderWriter&& reader_writer) {
    detail::ReadWrite_<seq_endpoint::Type::AcceptorProvider>(
        self, idx, cnt, reader_writer, nullptr);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::PushL(this Cntr& self, seq_cntr::Tag,
                                           size_t cnt, Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPushL(cnt, elem_cnt, slot_cnt));

    self.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
    self.elem_cnt = elem_cnt += cnt;

    void* elem{ 0 < elem_cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot, 0)
                             : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = 0;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ 0 }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        seq_endpoint::provider::Transfer(
            writer, (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
            elem_size, static_cast<ptrdiff_t>(elem_stride), cur_cnt);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::PushR(this Cntr& self, seq_cntr::Tag,
                                           size_t cnt, Writer&& writer,
                                           Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPushL(cnt, elem_cnt, slot_cnt));

    self.elem_cnt = elem_cnt += cnt;

    void* elem{ 0 < cnt ? (ReferElem)(data, elem_stride, slot_cnt, rot,
                                      elem_cnt - cnt)
                        : nullptr };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &self;
        dst_cursor->idx = elem_cnt - cnt;
        dst_cursor->elem = elem;
    }

    for (size_t idx{ elem_cnt - cnt }; 0 < cnt;) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        cnt -= cur_cnt;

        seq_endpoint::provider::Transfer(
            writer, (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
            elem_size, static_cast<ptrdiff_t>(elem_stride), cur_cnt);

        idx += cur_cnt;
    }
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::Insert(this Cntr& self, seq_cntr::Tag,
                                            Cursor* pos_cursor, size_t cnt,
                                            Writer&& writer,
                                            Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    void* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &self;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanInsert(idx, cnt, elem_cnt, slot_cnt));

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx };

    self.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        self.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
        self.AssignFromCircularArray(0, self, cnt, l_size);
    } else {
        self.AssignFromCircularArray(l_size + cnt, self, l_size, r_size);
    }

    pos_cursor->elem = (ReferElem)(data, elem_stride, slot_cnt, rot, idx);

    self.Write(seq_cntr::Tag{}, pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsWriter Writer>
constexpr void circular_array::Cntr::IdxInsert(this Cntr& self, size_t idx,
                                               size_t cnt, Writer&& writer) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanInsert(idx, cnt, elem_cnt, slot_cnt));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx };

    self.elem_cnt = elem_cnt += cnt;

    unsigned long long random_seed{ utils::GetRandom() };

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        self.rot = rot = (rot < cnt ? rot + slot_cnt : rot) - cnt;
        self.AssignFromCircularArray(0, self, cnt, l_size);
    } else {
        self.AssignFromCircularArray(l_size + cnt, self, l_size, r_size);
    }

    self.IdxWrite(idx, cnt, writer);
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::PopL(this Cntr& self, seq_cntr::Tag,
                                          size_t cnt, Reader&& reader) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot + cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPopL(cnt, elem_cnt));

    self.IdxRead(0, cnt, reader);

    self.rot = rot < slot_cnt ? rot : rot - slot_cnt;
    self.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { self.rot = 0; }
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::PopR(this Cntr& self, seq_cntr::Tag,
                                          size_t cnt, Reader&& reader) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanPopR(cnt, elem_cnt));

    self.IdxRead(elem_cnt - cnt, cnt, reader);

    self.elem_cnt = elem_cnt -= cnt;

    if (elem_cnt == 0) { self.rot = 0; }
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::Erase(this Cntr& self, seq_cntr::Tag,
                                           Cursor* pos_cursor, size_t cnt,
                                           Reader&& reader) {
    detail::CheckCursor_(self, pos_cursor);

    size_t idx{ pos_cursor->idx };

    self.IdxErase(idx, cnt, reader);

    pos_cursor->elem = idx < self.elem_cnt
                           ? (ReferElem)(self.data, self.elem_stride,
                                         self.slot_cnt, self.rot, idx)
                           : nullptr;
}

template <seq_cntr::IsReader Reader>
constexpr void circular_array::Cntr::IdxErase(this Cntr& self, size_t idx,
                                              size_t cnt, Reader&& reader) {
    detail::CheckCntr_(self);

    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanErase(idx, cnt, elem_cnt));

    if (cnt == 0) { return; }

    size_t l_size{ idx };
    size_t r_size{ elem_cnt - idx - cnt };

    unsigned long long random_seed{ utils::GetRandom() };

    self.IdxRead(idx, cnt, reader);

    if (utils::Choose2(l_size <= r_size, r_size <= l_size, &random_seed) == 0) {
        self.AssignFromCircularArray(cnt, self, 0, l_size);

        rot += cnt;
        self.rot = rot = rot < slot_cnt ? rot : rot - slot_cnt;
    } else {
        self.AssignFromCircularArray(l_size, self, l_size + cnt, r_size);
    }

    self.elem_cnt = (elem_cnt -= cnt);

    if (elem_cnt == 0) { self.rot = rot = 0; }
}

constexpr void circular_array::Cntr::EraseAll(this Cntr& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    self.rot = 0;
    self.elem_cnt = 0;
}

constexpr void circular_array::Cntr::CopyCursor(this Cntr const& self,
                                                seq_cntr::Tag,
                                                void const* src_cursor_,
                                                Cursor* dst_cursor) {
    Cursor const* src_cursor{ static_cast<Cursor const*>(src_cursor_) };

    detail::CheckCursor_(self, src_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    dst_cursor->cntr = &self;
    dst_cursor->idx = src_cursor->idx;
    dst_cursor->elem = src_cursor->elem;
}

constexpr bool circular_array::Cntr::AreEqualCursor(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor const* cursor_a,
                                                    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

constexpr comparison::Ordering circular_array::Cntr::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

constexpr size_t circular_array::Cntr::GetCursorDist(this Cntr const& self,
                                                     seq_cntr::Tag,
                                                     Cursor const* cursor_a,
                                                     Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

constexpr size_t circular_array::Cntr::GetCursorIdx(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor const* cursor_) {
    Cursor const* cursor{ static_cast<Cursor const*>(cursor_) };

    detail::CheckCursor_(self, cursor);

    return cursor->idx;
}

constexpr void circular_array::Cntr::CursorStepL(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

constexpr void circular_array::Cntr::CursorStepR(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

constexpr void circular_array::Cntr::CursorAdvanceL(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor,
                                                    size_t step) {
    detail::CheckCursor_(self, cursor);

    void* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    size_t idx{ cursor->idx - step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

constexpr void circular_array::Cntr::CursorAdvanceR(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    Cursor* cursor,
                                                    size_t step) {
    detail::CheckCursor_(self, cursor);

    void* data{ self.data };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= elem_cnt - cursor->idx);

    size_t idx{ cursor->idx + step };

    cursor->idx = idx;

    cursor->elem = idx < elem_cnt
                       ? (ReferElem)(data, elem_stride, slot_cnt, rot, idx)
                       : nullptr;
}

template <seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void circular_array::Cntr::AssignFromSeqCntr(
    this Cntr& self, size_t dst_beg, SrcSeqCntr const& src_seq_cntr,
    void* src_seq_cntr_cursor, size_t cnt) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(src_seq_cntr_cursor != nullptr);

    void* data{ self.data };
    size_t elem_size{ self.elem_size };
    size_t elem_stride{ self.elem_stride };
    size_t elem_cnt{ self.elem_cnt };
    size_t slot_cnt{ self.slot_cnt };
    size_t rot{ self.rot };

    size_t idx{ dst_beg };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::check_operation::CanDerefer(dst_beg, cnt, elem_cnt));

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(
            cnt, (GetLongestContSucr)(elem_cnt, slot_cnt, rot, idx)) };

        seq_cntr::Read(
            src_seq_cntr, src_seq_cntr_cursor, cur_cnt,
            lin_seq_endpoint::acceptor::Acceptor{
                .data = (ReferElem)(data, elem_stride, slot_cnt, rot, idx),
                .elem_size = elem_size,
                .elem_stride = static_cast<ptrdiff_t>(elem_stride),
                .elem_cnt = cur_cnt,
            },
            src_seq_cntr_cursor);

        idx += cur_cnt;
        cnt -= cur_cnt;
    }
}

constexpr void circular_array::SanityCheck(
    void const* cntr_, debug_utils::sanity::SanityCheckScope) {
    Cntr const* cntr{ static_cast<Cntr const*>(cntr_) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr != nullptr);

    detail::CheckCntr_(*cntr);
}

}  // namespace zeta::core
