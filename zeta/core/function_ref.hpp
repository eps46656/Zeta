#pragma once

namespace zeta::core::function_ref {

struct Kind {
    static constexpr char Func{ 1 };
    static constexpr char ContextualFunc{ 2 };
    static constexpr char ConstContextualFunc{ 3 };
};

template <typename Sig>
struct Ref;

template <typename Ret, typename... Args>
struct Ref<Ret(Args...)> {
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

    Ref() = default;

    Ref(Ref const&) = default;

    Ref(Ref&&) = default;

    Ref  // NOLINT(
         // google-explicit-constructor,
         // hicpp-explicit-conversions)
        (Ret (*func)(Args... args));

    Ref(void* context, Ret (*contextual_func)(void* context, Args... args));

    Ref(void const* context,
        Ret (*contextual_func)(void const* context, Args... args));

    template <typename Callable>
    Ref  // NOLINT(
         // google-explicit-constructor,
         // hicpp-explicit-conversions)
        (Callable& callable);

    template <typename Callable>
    Ref  // NOLINT(
         // google-explicit-constructor,
         // hicpp-explicit-conversions)
        (Callable const& callable);

    template <typename Callable>
    Ref(Callable&& callable) = delete;

    ~Ref() = default;

    Ref& operator=(Ref const&) = default;

    Ref& operator=(Ref&&) = default;

    Ret operator()(Args... args) const;
};

}  // namespace zeta::core::function_ref
