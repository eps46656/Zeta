#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/meta.hpp>

namespace zeta::core::tuple {

namespace detail {

template <typename T>
struct Wrapper_ {
    T value;
};
/*
We need to evaluate the size of T as a struct data member. For reference types,
sizeof(T) reflects alias semantics (i.e. sizeof(T&) == sizeof(T)), which does
not match their actual representation in an object. Therefore, we wrap T in
Wrapper_ and use sizeof(Wrapper_<T>) instead.
*/

template <size_t N>
struct LayoutResult_ {
    size_t align;
    size_t datasize;
    size_t size;

    size_t elem_aligns[N];
    size_t elem_sizes[N];
    size_t elem_offsets[N];
    size_t elem_paddings[N];
};

template <size_t N, typename... Elems>
struct CalcLayout_F_ {
    static constexpr LayoutResult_<sizeof...(Elems)> Call() {
        LayoutResult_<sizeof...(Elems)> result{
            CalcLayout_F_<N - 1, Elems...>::Call()
        };

        using WrappedElem = Wrapper_<meta::GetNthType<N - 1, Elems...>>;

        size_t cur_align{ alignof(WrappedElem) };
        size_t cur_size{ sizeof(WrappedElem) };

        size_t cur_offset{ integral_math::AlignUp(result.datasize, cur_align) };

        size_t prv_padding{ cur_offset - result.datasize };

        result.align = integral_math::LCM(result.align, cur_align);
        result.datasize = cur_offset + cur_size;
        result.elem_aligns[N - 1] = cur_align;
        result.elem_sizes[N - 1] = cur_size;
        result.elem_offsets[N - 1] = cur_offset;
        result.elem_paddings[N - 2] = prv_padding;

        if constexpr (N == sizeof...(Elems)) {
            result.size = integral_math::AlignUp(result.datasize, result.align);
            result.elem_paddings[N - 1] = result.size - result.datasize;
        }

        return result;
    }
};

template <typename... Elems>
struct CalcLayout_F_<1, Elems...> {
    static constexpr LayoutResult_<sizeof...(Elems)> Call() {
        using WrappedElem = Wrapper_<meta::GetNthType<0, Elems...>>;

        return {
            .align = alignof(WrappedElem),
            .datasize = sizeof(WrappedElem),
            .size = sizeof(WrappedElem),
            .elem_aligns{ alignof(WrappedElem) },
            .elem_sizes{ sizeof(WrappedElem) },
            .elem_offsets{ 0 },
            .elem_paddings{ 0 },
        };
    }
};

template <typename... Elems>
constexpr LayoutResult_<sizeof...(Elems)> CalcLayout_() {
    constexpr LayoutResult_<sizeof...(Elems)> ret{
        CalcLayout_F_<sizeof...(Elems), Elems...>::Call()
    };

    return ret;
}

namespace tag_ {

namespace value_list {
struct Insuff {};
struct Exact {};
}  // namespace value_list

namespace l_src_tuple {
struct Excess {};
struct Exact {};
struct Insuff {};
}  // namespace l_src_tuple

namespace r_src_tuple {
struct Excess {};
struct Exact {};
struct Insuff {};
}  // namespace r_src_tuple

}  // namespace tag_

template <typename Elem, size_t PaddingSize>
struct Leaf_ {
    Wrapper_<Elem> elem;
    unsigned char padding[PaddingSize];

    constexpr Leaf_() = default;

    template <typename Arg>
    constexpr Leaf_(Arg&& arg);
};

template <typename Elem>
struct Leaf_<Elem, 0> {
    Wrapper_<Elem> elem;

    constexpr Leaf_() = default;

    template <typename Arg>
    constexpr Leaf_(Arg&& arg);
};

template <size_t N, typename... Elems>
struct Node_ {
    Node_<N - 1, Elems...> node;
    Leaf_<meta::GetNthType<N - 1, Elems...>,
          CalcLayout_<Elems...>().elem_paddings[N - 1]>
        leaf;

