#pragma once

#include <zeta/core/tuple.hpp>

namespace zeta::core {

template <typename Elem, size_t PaddingSize>
template <typename Arg>
constexpr tuple::detail::Leaf_<Elem, PaddingSize>::Leaf_(Arg&& arg)
    : elem{ meta::Forward<Arg>(arg) } {}

template <typename Elem>
template <typename Arg>
constexpr tuple::detail::Leaf_<Elem, 0>::Leaf_(Arg&& arg)
    : elem{ meta::Forward<Arg>(arg) } {}

template <size_t N, typename... Elems>
template <typename... Args>
constexpr tuple::detail::Node_<N, Elems...>::Node_(tag_::value_list::Insuff,
                                                   Args&&... args)
    : node{ meta::Conditional<N - 1 == sizeof...(Args), tag_::value_list::Exact,
                              tag_::value_list::Insuff>{},
            meta::Forward<Args>(args)... } {}

template <size_t N, typename... Elems>
template <typename... Args>
constexpr tuple::detail::Node_<N, Elems...>::Node_(tag_::value_list::Exact,
                                                   Args&&... args)
    : node{ tag_::value_list::Exact{}, meta::Forward<Args>(args)... },
      leaf{ meta::Forward<meta::GetNthType<N - 1, Args&&...>>(
          meta::GetNthArg<N - 1>(args...)) } {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::l_src_tuple::Excess, Node_<SrcN, SrcElems...> const& src_node)
    : Node_(meta::Conditional<SrcN - 1 == N, tag_::l_src_tuple::Exact,
                              tag_::l_src_tuple::Excess>{},
            src_node.node) {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::l_src_tuple::Exact, Node_<SrcN, SrcElems...> const& src_node)
    : node{ tag_::l_src_tuple::Exact{}, src_node.node },
      leaf{ src_node.leaf } {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::l_src_tuple::Insuff, Node_<SrcN, SrcElems...> const& src_node)
    : node{ meta::Conditional<N - 1 == SrcN, tag_::l_src_tuple::Exact,
                              tag_::l_src_tuple::Insuff>{},
            src_node } {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::r_src_tuple::Excess, Node_<SrcN, SrcElems...>&& src_node)
    : Node_(meta::Conditional<SrcN - 1 == N, tag_::r_src_tuple::Exact,
                              tag_::r_src_tuple::Excess>{},
            meta::Move(src_node.node)) {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::r_src_tuple::Exact, Node_<SrcN, SrcElems...>&& src_node)
    : node{ tag_::r_src_tuple::Exact{}, meta::Move(src_node.node) },
      leaf{ meta::Move(src_node.leaf) } {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>::Node_(
    tag_::r_src_tuple::Insuff, Node_<SrcN, SrcElems...>&& src_node)
    : node{ meta::Conditional<N - 1 == SrcN, tag_::r_src_tuple::Exact,
                              tag_::r_src_tuple::Insuff>{},
            meta::Move(src_node) } {}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>&
tuple::detail::Node_<N, Elems...>::operator=(
    Node_<SrcN, SrcElems...> const& src_node) {
    if constexpr (SrcN < N) {
        this->node = src_node;
    } else if constexpr (N < SrcN) {
        *this = src_node.node;
    } else {
        this->node = src_node.node;
        this->leaf = src_node.leaf;
    }

    return *this;
}

template <size_t N, typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<N, Elems...>&
tuple::detail::Node_<N, Elems...>::operator=(
    Node_<SrcN, SrcElems...>&& src_node) {
    if constexpr (SrcN < N) {
        this->node = meta::Move(src_node);
    } else if constexpr (N < SrcN) {
        *this = meta::Move(src_node.node);
    } else {
        this->node = meta::Move(src_node.node);
        this->leaf = meta::Move(src_node.leaf);
    }

    return *this;
}

template <size_t N, typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<N, Elems...>::Get() & {
    ZETA_Core_StaticAssert(Idx < N);

    if constexpr (Idx < N - 1) {
        return this->node.template Get<Idx>();
    } else {
        return this->leaf.elem.value;
    }
}

template <size_t N, typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<N, Elems...>::Get() const& {
    ZETA_Core_StaticAssert(Idx < N);

    if constexpr (Idx < N - 1) {
        return this->node.template Get<Idx>();
    } else {
        return this->leaf.elem.value;
    }
}

template <size_t N, typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<N, Elems...>::Get() && {
    ZETA_Core_StaticAssert(Idx < N);

    if constexpr (Idx < N - 1) {
        return meta::Move(this->node).template Get<Idx>();
    } else {
        return meta::Move(this->leaf.elem.value);
    }
}

template <typename... Elems>
template <typename... Args>
constexpr tuple::detail::Node_<1, Elems...>::Node_(tag_::value_list::Insuff,
                                                   Args&&...) {}

template <typename... Elems>
template <typename... Args>
constexpr tuple::detail::Node_<1, Elems...>::Node_(tag_::value_list::Exact,
                                                   Args&&... args)
    : leaf{ meta::Forward<meta::GetNthType<N - 1, Args&&...>>(
          meta::GetNthArg<N - 1>(args...)) } {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(
    tag_::l_src_tuple::Excess, Node_<SrcN, SrcElems...> const& src_node)
    : Node_(meta::Conditional<SrcN - 1 == N, tag_::l_src_tuple::Exact,
                              tag_::l_src_tuple::Excess>{},
            src_node.node) {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(
    tag_::l_src_tuple::Exact, Node_<SrcN, SrcElems...> const& src_node)
    : leaf{ src_node.leaf } {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(
    tag_::l_src_tuple::Insuff, Node_<SrcN, SrcElems...> const&) {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(
    tag_::r_src_tuple::Excess, Node_<SrcN, SrcElems...>&& src_node)
    : Node_(meta::Conditional<SrcN - 1 == N, tag_::r_src_tuple::Exact,
                              tag_::r_src_tuple::Excess>{},
            meta::Move(src_node.node)) {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(
    tag_::r_src_tuple::Exact, Node_<SrcN, SrcElems...>&& src_node)
    : leaf{ meta::Move(src_node.leaf) } {}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>::Node_(tag_::r_src_tuple::Insuff,
                                                   Node_<SrcN, SrcElems...>&&) {
}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>&
tuple::detail::Node_<1, Elems...>::operator=(
    Node_<SrcN, SrcElems...> const& src_node) {
    if constexpr (SrcN < N) {
    } else if constexpr (N < SrcN) {
        *this = src_node.node;
    } else {
        this->leaf = src_node.leaf;
    }

    return *this;
}

template <typename... Elems>
template <size_t SrcN, typename... SrcElems>
constexpr tuple::detail::Node_<1, Elems...>&
tuple::detail::Node_<1, Elems...>::operator=(
    Node_<SrcN, SrcElems...>&& src_node) {
    if constexpr (SrcN < N) {
    } else if constexpr (N < SrcN) {
        *this = meta::Move(src_node.node);
    } else {
        this->leaf = meta::Move(src_node.leaf);
    }

    return *this;
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<1, Elems...>::Get() & {
    ZETA_Core_StaticAssert(Idx < N);
    return this->leaf.elem.value;
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<1, Elems...>::Get() const& {
    ZETA_Core_StaticAssert(Idx < N);
    return this->leaf.elem.value;
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::detail::Node_<1, Elems...>::Get() && {
    ZETA_Core_StaticAssert(Idx < N);
    return meta::Move(this->leaf.elem.value);
}

template <typename... Elems>
template <typename... Args>
constexpr tuple::Tuple<Elems...>::Tuple(Args&&... args)
    : node_{ meta::Conditional<sizeof...(Args) == sizeof...(Elems),
                               detail::tag_::value_list::Exact,
                               detail::tag_::value_list::Insuff>{},
             meta::Forward<Args>(args)... } {
    ZETA_Core_StaticAssert(sizeof...(Args) <= sizeof...(Elems));
}

template <typename... Elems>
template <typename... SrcElems>
constexpr tuple::Tuple<Elems...>::Tuple(Tuple<SrcElems...> const& t)
    : node_{ meta::Conditional<
                 sizeof...(SrcElems) < sizeof...(Elems),
                 detail::tag_::l_src_tuple::Insuff,
                 meta::Conditional<sizeof...(Elems) < sizeof...(SrcElems),
                                   detail::tag_::l_src_tuple::Excess,
                                   detail::tag_::l_src_tuple::Exact>>{},
             t.node_ } {}

template <typename... Elems>
template <typename... SrcElems>
constexpr tuple::Tuple<Elems...>::Tuple(Tuple<SrcElems...>&& t)
    : node_{ meta::Conditional<
                 sizeof...(SrcElems) < sizeof...(Elems),
                 detail::tag_::l_src_tuple::Insuff,
                 meta::Conditional<sizeof...(Elems) < sizeof...(SrcElems),
                                   detail::tag_::l_src_tuple::Excess,
                                   detail::tag_::l_src_tuple::Exact>>{},
             meta::Move(t.node_) } {}

template <typename... Elems>
template <typename... SrcElems>
constexpr tuple::Tuple<Elems...>& tuple::Tuple<Elems...>::operator=(
    Tuple<SrcElems...> const& t) {
    if (this != &t) { node_ = t.node_; }
    return *this;
}

template <typename... Elems>
template <typename... SrcElems>
constexpr tuple::Tuple<Elems...>& tuple::Tuple<Elems...>::operator=(
    Tuple<SrcElems...>&& t) {
    if (this != &t) { node_ = meta::Move(t.node_); }
    return *this;
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::Tuple<Elems...>::Get() & {
    return this->node_.template Get<Idx>();
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::Tuple<Elems...>::Get() const& {
    return this->node_.template Get<Idx>();
}

template <typename... Elems>
template <size_t Idx>
constexpr decltype(auto) tuple::Tuple<Elems...>::Get() && {
    return this->node_.template Get<Idx>();
}

template <typename... Args>
constexpr tuple::Tuple<>::Tuple(Args&&...) {}

template <typename... SrcElems>
constexpr tuple::Tuple<>::Tuple(Tuple<SrcElems...> const&) {}

template <typename... SrcElems>
constexpr tuple::Tuple<>::Tuple(Tuple<SrcElems...>&&) {}

template <typename... SrcElems>
constexpr tuple::Tuple<>& tuple::Tuple<>::operator=(Tuple<SrcElems...> const&) {
    return *this;
}

template <typename... SrcElems>
constexpr tuple::Tuple<>& tuple::Tuple<>::operator=(Tuple<SrcElems...>&&) {
    return *this;
}

template <typename... Args>
constexpr auto tuple::MakeTuple(Args&&... args) {
    return Tuple<Args&&...>{ meta::Forward<Args>(args)... };
}

namespace tuple::detail {

template <size_t N>
struct Apply_ {
    template <typename Func, typename TupleType, typename... UnpackedArgs>
    static constexpr void Call(Func&& func, TupleType&& t,
                               UnpackedArgs&&... unpacked_args) {
#pragma push_macro("GetArg")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GetArg(i) meta::Forward<TupleType>(t).template Get<i>()

#pragma push_macro("ForwardUnpackedArgs")
#define ForwardUnpackedArgs meta::Forward<UnpackedArgs>(unpacked_args)...

        if constexpr (N == 0) {
            return meta::Forward<Func>(func)(ForwardUnpackedArgs);
        } else if constexpr (N == 1) {
            return meta::Forward<Func>(func)(GetArg(0), ForwardUnpackedArgs);
        } else if constexpr (N == 2) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1),
                                             ForwardUnpackedArgs);
        } else if constexpr (N == 3) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1), GetArg(2),
                                             ForwardUnpackedArgs);
        } else if constexpr (N == 4) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1), GetArg(2),
                                             GetArg(3), ForwardUnpackedArgs);
        } else if constexpr (N == 5) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1), GetArg(2),
                                             GetArg(3), GetArg(4),
                                             ForwardUnpackedArgs);
        } else if constexpr (N == 6) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1), GetArg(2),
                                             GetArg(3), GetArg(4), GetArg(5),
                                             ForwardUnpackedArgs);
        } else if constexpr (N == 7) {
            return meta::Forward<Func>(func)(GetArg(0), GetArg(1), GetArg(2),
                                             GetArg(3), GetArg(4), GetArg(5),
                                             GetArg(6), ForwardUnpackedArgs);
        } else {
            return Apply_<N - 8>::Call(
                meta::Forward<Func>(func), meta::Forward<TupleType>(t),
                GetArg(N - 8), GetArg(N - 7), GetArg(N - 6), GetArg(N - 5),
                GetArg(N - 4), GetArg(N - 3), GetArg(N - 2), GetArg(N - 1),
                ForwardUnpackedArgs);
        }

#pragma pop_macro("ForwardUnpackedArgs")
#pragma pop_macro("GetArg")
    }
};

}  // namespace tuple::detail

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...>& t) {
    return detail::Apply_<sizeof...(Elems)>::Call(meta::Forward<Func>(func), t);
}

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...> const& t) {
    return detail::Apply_<sizeof...(Elems)>::Call(meta::Forward<Func>(func), t);
}

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...>&& t) {
    return detail::Apply_<sizeof...(Elems)>::Call(meta::Forward<Func>(func), t);
}

}  // namespace zeta::core
