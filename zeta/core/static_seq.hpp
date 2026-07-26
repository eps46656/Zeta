#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::static_seq {

template <typename Type_, Type_... Vals>
struct StaticSeq {
    using Type = Type_;
    static constexpr size_t Size{ sizeof...(Vals) };
};

namespace detail {

template <typename T, typename... TargetTypes>
struct IsStaticSeqOfType_ {
    static constexpr bool value{ false };
};

template <typename Type, Type... Vals, typename... TargetTypes>
struct IsStaticSeqOfType_<StaticSeq<Type, Vals...>, TargetTypes...> {
    static constexpr bool value{ meta::IsSame<Type, TargetTypes...> };
};

}  // namespace detail

template <typename StaticSeqPossibly, typename... TargetTypes>
constexpr bool IsStaticSeqOfAnyType{
    detail::IsStaticSeqOfType_<StaticSeqPossibly, TargetTypes...>::value
};

template <typename StaticSeqPossibly>
constexpr bool IsStaticSeq{
    IsStaticSeqOfAnyType<StaticSeqPossibly, meta::AlwaysMatchedTag>
};

namespace detail {

template <typename TA, typename TB>
struct Concat_;

template <typename Type_, Type_... ValsA, Type_... ValsB>
struct Concat_<StaticSeq<Type_, ValsA...>, StaticSeq<Type_, ValsB...>> {
    using Type = StaticSeq<Type_, ValsA..., ValsB...>;
};

}  // namespace detail

template <typename TA, typename TB>
using Concat = typename detail::Concat_<TA, TB>::Type;

namespace detail {

template <typename Integeral, Integeral Beg, Integeral Stride, size_t Cnt>
struct MakeLinearStaticIntegralSeq_ {
    using Type =
        Concat<typename MakeLinearStaticIntegralSeq_<Integeral, Beg, Stride,
                                                     Cnt / 2U>::Type,
               typename MakeLinearStaticIntegralSeq_<
                   Integeral, Beg + Stride*(Cnt / 2U), Stride,
                   Cnt - (Cnt / 2U)>::Type>;
};

template <typename Integeral, Integeral Beg, Integeral Stride>
struct MakeLinearStaticIntegralSeq_<Integeral, Beg, Stride, 0> {
    using Type = StaticSeq<Integeral>;
};

template <typename Integeral, Integeral Beg, Integeral Stride>
struct MakeLinearStaticIntegralSeq_<Integeral, Beg, Stride, 1> {
    using Type = StaticSeq<Integeral, Beg>;
};

}  // namespace detail

template <typename Integeral, Integeral Beg, Integeral Stride, size_t Cnt>
using MakeLinearStaticIntegralSeq =
    detail::MakeLinearStaticIntegralSeq_<Integeral, Beg, Stride, Cnt>::Type;

}  // namespace zeta::core::static_seq
