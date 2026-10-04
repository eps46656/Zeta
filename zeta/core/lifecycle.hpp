#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/tuple.ipp>

#define ZETA_Core_Lifecycle_PackConstructArgs(...) \
    ::zeta::core::lifecycle::detail::PackConstructArgs(__VA_ARGS__)

#define ZETA_Core_Lifecycle_UnpackConstructArg(Target, Arg, arg)            \
    ::zeta::core::lifecycle::detail::UnpackConstructArg<Target, 1>::Unpack( \
        ::zeta::core::meta::Forward<Arg>(arg))

#define ZETA_Core_Lifecycle_UnpackConstructArgs(Target, Args, args) \
    ::zeta::core::lifecycle::detail::                               \
        UnpackConstructArg<Target, sizeof...(Args)>::Unpack(        \
            ::zeta::core::meta::Forward<Args>(args))...

namespace zeta::core::lifecycle {

struct DirectConstructTag {};

namespace detail {

template <typename... Args>
struct ConstructArgsTuple : public tuple::Tuple<Args...> {};

template <typename Target, typename... Args>
struct TargetedConstructArgsTuple : public tuple::Tuple<Args...> {
    using TargetType = Target;

    template <size_t... ArgIdxes>
    constexpr Target Call(static_seq::StaticSeq<size_t, ArgIdxes...>) const {
        return { this->template ForwardAccess<ArgIdxes>()... };
    }

    constexpr operator Target() const {
        return this->Call(
            static_seq::MakeLinearStaticIntegralSeq<size_t, 0, 1,
                                                    sizeof...(Args)>{});
    }
};

template <typename... Args>
constexpr decltype(auto) PackConstructArgsFromConstructArgTuple(
    ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
    return construct_args_tuple;
}

template <typename... Args>
constexpr decltype(auto) PackConstructArgs(Args&&... args)
    requires(!requires {
        (PackConstructArgsFromConstructArgTuple)(meta::Forward<Args>(args)...);
    })
{
    return ConstructArgsTuple<Args&&...>{ { meta::Forward<Args>(args)... } };
}

template <typename... Args>
constexpr decltype(auto) PackConstructArgs(
    ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
    return (PackConstructArgsFromConstructArgTuple)(construct_args_tuple);
}

template <typename Target, size_t UpperArgsCnt>
struct UnpackConstructArg {
    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromConstructArgsTuple(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple,
        static_seq::StaticSeq<size_t, ArgIdxes...>) {
        if constexpr (UpperArgsCnt == 1) {
            return TargetedConstructArgsTuple<Target, Args&&...>{
                { construct_args_tuple.template ForwardAccess<ArgIdxes>()... }
            };
        } else {
            return meta::Forward<ConstructArgsTuple<Args&&...>>(
                construct_args_tuple);
        }
    }

    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromConstructArgsTuple(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
        return (UnpackFromConstructArgsTuple)(construct_args_tuple,
                                              static_seq::
                                                  MakeLinearStaticIntegralSeq<
                                                      size_t, 0, 1,
                                                      sizeof...(Args)>{});
    }

    template <typename Arg>
    static constexpr decltype(auto) Unpack(Arg&& arg)
        requires(!requires {
            (UnpackFromConstructArgsTuple)(meta::Forward<Arg>(arg));
        })
    {
        return meta::Forward<Arg>(arg);
    }

    template <typename... Args>
    static constexpr decltype(auto) Unpack(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
        return (UnpackFromConstructArgsTuple)(construct_args_tuple);
    }
};

}  // namespace detail

struct PreConstructorHook {
    constexpr PreConstructorHook() = default;

    template <typename Callable>
    constexpr PreConstructorHook(Callable&& callable);
};

template <typename Obj, typename... Args>
constexpr void InvokeConstructor(Obj* obj, Args&&... args);

template <typename Obj, typename... Args>
constexpr void InvokeLinSeqConstructor(Obj* data, size_t elem_stride,
                                       size_t elem_cnt, Args&&... args);

template <typename Obj>
constexpr void InvokeDestructor(Obj& obj);

template <typename Obj>
constexpr void InvokeLinSeqDestructor(Obj* data, size_t elem_stride,
                                      size_t elem_cnt);

enum struct DataLifeState : unsigned char {
    Mem = 0,
    Obj = 1,
};

enum struct DataTransferSemantics : unsigned char {
    Copy = 0,
    Move = 1,
    Reloc = 2,
};

enum struct DataView : unsigned char {
    Null = 0,
    ReadWriteMem = 1,
    ReadOnlyObj = 2,
    ReadWriteObj = 3,
};

constexpr bool IsMemView(DataView data_view);

constexpr bool IsObjView(DataView data_view);

constexpr bool CanRead(DataView data_view);

constexpr bool CanWrite(DataView data_view);

namespace detail {

namespace data_transfer_op_ {

using Underlying = unsigned char;

constexpr Underlying construct_flag{ 0b0 };
constexpr Underlying assign_flag{ 0b1 };

constexpr Underlying copy_flag{ 0b000 };
constexpr Underlying move_flag{ 0b010 };
constexpr Underlying reloc_flag{ 0b100 };
constexpr Underlying forward_flag{ 0b110 };

}  // namespace data_transfer_op_

}  // namespace detail

enum struct DataTransferOp : unsigned char {
    CopyConstruct = detail::data_transfer_op_::construct_flag |
        detail::data_transfer_op_::copy_flag,
    MoveConstruct = detail::data_transfer_op_::construct_flag |
        detail::data_transfer_op_::move_flag,
    RelocConstruct = detail::data_transfer_op_::construct_flag |
        detail::data_transfer_op_::reloc_flag,
    ForwardConstruct = detail::data_transfer_op_::construct_flag |
        detail::data_transfer_op_::forward_flag,

    CopyAssign = detail::data_transfer_op_::assign_flag |
        detail::data_transfer_op_::copy_flag,
    MoveAssign = detail::data_transfer_op_::assign_flag |
        detail::data_transfer_op_::move_flag,
    RelocAssign = detail::data_transfer_op_::assign_flag |
        detail::data_transfer_op_::reloc_flag,
    ForwardAssign = detail::data_transfer_op_::assign_flag |
        detail::data_transfer_op_::forward_flag,
};

template <typename T>
concept IsDataTransferOpLike =
    meta::IsSame<T, DataTransferOp> || meta::IsValueWrapperT<T, DataTransferOp>;

constexpr DataTransferOp DeriveDataTransferOp(
    DataLifeState dst_life_state, DataTransferSemantics src_transfer_semantics);

template <typename Dst, typename... Srcs>
constexpr void DataTransfer(DataTransferOp data_transfer_op, Dst* dst,
                            Srcs&&... srcs);

template <DataTransferOp transfer_op, typename Dst, typename... Srcs>
constexpr void DataTransfer(meta::AutoValueWrapper<transfer_op>, Dst* dst,
                            Srcs&&... srcs);

}  // namespace zeta::core::lifecycle
