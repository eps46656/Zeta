
#include <zeta/core/debugger.h>
#include <zeta/core/jump.h>

#include <algorithm>
#include <iostream>
#include <random>

std::mt19937_64 en;

Zeta_JumpBlock main_jmp_blk;

Zeta_JumpBlock g_jmp_blk;
void* g_stack;

void F(Zeta_JumpBlock* jmp_blk) {
    ZETA_Core_PrintCurPos;

    int a = 5;

    Zeta_Jump(jmp_blk, 2);

    ZETA_Core_PrintVar(a);
}

void F_Frame(Zeta_JumpBlock* jmp_blk) {
    ZETA_Core_PrintCurPos;
    F(jmp_blk);
    ZETA_Core_PrintCurPos;
}

#define PrintStack                                     \
    {                                                  \
        char c;                                        \
        unsigned long long stack = ZETA_PtrToAddr(&c); \
        ZETA_Core_PrintVar(stack);                     \
    }

#define STACK_SIZE (4ULL * 1024 * 1024)

void* AllocateStack(size_t stack_size) {
    stack_size = std::max<size_t>(stack_size, 4 * 1024);

    void* stack = (unsigned char*)malloc(stack_size) + stack_size - 32;

    ZETA_Core_DebugAssert(__builtin_is_aligned(stack, 16));

    return stack;
}

__attribute__((sysv_abi)) void G_Entry() {
    unsigned long long rc = Zeta_SetJump(&g_jmp_blk);

    ZETA_Core_PrintVar(rc);

    Zeta_Jump(&main_jmp_blk, rc + 1);
}

void main1() {
    unsigned long long rc = Zeta_SetJump(&main_jmp_blk);

    ZETA_Core_PrintVar(main_jmp_blk.regs[0]);

    if (rc == 0) {
        ZETA_Core_PrintCurPos;

        g_stack = AllocateStack(STACK_SIZE);

        ZETA_Core_PrintCurPos;
        ZETA_Core_PrintVar((unsigned long long)(G_Entry));

        Zeta_SetJumpEntry(&g_jmp_blk, G_Entry, g_stack);

        ZETA_Core_PrintCurPos;

        Zeta_Jump(&g_jmp_blk, 1);
    }

    if (80 <= rc) { return; }

    ZETA_Core_PrintVar(rc);

    Zeta_Jump(&g_jmp_blk, rc + 1);

    PrintStack;
}

int main() {
    main1();
    printf("ok\n");
    return 0;
}
