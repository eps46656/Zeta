/*
 * Advanced Operating System
 * Midterm
 * Problem 3
 * O(1) real time scheduler
 *
 * Author: r12922135
 */

/*

Compile:
    python ZetaExe/build.py ZetaExe/test_o1sh.exe

 */

#include <zeta/core/multi_level_vector.h>
#include <zeta/core_test/std_allocator.h>

#include <climits>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>

using TIME_T = unsigned long long;

#define TIME_LT(x, y) (!TIME_LE(y, x))
#define TIME_GT(x, y) (!TIME_LE(x, y))
#define TIME_LE(x, y)                                       \
    (((TIME_T)(y) + time_ring - (TIME_T)(x)) % time_ring <= \
     time_ring / (TIME_T)(2))
#define TIME_GE(x, y) TIME_LE(y, x)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct TaskInfo {
    int id;
    TIME_T exe_time;
    TIME_T period;
    TIME_T rm_time;
    TIME_T join_time;
};

int num_of_runs;

int run;

#define SCHEDULE_RM (0)
#define SCHEDULE_EDF (1)

int schedule;

int display;

#define MAX_TASKS_NUM (129)

int tasks_num;

TaskInfo tasks[MAX_TASKS_NUM];

TIME_T time_ring;

TIME_T cur_time;

TaskInfo* cur_task;
int cur_task_state;

#define TASK_STATE_END (0)
#define TASK_STATE_PAUSE (1)

#define ACT_START (0)
#define ACT_END (1)
#define ACT_PREEMPTED (2)
#define ACT_RESUME (3)

#define ABS_CUR_TIME_BASE (1000000000)
#define ABS_CUR_TIME_BASE_ORDER (9)

std::vector<unsigned long long> abs_cur_time;

StdAllocator mlv_node_allocator_;
StdAllocator mlv_table_allocator_;

Zeta_Core_Allocator mlv_node_allocator;
Zeta_Core_Allocator mlv_table_allocator;

Zeta_Core_MultiLevelVector join_mlv;
Zeta_Core_MultiLevelVector ready_mlv;

size_t GetIdx(Zeta_Core_MultiLevelVector const* mlv, size_t const* idxes) {
    size_t idx = idxes[0];

    for (int level_i = 1; level_i < mlv->level; ++level_i) {
        idx = idx * mlv->branch_nums[level_i] + idxes[level_i];
    }

    return idx;
}

void GetIdxes(Zeta_Core_MultiLevelVector const* mlv, size_t* dst, size_t idx) {
    for (int level_i = mlv->level - 1; 0 <= level_i; --level_i) {
        dst[level_i] = idx % mlv->branch_nums[level_i];
        idx /= mlv->branch_nums[level_i];
    }

    ZETA_Core_DebugAssert(idx == 0);
}

void Allocator_Init() {
    Zeta_Core_Allocator_Init(&mlv_node_allocator);
    Zeta_Core_Allocator_Init(&mlv_table_allocator);
}

void MLV_Init() {
    int max_brach_order = 9;

    size_t space = (size_t)time_ring * (size_t)tasks_num;

    int space_order = 1;
    for (size_t k = space; 1 < k; k /= 2) { ++space_order; }

    int level = (space_order + max_brach_order - 1) / max_brach_order;

    int m = space_order / level;
    int l = space_order % level;

    join_mlv.level = level;
    ready_mlv.level = level;

    for (int level_i = 0; level_i < level; ++level_i) {
        int k = 1 << (m + (level_i < l));
        join_mlv.branch_nums[level_i] = k;
        ready_mlv.branch_nums[level_i] = k;
    }

    join_mlv.node_allocator = &mlv_node_allocator;
    join_mlv.table_allocator = &mlv_table_allocator;

    ready_mlv.node_allocator = &mlv_node_allocator;
    ready_mlv.table_allocator = &mlv_table_allocator;

    Zeta_Core_MultiLevelVector_Init(&join_mlv);
    Zeta_Core_MultiLevelVector_Init(&ready_mlv);
}

TIME_T preemption_cnt;

std::stringstream ss;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ReadyMLV_Insert(TaskInfo* task) {
    size_t idxes[ZETA_MultiLevelVector_max_level];

    GetIdxes(
        &ready_mlv, idxes,
        schedule == SCHEDULE_RM
            ? (size_t)task->period * (size_t)tasks_num + (size_t)task->id
            : (size_t)task->join_time * (size_t)tasks_num + (size_t)task->id);

    *Zeta_Core_MultiLevelVector_Insert(&ready_mlv, idxes) = task;
}

