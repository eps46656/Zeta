.intel_syntax

.equ Zeta_JumpBlock__rip, 0
.equ Zeta_JumpBlock__rbx, 8
.equ Zeta_JumpBlock__rsp, 16
.equ Zeta_JumpBlock__rbp, 24
.equ Zeta_JumpBlock__r12, 32
.equ Zeta_JumpBlock__r13, 40
.equ Zeta_JumpBlock__r14, 48
.equ Zeta_JumpBlock__r15, 56

.text

.globl Zeta_SetJump
Zeta_SetJump:
    # rdi jmp_blk

    mov rax, 0

    mov r8, [rsp]
    mov [rdi + Zeta_JumpBlock__rip], r8

    mov [rdi + Zeta_JumpBlock__rbx], rbx

    mov r8, rsp
    add r8, 8
    mov [rdi + Zeta_JumpBlock__rsp], r8

    mov [rdi + Zeta_JumpBlock__rbp], rbp
    mov [rdi + Zeta_JumpBlock__r12], r12
    mov [rdi + Zeta_JumpBlock__r13], r13
    mov [rdi + Zeta_JumpBlock__r14], r14
    mov [rdi + Zeta_JumpBlock__r15], r15

    ret

.globl Zeta_Jump
Zeta_Jump:
    # rdi jmp_blk
    # rsi code

    # ret = code
    mov rax, rsi

    mov r8, [rdi + Zeta_JumpBlock__rip]

    mov rbx, [rdi + Zeta_JumpBlock__rbx]
    mov rsp, [rdi + Zeta_JumpBlock__rsp]
    mov rbp, [rdi + Zeta_JumpBlock__rbp]
    mov r12, [rdi + Zeta_JumpBlock__r12]
    mov r13, [rdi + Zeta_JumpBlock__r13]
    mov r14, [rdi + Zeta_JumpBlock__r14]
    mov r15, [rdi + Zeta_JumpBlock__r15]

    jmp r8

.globl Zeta_SetJumpEntry
Zeta_SetJumpEntry:
    # rdi jmp_blk
    # rsi jmp_entry
    # rdx stack_top

    mov [rdi + Zeta_JumpBlock__rip], rsi
    mov [rdi + Zeta_JumpBlock__rbx], rbx
    mov [rdi + Zeta_JumpBlock__rsp], rdx
    mov [rdi + Zeta_JumpBlock__rbp], rdx
    mov [rdi + Zeta_JumpBlock__r12], r12
    mov [rdi + Zeta_JumpBlock__r13], r13
    mov [rdi + Zeta_JumpBlock__r14], r14
    mov [rdi + Zeta_JumpBlock__r15], r15

    ret
