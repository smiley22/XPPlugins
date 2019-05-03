#include "util.h"

long long get_time_ms(void) {
#ifdef IBM
    const long long WINDOWS_TICK = 10000000LL;
    const long long SEC_TO_UNIX_EPOCH = 11644473600LL;
    SYSTEMTIME st;
    GetSystemTime(&st);
    FILETIME ft;
    if (!SystemTimeToFileTime(&st, &ft))
        return -1;
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#endif
}
