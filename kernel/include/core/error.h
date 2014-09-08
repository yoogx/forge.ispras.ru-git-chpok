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
 * Created by julien on Mon Jan 19 10:51:40 2009 
 */

#ifdef POK_NEEDS_ERROR_HANDLING

#ifndef __POK_CORE_ERROR_H__
#define __POK_CORE_ERROR_H__

#include <types.h>
#include <core/sched.h>

#define POK_ERROR_MAX_MSG_SIZE                  250

typedef uint8_t pok_error_kind_t;
#define POK_ERROR_KIND_INVALID                   0 // this is "NULL" error - that is, no error
#define POK_ERROR_KIND_DEADLINE_MISSED          10
#define POK_ERROR_KIND_APPLICATION_ERROR        11
#define POK_ERROR_KIND_NUMERIC_ERROR            12
#define POK_ERROR_KIND_ILLEGAL_REQUEST          13
#define POK_ERROR_KIND_STACK_OVERFLOW           14
#define POK_ERROR_KIND_MEMORY_VIOLATION         15
#define POK_ERROR_KIND_HARDWARE_FAULT           16
#define POK_ERROR_KIND_POWER_FAIL               17
#define POK_ERROR_KIND_PARTITION_CONFIGURATION  30 
#define POK_ERROR_KIND_PARTITION_INIT           31
#define POK_ERROR_KIND_PARTITION_SCHEDULING     32
#define POK_ERROR_KIND_PARTITION_HANDLER        33
#define POK_ERROR_KIND_PARTITION_PROCESS        34
#define POK_ERROR_KIND_KERNEL_INIT              50
#define POK_ERROR_KIND_KERNEL_SCHEDULING        51
#define POK_ERROR_KIND_KERNEL_CONFIG            52


typedef uint8_t pok_error_action_t;
#define POK_ERROR_ACTION_IGNORE 1
#define POK_ERROR_ACTION_IDLE 2
#define POK_ERROR_ACTION_COLD_START 3
#define POK_ERROR_ACTION_WARM_START 4

typedef uint8_t pok_error_level_t;
#define POK_ERROR_LEVEL_PARTITION 1 // fixed recovery action is taken
#define POK_ERROR_LEVEL_PROCESS 2 // error can be dealt with by error handler

typedef struct
{
   pok_error_kind_t     error_kind;
   pok_thread_id_t      failed_thread;
   uintptr_t            failed_addr;
   size_t               msg_size;
   char                 msg[POK_ERROR_MAX_MSG_SIZE];
} pok_error_status_t;

typedef struct 
{
    pok_error_kind_t kind; // error code
    pok_error_level_t level; // can it be passed to error handler OR action should be taken immediately?
    pok_error_action_t action; // action to take if level is 'partition' OR if error handler is not created
    pok_error_kind_t target_error_code; // error code to pass to error handler process (has to be defined only if level is 'process')
} pok_error_hm_partition_t;

/*
 * Creates an error-handler thread for the current partition.
 *
 * It's created in stopped state.
 */
pok_ret_t   pok_error_thread_create (uint32_t stack_size, void* entry);

/*
 * Raises a thread-level error
 * (which might be promoted to partition error in some circumstances).
 *
 * If everything is OK, this automatically resets the context of error handler
 * and marks it as runnable.
 *
 * Caller is expected to call pok_sched() afterwards, which will
 * then switch to the error handler.
 *
 * Note: cannot be called when there's another error being
 * handled (TODO remove this limitation?)
 */
pok_ret_t  pok_error_raise_thread(
        pok_error_kind_t error, 
        pok_thread_id_t thread_id, 
        const char *message,
        size_t message_length);

/*
 * Used for simple synchronous errors without error messages and that stuff.
 */
#define POK_ERROR_CURRENT_THREAD(error) pok_error_raise_thread(error, POK_SCHED_CURRENT_THREAD, NULL, 0)

/*
 * Returns: POK_ERRNO_OK, if current thread is the error handler for the current partition.
 *          POK_ERRNO_UNAVAILABLE, otherwise.
 */
pok_ret_t pok_error_is_handler(void);

/*
 * Raises partition-level error.
 */
void        pok_error_raise_partition(pok_partition_id_t partition, pok_error_kind_t error);

/*
 * Raises kernel (system) error.
 */
void        pok_error_raise_kernel(pok_error_kind_t error);

/*
 * Raise thread-level application error.
 *
 * This's a system call, it validates a couple of parameters
 * and passes them to pok_thread_error almost verbatim.
 */
pok_ret_t   pok_error_raise_application_error (const char* msg, size_t msg_size);

/*
 * Pops an error from partition error queue.
 */
pok_ret_t   pok_error_get (pok_error_status_t* status);

#endif

#endif
