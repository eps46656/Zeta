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

void Coroutine_Jump(CoroutineBlock* dst_cor_blk, int code) {
    Coroutine_Jump_Core(dst_cor_blk, code);
}

int Coroutine_Switch(CoroutineBlock* src_cor_blk, CoroutineBlock* dst_cor_blk,
                     int code) {
    return Coroutine_Switch_Core(src_cor_blk, dst_cor_blk, code);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct Task {
    EventLoop* event_loop;

    CoroutineBlock cor_blk;

    void* stack;
    size_t stack_size;

    void* task_entry_context;
    TaskEntry task_entry;
};

typedef void (*TaskEntry)(void* context);

#define SWITCH_CODE_EMPTY (0)
#define SWITCH_CODE_CONTINUE (1)

struct EventLoop {
    size_t stack_size{ 16 * 1024 * 1024 };

    EventLoop();

    void CreateTask(void* task_entry_context, TaskEntry task_entry);

    int Switch();

private:
    void* switch_nod_stack_;
    size_t switch_nod_stack_size_;
    CoroutineBlock switch_nod_cor_blk_;

    std::list<Task*> tasks_l_;

    Task* exited_task_;

    static __attribute__((sysvabi, noreturn)) void SwitchNod_(void* event_loop);

    static __attribute__((sysv_abi, noreturn)) void TaskFrame_(void* task);
};

EventLoop::EventLoop() {
    this->switch_nod_stack_ = std::malloc(stack_size);
    this->switch_nod_stack_size_ = stack_size;

    this->exited_task_ = nullptr;

    Coroutine_SetFunc(&switch_nod_cor_blk_, this, SwitchNod_,
                      this->switch_nod_stack_ + this->switch_nod_stack_size_);
}

void EventLoop::CreateTask(void* task_entry_context, TaskEntry task_entry) {
    Task* task{ new Task };

    task->event_loop = this;

    task->stack = std::malloc(stack_size);
    task->stack_size = stack_size;

    task->task_entry_context = task_entry_context;
    task->task_entry = task_entry;

    Coroutine_SetFunc(&task->cor_blk, &task, TaskFrame_,
                      task->stack + task->stack_size);

    this->tasks_l_.push_back(task);
}

int EventLoop::Switch() {
    if (glb_cur_task->event_loop != nullptr) {
        glb_cur_task->event_loop->tasks_l_.push_back(glb_cur_task);
    }

    return Coroutine_Switch(&glb_cur_task->cor_blk, &this->switch_nod_cor_blk_,
                            0);
}

void EventLoop::SwitchNod_(void* event_loop_) {
    EventLoop* event_loop = (EventLoop*)event_loop_;

    Task* exited_task = event_loop->exited_task_;

    if (exited_task != nullptr) {
        std::free(exited_task->stack);
        delete exited_task;
        event_loop->exited_task_ = nullptr;
    }

    Task* target_task;
    int code;

    if (event_loop->tasks_l_.empty()) {
        target_task = &main_task;
        code = SWITCH_CODE_EMPTY;
    } else {
        target_task = event_loop->tasks_l_.front();
        event_loop->tasks_l_.pop_front();
        code = SWITCH_CODE_CONTINUE;
    }

    glb_cur_task = target_task;

    Coroutine_Jump(&target_task->cor_blk, code);
}

void EventLoop::TaskFrame_(void* task_) {
    Task* task = (Task*)task_;

    task->task_entry(task->task_entry_context);

    event_loop->exited_task_ = task;

    Coroutine_Jump(&event_loop->switch_nod_cor_blk_, 0);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Task main_task{
    .event_loop{ nullptr },
    .cor_blk{},

    .stack{ nullptr },
    .stack_size{ 0 },

    .task_entry_context{ nullptr },
    .task_entry{ nullptr },
};

Task* glb_cur_task = &main_task;

EventLoop event_loop_instance;

EventLoop* event_loop{ &event_loop_instance };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void F(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("F: %d\n", i);
        event_loop->Switch();
    }
}

void G(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("G: %d\n", i);
        event_loop->Switch();
    }
}

void H(void* context) {
    ZETA_Core_Unused(context);

    for (int i{ 0 }; i < 100; ++i) {
        printf("H: %d\n", i);
        event_loop->Switch();
    }
}

void main1() {
    event_loop->CreateTask(NULL, G);
    event_loop->CreateTask(NULL, F);
    event_loop->CreateTask(NULL, H);

    while (event_loop->Switch() == SWITCH_CODE_CONTINUE) {}

    return;
}

int main() {
    main1();
    printf("ok\n");
    return 0;
};
