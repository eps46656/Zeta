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

template <typename Elem, typename... Args>
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

template <typename Obj>
constexpr void lifecycle::InvokeDestructor(Obj& obj) {
    obj.~Obj();
}

template <typename Elem>
constexpr void lifecycle::InvokeLinSeqDestructor(Elem* data, size_t elem_stride,
                                                 size_t elem_cnt) {
    lin_seq_utils::Check(data, elem_stride, elem_cnt);

    if (elem_cnt == 0) { return; }

    for (size_t i{ elem_stride == 0 ? 1 : elem_cnt }; 0 < i--;
         data = utils::PtrInc(data, elem_stride)) {
        (InvokeDestructor)(*data);
    }
}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
constexpr lifecycle::StorageView<Semantics, Obj>::StorageView(Obj* ptr)
    : ptr{ ptr } {}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
template <meta::IsObj FromObj>
constexpr lifecycle::StorageView<Semantics, Obj>::StorageView(FromObj* from_ptr)
    : ptr{ from_ptr } {}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
template <lifecycle::StorageSemantics FromSemantics>
constexpr lifecycle::StorageView<Semantics, Obj>::StorageView(
    StorageView<FromSemantics, Obj> const& from_view)
    requires((IsStorageSemanticsConvertible)(FromSemantics, semantics))
    : ptr{ from_view.ptr } {}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
constexpr lifecycle::StorageView<Semantics, Obj>::StorageView(
    DynamicStorageView<Obj> const& from_view)
    : ptr{ from_view.ptr } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(from_view.semantics, semantics));
}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
constexpr decltype(auto) lifecycle::StorageView<Semantics, Obj>::GetRef(
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

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
template <lifecycle::StorageSemantics ToSemantics>
constexpr lifecycle::StorageView<ToSemantics, Obj>
lifecycle::StorageView<Semantics, Obj>::ToStatic(this StorageView const& self)
    requires((IsStorageSemanticsConvertible)(semantics, ToSemantics))
{
    return { self.ptr };
}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
constexpr lifecycle::DynamicStorageView<Obj>
lifecycle::StorageView<Semantics, Obj>::ToDynamic(
    this StorageView const& self) {
    return { semantics, self.ptr };
}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
template <lifecycle::StorageSemantics FromSemantics>
constexpr lifecycle::StorageView<Semantics, Obj>&
lifecycle::StorageView<Semantics, Obj>::operator=(
    this StorageView const& self,
    StorageView<FromSemantics, Obj> const& from_view)
    requires((IsStorageSemanticsConvertible)(FromSemantics, semantics))
{
    self.ptr = from_view.ptr;

    return self;
}

template <lifecycle::StorageSemantics Semantics, meta::IsNonConstObj Obj>
constexpr lifecycle::StorageView<Semantics, Obj>&
lifecycle::StorageView<Semantics, Obj>::operator=(
    this StorageView const& self, DynamicStorageView<Obj> const& from_view) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(from_view.semantics, semantics));

    self.ptr = from_view.ptr;

    return self;
}

template <meta::IsObj Obj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageSemantics semantics, Obj* ptr)
    : semantics{ semantics }, ptr{ ptr } {}

template <meta::IsObj Obj>
template <meta::IsObj FromObj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageSemantics semantics, FromObj* from_ptr)
    : semantics{ semantics }, ptr{ from_ptr } {}

template <meta::IsObj Obj>
template <lifecycle::StorageSemantics FromSemantics, meta::IsObj FromObj>
constexpr lifecycle::DynamicStorageView<Obj>::DynamicStorageView(
    StorageView<FromSemantics, FromObj> const& from_view)
    : semantics{ FromSemantics },
      ptr{ const_cast<meta::RemoveConst<FromObj>*>(from_view.ptr) } {}

template <meta::IsObj Obj>
template <lifecycle::StorageSemantics Semantics>
constexpr lifecycle::StorageView<Semantics, Obj>
lifecycle::DynamicStorageView<Obj>::ToStatic(
    this DynamicStorageView const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (IsStorageSemanticsConvertible)(self.semantics, Semantics));

    return { self.ptr };
}

template <meta::IsObj Obj>
template <lifecycle::StorageSemantics FromSemantics, meta::IsObj FromObj>
constexpr lifecycle::DynamicStorageView<Obj>&
lifecycle::DynamicStorageView<Obj>::operator=(
    this DynamicStorageView& self,
    StorageView<FromSemantics, Obj> const& from_view) {
    self.semantics = FromSemantics;
    self.ptr       = const_cast<meta::RemoveConst<FromObj>*>(from_view.ptr);

    return self;
}

template <meta::IsObj Obj>
template <meta::IsObj FromObj>
constexpr lifecycle::DynamicStorageView<Obj>&
lifecycle::DynamicStorageView<Obj>::operator=(
    this DynamicStorageView& self,
    DynamicStorageView<FromObj> const& from_view) {
    self.semantics = from_view.semantics;
    self.ptr       = from_view.ptr;

    return self;
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

template <meta::IsObj DstObj, typename SrcObj>
constexpr void lifecycle::DefaultStorageRelocConstruct(DstObj* dst,
                                                       SrcObj&& src) {
    new (dst) DstObj{ meta::Forward<SrcObj>(src) };
    (InvokeDestructor)(src);
}

template <typename DstObj, typename SrcObj>
constexpr void lifecycle::DefaultStorageRelocAssign(DstObj&& dst,
                                                    SrcObj&& src) {
    meta::Forward<DstObj>(dst) = meta::Forward<SrcObj>(src);
    (InvokeDestructor)(src);
}

template <lifecycle::IsDstStorageView DstStorageView,
          lifecycle::IsSrcStorageView SrcStorageView>
constexpr void lifecycle::StorageTransfer(DstStorageView dst,
                                          SrcStorageView src) {
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
        dst.GetRef() = src.GetRef();
    } else if constexpr (dst_semantics ==
                             StorageSemantics::ConstructingMemToObj &&
                         src_semantics ==
                             StorageSemantics::DestructingObjToMem) {
        if constexpr (requires {
                          DstObj::ZETA_Core_Lifecycle_RelocConstructTag_Name;
                      }) {
            new (dst.ptr) DstObj{ src };
        } else {
            (DefaultStorageRelocConstruct)(dst.ptr, src.GetRef());
        }
    } else if constexpr (dst_semantics ==
                             StorageSemantics::ConstructingMemToObj &&
                         src_is_obj) {
        new (dst.ptr) DstObj{ src.GetRef() };
    } else if constexpr (dst_is_obj &&
                         src_semantics ==
                             StorageSemantics::DestructingObjToMem) {
        if constexpr (requires {
                          DstObj::ZETA_Core_Lifecycle_RelocAssignTag_Name;
                      }) {
            dst.GetRef() = src;
        } else {
            (DefaultStorageRelocAssign)(dst.GetRef(), src.GetRef());
        }
    } else {
        static_assert(false);
    }
}

}  // namespace zeta::core
