// #include <zeta/core_test/cascade_alloc_utils.h>
// #include <zeta/core_test/multi_level_circular_array_utils.h>
// #include <zeta/core_test/seg_vector_utils.h>
// #include <zeta/core_test/staging_vector_utils.h>
#include <chrono>
#include <csignal>
#include <deque>
#include <iostream>
#include <random>
#include <set>
#include <unordered_set>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/circular_array_utils.hpp>
#include <zeta/core_test/debug_deque_utils.hpp>
#include <zeta/core_test/pod_value.hpp>
#include <zeta/core_test/ptr_iter.ipp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/timer.hpp>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void test_seq_cntr() {
    unsigned random_seed{ static_cast<unsigned>(time(NULL)) };
    unsigned fixed_seed{ 1735451159 };

    // unsigned seed { random_seed};
    unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::SeqCntr seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 1024 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_a_origin }, origin_size);

    zeta::core::SeqCntr seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b{
        zeta::core_test::circular_array_utils::Create<
            zeta::core_test::PODValue>(sizeof(zeta::core_test::PODValue) * 3,
                                       1024 * 1024)
    };

    /*
    zeta::core::SeqCntr seq_cntr_b{
    zeta::core_test::MultiLevelCircularArray_Create<zeta::core_test::PODValue>(
    sizeof(PODValue) * 3, 7) };
    */

    /*
    zeta::core::SeqCntr seq_cntr_b{
    zeta::core_test::SegVectorUtils_Create<zeta::core_test::PODValue>(
    sizeof(PODValue) * 3, 7) };
    */

    size_t max_op_size = 1024;

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        // zeta::core::SeqCntr_Assign(seq_cntr_a, seq_cntr_a_origin);
        // zeta::core::SeqCntr_Assign(seq_cntr_b, seq_cntr_a_origin);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>(
            { seq_cntr_a, seq_cntr_b },

            256,  // iter_cnt

            max_op_size,  // read_max_op_size
            max_op_size,  // write_max_op_size
            max_op_size,  // push_l_max_op_size
            max_op_size,  // push_r_max_op_size
            max_op_size,  // pop_l_max_op_size
            max_op_size,  // pop_r_max_op_size
            max_op_size,  // insert_max_op_size
            max_op_size,  // erase_max_op_size

            max_op_size,  // cursor_step_l_max_op_size
            max_op_size,  // cursor_step_r_max_op_size
            max_op_size,  // cursor_advance_l_op_size
            max_op_size   // cursor_advance_r_op_size
        );
    }

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a_origin);
}

/*
void test_staging_vector() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::SeqCntr seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 1024 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_a_origin }, origin_size);

    zeta::core::SeqCntr seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_b_origin, sizeof(PODValue) * 3, 7)
    };

    size_t max_op_size = 1024;

    for (size_t _ = 0; _ < 4; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a, seq_cntr_a_origin);

        zeta::core::SeqCntr_Assign(seq_cntr_b_origin, seq_cntr_a_origin);

        Zeta_Core_StagingVector_Reset(seq_cntr_b.context);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>(
            { seq_cntr_a, seq_cntr_b },

            16,  // iter_cnt

            max_op_size,  // read_max_op_size
            max_op_size,  // write_max_op_size
            max_op_size,  // push_l_max_op_size
            max_op_size,  // push_r_max_op_size
            max_op_size,  // pop_l_max_op_size
            max_op_size,  // pop_r_max_op_size
            max_op_size,  // insert_max_op_size
            max_op_size,  // erase_max_op_size

            max_op_size,  // cursor_step_l_max_op_size
            max_op_size,  // cursor_step_r_max_op_size
            max_op_size,  // cursor_advance_l_op_size
            max_op_size   // cursor_advance_r_op_size
        );
    }

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a_origin);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b_origin);
}
*/

#if 0

