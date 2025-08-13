#include <cascade_allocator.h>
#include <flow.h>
#include <memory.h>
#include <zeta/core/debugger.h>

#include <iostream>

#define STACK_SIZE (256 * 1024)

void* AllocateStack() {
    return (unsigned char*)std::malloc(STACK_SIZE) + STACK_SIZE;
}

void DeallocateStack(void* stack) {
    return std::free((unsigned char*)stack - STACK_SIZE);
}

Zeta_CascadeAllocator cascade_allocator_instance;
Zeta_Core_Allocator cascade_allocator;

void Zeta_CoreTest_InitCascadeAllocator() {
    size_t mem_size{ 256 * 1024 * 1024 };

    cascade_allocator_instance.align = 16;
    cascade_allocator_instance.mem = std::malloc(mem_size);
    cascade_allocator_instance.size = mem_size;

    Zeta_CascadeAllocator_Init(&cascade_allocator_instance);

    if (zeta_memory_cascade_allocator == NULL) {
        zeta_memory_cascade_allocator = &zeta_cascade_allocator_vtable;
        zeta_memory_cascade_allocator_context = &cascade_allocator;
    }
}

void* botton_exception_handler_stack;
Zeta_Core_FlowBlock bottom_exception_handler_blk;

Zeta_Core_FlowBlock* prv_exception_handler_blk = &bottom_exception_handler_blk;

ZETA_Core_DeclareStruct(ExceptionBlock);

struct ExceptionBlock {
    int code;
    char const* message;
};

__attribute__((sysv_abi, noreturn)) void ButtomExceptionHandler(
    void* context, void* exception_blk_) {
    ZETA_Core_Unused(context);

    ExceptionBlock* exception_blk = (ExceptionBlock*)exception_blk_;

    if (exception_blk == NULL) {
        ZETA_Core_PrintVar("NULL");
        exit(0);
    }

    ZETA_Core_PrintVar(exception_blk->code);
    ZETA_Core_PrintVar(exception_blk->message);

    ZETA_Core_Allocator_Deallocate(zeta_memory_cascade_allocator,

                                   exception_blk);

    exit(1);
}

void G() {
    int exception_code;

    std::cout << "input exceotion code:";
    std::cin >> exception_code;

    ExceptionBlock* exception_blk;

    switch (exception_code) {
        case 0: ZETA_Core_PrintVar("no exception"); return;

        case 1:
            ZETA_Core_PrintVar("throw exception 1");

            exception_blk = (ExceptionBlock*)ZETA_Core_Allocator_SafeAllocate(
                zeta_memory_cascade_allocator, alignof(ExceptionBlock),
                sizeof(ExceptionBlock));

            exception_blk->code = 1;
            exception_blk->message = "exception 1";

            ZETA_Core_Flow_Jump(prv_exception_handler_blk, exception_blk);

            return;

        case 2:
            ZETA_Core_PrintVar("throw exception 2");

            exception_blk = (ExceptionBlock*)ZETA_Core_Allocator_SafeAllocate(
                zeta_memory_cascade_allocator, alignof(ExceptionBlock),
                sizeof(ExceptionBlock));

            exception_blk->code = 2;
            exception_blk->message = "exception 2";

            ZETA_Core_Flow_Jump(prv_exception_handler_blk, exception_blk);

            return;

        case 3:
            ZETA_Core_PrintVar("throw exception 3");

            exception_blk = (ExceptionBlock*)ZETA_Core_Allocator_SafeAllocate(
                zeta_memory_cascade_allocator, alignof(ExceptionBlock),
                sizeof(ExceptionBlock));

            exception_blk->code = 3;
            exception_blk->message = "exception 3";

            ZETA_Core_Flow_Jump(prv_exception_handler_blk, exception_blk);

            return;
    }
};

void F() {
    Zeta_Core_FlowBlock cur_exception_handler_blk;

    Zeta_Core_FlowBlock* origin_prv_exception_handler_blk =
        prv_exception_handler_blk;

    prv_exception_handler_blk = &cur_exception_handler_blk;

    void* code = ZETA_Core_Flow_Switch(&cur_exception_handler_blk,
                                       &cur_exception_handler_blk, NULL);

    ZETA_Core_PrintCurPos;

    if (code == NULL) {
        ZETA_Core_PrintVar("normal a");

        G();

        ZETA_Core_PrintVar("normal b");

        return;
    }

    ExceptionBlock* exception_blk = (ExceptionBlock*)ZETA_Core_AddrToPtr(code);

    switch (exception_blk->code) {
        case 1:
            ZETA_Core_PrintVar("catched exception 1");

            ZETA_Core_PrintVar(exception_blk->message);

            ZETA_Core_Allocator_Deallocate(zeta_memory_cascade_allocator,

                                           exception_blk);

            break;

        case 2:
            ZETA_Core_PrintVar("catched exception 2");

            ZETA_Core_PrintVar(exception_blk->message);

            ZETA_Core_Allocator_Deallocate(zeta_memory_cascade_allocator,

                                           exception_blk);

            break;

        default:
            ZETA_Core_PrintVar("unrecognized exception");
            ZETA_Core_PrintVar("cascaded throw exception");

            ZETA_Core_Flow_Jump(origin_prv_exception_handler_blk, code);
    }
};

void main1() {
    Zeta_CoreTest_InitCascadeAllocator();

    botton_exception_handler_stack = AllocateStack();

    ZETA_Core_Flow_SetFunc(&bottom_exception_handler_blk, NULL,
                           ButtomExceptionHandler,
                           botton_exception_handler_stack);

    ZETA_Core_PrintCurPos;

    F();

    ZETA_Core_PrintCurPos;
}

int main() {
    ZETA_Core_PrintCurPos;

    main1();

    ZETA_Core_PrintCurPos;

    return 0;
}
