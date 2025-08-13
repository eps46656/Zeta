
#include <zeta/core/lin_space_allocator.h>
#include <zeta/core_test/random.h>
#include <zeta/core_test/std_allocator.h>

#include <ctime>
#include <vector>

Zeta_CoreTest_StdAllocator std_allocator_;

Zeta_Core_Allocator std_allocator{ &Zeta_CoreTest_StdAllocator_allocator_vtable,
                                   &std_allocator_ };

Zeta_Core_MemRecorder* ns;

size_t LinSpaceAllocator_Allocate(Zeta_Core_LinSpaceAllocator* lsa,
                                  size_t size) {
    ZETA_Core_PrintCurPos;

    size_t ret = Zeta_Core_LinSpaceAllocator_Allocate(lsa, size);

    ZETA_Core_PrintCurPos;

    Zeta_Core_LinSpaceAllocator_Sanitize(lsa, ns);

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemCheck_MatchRecords(std_allocator_.mem_recorder, ns);

    return ret;
}

void LinSpaceAllocator_Deallocate(Zeta_Core_LinSpaceAllocator* lsa,
                                  size_t idx) {
    ZETA_Core_PrintCurPos;

    Zeta_Core_LinSpaceAllocator_Deallocate(lsa, idx);

    ZETA_Core_PrintCurPos;

    Zeta_Core_LinSpaceAllocator_Sanitize(lsa, ns);

    ZETA_Core_PrintCurPos;

    Zeta_Core_MemCheck_MatchRecords(std_allocator_.mem_recorder, ns);

    ZETA_Core_PrintCurPos;
}

#define RangeLoop(type, var, beg, end, delta) \
    for (type var = (beg); var < (end); var += delta)

void main1() {
    ZETA_Core_PrintCurPos;

    size_t beg = 16 * 1024 * 1024;
    size_t size = 4 * 1024;

    Zeta_Core_LinSpaceAllocator lin_space_allocator;

    lin_space_allocator.beg = beg;
    lin_space_allocator.end = beg + size;

    lin_space_allocator.strategy =
        ZETA_Core_LinSpaceAllocator_AllocationStrategy_TwiceBestFit;

    lin_space_allocator.node_allocator = std_allocator;

    ZETA_Core_PrintCurPos;

    Zeta_Core_LinSpaceAllocator_Init(&lin_space_allocator);

    ZETA_Core_PrintCurPos;

    Zeta_Core_LinSpaceAllocator_Sanitize(&lin_space_allocator, NULL);

    ZETA_Core_PrintCurPos;

    ns = Zeta_Core_MemRecorder_Create();

    std::vector<size_t> idxes;

    size_t K{ 256 };

    RangeLoop(size_t, test_i, 0, 64, 1) {
        ZETA_Core_PrintVar(test_i);

        RangeLoop(size_t, ZETA_Core_TmpName, 0, 64, 1) {
            idxes.push_back(LinSpaceAllocator_Allocate(
                &lin_space_allocator,
                Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, K)));
        }

        RangeLoop(size_t, test_j, 0, 64, 1) {
            ZETA_Core_PrintVar(test_j);

            RangeLoop(size_t, ZETA_Core_TmpName, 0, 1024, 1) {
                size_t idx = LinSpaceAllocator_Allocate(
                    &lin_space_allocator,
                    Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, K));

                if (idx != ZETA_Core_size_max) { idxes.push_back(idx); }
            }

            RangeLoop(size_t, ZETA_Core_TmpName, 0, 1024, 1) {
                if (idxes.empty()) { break; }

                long long target{
                    Zeta_CoreTest_GetRandomInt<long long, long long>(
                        0, idxes.size() - 1)
                };

                LinSpaceAllocator_Deallocate(&lin_space_allocator,
                                             idxes[target]);
                idxes[target] = idxes.back();
                idxes.pop_back();
            }
        }
    }
}

int main() {
    unsigned long long random_seed{ (unsigned long long)time(NULL) };
    unsigned long long fixed_seed{ 0123456 };

    ZETA_Core_Unused(random_seed);
    ZETA_Core_Unused(fixed_seed);

    unsigned long long seed = random_seed;

    Zeta_CoreTest_SetRandomSeed(seed);

    ZETA_Core_PrintCurPos;

    main1();

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar("ok");

    return 0;
}
