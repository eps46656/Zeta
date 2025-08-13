.intel_syntax

.equ ZETA_Core_FlowBlock_rip, 0
.equ ZETA_Core_FlowBlock_rbx, 8
.equ ZETA_Core_FlowBlock_rsp, 16
.equ ZETA_Core_FlowBlock_rbp, 24
.equ ZETA_Core_FlowBlock_r12, 32
.equ ZETA_Core_FlowBlock_r13, 40
.equ ZETA_Core_FlowBlock_r14, 48
.equ ZETA_Core_FlowBlock_r15, 56

.text

.globl Zeta_Core_Flow_SetFunc_
Zeta_Core_Flow_SetFunc_:
    # rdi flow_blk
    # rsi flow_func_context
    # rdx flow_func
    # rcx stack_top

    mov rax, 0
    mov [rdi + ZETA_Core_FlowBlock_rbx], rax
    mov [rdi + ZETA_Core_FlowBlock_r13], rax
    mov [rdi + ZETA_Core_FlowBlock_r14], rax
    mov [rdi + ZETA_Core_FlowBlock_r15], rax

    mov rax, rcx
    sub rax, 8

    mov [rdi + ZETA_Core_FlowBlock_rip], rdx
    mov [rdi + ZETA_Core_FlowBlock_rsp], rax
    mov [rdi + ZETA_Core_FlowBlock_rbp], rcx
    mov [rdi + ZETA_Core_FlowBlock_r12], rsi

    ret

.globl Zeta_Core_Flow_Jump_
Zeta_Core_Flow_Jump_:
    # rdi dst_flow_blk
    # rsi code

    mov rcx, [rdi + ZETA_Core_FlowBlock_rip]
    mov rbx, [rdi + ZETA_Core_FlowBlock_rbx]
    mov rsp, [rdi + ZETA_Core_FlowBlock_rsp]
    mov rbp, [rdi + ZETA_Core_FlowBlock_rbp]
    mov r12, [rdi + ZETA_Core_FlowBlock_r12]
    mov r13, [rdi + ZETA_Core_FlowBlock_r13]
    mov r14, [rdi + ZETA_Core_FlowBlock_r14]
    mov r15, [rdi + ZETA_Core_FlowBlock_r15]

    mov rdi, r12

    mov rax, rsi

    jmp rcx

.globl Zeta_Core_Flow_Switch_
Zeta_Core_Flow_Switch_:
    # rdi src_flow_blk
    # rsi dst_flow_blk
    # rdx code

    pop rcx // return address

    mov [rdi + ZETA_Core_FlowBlock_rip], rcx
    mov [rdi + ZETA_Core_FlowBlock_rbx], rbx
    mov [rdi + ZETA_Core_FlowBlock_rsp], rsp
    mov [rdi + ZETA_Core_FlowBlock_rbp], rbp
    mov [rdi + ZETA_Core_FlowBlock_r12], r12
    mov [rdi + ZETA_Core_FlowBlock_r13], r13
    mov [rdi + ZETA_Core_FlowBlock_r14], r14
    mov [rdi + ZETA_Core_FlowBlock_r15], r15

    mov rcx, [rsi + ZETA_Core_FlowBlock_rip]
    mov rbx, [rsi + ZETA_Core_FlowBlock_rbx]
    mov rsp, [rsi + ZETA_Core_FlowBlock_rsp]
    mov rbp, [rsi + ZETA_Core_FlowBlock_rbp]
    mov r12, [rsi + ZETA_Core_FlowBlock_r12]
    mov r13, [rsi + ZETA_Core_FlowBlock_r13]
    mov r14, [rsi + ZETA_Core_FlowBlock_r14]
    mov r15, [rsi + ZETA_Core_FlowBlock_r15]

    mov rdi, r12
    mov rsi, rdx

    mov rax, rdx

    jmp rcx