TaskInfo* ReadyMLV_Pop() {
    if (Zeta_Core_MultiLevelVector_GetSize(&ready_mlv) == 0) {
        return &tasks[0];
    }

    size_t idxes[ZETA_MultiLevelVector_max_level];

    GetIdxes(
        &ready_mlv, idxes,
        schedule == SCHEDULE_RM ? 0 : (size_t)cur_time * (size_t)tasks_num);

    void** ele =
        Zeta_Core_MultiLevelVector_FindNextNotNull(&ready_mlv, idxes, TRUE);

    if (ele == NULL) {
        ele =
            Zeta_Core_MultiLevelVector_FindNextNotNull(&ready_mlv, idxes, TRUE);
    }

    TaskInfo* ret = (TaskInfo*)(*ele);
    Zeta_Core_MultiLevelVector_Erase(&ready_mlv, idxes);

    return ret;
}

void JoinMLV_Insert(TaskInfo* task) {
    size_t idxes[ZETA_MultiLevelVector_max_level];
    GetIdxes(&join_mlv, idxes,
             (size_t)task->join_time * (size_t)tasks_num + task->id);

    *Zeta_Core_MultiLevelVector_Insert(&join_mlv, idxes) = task;
}

void IncAbsCurTime(unsigned long long k) {
    for (size_t i{ 0 }; 0 < k; k /= ABS_CUR_TIME_BASE, ++i) {
        if (abs_cur_time.size() <= i) { abs_cur_time.push_back(0); }
        k += abs_cur_time[i];
        abs_cur_time[i] = k % ABS_CUR_TIME_BASE;
    }
}

void PrintAbsCurTime(std::ostream& os) {
    int size{ static_cast<int>(abs_cur_time.size()) };

    if (size == 0) {
        os << '0';
        return;
    }

    os << abs_cur_time.back();

    for (int i{ size - 2 }; 0 <= i; --i) {
        os << std::setfill('0') << std::setw(ABS_CUR_TIME_BASE_ORDER)
           << abs_cur_time[i];
    }
}

