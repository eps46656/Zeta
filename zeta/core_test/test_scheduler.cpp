

#include <zeta/core/llist_node.h>
#include <zeta/core/multi_level_table.h>
#include <zeta/core_test/ChainingMLTScheduler.h>
#include <zeta/core_test/ChainingStdRBTScheduler.h>
#include <zeta/core_test/NaiveStdRBTScheduler.h>

#include <chrono>
#include <deque>
#include <map>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

#define MEASURE_USER_TIME TRUE

#if MEASURE_USER_TIME

#include <sys/times.h>
#include <unistd.h>

#endif

size_t priority_ring{ (size_t)1 << 32 };
size_t tasks_num{ (size_t)1 << 6 };

std::mt19937_64 rand_en;

size_t min_priority_stride{ 1 };
size_t max_priority_stride{ (size_t)1 << 24 };

std::uniform_int_distribution<size_t> time_generator{ 0, priority_ring - 1 };

std::uniform_int_distribution<size_t> priority_delta_generator{
    min_priority_stride, max_priority_stride
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void main1() {
    ZETA_Core_PrintPos;

    time_t seed = time(NULL);
    ZETA_Core_PrintVar(seed);

    rand_en.seed(seed);

    NaiveStdRBTScheduler scheduler_a{ 0, tasks_num };

    ChainingStdRBTScheduler scheduler_b{ 0, tasks_num };
    // ChainingMLTScheduler scheduler_b{ 0, priority_ring, tasks_num };

    ZETA_Core_PrintPos;

    for (size_t task_id{ 0 }; task_id < tasks_num; ++task_id) {
        size_t priority = priority_delta_generator(rand_en);

        scheduler_a.PushTask(priority, task_id);
        scheduler_b.PushTask(priority, task_id);
    }

    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    for (size_t s{ 0 }; s < 16 * 1024 * 1024; ++s) {
        std::pair<size_t, size_t> scheduler_a_p{ scheduler_a.PopTask() };
        std::pair<size_t, size_t> scheduler_b_p{ scheduler_b.PopTask() };

        ZETA_Core_DebugAssert(scheduler_a_p.first == scheduler_b_p.first);
        // ZETA_Core_DebugAssert(scheduler_a_p.second == scheduler_b_p.second);

        size_t nxt_priority{ scheduler_a_p.first };

        // printf("%llu %llu\n", nxt_priority, nxt_task_id);

        ZETA_Core_DebugAssert(scheduler_a_p.second != tasks_num);
        ZETA_Core_DebugAssert(scheduler_b_p.second != tasks_num);

        size_t priority_delta{ priority_delta_generator(rand_en) };

        nxt_priority = (nxt_priority + priority_delta) % priority_ring;

        scheduler_a.PushTask(nxt_priority, scheduler_a_p.second);

        scheduler_b.PushTask(nxt_priority, scheduler_b_p.second);
    }

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();

    std::cout << "time difference = "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       begin)
                     .count()
              << "[ms]\n";
}

void main2() {
    time_t seed = time(NULL);
    // time_t seed = 1715684680;
    ZETA_Core_PrintVar(seed);

    rand_en.seed(seed);

    // NaiveStdRBTScheduler scheduler{ 0, tasks_num };
    // ChainingStdRBTScheduler scheduler{ 0, tasks_num };
    ChainingMLTScheduler scheduler{ 0, priority_ring, tasks_num };

    for (size_t task_id{ 0 }; task_id < tasks_num; ++task_id) {
        size_t priority = priority_delta_generator(rand_en);

        scheduler.PushTask(priority, task_id);
    }

    size_t hash_code{ 0 };

    size_t pre_scheduling_num{ 1 * 1024 * 1024 };
    size_t scheduling_num{ 16 * 1024 * 1024 };

    std::deque<size_t> priority_deltas;

    for (size_t s{ 0 }; s < pre_scheduling_num + scheduling_num; ++s) {
        priority_deltas.push_back(priority_delta_generator(rand_en));
    }

    size_t max_mem_usage{ 0 };

    for (size_t s{ 0 }; s < pre_scheduling_num; ++s) {
        std::pair<size_t, size_t> p{ scheduler.PopTask() };

        size_t nxt_priority{ p.first };
        size_t nxt_task_id{ p.second };

        hash_code = hash_code * 23 + nxt_priority;

        ZETA_Core_DebugAssert(nxt_task_id != tasks_num);

        size_t priority_delta{ priority_deltas.front() };
        priority_deltas.pop_front();

        nxt_priority = (nxt_priority + priority_delta) % priority_ring;

        scheduler.PushTask(nxt_priority, nxt_task_id);

        max_mem_usage = std::max(max_mem_usage, scheduler.GetMemUsage());
    }

#if MEASURE_USER_TIME

    tms beg_time;
    times(&beg_time);

#else

    std::chrono::steady_clock::time_point begin{
        std::chrono::steady_clock::now()
    };

#endif

    for (size_t s{ 0 }; s < scheduling_num; ++s) {
        std::pair<size_t, size_t> p{ scheduler.PopTask() };

        size_t nxt_priority{ p.first };
        size_t nxt_task_id{ p.second };

        hash_code = hash_code * 47 + nxt_priority;

        ZETA_Core_DebugAssert(nxt_task_id != tasks_num);

        size_t priority_delta{ priority_deltas.front() };
        priority_deltas.pop_front();

        nxt_priority = (nxt_priority + priority_delta) % priority_ring;

        scheduler.PushTask(nxt_priority, nxt_task_id);

        max_mem_usage = std::max(max_mem_usage, scheduler.GetMemUsage());
    }

    ZETA_Core_PrintVar(hash_code);

    ZETA_Core_PrintVar(max_mem_usage);

#if MEASURE_USER_TIME

    tms end_time;
    times(&end_time);

    clock_t clk_per_sec{ sysconf(_SC_CLK_TCK) };

    size_t sys_time_diff{ static_cast<size_t>(
        (end_time.tms_stime - beg_time.tms_stime) * 1000 / clk_per_sec) };

    size_t user_time_diff{ static_cast<size_t>(
        (end_time.tms_utime - beg_time.tms_utime) * 1000 / clk_per_sec) };

    ZETA_Core_PrintVar(sys_time_diff);
    ZETA_Core_PrintVar(user_time_diff);

    size_t time_diff{ user_time_diff };

#else

    std::chrono::steady_clock::time_point end{
        std::chrono::steady_clock::now()
    };

    size_t time_diff{ static_cast<size_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
            .count()) };

#endif

    std::cout << "time difference = " << time_diff << "[ms]\n";
    std::cout << "  max mem usage = " << max_mem_usage << "[bytes]\n";
}

int main() {
    main2();
    printf("ok\n");
    return 0;
}
