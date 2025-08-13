#pragma once

#include <zeta/core/define.h>

ZETA_Core_DeclareStruct(Zeta_CFScheduler);

struct Zeta_CFScheduler {
    //
};

struct Zeta_CFScheduler_TaskNode {
    size_t id;

    //
};

struct Zeta_CFScheduler_TaskBuilder {
    size_t id;

    //
};

void Zeta_CFScheduler_Init(void* cfs);

void* Zeta_CFScheduler_GetTask(void* cfs, size_t id);

void* Zeta_CFScheduler_TaskExecute(void* cfs, void* ts,
                                   unsigned long long delta);
