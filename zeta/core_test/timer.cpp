#include <zeta/core_test/timer.hpp>

#if defined(__LINUX__)

#include <sys/times.h>
#include <unistd.h>

#elif defined(_WIN32) || defined(_WIN64)

// IWYU: no_include, include "minwindef.h"
// IWYU: no_include, include "processthreadsapi.h"
#include <windows.h>

#else

#error "Unsupported system."

#endif

namespace zeta::core_test {

unsigned long long GetTime() {
#if defined(__LINUX__)
    static clock_t clk_per_sec{ sysconf(_SC_CLK_TCK) };

    tms time;
    times(&time);

    return time.tms_utime * 1000 / clk_per_sec;
#elif defined(_WIN32) || defined(_WIN64)
    HANDLE process = GetCurrentProcess();

    FILETIME crt_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    ULARGE_INTEGER user_time_64;

    GetProcessTimes(process, &crt_time, &exit_time, &kernel_time, &user_time);

    user_time_64.LowPart = user_time.dwLowDateTime;
    user_time_64.HighPart = user_time.dwHighDateTime;

    return user_time_64.QuadPart / 10000;
#endif
}

}  // namespace zeta::core_test
