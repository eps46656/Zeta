#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/tuple.ipp>

#define ZETA_Core_Lifecycle_PackConstructArgs(...) \
    ::zeta::core::lifecycle::detail::PackConstructArgs(__VA_ARGS__)

#define ZETA_Core_Lifecycle_UnpackConstructArgs(Target, Arg, arg) \
    ::zeta::core::lifecycle::detail::Unpack(                      \
        ::zeta::core::meta::TypeWrapper<Target>{},                \
        ::zeta::core::meta::Forward<Arg>(arg))

namespace zeta::core::lifecycle {

struct DirectConstructTag {};

namespace detail {

template <typename... Args>
struct ConstructArgsTuple : public tuple::Tuple<Args...> {};

template <typename T>
struct IsConstructArgsTupleImpl_ {
    static constexpr bool value{ false };
};

template <typename... Args>
struct IsConstructArgsTupleImpl_<ConstructArgsTuple<Args...>> {
    static constexpr bool value{ true };
};

template <typename T>
constexpr bool IsConstructArgsTuple{ IsConstructArgsTupleImpl_<T>::value };

constexpr ConstructArgsTuple<> Pack() { return {}; }

template <typename Arg0, typename... Args>
constexpr decltype(auto) Pack(Arg0&& arg0, Args&&... args) {
    if constexpr (sizeof...(Args) == 0 &&
                  IsConstructArgsTuple<
                      meta::RemoveConstVolatileReference<Arg0>>) {
        return meta::Forward<Arg0>(arg0);
    } else {
        return ConstructArgsTuple<Arg0&&, Args&&...>{
            meta::Forward<Arg0>(arg0), meta::Forward<Args>(args)...
        };
    }
}

template <typename Target, typename... Args, size_t... ArgIdxes>
constexpr Target UnpackConstructArgsTuple(
    meta::TypeWrapper<Target>,
    ConstructArgsTuple<Args&&...> const& construct_args_tuple,
    static_seq::StaticSeq<size_t, ArgIdxes...>) {
    return { construct_args_tuple.template ForwardAccess<ArgIdxes>()... };
}

template <typename Target>
constexpr Target Unpack(meta::TypeWrapper<Target>) {
    return {};
}

template <typename Target, typename Arg0, typename... Args>
constexpr decltype(auto) Unpack(meta::TypeWrapper<Target>, Arg0&& arg0,
                                Args&&... args) {
    if constexpr (sizeof...(Args) == 0 &&
                  IsConstructArgsTuple<
                      meta::RemoveConstVolatileReference<Arg0>>) {
        return (
            UnpackConstructArgsTuple)(meta::TypeWrapper<Target>{},
                                      meta::Forward<Arg0>(arg0),
                                      static_seq::MakeLinearStaticIntegralSeq<
                                          size_t, 0, 1, sizeof...(Args)>{});
    } else {
        return Target{ meta::Forward<Arg0>(arg0),
                       meta::Forward<Args>(args)... };
    }
}

}  // namespace detail

struct PreConstructorHook {
    constexpr PreConstructorHook() = default;

    template <typename Callable>
    constexpr PreConstructorHook(Callable&& callable);
};

template <meta::IsNonConstObjectType Obj, typename... Args>
constexpr void InvokeConstructor(Obj* obj, Args&&... args);

template <meta::IsNonConstObjectType Elem, typename... Args>
constexpr void InvokeLinSeqConstructor(Elem* data, size_t elem_stride,
                                       size_t elem_cnt, Args&&... args);

template <meta::IsNonConstObjectType Obj>
constexpr void InvokeDestructor(Obj& obj);

template <meta::IsNonConstObjectType Elem>
constexpr void InvokeLinSeqDestructor(Elem* data, size_t elem_stride,
                                      size_t elem_cnt);

enum struct StorageSemantics : unsigned char {
    Null,

    ConstructingMemToObj,
    DestructingObjToMem,