void F() {
    std::cin >> schedule >> display >> tasks_num;
    ++tasks_num;

    ZETA_Core_DebugAssert(schedule == SCHEDULE_RM || schedule == SCHEDULE_EDF);

    ZETA_Core_DebugAssert(display == 0 || display == 1);

    ZETA_Core_DebugAssert(0 < tasks_num);
    ZETA_Core_DebugAssert(tasks_num <= MAX_TASKS_NUM);

    TIME_T max_period = 0;

    for (int task_i = 1; task_i < tasks_num; ++task_i) {
        std::cin >> tasks[task_i].exe_time >> tasks[task_i].period;

        tasks[task_i].rm_time = tasks[task_i].exe_time;
        tasks[task_i].join_time = tasks[task_i].period;

        ZETA_Core_DebugAssert(0 < tasks[task_i].exe_time);
        ZETA_Core_DebugAssert(0 < tasks[task_i].period);

        max_period = std::max(max_period, tasks[task_i].period);
    }

    tasks[0].rm_time = max_period;
    tasks[0].period = max_period + 1;

    time_ring = 1;
    while (time_ring < max_period) { time_ring *= 2; }
    time_ring *= 2;

    MLV_Init();

    cur_time = 0;

    cur_task = &tasks[0];
    cur_task_state = TASK_STATE_END;

    preemption_cnt = 0;

    bool_t success = TRUE;

    abs_cur_time.clear();

    ss.clear();
    ss.str("");

    for (int task_i = 1; task_i < tasks_num; ++task_i) {
        if (tasks[task_i].period < tasks[task_i].exe_time) {
            success = FALSE;
            goto SCHEDULE_END;
        }
    }

    for (int task_i = 1; task_i < tasks_num; ++task_i) {
        ReadyMLV_Insert(tasks + task_i);
    }

    for (;;) {
        size_t origin_join_mlv_size =
            Zeta_Core_MultiLevelVector_GetSize(&join_mlv);

        TIME_T nxt_join_time;

        {
            size_t idxes[ZETA_MultiLevelVector_max_level];
            GetIdxes(&join_mlv, idxes, (size_t)cur_time * (size_t)tasks_num);

            while (0 < Zeta_Core_MultiLevelVector_GetSize(&join_mlv)) {
                void** ele = Zeta_Core_MultiLevelVector_FindNextNotNull(
                    &join_mlv, idxes, TRUE);

                if (ele == NULL) {
                    ele = Zeta_Core_MultiLevelVector_FindNextNotNull(
                        &join_mlv, idxes, TRUE);
                }

                TaskInfo* join_task = (TaskInfo*)(*ele);

                TIME_T join_time = join_task->join_time;

                if (cur_time != join_time) {
                    nxt_join_time = join_time;
                    break;
                }

                Zeta_Core_MultiLevelVector_Erase(&join_mlv, idxes);

                join_task->rm_time = join_task->exe_time;
                join_task->join_time =
                    (join_task->join_time + join_task->period) % time_ring;

                ReadyMLV_Insert(join_task);
            }
        }

        bool_t is_end = (origin_join_mlv_size == (size_t)tasks_num - 1) &&
                        (Zeta_Core_MultiLevelVector_GetSize(&join_mlv) == 0);

        TaskInfo* nxt_task = ReadyMLV_Pop();

        if (cur_task == nxt_task) {
            ZETA_Core_DebugAssert(cur_task != &tasks[0]);

            if (display && cur_task_state == TASK_STATE_END) {
                PrintAbsCurTime(ss);

                ss << ' ' << cur_task->id << ' ' << ACT_END;
                if (!is_end) { ss << ' ' << nxt_task->id << ' ' << ACT_START; }
                ss << '\n';
            }
        } else {
            if (cur_task_state == TASK_STATE_PAUSE) { ++preemption_cnt; }

            if (display) {
                bool_t a = cur_task != &tasks[0];
                bool_t b = !is_end && nxt_task != &tasks[0];

                if (a || b) { PrintAbsCurTime(ss); }

                if (a) {
                    ss << ' ' << cur_task->id << ' '
                       << (cur_task_state == TASK_STATE_END ? ACT_END
                                                            : ACT_PREEMPTED);
                }

                if (b) {
                    ss << ' ' << nxt_task->id << ' '
                       << (nxt_task->exe_time == nxt_task->rm_time
                               ? ACT_START
                               : ACT_RESUME);
                }

                if (a || b) { ss << '\n'; }
            }
        }

        TIME_T finish_time = (cur_time + nxt_task->rm_time) % time_ring;

        if (nxt_task != &tasks[0] &&
            TIME_LT(nxt_task->join_time, finish_time)) {
            success = FALSE;
            break;
        }

        TIME_T nxt_time = finish_time;

        if (Zeta_Core_MultiLevelVector_GetSize(&join_mlv) != 0) {
            if (TIME_LT(nxt_join_time, nxt_time)) { nxt_time = nxt_join_time; }
        }

        TIME_T delta_time = (nxt_time + time_ring - cur_time) % time_ring;

        ZETA_Core_DebugAssert(0 < delta_time);

        if (nxt_task == &tasks[0]) {
            cur_task_state = TASK_STATE_END;
        } else {
            nxt_task->rm_time -= delta_time;

            if (nxt_task->rm_time == 0) {
                cur_task_state = TASK_STATE_END;

                if (nxt_task->join_time == 62528) { ZETA_Pause; }

                JoinMLV_Insert(nxt_task);
            } else {
                cur_task_state = TASK_STATE_PAUSE;
                ReadyMLV_Insert(nxt_task);
            }
        }

        if (is_end) { break; }

        IncAbsCurTime(delta_time);

        cur_time = nxt_time;
        cur_task = nxt_task;
    }

SCHEDULE_END:

    std::cout << run << '\n';

    if (success) {
        if (display) { std::cout << ss.rdbuf(); }

        PrintAbsCurTime(std::cout);

        std::cout << ' ' << preemption_cnt << '\n';
    } else {
        std::cout << "0 0\n";
    }

    Zeta_Core_MultiLevelVector_EraseAll(&join_mlv);
    Zeta_Core_MultiLevelVector_EraseAll(&ready_mlv);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int main() {
    std::cin.tie(nullptr);
    std::ios_base::sync_with_stdio(FALSE);

    Allocator_Init();

    std::cin >> num_of_runs;

    for (int task_i = 0; task_i < MAX_TASKS_NUM; ++task_i) {
        tasks[task_i].id = task_i;
    }

    for (run = 1; run <= num_of_runs; ++run) { F(); }

    return 0;
}
