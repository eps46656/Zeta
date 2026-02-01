#pragma once

namespace zeta::core {

struct FunctionRefKind {
    static constexpr char Func{ 1 };
    static constexpr char ContextualFunc{ 2 };
    static constexpr char ConstContextualFunc{ 3 };
};

template <typename Sig>
struct FunctionRef;

template <typename Ret, typename... Args>
struct FunctionRef<Ret(Args...)> {
    union {
        struct {
            Ret (*ptr)(Args... args);
        } func;

        struct {
            void* context;
            Ret (*ptr)(void* context, Args... args);
        } contextual_func;

        struct {
            void const* context;
            Ret (*ptr)(void const* context, Args... args);
        } const_contextual_func;
    };

    char kind;

    // -------------------------------------------------------------------------

    FunctionRef() = default;

    FunctionRef(FunctionRef const&) = default;

    FunctionRef(FunctionRef&&) = default;

    FunctionRef(Ret (*func)(Args... args));

    FunctionRef(void* context,
                Ret (*contextual_func)(void* context, Args... args));

    FunctionRef(void const* context,
                Ret (*contextual_func)(void const* context, Args... args));

    template <typename Callable>
    FunctionRef(Callable& callable);  // NOLINT(
                                      // google-explicit-constructor,
                                      // hicpp-explicit-conversions)

    // -------------------------------------------------------------------------

    FunctionRef& operator=(FunctionRef const&) = default;

    // -------------------------------------------------------------------------

    Ret operator()(Args... args) const;
};

}  // namespace zeta::core
