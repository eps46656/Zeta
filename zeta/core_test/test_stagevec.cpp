// #include <zeta/core_test/cascade_alloc_utils.h>
// #include <zeta/core_test/multi_level_circular_array_utils.h>
#include <iostream>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core_test/circular_array_utils.hpp>
#include <zeta/core_test/debug_deque_utils.hpp>
// #include <zeta/core_test/multi_level_circular_array_utils.hpp>
#include <zeta/core_test/pod_value.hpp>
#include <zeta/core_test/ptr_iter.ipp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/seg_vector_utils.hpp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/staging_seg_vector_utils.hpp>
#include <zeta/core_test/timer.hpp>

using CircularArray = zeta::core::circular_array::Cntr;

using DebugDeque = zeta::core::debug_deque::Cntr;

inline void test_seq_cntr() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1780833973 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 256 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit(
        std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a_origin },
        origin_size);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    /*
    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::circular_array_utils::Create<
            zeta::core_test::PODValue>(sizeof(zeta::core_test::PODValue) * 3,
                                       1024 * 1024)
    };
    */

    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::seg_vector_utils::Create<zeta::core_test::PODValue>(
            sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    /*
    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::multi_level_circular_array_utils::Create<
            zeta::core_test::PODValue>(sizeof(zeta::core_test::PODValue) * 3, 7)
    };
    */

    size_t max_op_size{ 256 };

    for (size_t _{ 0 }; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        // zeta::core::SeqCntr_Assign(seq_cntr_a, seq_cntr_a_origin);
        // zeta::core::SeqCntr_Assign(seq_cntr_b, seq_cntr_a_origin);

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a,
                                                         &seq_cntr_b },

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

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_a_origin);
}

inline void test_staging_seg_vector() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1735663456U };

    // unsigned seed = random_seed;
    unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b_origin{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 64 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit(
        std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a_origin },
        origin_size);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::staging_seg_vector_utils::Create<
            zeta::core_test::PODValue>(seq_cntr_b_origin,
                                       sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    size_t max_op_size{ 128 };

    for (size_t _ = 0; _ < 8; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::seq_cntr::Assign(seq_cntr_a, seq_cntr_a_origin);

        zeta::core::seq_cntr::Assign(seq_cntr_b_origin, seq_cntr_a_origin);

        static_cast<
            zeta::core_test::staging_seg_vector_utils::StagingSegVector*>(
            seq_cntr_b.cntr)
            ->Reset();

        zeta::core_test::seq_cntr_utils::Sanitize(&seq_cntr_b);

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a,
                                                         &seq_cntr_b },

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

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_a_origin);
    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_b_origin);
}

inline void test_staging_seg_vector_copy_init() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 17354521 };

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::seq_cntr_ref::Ref dd_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref dd_b{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 64 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit(
        std::vector<zeta::core::seq_cntr_ref::Ref*>{ &dd_a }, origin_size);

    zeta::core::seq_cntr_ref::Ref dd_c{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref sv_a{
        zeta::core_test::staging_seg_vector_utils::Create<
            zeta::core_test::PODValue>(dd_b,
                                       sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    size_t max_op_size = 128;

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::seq_cntr::Assign(dd_b, dd_a);
        zeta::core::seq_cntr::Assign(dd_c, dd_a);

        static_cast<
            zeta::core_test::staging_seg_vector_utils::StagingSegVector*>(
            sv_a.cntr)
            ->Reset();

        zeta::core_test::seq_cntr_utils::Sanitize(&dd_c);
        zeta::core_test::seq_cntr_utils::Sanitize(&sv_a);

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &dd_c, &sv_a },

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

        zeta::core::seq_cntr_ref::Ref sv_b{
            zeta::core_test::staging_seg_vector_utils::Create<
                zeta::core_test::PODValue>(
                dd_b, sizeof(zeta::core_test::PODValue) * 3, 7,
                *static_cast<zeta::core_test::staging_seg_vector_utils::
                                 StagingSegVector*>(sv_a.cntr))
        };

        zeta::core_test::seq_cntr_utils::Sanitize(&sv_b);

        zeta::core_test::seq_cntr_utils::SyncCompare(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &dd_c, &sv_a, &sv_b });

        zeta::core_test::seq_cntr_utils::Destroy(&sv_b);
    }

    zeta::core_test::seq_cntr_utils::Destroy(&dd_a);
    zeta::core_test::seq_cntr_utils::Destroy(&dd_b);
    zeta::core_test::seq_cntr_utils::Destroy(&dd_c);

    zeta::core_test::seq_cntr_utils::Destroy(&sv_a);
}

#if 0

