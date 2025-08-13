#include <stdio.h>
#include <stdlib.h>
#include <zeta/core/define.h>

#include <list>
#include <unordered_map>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct CoroutineBlock {
    unsigned long long regs[8];
};

typedef
    __attribute__((sysv_abi, noreturn)) void (*CoroutineFunc)(void* context);

extern "C" __attribute__((sysv_abi)) void Coroutine_SetFunc_Core(
    CoroutineBlock* cor_blk, void* cor_func_context, CoroutineFunc cor_func,
    void* stack_top);

extern "C" __attribute__((sysv_abi, noreturn)) void Coroutine_Jump_Core(
    CoroutineBlock* dst_cor_blk, int code);

extern "C" __attribute__((sysv_abi)) int Coroutine_Switch_Core(
    CoroutineBlock* src_cor_blk, CoroutineBlock* dst_cor_blk, int code);

void Coroutine_SetFunc(CoroutineBlock* cor_blk, void* cor_func_conetxt,
                       CoroutineFunc cor_func, void* stack_top) {
    Coroutine_SetFunc_Core(cor_blk, cor_func_conetxt, cor_func, stack_top);
}

__attribute__((noreturn)) void Coroutine_Jump(CoroutineBlock* dst_cor_blk,
                                              int code) {
    Coroutine_Jump_Core(dst_cor_blk, code);
}

int Coroutine_Switch(CoroutineBlock* src_cor_blk, CoroutineBlock* dst_cor_blk,
                     int code) {
    return Coroutine_Switch_Core(src_cor_blk, dst_cor_blk, code);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

constexpr size_t stack_size{ 16 * 1024 * 1024 };

void* AllocateStack() {
    void* stack{ std::malloc(stack_size) };
    return (unsigned char*)stack + (stack_size - 1024 * 1024);
}

void DeallocateStack(void* stack) {
    std::free((unsigned char*)stack - (stack_size - 1024 * 1024));
}

void* f_stack{ AllocateStack() };
void* g_stack{ AllocateStack() };
void* h_stack{ AllocateStack() };

CoroutineBlock main_cor_blk;
CoroutineBlock f_cor_blk;
CoroutineBlock g_cor_blk;
CoroutineBlock h_cor_blk;

bool f_is_finished{ false };
bool g_is_finished{ false };
bool h_is_finished{ false };

__attribute__((sysv_abi, noreturn)) void FFunc(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("FFunc: %d\n", i);

        if (!g_is_finished) {
            Coroutine_Switch(&f_cor_blk, &g_cor_blk, 0);
            continue;
        }

        if (!h_is_finished) {
            Coroutine_Switch(&f_cor_blk, &h_cor_blk, 0);
            continue;
        }
    }

    f_is_finished = true;

    if (!g_is_finished) { Coroutine_Jump(&g_cor_blk, 0); }

    if (!h_is_finished) { Coroutine_Jump(&h_cor_blk, 0); }

    Coroutine_Jump(&main_cor_blk, 0);
}

__attribute__((sysv_abi, noreturn)) void GFunc(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("GFunc: %d\n", i);

        if (!h_is_finished) {
            Coroutine_Switch(&g_cor_blk, &h_cor_blk, 0);
            continue;
        }

        if (!f_is_finished) {
            Coroutine_Switch(&g_cor_blk, &f_cor_blk, 0);
            continue;
        }
    }

    g_is_finished = true;

    if (!h_is_finished) { Coroutine_Jump(&h_cor_blk, 0); }

    if (!f_is_finished) { Coroutine_Jump(&f_cor_blk, 0); }

    Coroutine_Jump(&main_cor_blk, 0);
}

__attribute__((sysv_abi, noreturn)) void HFunc(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("HFunc: %d\n", i);

        if (!f_is_finished) {
            Coroutine_Switch(&h_cor_blk, &f_cor_blk, 0);
            continue;
        }

        if (!g_is_finished) {
            Coroutine_Switch(&h_cor_blk, &g_cor_blk, 0);
            continue;
        }
    }

    f_is_finished = true;

    if (!g_is_finished) { Coroutine_Jump(&f_cor_blk, 0); }

    if (!h_is_finished) { Coroutine_Jump(&g_cor_blk, 0); }

    Coroutine_Jump(&main_cor_blk, 0);
}

void main1() {
    Zeta_Coroutine_SetFunc(&f_cor_blk, nullptr, FFunc, f_stack);
    Coroutine_SetFunc(&g_cor_blk, nullptr, GFunc, g_stack);
    Coroutine_SetFunc(&h_cor_blk, nullptr, HFunc, h_stack);

    ZETA_Core_PrintCurPos;

    Coroutine_Switch(&main_cor_blk, &f_cor_blk, 0);

    ZETA_Core_PrintCurPos;

    return;
}

int main() {
    main1();
    printf("ok\n");
    return 0;
};
