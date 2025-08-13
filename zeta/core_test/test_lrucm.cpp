#include <cascade_allocator.h>
#include <zeta/core_test/caching_array_utils.h>
#include <zeta/core_test/cascade_alloc_utils.h>
#include <zeta/core_test/debug_deque_utils.h>
#include <zeta/core_test/lru_cache_manager_utils.h>
#include <zeta/core_test/pod_value.h>
#include <zeta/core_test/random.h>
#include <zeta/core_test/timer.h>

#include <random>
#include <unordered_map>
#include <unordered_set>

byte_t mem[128 * 1024 * 1024];

Zeta_CascadeAllocator allocator;

#define FOR_LOOP_(tmp_end, type, var, beg, end) \
    for (type var = (beg), tmp_end = (end); var != tmp_end; ++var)

#define FOR_LOOP(type, var, beg, end) \
    FOR_LOOP_(ZETA_Core_TmpName, type, var, (beg), (end))

void test_cm() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1735451159;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_SetRandomSeed(seed);

    Zeta_CoreTest_InitCascadeAllocator();

    Zeta_Core_SeqCntr* origin{ Zeta_CoreTest_DebugDeque_Create<PODValue>() };

    Zeta_CoreTest_SeqCntrUtils_SyncRandomInit<PODValue>({ origin }, 1024 * 256);

    Zeta_CacheManager* cm{ LRUCacheManagerUtils_Create(origin, 7) };

    Zeta_Core_SeqCntr* seq_cntr_a{
        Zeta_CoreTest_DebugDeque_Create<PODValue>()
    };
    Zeta_Core_SeqCntr_Assign(seq_cntr_a, origin);

    ZETA_Core_PrintCurPos;

    Zeta_Core_SeqCntr* seq_cntr_b{ Zeta_CoreTest_CachingArray_Create(cm, 8) };

    ZETA_Core_PrintCurPos;

    Zeta_Core_SeqCntr* seq_cntr_c{ Zeta_CoreTest_CachingArray_Create(cm, 8) };

    size_t max_op_size = 1024;

    ZETA_Core_PrintCurPos;

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        // Zeta_Core_SeqCntr_Assign(seq_cntr_a, seq_cntr_a_origin);
        // Zeta_Core_SeqCntr_Assign(seq_cntr_b, seq_cntr_a_origin);

        FOR_LOOP(size_t, ZETA_Core_TmpName, 0, 16) {
            Zeta_CoreTest_SeqCntrUtils_DoRandomOperations<PODValue>(
                { seq_cntr_a, seq_cntr_b },

                4,  // iter_cnt

                max_op_size,  // read_max_op_size
                max_op_size,  // write_max_op_size
                0,            // push_l_max_op_size
                0,            // push_r_max_op_size
                0,            // pop_l_max_op_size
                0,            // pop_r_max_op_size
                0,            // insert_max_op_size
                0,            // erase_max_op_size

                max_op_size,  // cursor_step_l_max_op_size
                max_op_size,  // cursor_step_r_max_op_size
                max_op_size,  // cursor_advance_l_op_size
                max_op_size   // cursor_advance_r_op_size
            );

            Zeta_LRUCacheManager_Flush(cm->context, ZETA_Core_size_max);
        }

        Zeta_CoreTest_SeqCntrUtils_SyncCompare<PODValue>(
            { seq_cntr_a, seq_cntr_c });
    }
};

// -----------------------------------------------------------------------------

int main() {
    unsigned long long beg_time{ GetTime() };

    test_cm();

    unsigned long long end_time{ GetTime() };

    unsigned long long duration{ end_time - beg_time };

    ZETA_Core_PrintVar(duration);
}
