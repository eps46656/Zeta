#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_JumpBlock);

#if defined(__x86_64)

struct Zeta_Core_JumpBlock {
    unsigned long long regs[8];
};

#endif

typedef __attribute__((sysv_abi)) void (*Zeta_Core_JumpEntry)();

__attribute__((sysv_abi)) unsigned long long Zeta_Core_SetJump(
    Zeta_Core_JumpBlock* jmp_blk);

__attribute__((sysv_abi, noreturn)) unsigned long long Zeta_Core_Jump(
    Zeta_Core_JumpBlock* jmp_blk, unsigned code);

__attribute__((sysv_abi)) void Zeta_Core_SetJumpEntry(
    Zeta_Core_JumpBlock* jmp_blk, Zeta_Core_JumpEntry jmp_entry,
    void* stack_top);

ZETA_Core_ExternC_End;
