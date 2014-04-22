/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifdef POK_NEEDS_THREADS

#include <types.h>
#include <core/syscall.h>

pok_ret_t pok_thread_sleep (int64_t ms)
{
    int32_t val;
    if (ms > INT32_MAX) {
        return POK_ERRNO_ERANGE;
    } else if (ms < 0) {
        val = -1;
    } else {
        val = (int32_t) ms;
    }
   return pok_syscall2 (POK_SYSCALL_THREAD_SLEEP, (uint32_t)ms, 0);
}

#endif
