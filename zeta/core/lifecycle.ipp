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

template <meta::IsNonConstObjectType Obj, typename... Args>
constexpr void lifecycle::InvokeConstructor(Obj* obj, Args&&... args) {
    new (obj) Obj{ meta::Forward<Args>(args)... };
}

template <meta::IsNonConstObjectType Elem, typename... Args>
constexpr void lifecycle::InvokeLinSeqConstructor(Elem* data,
                                                  size_t elem_stride,
                                                  size_t elem_cnt,
                                                  Args&&... args) {
    lin_seq_utils::Check(data, elem_stride, elem_cnt);

    if (elem_cnt == 0) { return; }

    for (size_t i{ elem_stride == 0 ? 1 : elem_cnt }; 0 < i--;
         data = utils::PtrInc(data, elem_stride)) {
        (InvokeConstructor)(data, meta::Forward<Args>(args)...);
    }
}

template <meta::IsNonConstObjectType Obj>
constexpr void lifecycle::InvokeDestructor(Obj& obj) {
    obj.~Obj();
}

template <meta::IsNonConstObjectType Elem>
constexpr void lifecycle::InvokeLinSeqDestructor(Elem* data, size_t elem_stride,
                                                 size_t elem_cnt) {
    lin_seq_utils::Check(data, elem_stride, elem_cnt);

    if (elem_cnt == 0) { return; }

    for (size_t i{ elem_stride == 0 ? 1 : elem_cnt }; 0 < i--;
         data = utils::PtrInc(data, elem_stride)) {
        (InvokeDestructor)(*data);
    }
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr lifecycle::StorageView<semantics, Obj>::StorageView(Obj* ptr)
    : ptr{ ptr } {
    this->AssertIsValid();
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
template <meta::IsObjectType FromObj>
constexpr lifecycle::StorageView<semantics, Obj>::StorageView(FromObj* from_ptr)
    : ptr{ from_ptr } {
    this->AssertIsValid();
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics from_semantics>
constexpr lifecycle::StorageView<semantics, Obj>::StorageView(
    StorageView<from_semantics, Obj> const& from_view)
    requires((IsStorageSemanticsConvertible)(from_semantics, semantics))
    : ptr{ from_view.ptr } {
    from_view.AssertIsValid();
    this->AssertIsValid();
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr lifecycle::StorageView<semantics, Obj>::StorageView(
    DynamicStorageView<Obj> const& from_view)
    : ptr{ from_view.ptr } {
    from_view.AssertIsValid();
    this->AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(from_view.semantics, semantics));
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr bool lifecycle::StorageView<semantics, Obj>::IsNull() {
    return false;
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr decltype(auto) lifecycle::StorageView<semantics, Obj>::GetRef(
    this StorageView const& self)
    requires(StorageView::semantics ==
                 lifecycle::StorageSemantics::DestructingObjToMem ||
             StorageView::semantics == lifecycle::StorageSemantics::LeftObj ||
             StorageView::semantics ==
                 lifecycle::StorageSemantics::ConstLeftObj ||
             StorageView::semantics == lifecycle::StorageSemantics::RightObj ||
             StorageView::semantics ==
                 lifecycle::StorageSemantics::ConstRightObj)
{
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.ptr != nullptr);

    if constexpr (semantics == StorageSemantics::DestructingObjToMem) {
        return static_cast<Obj&&>(*self.ptr);
    } else if constexpr (semantics == StorageSemantics::LeftObj) {
        return static_cast<Obj&>(*self.ptr);
    } else if constexpr (semantics == StorageSemantics::ConstLeftObj) {
        return static_cast<Obj const&>(*self.ptr);
    } else if constexpr (semantics == StorageSemantics::RightObj) {
        return static_cast<Obj&&>(*self.ptr);
    } else if constexpr (semantics == StorageSemantics::ConstRightObj) {
        return static_cast<Obj const&&>(*self.ptr);
    } else {
        static_assert(false);
    }
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics to_semantics>
constexpr lifecycle::StorageView<to_semantics, Obj>
lifecycle::StorageView<semantics, Obj>::ToStatic(this StorageView const& self)
    requires((IsStorageSemanticsConvertible)(semantics, to_semantics))
{
    return { self.ptr };
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr lifecycle::DynamicStorageView<Obj>
lifecycle::StorageView<semantics, Obj>::ToDynamic(
    this StorageView const& self) {
    return { semantics, self.ptr };
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics from_semantics>
constexpr lifecycle::StorageView<semantics, Obj>&
lifecycle::StorageView<semantics, Obj>::operator=(
    this StorageView const& self,
    StorageView<from_semantics, Obj> const& from_view)
    requires((IsStorageSemanticsConvertible)(from_semantics, semantics))
{
    self.ptr = from_view.ptr;

    return self;
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr lifecycle::StorageView<semantics, Obj>&
lifecycle::StorageView<semantics, Obj>::operator=(
    this StorageView const& self, DynamicStorageView<Obj> const& from_view) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(from_view.semantics, semantics));

    self.ptr = from_view.ptr;

    return self;
}

template <lifecycle::StorageSemantics semantics, meta::IsNonConstObjectType Obj>
constexpr void lifecycle::StorageView<semantics, Obj>::AssertIsValid() {}

template <meta::IsNonConstObjectType Obj>
constexpr lifecycle::StorageView<lifecycle::StorageSemantics::Null, Obj>::
    StorageView(DynamicStorageView<Obj> const& from_view) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(from_view.semantics ==
                                            lifecycle::StorageSemantics::Null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(from_view.ptr == nullptr);
}

template <meta::IsNonConstObjectType Obj>
constexpr bool
lifecycle::StorageView<lifecycle::StorageSemantics::Null, Obj>::IsNull() {
    return true;
}

template <meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics to_semantics>
constexpr lifecycle::StorageView<to_semantics, Obj>
lifecycle::StorageView<lifecycle::StorageSemantics::Null, Obj>::ToStatic()
    requires(to_semantics == lifecycle::StorageSemantics::Null)
{
    return {};
}

template <meta::IsNonConstObjectType Obj>
constexpr lifecycle::DynamicStorageView<Obj>
lifecycle::StorageView<lifecycle::StorageSemantics::Null, Obj>::ToDynamic() {
    return { lifecycle::StorageSemantics::Null, nullptr };
}

template <meta::IsNonConstObjectType Obj>
constexpr void
lifecycle::StorageView<lifecycle::StorageSemantics::Null, Obj>::AssertIsValid(
    this StorageView const&) {}

template <meta::IsNonConstObjectType Obj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageSemantics semantics, Obj* ptr)
    : semantics{ semantics }, ptr{ ptr } {
    this->AssertIsValid();
}

template <meta::IsNonConstObjectType Obj>
template <meta::IsObjectType FromObj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageSemantics semantics, FromObj* from_ptr)
    : semantics{ semantics }, ptr{ from_ptr } {
    this->AssertIsValid();
}

template <meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics from_semantics,
          meta::IsObjectType FromObj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageView<from_semantics, FromObj> const& from_view)
    : semantics{ from_semantics },
      ptr{ const_cast<meta::RemoveConst<FromObj>*>(from_view.ptr) } {
    from_view.AssertIsValid();
    this->AssertIsValid();
}

template <meta::IsNonConstObjectType Obj>
constexpr bool lifecycle::DynamicStorageView<Obj>::IsNull(
    this DynamicStorageView const& self) {
    self.AssertIsValid();
    return self.semantics == StorageSemantics::Null;
}

template <meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics semantics>
constexpr lifecycle::StorageView<semantics, Obj>
lifecycle::DynamicStorageView<Obj>::ToStatic(
    this DynamicStorageView const& self) {
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(self.semantics, semantics));

    if constexpr (semantics == lifecycle::StorageSemantics::Null) {
        return {};
    } else {
        return { self.ptr };
    }
}

template <meta::IsNonConstObjectType Obj>
template <lifecycle::StorageSemantics from_semantics,
          meta::IsNonConstObjectType FromObj>
constexpr lifecycle::DynamicStorageView<Obj>&
lifecycle::DynamicStorageView<Obj>::operator=(
    this DynamicStorageView& self,
    StorageView<from_semantics, FromObj> const& from_view) {
    self.AssertIsValid();
    from_view.AssertIsValid();

    self.semantics = from_semantics;
    self.ptr       = const_cast<meta::RemoveConst<FromObj>*>(from_view.ptr);

    return self;
}

template <meta::IsNonConstObjectType Obj>
template <meta::IsNonConstObjectType FromObj>
constexpr lifecycle::DynamicStorageView<Obj>&
lifecycle::DynamicStorageView<Obj>::operator=(
    this DynamicStorageView& self,
    DynamicStorageView<FromObj> const& from_view) {
    self.AssertIsValid();
    from_view.AssertIsValid();

    self.semantics = from_view.semantics;
    self.ptr       = from_view.ptr;

    return self;
}

template <meta::IsNonConstObjectType Obj>
constexpr void lifecycle::DynamicStorageView<Obj>::AssertIsValid(
    this DynamicStorageView const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemantics)(self.semantics));
}

