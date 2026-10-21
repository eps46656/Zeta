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
                  IsConstructArgsTuple<meta::RemoveCVRef<Arg0>>) {
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
                  IsConstructArgsTuple<meta::RemoveCVRef<Arg0>>) {
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

template <typename Obj, typename... Args>
constexpr void InvokeConstructor(Obj* obj, Args&&... args);

template <typename Elem, typename... Args>
constexpr void InvokeLinSeqConstructor(Elem* data, size_t elem_stride,
                                       size_t elem_cnt, Args&&... args);

template <typename Obj>
constexpr void InvokeDestructor(Obj& obj);

template <typename Elem>
constexpr void InvokeLinSeqDestructor(Elem* data, size_t elem_stride,
                                      size_t elem_cnt);

enum struct StorageSemantics : unsigned char {
    ConstructingMemToObj = 0,
    DestructingObjToMem  = 1,

    LeftObj       = 2,
    ConstLeftObj  = 3,
    RightObj      = 4,
    ConstRightObj = 5,
};

template <typename T>
concept IsStorageSemantics = meta::IsSame<T, StorageSemantics>;

namespace detail {

template <typename T>
struct ForwardStorageSemanticsImpl_ {
    static_assert(false);
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T> {
    static constexpr StorageSemantics value{ StorageSemantics::RightObj };
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T&> {
    static constexpr StorageSemantics value{ StorageSemantics::LeftObj };
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T&&> {
    static constexpr StorageSemantics value{ StorageSemantics::RightObj };
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T const> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstRightObj };
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T const&> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstLeftObj };
};

template <meta::IsNonConstObj T>
struct ForwardStorageSemanticsImpl_<T const&&> {
    static constexpr StorageSemantics value{ StorageSemantics::ConstRightObj };
};

}  // namespace detail

template <typename T>
constexpr StorageSemantics ForwardStorageSemantics{
    detail::ForwardStorageSemanticsImpl_<T>::value
};

template <StorageSemantics Semantics>
concept IsNonConstStorageSemantics =
    Semantics == StorageSemantics::ConstructingMemToObj ||
    Semantics == StorageSemantics::DestructingObjToMem ||
    Semantics == StorageSemantics::LeftObj ||
    Semantics == StorageSemantics::RightObj;

template <StorageSemantics Semantics>
concept IsConstStorageSemantics = Semantics == StorageSemantics::ConstLeftObj ||
                                  Semantics == StorageSemantics::ConstRightObj;

template <StorageSemantics Semantics>
concept IsLeftStorageSemantics = Semantics == StorageSemantics::LeftObj ||
                                 Semantics == StorageSemantics::ConstLeftObj;

template <StorageSemantics Semantics>
concept IsRightStorageSemantics = Semantics == StorageSemantics::RightObj ||
                                  Semantics == StorageSemantics::ConstRightObj;

template <StorageSemantics Semantics>
concept IsDstStorageSemantics =
    Semantics == StorageSemantics::ConstructingMemToObj ||
    Semantics == StorageSemantics::LeftObj ||
    Semantics == StorageSemantics::RightObj;

template <StorageSemantics Semantics>
concept IsSrcStorageSemantics =
    Semantics == StorageSemantics::DestructingObjToMem ||
    Semantics == StorageSemantics::LeftObj ||
    Semantics == StorageSemantics::ConstLeftObj ||
    Semantics == StorageSemantics::RightObj ||
    Semantics == StorageSemantics::ConstRightObj;

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

template <meta::IsObj Obj>
struct DynamicStorageView;

template <StorageSemantics Semantics_, meta::IsNonConstObj Obj_>
struct StorageView {
    static constexpr StorageSemantics semantics{ Semantics_ };

    using Obj = Obj_;

    meta::MakeConstIf<Obj, IsConstStorageSemantics<semantics>>* ptr;

    constexpr StorageView(Obj* ptr);

    template <meta::IsObj FromObj>
    constexpr StorageView(FromObj* from_ptr);

    template <StorageSemantics FromSemantics>
    constexpr StorageView(StorageView<FromSemantics, Obj> const& from_view)
        requires((IsStorageSemanticsConvertible)(FromSemantics, semantics));

    constexpr StorageView(DynamicStorageView<Obj> const& from_view);

    constexpr decltype(auto) GetRef(this StorageView const& self)
        requires(semantics == StorageSemantics::DestructingObjToMem ||
                 semantics == StorageSemantics::LeftObj ||
                 semantics == StorageSemantics::ConstLeftObj ||
                 semantics == StorageSemantics::RightObj ||
                 semantics == StorageSemantics::ConstRightObj);

    template <StorageSemantics ToSemantics>
    constexpr StorageView<ToSemantics, Obj> ToStatic(
        this StorageView const& self)
        requires((IsStorageSemanticsConvertible)(semantics, ToSemantics));

    constexpr DynamicStorageView<Obj> ToDynamic(this StorageView const& self);

    template <StorageSemantics FromSemantics>
    constexpr StorageView& operator=(
        this StorageView const& self,
        StorageView<FromSemantics, Obj> const& from_view)
        requires((IsStorageSemanticsConvertible)(FromSemantics, semantics));

    constexpr StorageView& operator=(this StorageView const& self,
                                     DynamicStorageView<Obj> const& from_view);
};

template <meta::IsObj Obj_>
struct DynamicStorageView {
    using Obj = Obj_;

    StorageSemantics semantics;

    Obj* ptr;

    constexpr DynamicStorageView(StorageSemantics semantics, Obj* from_ptr);

    template <meta::IsObj FromObj>
    constexpr DynamicStorageView(StorageSemantics semantics, FromObj* from_ptr);

    template <StorageSemantics FromSemantics, meta::IsObj FromObj>
    constexpr DynamicStorageView(
        StorageView<FromSemantics, FromObj> const& from_view);

    template <StorageSemantics Semantics>
    constexpr StorageView<Semantics, Obj> ToStatic(
        this DynamicStorageView const& self);

    template <StorageSemantics FromSemantics, meta::IsObj FromObj>
    constexpr DynamicStorageView& operator=(
        this DynamicStorageView& self,
        StorageView<FromSemantics, Obj> const& from_view);

    template <meta::IsObj FromObj>
    constexpr DynamicStorageView& operator=(
        this DynamicStorageView& self,
        DynamicStorageView<FromObj> const& from_view);
};

template <meta::IsNonConstObj Obj>
using ConstructingMemToObjView =
    StorageView<StorageSemantics::ConstructingMemToObj, Obj>;

template <meta::IsNonConstObj Obj>
using DestructingObjToMemView =
    StorageView<StorageSemantics::DestructingObjToMem, Obj>;

template <meta::IsNonConstObj Obj>
using LeftObjView = StorageView<StorageSemantics::LeftObj, Obj>;

template <meta::IsNonConstObj Obj>
using ConstLeftObjView = StorageView<StorageSemantics::ConstLeftObj, Obj>;

template <meta::IsNonConstObj Obj>
using RightObjView = StorageView<StorageSemantics::RightObj, Obj>;

template <meta::IsNonConstObj Obj>
using ConstRightObjView = StorageView<StorageSemantics::ConstRightObj, Obj>;

template <typename T, meta::IsNonConstObj Obj>
using ForwardStorageView = StorageView<ForwardStorageSemantics<T>, Obj>;

namespace detail {

template <typename T, meta::IsObj TargetObj>
struct IsStorageViewImpl_ {
    static constexpr bool value{ false };
};

template <StorageSemantics Semantics, meta::IsObj Obj, meta::IsObj TargetObj>
struct IsStorageViewImpl_<StorageView<Semantics, Obj>, TargetObj> {
    static constexpr bool value{ meta::IsMatched<Obj, TargetObj> };
};

}  // namespace detail

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsStorageView = detail::IsStorageViewImpl_<T, TargetObj>::value;

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsDstStorageView =
    IsStorageView<T, TargetObj> && IsDstStorageSemantics<T::semantics>;

template <typename T, typename TargetObj = meta::AlwaysMatchedTag>
concept IsSrcStorageView =
    IsStorageView<T, TargetObj> && IsSrcStorageSemantics<T::semantics>;

template <typename T>
constexpr LeftObjView<T> MakeStorageView(T& obj);

template <typename T>
constexpr ConstLeftObjView<T> MakeStorageView(T const& obj);

template <typename T>
constexpr RightObjView<T> MakeStorageView(T&& obj);

template <typename T>
constexpr ConstRightObjView<T> MakeStorageView(T const&& obj);

#define ZETA_Core_Lifecycle_RelocConstructTag_Name \
    zeta_core_lifecycle_reloc_construct_tag

#define ZETA_Core_Lifecycle_RelocConstructTag_Declare \
    using ZETA_Core_Lifecycle_RelocConstructTag_Name = void;

#define ZETA_Core_Lifecycle_RelocAssignTag_Name \
    zeta_core_lifecycle_reloc_assign_tag

#define ZETA_Core_Lifecycle_RelocAssignTag_Declare \
    using ZETA_Core_Lifecycle_RelocAssignTag_Name = void;

template <meta::IsObj DstObj, typename SrcObj>
constexpr void DefaultStorageRelocConstruct(DstObj* dst, SrcObj&& src);

template <typename DstObj, typename SrcObj>
constexpr void DefaultStorageRelocAssign(DstObj&& dst, SrcObj&& src);

template <IsDstStorageView DstStorageView, IsSrcStorageView SrcStorageView>
constexpr void StorageTransfer(DstStorageView dst, SrcStorageView src);

enum struct StorageLifeState : unsigned char {
    Mem = 0,
    Obj = 1,
};

}  // namespace zeta::core::lifecycle
