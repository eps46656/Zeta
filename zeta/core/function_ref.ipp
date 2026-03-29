#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Ret, typename... Args>
FunctionRef<Ret(Args...)>::FunctionRef(Ret (*func)(Args... args))
    : func{ func }, kind{ FunctionRefKind::Func } {}

template <typename Ret, typename... Args>
FunctionRef<Ret(Args...)>::FunctionRef(void* context,
                                       Ret (*contextual_func_)(void* context,
                                                               Args... args))
    : contextual_func{ context, contextual_func_ },
      kind{ FunctionRefKind::ContextualFunc } {}

template <typename Ret, typename... Args>
FunctionRef<Ret(Args...)>::FunctionRef(
    void const* context,
    Ret (*contextual_func_)(void const* context, Args... args))
    : const_contextual_func{ context, contextual_func_ },
      kind{ FunctionRefKind::ConstContextualFunc } {}

template <typename Ret, typename... Args>
template <typename Callable>
FunctionRef<Ret(Args...)>::FunctionRef(Callable& callable)
    : contextual_func {
        .context = &callable,
        .ptr = [](void* context, Args... args) -> Ret {
            return static_cast<Ret>((*static_cast<Callable*>(context))(
                meta::Forward<Args>(args)...));
        },
    },
    kind { FunctionRefKind::ContextualFunc} {}

template <typename Ret, typename... Args>
template <typename Callable>
FunctionRef<Ret(Args...)>::FunctionRef(Callable const& callable):
    const_contextual_func{
        .context = &callable,
            .ptr = [](void const* context, Args... args) -> Ret {
                return static_cast<Ret>((*static_cast<Callable*>(context))(
                    meta::Forward<Args>(args)...));
            },
    },
    kind { FunctionRefKind::ConstContextualFunc } {}

template <typename Ret, typename... Args>
Ret FunctionRef<Ret(Args...)>::operator()(Args... args) const {
    ZETA_Core_DebugAssert(this->kind == FunctionRefKind::Func ||
                          this->kind == FunctionRefKind::ContextualFunc ||
                          this->kind == FunctionRefKind::ConstContextualFunc);

    switch (this->kind) {
    case FunctionRefKind::Func:
        ZETA_Core_DebugAssert(this->func.ptr != nullptr);
        return this->func.ptr(meta::Forward<Args>(args)...);

    case FunctionRefKind::ContextualFunc:
        ZETA_Core_DebugAssert(this->contextual_func.ptr != nullptr);
        return this->contextual_func.ptr(this->contextual_func.context,
                                         meta::Forward<Args>(args)...);

    case FunctionRefKind::ConstContextualFunc:
        ZETA_Core_DebugAssert(this->const_contextual_func.ptr != nullptr);
        return this->const_contextual_func.ptr(
            this->const_contextual_func.context, meta::Forward<Args>(args)...);

    default: __builtin_unreachable();
    }
}

}  // namespace zeta::core