    LeftObj,
    ConstLeftObj,
    RightObj,
    ConstRightObj,
};

template <typename T>
concept IsStorageSemanticsType = meta::IsSame<T, StorageSemantics>;

constexpr bool IsStorageSemantics(StorageSemantics semantics) {
    return semantics == StorageSemantics::Null ||
           semantics == StorageSemantics::ConstructingMemToObj ||
           semantics == StorageSemantics::DestructingObjToMem ||
           semantics == StorageSemantics::LeftObj ||
           semantics == StorageSemantics::ConstLeftObj ||
           semantics == StorageSemantics::RightObj ||
           semantics == StorageSemantics::ConstRightObj;
}

template <StorageSemantics semantics>
concept IsConstStorageSemantics = semantics == StorageSemantics::ConstLeftObj ||
                                  semantics == StorageSemantics::ConstRightObj;

constexpr bool IsDstStorageSemantics(StorageSemantics semantics) {
    return semantics == StorageSemantics::ConstructingMemToObj ||
           semantics == StorageSemantics::LeftObj ||
           semantics == StorageSemantics::RightObj;
}

constexpr bool IsSrcStorageSemantics(StorageSemantics semantics) {
    return semantics == StorageSemantics::DestructingObjToMem ||
           semantics == StorageSemantics::LeftObj ||
           semantics == StorageSemantics::ConstLeftObj ||
           semantics == StorageSemantics::RightObj ||
           semantics == StorageSemantics::ConstRightObj;
}

constexpr bool IsStorageSemanticsConvertible(StorageSemantics semantics_from,
                                             StorageSemantics semantics_to) {
    return semantics_from == semantics_to ||
           (semantics_from == StorageSemantics::LeftObj &&
            semantics_to == StorageSemantics::ConstLeftObj) ||
           (semantics_from == StorageSemantics::RightObj &&
            semantics_to == StorageSemantics::ConstRightObj) ||
           (semantics_from == StorageSemantics::RightObj &&
            semantics_to == StorageSemantics::ConstLeftObj) ||
           (semantics_from == StorageSemantics::ConstRightObj &&
            semantics_to == StorageSemantics::ConstLeftObj);
}

template <StorageSemantics semantics>
constexpr StorageSemantics StorageSemanticsAddConst{ []() {
    if constexpr (semantics == StorageSemantics::LeftObj) {
        return StorageSemantics::ConstLeftObj;
    } else if constexpr (semantics == StorageSemantics::ConstLeftObj) {
        return StorageSemantics::ConstLeftObj;
    } else if constexpr (semantics == StorageSemantics::RightObj) {
        return StorageSemantics::ConstRightObj;
    } else if constexpr (semantics == StorageSemantics::ConstRightObj) {
        return StorageSemantics::ConstRightObj;
    } else {
        static_assert(false);
    }
} };

template <StorageSemantics semantics>
constexpr StorageSemantics StorageSemanticsRemoveConst{ []() {
    if constexpr (semantics == StorageSemantics::LeftObj) {
        return StorageSemantics::LeftObj;
    } else if constexpr (semantics == StorageSemantics::ConstLeftObj) {
        return StorageSemantics::LeftObj;
    } else if constexpr (semantics == StorageSemantics::RightObj) {
        return StorageSemantics::RightObj;
    } else if constexpr (semantics == StorageSemantics::ConstRightObj) {
        return StorageSemantics::RightObj;
    } else {
        static_assert(false);
    }
} };

template <StorageSemantics semantics>
constexpr StorageSemantics StorageSemanticsToLeft{ []() {
    if constexpr (semantics == StorageSemantics::LeftObj) {
        return StorageSemantics::LeftObj;
    } else if constexpr (semantics == StorageSemantics::ConstLeftObj) {
        return StorageSemantics::ConstLeftObj;
    } else if constexpr (semantics == StorageSemantics::RightObj) {
        return StorageSemantics::LeftObj;
    } else if constexpr (semantics == StorageSemantics::ConstRightObj) {
        return StorageSemantics::ConstLeftObj;
    } else {
        static_assert(false);
    }
} };

template <StorageSemantics semantics>
constexpr StorageSemantics StorageSemanticsToRight{ []() {
    if constexpr (semantics == StorageSemantics::LeftObj) {
        return StorageSemantics::RightObj;
    } else if constexpr (semantics == StorageSemantics::ConstLeftObj) {
        return StorageSemantics::ConstRightObj;
    } else if constexpr (semantics == StorageSemantics::RightObj) {
        return StorageSemantics::RightObj;
    } else if constexpr (semantics == StorageSemantics::ConstRightObj) {
        return StorageSemantics::ConstRightObj;
    } else {
        static_assert(false);
    }
} };

namespace detail {

template <typename T>
struct ForwardStorageSemanticsImpl_ {
    static_assert(false);
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T> {
    static constexpr StorageSemantics value{ StorageSemantics::RightObj };
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T&> {
    static constexpr StorageSemantics value{ StorageSemantics::LeftObj };
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T&&> {
    static constexpr StorageSemantics value{ StorageSemantics::RightObj };
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T const> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstRightObj };
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T const&> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstLeftObj };
};

template <meta::IsNonConstObjectType T>
struct ForwardStorageSemanticsImpl_<T const&&> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstRightObj };
};

}  // namespace detail

