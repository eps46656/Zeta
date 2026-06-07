#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Ret, typename... Args>
function_ref::Ref<Ret(Args...)>::Ref(Ret (*func)(Args... args))
    : func{ func }, kind{ Kind::Func } {}

template <typename Ret, typename... Args>
function_ref::Ref<Ret(Args...)>::Ref(void* context,
                                     Ret (*contextual_func_)(void* context,
                                                             Args... args))
    : contextual_func{ context, contextual_func_ },
      kind{ Kind::ContextualFunc } {}

template <typename Ret, typename... Args>
function_ref::Ref<Ret(Args...)>::Ref(
    void const* context,
    Ret (*contextual_func_)(void const* context, Args... args))
    : const_contextual_func{ context, contextual_func_ },
      kind{ Kind::ConstContextualFunc } {}

template <typename Ret, typename... Args>
template <typename Callable>
function_ref::Ref<Ret(Args...)>::Ref(Callable& callable)
    : contextual_func {
        .context = &callable,
        .ptr = [](void* context, Args... args) -> Ret {
            return static_cast<Ret>((*static_cast<Callable*>(context))(
                meta::Forward<Args>(args)...));
        },
    },
    kind { Kind::ContextualFunc} {}

template <typename Ret, typename... Args>
template <typename Callable>
function_ref::Ref<Ret(Args...)>::Ref(Callable const& callable):
    const_contextual_func{
        .context = &callable,
        .ptr = [](void const* context, Args... args) -> Ret {
            return static_cast<Ret>((*static_cast<Callable const*>(context))(meta::Forward<Args>(args)...));
        },
    },
    kind { Kind::ConstContextualFunc } {}

template <typename Ret, typename... Args>
Ret function_ref::Ref<Ret(Args...)>::operator()(Args... args) const {
    ZETA_Core_DebugAssert(this->kind == Kind::Func ||
                          this->kind == Kind::ContextualFunc ||
                          this->kind == Kind::ConstContextualFunc);

    switch (this->kind) {
    case Kind::Func:
        ZETA_Core_DebugAssert(this->func.ptr != nullptr);
        return this->func.ptr(meta::Forward<Args>(args)...);

    case Kind::ContextualFunc:
        ZETA_Core_DebugAssert(this->contextual_func.ptr != nullptr);
        return this->contextual_func.ptr(this->contextual_func.context,
                                         meta::Forward<Args>(args)...);

    case Kind::ConstContextualFunc:
        ZETA_Core_DebugAssert(this->const_contextual_func.ptr != nullptr);
        return this->const_contextual_func.ptr(
            this->const_contextual_func.context, meta::Forward<Args>(args)...);

    default: __builtin_unreachable();
    }
}

}  // namespace zeta::core
