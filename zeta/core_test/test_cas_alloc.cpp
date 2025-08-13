#include <zeta/core/cascade_allocator.h>
#include <zeta/core/debug_hash_table.h>
#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>

#include <iostream>
#include <map>
#include <random>

typedef long long unsigned llu;

byte_t mem[128 * 1024 * 1024];

Zeta_CascadeAllocator allocator;

Zeta_Core_MemRecorder* mem_recorder;

std::mt19937_64 en;
std::uniform_int_distribution<size_t> size_generator{ 16, 2 * 1024 };
std::uniform_int_distribution<size_t> idx_generator{ 0, ZETA_Core_size_max };

void* MyAlloc(size_t size) {
    if (size == 0) { return NULL; }

    void* ptr = Zeta_CascadeAllocator_Allocate(&allocator, size);

    ZETA_Core_DebugAssert(ptr != NULL);

    Zeta_Core_MemRecorder_Record(mem_recorder, ptr, size);

    return ptr;
}

void MyFree(void* ptr) {
    Zeta_CascadeAllocator_Deallocate(&allocator, ptr);
    ZETA_Core_DebugAssert(Zeta_Core_MemRecorder_Unrecord(mem_recorder, ptr));
}

void CheckAllocator() {
    Zeta_Core_MemRecorder* records{ Zeta_Core_MemRecorder_Create() };

    Zeta_CascadeAllocator_Sanitize(&allocator, records);

    Zeta_Core_MemCheck_MatchRecords(records, mem_recorder);

    Zeta_Core_MemRecorder_Destroy(records);
}

void main1() {
    unsigned seed = time(NULL);

    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    allocator.align = 16;
    allocator.mem = mem;
    allocator.size = sizeof(mem);

    Zeta_CascadeAllocator_Init(&allocator);

    mem_recorder = Zeta_Core_MemRecorder_Create();

    CheckAllocator();

    std::vector<void*> ptrs;

    ZETA_Core_PrintCurPos;

    for (int test_i = 0; test_i < 2000; ++test_i) {
        Zeta_Core_Debugger_ClearPipe();

        size_t size = size_generator(en);

        ptrs.push_back(MyAlloc(size));

        CheckAllocator();
    }

    for (int _ = 0; _ < 100; ++_) {
        ZETA_Core_PrintVar(_);

        for (int test_i = 0; test_i < 1000; ++test_i) {
            Zeta_Core_Debugger_ClearPipe();

            size_t size = size_generator(en);

            ptrs.push_back(MyAlloc(size));

            CheckAllocator();
        }

        ZETA_Core_PrintCurPos;

        for (int test_i = 0; test_i < 1000; ++test_i) {
            Zeta_Core_Debugger_ClearPipe();

            size_t idx = idx_generator(en) % ptrs.size();

            MyFree(ptrs[idx]);

            ptrs[idx] = ptrs.back();
            ptrs.pop_back();

            CheckAllocator();
        }

        ZETA_Core_PrintCurPos;
    }

    Zeta_Core_MemRecorder_Destroy(mem_recorder);
}

int main() {
    main1();

    std::cout << "ok\n";

    return 0;
}