template <typename T>
constexpr StorageSemantics ForwardStorageSemantics{
    detail::ForwardStorageSemanticsImpl_<T>::value
};

template <meta::IsNonConstObjectType Obj>
struct DynamicStorageView;

template <StorageSemantics semantics_, meta::IsNonConstObjectType Obj_>
struct StorageView {
    static constexpr StorageSemantics semantics{ semantics_ };

    using Obj = Obj_;

    meta::ConditionalAddConst<IsConstStorageSemantics<semantics>, Obj>* ptr;

    constexpr StorageView(Obj* ptr);

    template <meta::IsObjectType FromObj>
    constexpr StorageView(FromObj* from_ptr);

    template <StorageSemantics from_semantics>
    constexpr StorageView(StorageView<from_semantics, Obj> const& from_view)
        requires((IsStorageSemanticsConvertible)(from_semantics, semantics));

    constexpr StorageView(DynamicStorageView<Obj> const& from_view);

    static constexpr bool IsNull();

    constexpr decltype(auto) GetRef(this StorageView const& self)
        requires(semantics == StorageSemantics::DestructingObjToMem ||
                 semantics == StorageSemantics::LeftObj ||
                 semantics == StorageSemantics::ConstLeftObj ||
                 semantics == StorageSemantics::RightObj ||
                 semantics == StorageSemantics::ConstRightObj);

    template <StorageSemantics to_semantics>
    constexpr StorageView<to_semantics, Obj> ToStatic(
        this StorageView const& self)
        requires((IsStorageSemanticsConvertible)(semantics, to_semantics));

    constexpr DynamicStorageView<Obj> ToDynamic(this StorageView const& self);

    template <StorageSemantics from_semantics>
    constexpr StorageView& operator=(
        this StorageView const& self,
        StorageView<from_semantics, Obj> const& from_view)
        requires((IsStorageSemanticsConvertible)(from_semantics, semantics));

    constexpr StorageView& operator=(this StorageView const& self,
                                     DynamicStorageView<Obj> const& from_view);

    friend constexpr bool operator==(StorageView const& lhs,
                                     StorageView const& rhs) = default;

    friend constexpr bool operator!=(StorageView const& lhs,
                                     StorageView const& rhs) = default;

    static constexpr void AssertIsValid();
};

template <meta::IsNonConstObjectType Obj_>
struct StorageView<StorageSemantics::Null, Obj_> {
    static constexpr StorageSemantics semantics{ StorageSemantics::Null };

    using Obj = Obj_;

    constexpr StorageView() = default;

    constexpr StorageView(StorageView const& from_view) = default;

    constexpr StorageView(DynamicStorageView<Obj> const& from_view);

    static constexpr bool IsNull();

    template <StorageSemantics to_semantics>
    static constexpr StorageView<to_semantics, Obj> ToStatic()
        requires(to_semantics == StorageSemantics::Null);

    static constexpr DynamicStorageView<Obj> ToDynamic();

    friend constexpr bool operator==(StorageView const& lhs,
                                     StorageView const& rhs) = default;

    friend constexpr bool operator!=(StorageView const& lhs,
                                     StorageView const& rhs) = default;

    constexpr void AssertIsValid(this StorageView const& self);
};

template <meta::IsNonConstObjectType Obj_>
struct DynamicStorageView {
    using Obj = Obj_;

    StorageSemantics semantics;

    Obj* ptr;

    constexpr DynamicStorageView(StorageSemantics semantics, Obj* from_ptr);

    template <meta::IsObjectType FromObj>
    constexpr DynamicStorageView(StorageSemantics semantics, FromObj* from_ptr);

    template <StorageSemantics from_semantics, meta::IsObjectType FromObj>
    constexpr DynamicStorageView(
        StorageView<from_semantics, FromObj> const& from_view);

