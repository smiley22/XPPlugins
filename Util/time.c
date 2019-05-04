/**
 * Utility library for X-Plane 11 Plugins.
 *
 * Static library containing common functionality for stuff like logging and
 * dealing with configuration files. Linked against by most plugins in the
 * solution.
 *
 * Copyright 2019 Torben Könke.
 */
#include "util.h"

long long get_time_ms() {
#ifdef IBM
    static const __int64 EPOCH = ((__int64)116444736000000000ULL);
    SYSTEMTIME st;
    FILETIME ft;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    __int64 time = (((__int64)ft.dwHighDateTime) << 32) +
        ft.dwLowDateTime;
    return 1000L * ((time - EPOCH) / 10000000L) + st.wMilliseconds;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#endif
}