    constexpr Node_() = default;

    template <typename... Args>
    constexpr Node_(tag_::value_list::Insuff, Args&&... args);

    template <typename... Args>
    constexpr Node_(tag_::value_list::Exact, Args&&... args);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Excess,
                    Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Exact,
                    Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Insuff,
                    Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Excess,
                    Node_<SrcN, SrcElems...>&& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Exact,
                    Node_<SrcN, SrcElems...>&& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Insuff,
                    Node_<SrcN, SrcElems...>&& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_& operator=(Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_& operator=(Node_<SrcN, SrcElems...>&& src_node);

    template <size_t Idx>
    constexpr decltype(auto) Get() &;

    template <size_t Idx>
    constexpr decltype(auto) Get() const&;

    template <size_t Idx>
    constexpr decltype(auto) Get() &&;
} __attribute__((packed));

template <typename... Elems>
struct Node_<1, Elems...> {
    static constexpr size_t N{ 1 };

    Leaf_<meta::GetNthType<0, Elems...>,
          CalcLayout_<Elems...>().elem_paddings[0]>
        leaf;

    constexpr Node_() = default;

    template <typename... Args>
    constexpr Node_(tag_::value_list::Insuff, Args&&...);

    template <typename... Args>
    constexpr Node_(tag_::value_list::Exact, Args&&... args);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Excess,
                    Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Exact,
                    Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::l_src_tuple::Insuff, Node_<SrcN, SrcElems...> const&);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Excess,
                    Node_<SrcN, SrcElems...>&& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Exact,
                    Node_<SrcN, SrcElems...>&& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_(tag_::r_src_tuple::Insuff, Node_<SrcN, SrcElems...>&&);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_& operator=(Node_<SrcN, SrcElems...> const& src_node);

    template <size_t SrcN, typename... SrcElems>
    constexpr Node_& operator=(Node_<SrcN, SrcElems...>&& src_node);

    template <size_t Idx>
    constexpr decltype(auto) Get() &;

    template <size_t Idx>
    constexpr decltype(auto) Get() const&;

    template <size_t Idx>
    constexpr decltype(auto) Get() &&;
} __attribute__((packed));

}  // namespace detail

template <typename... Elems>
struct Tuple {
private:
    alignas(detail::CalcLayout_<Elems...>().align)
        detail::Node_<sizeof...(Elems), Elems...> node_;

public:
    constexpr Tuple() = default;

    template <typename... Args>
    constexpr Tuple(Args&&... args);

    template <typename... SrcElems>
    constexpr Tuple(Tuple<SrcElems...> const& t);

    template <typename... SrcElems>
    constexpr Tuple(Tuple<SrcElems...>&& t);

    template <typename... SrcElems>
    constexpr Tuple& operator=(Tuple<SrcElems...> const& t);

    template <typename... SrcElems>
    constexpr Tuple& operator=(Tuple<SrcElems...>&& t);

    template <size_t Idx>
    constexpr decltype(auto) Get() &;

    template <size_t Idx>
    constexpr decltype(auto) Get() const&;

    template <size_t Idx>
    constexpr decltype(auto) Get() &&;
};

template <>
struct Tuple<> {
    constexpr Tuple() = default;

    template <typename... Args>
    constexpr Tuple(Args&&...);

    template <typename... SrcElems>
    constexpr Tuple(Tuple<SrcElems...> const&);

    template <typename... SrcElems>
    constexpr Tuple(Tuple<SrcElems...>&&);

    template <typename... SrcElems>
    constexpr Tuple& operator=(Tuple<SrcElems...> const&);

    template <typename... SrcElems>
    constexpr Tuple& operator=(Tuple<SrcElems...>&&);
};

template <typename... Args>
constexpr auto MakeTuple(Args&&... args);

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...>& t);

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...> const& t);

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...>&& t);

}  // namespace zeta::core::tuple
