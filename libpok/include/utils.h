#ifndef __POK_UTILS_H__
#define __POK_UTILS_H__

/* 
 * Useful functions for enforce alignment.
 * 
 * By analogy with Linux kernel.
 */

/* 
 * Return minimal value, which is equal-or-more than given one
 * and has required alignment.
 */
static inline unsigned long ALIGN(unsigned long val, unsigned long align)
{
    unsigned long mask = align - 1;
    return (val + mask) & ~mask;
}

/* 
 * Return minimal pointer, which is equal-or-more than
 * given one and has required alignment.
 */
static inline void* ALIGN_PTR(void* ptr, unsigned long align)
{
    return (void*)ALIGN((unsigned long)ptr, align);
}

/* Return number of elements in the array.
 * 
 * The array should be declared as '<type> arr[<num>];'.
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0])

// TODO: this should be removed when kernel will accept time in nanoseconds.
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

// TODO: this should be removed as it transforms possible read-only string.
void strtoupper(char* s);

#endif
