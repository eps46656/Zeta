#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lin_seq_utils.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <typename Callable>
constexpr lifecycle::PreConstructorHook::PreConstructorHook(
    Callable&& callable) {
    meta::Forward<Callable>(callable)();
}

template <typename Obj, typename... Args>
constexpr void lifecycle::InvokeConstructor(Obj* obj, Args&&... args) {
    new (obj) Obj{ meta::Forward<Args>(args)... };
}

template <typename Obj, typename... Args>
constexpr void lifecycle::InvokeLinSeqConstructor(Obj* data, size_t elem_stride,
                                                  size_t elem_cnt,
                                                  Args&&... args) {
    lin_seq_utils::Check(data, elem_stride, elem_cnt);

    if (elem_cnt == 0) { return; }

    for (size_t i{ elem_stride == 0 ? 1 : elem_cnt }; 0 < i--;
         data = utils::PtrInc(data, elem_stride)) {
        (InvokeConstructor)(data, meta::Forward<Args>(args)...);
    }
}

template <typename Obj>
constexpr void lifecycle::InvokeDestructor(Obj& obj) {
    obj.~Obj();
}

template <typename Obj>
constexpr void lifecycle::InvokeLinSeqDestructor(Obj* data, size_t elem_stride,
                                                 size_t elem_cnt) {
    lin_seq_utils::Check(data, elem_stride, elem_cnt);

    if (elem_cnt == 0) { return; }

    for (size_t i{ elem_stride == 0 ? 1 : elem_cnt }; 0 < i--;
         data = utils::PtrInc(data, elem_stride)) {
        (InvokeDestructor)(*data);
    }
}

constexpr bool lifecycle::IsMemView(DataView view) {
    switch (view) {
    case DataView::Null: return false;
    case DataView::ReadWriteMem: return true;
    case DataView::ReadOnlyObj: return false;
    case DataView::ReadWriteObj: return false;
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr bool lifecycle::IsObjView(DataView view) {
    switch (view) {
    case DataView::Null: return false;
    case DataView::ReadWriteMem: return false;
    case DataView::ReadOnlyObj: return true;
    case DataView::ReadWriteObj: return true;
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr bool lifecycle::CanRead(DataView view) {
    switch (view) {
    case DataView::Null: return false;
    case DataView::ReadWriteMem: return true;
    case DataView::ReadOnlyObj: return true;
    case DataView::ReadWriteObj: return true;
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr bool lifecycle::CanWrite(DataView view) {
    switch (view) {
    case DataView::Null: return false;
    case DataView::ReadWriteMem: return false;
    case DataView::ReadOnlyObj: return false;
    case DataView::ReadWriteObj: return true;
    }

    ZETA_Core_DebugUtils_Diag_Unreachable();
}

constexpr lifecycle::DataTransferOp lifecycle::DeriveDataTransferOp(
    DataLifeState dst_life_state,
    DataTransferSemantics src_transfer_semantics) {
    detail::data_transfer_op_::Underlying ret_life_state{ 0 };

    switch (dst_life_state) {
    case DataLifeState::Mem:
        ret_life_state |= detail::data_transfer_op_::construct_flag;
        break;
    case DataLifeState::Obj:
        ret_life_state |= detail::data_transfer_op_::assign_flag;
        break;
    }

    detail::data_transfer_op_::Underlying ret_transfer_semantics{ 0 };

    switch (src_transfer_semantics) {
    case DataTransferSemantics::Copy:
        ret_transfer_semantics |= detail::data_transfer_op_::copy_flag;
        break;
    case DataTransferSemantics::Move:
        ret_transfer_semantics |= detail::data_transfer_op_::move_flag;
        break;
    case DataTransferSemantics::Reloc:
        ret_transfer_semantics |= detail::data_transfer_op_::reloc_flag;
        break;
    }

    return static_cast<DataTransferOp>(ret_life_state | ret_transfer_semantics);
}

namespace lifecycle::detail {

template <typename Arg0, typename... Args>
constexpr decltype(auto) GetFirstArg_(Arg0&& arg0, Args&&...) {
    return meta::Forward<Arg0>(arg0);
}

}  // namespace lifecycle::detail

template <typename Dst, typename... Srcs>
constexpr void lifecycle::DataTransfer(DataTransferOp transfer_op, Dst* dst,
                                       Srcs&&... srcs) {
    static_assert(!meta::IsConst<Dst>);

    switch (transfer_op) {
    case DataTransferOp::CopyConstruct: new (dst) Dst{ srcs... }; break;

    case DataTransferOp::MoveConstruct:
        new (dst) Dst{ meta::Move(srcs)... };
        break;

    case DataTransferOp::ForwardConstruct:
        new (dst) Dst{ meta::Forward<Srcs>(srcs)... };
        break;

    case DataTransferOp::RelocConstruct:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof...(Srcs) == 1);
        new (dst) Dst{ detail::GetFirstArg_(meta::Move(srcs)...) };
        (InvokeDestructor)(detail::GetFirstArg_(srcs...));
        break;

    case DataTransferOp::CopyAssign:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(srcs...);
        break;

    case DataTransferOp::MoveAssign:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Move(srcs)...);
        break;

    case DataTransferOp::ForwardAssign:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Forward<Srcs>(srcs)...);
        break;

    case DataTransferOp::RelocAssign:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Move(srcs)...);
        (InvokeDestructor)(detail::GetFirstArg_(srcs...));
        break;
    }
}

template <lifecycle::DataTransferOp data_transfer_op, typename Dst,
          typename... Srcs>
constexpr void lifecycle::DataTransfer(meta::AutoValueWrapper<data_transfer_op>,
                                       Dst* dst, Srcs&&... srcs) {
    static_assert(!meta::IsConst<Dst>);

    if constexpr (data_transfer_op == DataTransferOp::CopyConstruct) {
        new (dst) Dst{ srcs... };
    } else if constexpr (data_transfer_op == DataTransferOp::MoveConstruct) {
        new (dst) Dst{ meta::Move(srcs)... };
    } else if constexpr (data_transfer_op == DataTransferOp::ForwardConstruct) {
        new (dst) Dst{ meta::Forward<Srcs>(srcs)... };
    } else if constexpr (data_transfer_op == DataTransferOp::RelocConstruct) {
        new (dst) Dst{ detail::GetFirstArg_(meta::Move(srcs)...) };
        (InvokeDestructor)(detail::GetFirstArg_(srcs...));
    } else if constexpr (data_transfer_op == DataTransferOp::CopyAssign) {
        static_assert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(srcs...);
    } else if constexpr (data_transfer_op == DataTransferOp::MoveAssign) {
        static_assert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Move(srcs)...);
    } else if constexpr (data_transfer_op == DataTransferOp::ForwardAssign) {
        static_assert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Forward<Srcs>(srcs)...);
    } else if constexpr (data_transfer_op == DataTransferOp::RelocAssign) {
        static_assert(sizeof...(Srcs) == 1);
        *dst = detail::GetFirstArg_(meta::Move(srcs)...);
        (InvokeDestructor)(detail::GetFirstArg_(srcs...));
    } else {
        static_assert(false);
    }
}

}  // namespace zeta::core
