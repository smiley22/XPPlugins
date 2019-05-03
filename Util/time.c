#include "util.h"

long long get_time_ms() {
#ifdef IBM
    static const __int64 EPOCH = ((__int64)116444736000000000ULL);
    SYSTEMTIME system_time;
    FILETIME file_time;
    __int64 time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((__int64)file_time.dwLowDateTime);
    time += ((__int64)file_time.dwHighDateTime) << 32;
    return 1000L * ((time - EPOCH) / 10000000L) + system_time.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#endif
}