void test_staging_vector_copy() {
    unsigned random_seed = time(nullptr);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a_origin{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b_origin{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_c_origin{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    ZETA_Core_PrintCurPos;

    size_t origin_size{ 1024 * 1024 };

    ZETA_Core_PrintCurPos;

    seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_a_origin }, origin_size);

    ZETA_Core_PrintCurPos;

    zeta::core::seq_cntr_ref::Ref seq_cntr_a{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_b_origin, sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_c{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_c_origin, sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a_origin, seq_cntr_a);
        zeta::core::SeqCntr_Assign(seq_cntr_b_origin, seq_cntr_a);
        zeta::core::SeqCntr_Assign(seq_cntr_c_origin, seq_cntr_a);

        Zeta_Core_StagingVector_Reset(seq_cntr_b.context);
        Zeta_Core_StagingVector_Reset(seq_cntr_c.context);

        seq_cntr_utils::DoRandomOperations<
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

        seq_cntr_utils::DoRandomOperations<
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

        seq_cntr_utils::Sanitize(seq_cntr_b);
        seq_cntr_utils::Sanitize(seq_cntr_c);

        seq_cntr_utils::DoRandomOperations<
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

    seq_cntr_utils::Destroy(seq_cntr_a);
    seq_cntr_utils::Destroy(seq_cntr_b);
    seq_cntr_utils::Destroy(seq_cntr_c);

    seq_cntr_utils::Destroy(seq_cntr_a_origin);
    seq_cntr_utils::Destroy(seq_cntr_b_origin);
    seq_cntr_utils::Destroy(seq_cntr_c_origin);
}

#endif

inline void test_staging_vector_collapse() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1775408345 };

    // unsigned seed = random_seed;
    unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    /*

    seq_cntr_base <- seq_cntr_b <- seq_cntr_c

    seq_cntr_a

    collapse

    seq_cntr_base <- seq_cntr_c

    seq_cntr_base <- seq_cntr_b

    seq_cntr_a

    */

    zeta::core::seq_cntr_ref::Ref seq_cntr_base{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 32 };

    zeta::core_test::seq_cntr_utils::SyncRandomInit(
        std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_base },
        origin_size);

    zeta::core::seq_cntr_ref::Ref seq_cntr_a{
        zeta::core_test::debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        zeta::core_test::staging_seg_vector_utils::Create<
            zeta::core_test::PODValue>(seq_cntr_base,
                                       sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::seq_cntr::Assign(seq_cntr_a, seq_cntr_base);

        static_cast<
            zeta::core_test::staging_seg_vector_utils::StagingSegVector*>(
            seq_cntr_b.cntr)
            ->Reset();

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a,
                                                         &seq_cntr_b },

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

        zeta::core::seq_cntr_ref::Ref seq_cntr_c{
            zeta::core_test::staging_seg_vector_utils::Create<
                zeta::core_test::PODValue>(
                seq_cntr_b, sizeof(zeta::core_test::PODValue) * 3, 7)
        };

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a,
                                                         &seq_cntr_c },

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

        ZETA_Core_PrintVar(seq_cntr_base.cntr);
        ZETA_Core_PrintVar(seq_cntr_b.cntr);
        ZETA_Core_PrintVar(seq_cntr_c.cntr);

        static_cast<
            zeta::core_test::staging_seg_vector_utils::StagingSegVector*>(
            seq_cntr_c.cntr)
            ->Collapse(*static_cast<zeta::core_test::staging_seg_vector_utils::
                                        StagingSegVector*>(seq_cntr_b.cntr),
                       zeta::core::seq_cntr_ref::Ref{ *static_cast<
                           zeta::core_test::debug_deque_utils::DebugDeque*>(
                           seq_cntr_base.cntr) });

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::Sanitize(&seq_cntr_b);
        zeta::core_test::seq_cntr_utils::Sanitize(&seq_cntr_c);

        ZETA_Core_PrintCurPos;

        zeta::core_test::seq_cntr_utils::DoRandomOperations(
            std::vector<zeta::core::seq_cntr_ref::Ref*>{ &seq_cntr_a,
                                                         &seq_cntr_c },

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

        zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_c);
    }

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_a);
    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_b);

    zeta::core_test::seq_cntr_utils::Destroy(&seq_cntr_base);
}

#if 0

void test_staging_vector_write_back() {
    unsigned random_seed = time(nullptr);
    unsigned fixed_seed = 1735990806;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    zeta::core_test::InitCascadeAllocator();

    zeta::core::seq_cntr_ref::Ref seq_cntr_base{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_a_origin{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_a{
        zeta::core_test::StagingVector_Create<zeta::core_test::PODValue>(
            seq_cntr_a_origin, sizeof(zeta::core_test::PODValue) * 3, 7)
    };

    zeta::core::seq_cntr_ref::Ref seq_cntr_b{
        debug_deque_utils::Create<zeta::core_test::PODValue>()
    };

    size_t origin_size{ 1024 * 1024 };

    seq_cntr_utils::SyncRandomInit<zeta::core_test::PODValue>(
        { seq_cntr_base }, origin_size);

    for (size_t _ = 0; _ < 16; ++_) {
        ZETA_Core_PrintVar(_);

        zeta::core::SeqCntr_Assign(seq_cntr_a_origin, seq_cntr_base);

        zeta::core::SeqCntr_Assign(seq_cntr_b, seq_cntr_base);

        Zeta_Core_StagingVector_Reset(seq_cntr_a.context);

        seq_cntr_utils::DoRandomOperations<
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

        seq_cntr_utils::SyncCompare<zeta::core_test::PODValue>(
            { seq_cntr_a_origin, seq_cntr_b });
    }

    seq_cntr_utils::Destroy(seq_cntr_base);

    seq_cntr_utils::Destroy(seq_cntr_a);
    seq_cntr_utils::Destroy(seq_cntr_b);

    seq_cntr_utils::Destroy(seq_cntr_a_origin);
}

#endif

int main() {
    printf("main start\n");

    ZETA_Core_PrintCurPos;

    unsigned long long beg_time{ zeta::core_test::GetTime() };

    // test_seq_cntr();
    test_staging_seg_vector();
    // test_staging_seg_vector_copy_init();
    // test_staging_vector_collapse();

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