template <typename T>
constexpr lifecycle::LeftObjView<T> lifecycle::MakeStorageView(T& obj) {
    return { &obj };
}

template <typename T>
constexpr lifecycle::ConstLeftObjView<T> lifecycle::MakeStorageView(
    T const& obj) {
    return { &obj };
}

template <typename T>
constexpr lifecycle::RightObjView<T> lifecycle::MakeStorageView(T&& obj) {
    return { &obj };
}

template <typename T>
constexpr lifecycle::ConstRightObjView<T> lifecycle::MakeStorageView(
    T const&& obj) {
    return { &obj };
}

template <lifecycle::IsDstStorageView DstStorageView,
          lifecycle::IsSrcStorageView SrcStorageView>
constexpr void lifecycle::StorageTransfer(DstStorageView dst,
                                          SrcStorageView src) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst.ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src.ptr != nullptr);

    constexpr StorageSemantics dst_semantics{ DstStorageView::semantics };
    constexpr StorageSemantics src_semantics{ SrcStorageView::semantics };

    using DstObj = typename DstStorageView::Obj;

    constexpr bool dst_is_obj{ dst_semantics == StorageSemantics::LeftObj ||
                               dst_semantics == StorageSemantics::RightObj };

    constexpr bool src_is_obj{
        src_semantics == StorageSemantics::LeftObj ||
        src_semantics == StorageSemantics::ConstLeftObj ||
        src_semantics == StorageSemantics::RightObj ||
        src_semantics == StorageSemantics::ConstRightObj
    };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst.ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src.ptr != nullptr);

    if constexpr (dst_is_obj && src_is_obj) {
        auto&& dst_ref{ dst.GetRef() };
        auto&& src_ref{ src.GetRef() };

        dst_ref = src_ref;
    } else if constexpr (dst_semantics ==
                             StorageSemantics::ConstructingMemToObj &&
                         src_semantics ==
                             StorageSemantics::DestructingObjToMem) {
        new (dst.ptr) DstObj{ src.GetRef() };
        (InvokeDestructor)(*src.ptr);
    } else if constexpr (dst_semantics ==
                             StorageSemantics::ConstructingMemToObj &&
                         src_is_obj) {
        new (dst.ptr) DstObj{ src.GetRef() };
    } else if constexpr (dst_is_obj &&
                         src_semantics ==
                             StorageSemantics::DestructingObjToMem) {
        auto&& dst_ref{ dst.GetRef() };
        auto&& src_ref{ src.GetRef() };

        dst_ref = src_ref;

        (InvokeDestructor)(*src.ptr);
    } else {
        static_assert(false);
    }
}

