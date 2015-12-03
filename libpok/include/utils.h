#ifndef __POK_UTILS_H__
#define __POK_UTILS_H__

#include <arinc653/types.h>

static inline int64_t arinc_time_to_ms(SYSTEM_TIME_TYPE time) {
    if (time < 0) {
        return -1;
    }
    uint64_t unsigned_time = (uint64_t) time;
    const uint64_t divisor = 1000000;
    uint64_t result = unsigned_time / divisor;
    // round up
    if (result * divisor != unsigned_time) result++;
    return (int64_t) result;
}

static inline SYSTEM_TIME_TYPE ms_to_arinc_time(int64_t time) {
    if (time < 0) {
        return -1;
    }
    return time * 1000000;
}
void strtoupper(char* s);

#endif