void test_staging_vector_copy() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::SeqCntr seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_c_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    ZETA_Core_PrintCurPos;

    size_t origin_size{ 1024 * 1024 };

    ZETA_Core_PrintCurPos;

    zeta::core_test::seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_a_origin }, origin_size);

    ZETA_Core_PrintCurPos;

    zeta::core::SeqCntr seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_b_origin, sizeof(PODValue) * 3, 7)
    };

    zeta::core::SeqCntr seq_cntr_c{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_c_origin, sizeof(PODValue) * 3, 7)
    };

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a_origin, seq_cntr_a);
        zeta::core::SeqCntr_Assign(seq_cntr_b_origin, seq_cntr_a);
        zeta::core::SeqCntr_Assign(seq_cntr_c_origin, seq_cntr_a);

        Zeta_Core_StagingVector_Reset(seq_cntr_b.context);
        Zeta_Core_StagingVector_Reset(seq_cntr_c.context);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_b },

                                       512,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_c },

                                       512,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        Zeta_Core_StagingVector_Copy(seq_cntr_c.context, seq_cntr_b.context);

        zeta::core_test::seq_cntr_utils::Sanitize(seq_cntr_b);
        zeta::core_test::seq_cntr_utils::Sanitize(seq_cntr_c);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_c },

                                       512,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );
    }

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_c);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a_origin);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b_origin);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_c_origin);
}

void test_staging_vector_collapse() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    /*

    seq_cntr_base -> seq_cntr_b -> seq_cntr_c

    seq_cntr_a

    collapse

    seq_cntr_base -> seq_cntr_b

    seq_cntr_base -> seq_cntr_c

    seq_cntr_a

    */

    zeta::core::SeqCntr seq_cntr_base{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 1024 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_base }, origin_size);

    zeta::core::SeqCntr seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_b{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_base, sizeof(PODValue) * 3, 7)
    };

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a, seq_cntr_base);

        Zeta_Core_StagingVector_Reset(seq_cntr_b.context);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_b },

                                       16,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        ZETA_Core_PrintCurPos;

        zeta::core::SeqCntr seq_cntr_c{
            zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
                seq_cntr_b, sizeof(PODValue) * 3, 7)
        };

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_c },

                                       16,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        ZETA_Core_PrintCurPos;

        Zeta_Core_StagingVector_Collapse(seq_cntr_c.context);

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::Sanitize(seq_cntr_b);
        zeta::core_test::seq_cntr_utils::Sanitize(seq_cntr_c);

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_c },

                                       16,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_c);
    }

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_base);
}

void test_staging_vector_write_back() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1735990806;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core_test::InitCascadeAllocator();

    zeta::core::SeqCntr seq_cntr_base{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::SeqCntr seq_cntr_a{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_a_origin, sizeof(PODValue) * 3, 7)
    };

    zeta::core::SeqCntr seq_cntr_b{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 1024 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_base }, origin_size);

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a_origin, seq_cntr_base);

        zeta::core::SeqCntr_Assign(seq_cntr_b, seq_cntr_base);

        Zeta_Core_StagingVector_Reset(seq_cntr_a.context);

        zeta::core_test::seq_cntr_utils::DoRandomOperations<
            zeta::core_test::PODValue>({ seq_cntr_a, seq_cntr_b },

                                       256,  // iter_cnt

                                       16,  // read_max_op_size
                                       16,  // write_max_op_size
                                       16,  // push_l_max_op_size
                                       16,  // push_r_max_op_size
                                       16,  // pop_l_max_op_size
                                       16,  // pop_r_max_op_size
                                       16,  // insert_max_op_size
                                       16,  // erase_max_op_size

                                       16,  // cursor_step_l_max_op_size
                                       16,  // cursor_step_r_max_op_size
                                       16,  // cursor_advance_l_op_size
                                       16   // cursor_advance_r_op_size
        );

        Zeta_Core_StagingVector_WriteBack(
            seq_cntr_a.context, ZETA_StagingVector_WriteBackStrategy_LR,
            1,   // cost_coeff_read
            2,   // cost_coeff_write
            10,  // cost_coeff_insert
            10   // cost_coeff_erase
        );

        zeta::core_test::seq_cntr_utils::SyncCompare<zeta::core_test::PODValue>(
            { seq_cntr_a_origin, seq_cntr_b });
    }

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_base);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(seq_cntr_a_origin);
}

#endif

int main() {
    printf("main start\n");

    ZETA_Core_PrintCurPos;

    unsigned long long beg_time{ zeta::core_test::GetTime() };

    test_seq_cntr();
    // main1();

    unsigned long long end_time{ zeta::core_test::GetTime() };

    printf("ok\a\n");

    std::cout << "Time difference = " << end_time - beg_time << "[ms]\n";

    /*

    DD: 4073[ms]

    RadixDeque: 325[ms]

    RadixVector: 233[ms]

    */

    return 0;
}