template <lifecycle::IsDstStorageView DstStorageView,
          meta::IsNonConstObjectType SrcObj>
constexpr void lifecycle::StorageTransfer(DstStorageView dst,
                                          DynamicStorageView<SrcObj> src) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsSrcStorageSemantics)(src.semantics));

#pragma push_macro("Case")
#define Case(semantics_name)                                                 \
    case StorageSemantics::semantics_name:                                   \
        (StorageTransfer)(                                                   \
            dst, src.template ToStatic<StorageSemantics::semantics_name>()); \
        break;                                                               \
        static_assert(true)

    switch (src.semantics) {
        Case(DestructingObjToMem);
        Case(LeftObj);
        Case(ConstLeftObj);
        Case(RightObj);
        Case(ConstRightObj);

    case StorageSemantics::Null:
    case StorageSemantics::ConstructingMemToObj:
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

#pragma pop_macro("Case")
}

template <meta::IsNonConstObjectType DstObj,
          lifecycle::IsSrcStorageView SrcStorageView>
constexpr void lifecycle::StorageTransfer(DynamicStorageView<DstObj> dst,
                                          SrcStorageView src) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsDstStorageSemantics)(dst.semantics));

#pragma push_macro("Case")
#define Case(semantics_name)                                                 \
    case StorageSemantics::semantics_name:                                   \
        (StorageTransfer)(                                                   \
            dst.template ToStatic<StorageSemantics::semantics_name>(), src); \
        break;                                                               \
        static_assert(true)

    switch (dst.semantics) {
        Case(ConstructingMemToObj);
        Case(LeftObj);
        Case(RightObj);

    case StorageSemantics::Null:
    case StorageSemantics::DestructingObjToMem:
    case StorageSemantics::ConstLeftObj:
    case StorageSemantics::ConstRightObj:
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

#pragma pop_macro("Case")
}