    constexpr bool IsNull(this DynamicStorageView const& self);

    template <StorageSemantics semantics>
    constexpr StorageView<semantics, Obj> ToStatic(
        this DynamicStorageView const& self);

    template <StorageSemantics from_semantics,
              meta::IsNonConstObjectType FromObj>
    constexpr DynamicStorageView& operator=(
        this DynamicStorageView& self,
        StorageView<from_semantics, FromObj> const& from_view);

    template <meta::IsNonConstObjectType FromObj>
    constexpr DynamicStorageView& operator=(
        this DynamicStorageView& self,
        DynamicStorageView<FromObj> const& from_view);

    constexpr void AssertIsValid(this DynamicStorageView const& self);
};

template <meta::IsNonConstObjectType Obj>
using NullView = StorageView<StorageSemantics::Null, Obj>;

template <meta::IsNonConstObjectType Obj>
using ConstructingMemToObjView =
    StorageView<StorageSemantics::ConstructingMemToObj, Obj>;

template <meta::IsNonConstObjectType Obj>
using DestructingObjToMemView =
    StorageView<StorageSemantics::DestructingObjToMem, Obj>;

template <meta::IsNonConstObjectType Obj>
using LeftObjView = StorageView<StorageSemantics::LeftObj, Obj>;

template <meta::IsNonConstObjectType Obj>
using ConstLeftObjView = StorageView<StorageSemantics::ConstLeftObj, Obj>;

template <meta::IsNonConstObjectType Obj>
using RightObjView = StorageView<StorageSemantics::RightObj, Obj>;

template <meta::IsNonConstObjectType Obj>
using ConstRightObjView = StorageView<StorageSemantics::ConstRightObj, Obj>;

template <typename T, meta::IsNonConstObjectType Obj>
using ForwardStorageView = StorageView<ForwardStorageSemantics<T>, Obj>;

namespace detail {

template <typename T, meta::IsObjectType TargetObj>
struct IsStorageViewImpl_ {
    static constexpr bool value{ false };
};

template <StorageSemantics semantics, meta::IsObjectType Obj,
          meta::IsObjectType TargetObj>
struct IsStorageViewImpl_<StorageView<semantics, Obj>, TargetObj> {
    static constexpr bool value{ meta::IsMatched<Obj, TargetObj> };
};

}  // namespace detail

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsStorageView = detail::IsStorageViewImpl_<T, TargetObj>::value;

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsDstStorageView =
    IsStorageView<T, TargetObj> && (IsDstStorageSemantics)(T::semantics);

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsOptionalDstStorageView =
    IsStorageView<T, TargetObj> &&
    (T::IsNull() || IsDstStorageView<T, TargetObj>);

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsSrcStorageView =
    IsStorageView<T, TargetObj> && (IsSrcStorageSemantics)(T::semantics);

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsOptionalSrcStorageView =
    IsStorageView<T, TargetObj> &&
    (T::IsNull() || IsSrcStorageView<T, TargetObj>);

template <typename T>
constexpr LeftObjView<T> MakeStorageView(T& obj);

template <typename T>
constexpr ConstLeftObjView<T> MakeStorageView(T const& obj);

template <typename T>
constexpr RightObjView<T> MakeStorageView(T&& obj);

template <typename T>
constexpr ConstRightObjView<T> MakeStorageView(T const&& obj);

template <IsDstStorageView DstStorageView, IsSrcStorageView SrcStorageView>
constexpr void StorageTransfer(DstStorageView dst, SrcStorageView src);

template <IsDstStorageView DstStorageView, meta::IsNonConstObjectType SrcObj>
constexpr void StorageTransfer(DstStorageView dst,
                               DynamicStorageView<SrcObj> src);

template <meta::IsNonConstObjectType DstObj, IsSrcStorageView SrcStorageView>
constexpr void StorageTransfer(DynamicStorageView<DstObj> dst,
                               SrcStorageView src);

template <meta::IsNonConstObjectType DstObj, meta::IsNonConstObjectType SrcObj>
constexpr void StorageTransfer(DynamicStorageView<DstObj> dst,
                               DynamicStorageView<SrcObj> src);

enum struct StorageLifeState : unsigned char {
    Mem = 0,
    Obj = 1,
};

}  // namespace zeta::core::lifecycle