template <meta::IsNonConstObjectType DstObj, meta::IsNonConstObjectType SrcObj>
constexpr void lifecycle::StorageTransfer(DynamicStorageView<DstObj> dst,
                                          DynamicStorageView<SrcObj> src) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsDstStorageSemantics)(dst.semantics));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsSrcStorageSemantics)(src.semantics));

    static_assert(
        integral::WidthOf<meta::GetEnumUnderlyingType<StorageSemantics>> <= 8);

    static_assert(16 <= integral::WidthOf<unsigned>);

#pragma push_macro("MakeSemanticsPair")
#define MakeSemanticsPair(dst_semantics_name, src_semantics_name)             \
    (static_cast<unsigned>(meta::EnumToUnderlying(dst_semantics_name)) << 8 | \
     static_cast<unsigned>(meta::EnumToUnderlying(src_semantics_name)))

#pragma push_macro("Case")
#define Case(dst_semantics_name, src_semantics_name)                        \
    case MakeSemanticsPair(StorageSemantics::dst_semantics_name,            \
                           StorageSemantics::src_semantics_name):           \
        (StorageTransfer)(                                                  \
            dst.template ToStatic<StorageSemantics::dst_semantics_name>(),  \
            src.template ToStatic<StorageSemantics::src_semantics_name>()); \
        break;                                                              \
        static_assert(true)

    switch (MakeSemanticsPair(dst.semantics, src.semantics)) {
        Case(ConstructingMemToObj, DestructingObjToMem);
        Case(ConstructingMemToObj, LeftObj);
        Case(ConstructingMemToObj, ConstLeftObj);
        Case(ConstructingMemToObj, RightObj);
        Case(ConstructingMemToObj, ConstRightObj);

        Case(LeftObj, DestructingObjToMem);
        Case(LeftObj, LeftObj);
        Case(LeftObj, ConstLeftObj);
        Case(LeftObj, RightObj);
        Case(LeftObj, ConstRightObj);

        Case(RightObj, DestructingObjToMem);
        Case(RightObj, LeftObj);
        Case(RightObj, ConstLeftObj);
        Case(RightObj, RightObj);
        Case(RightObj, ConstRightObj);

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

#pragma pop_macro("Case")
#pragma pop_macro("MakeSemanticsPair")
}

}  // namespace zeta::core
