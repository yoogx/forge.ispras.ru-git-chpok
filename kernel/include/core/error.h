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

#define POK_ERROR_KIND_INVALID                   9 // this is "NULL" error - that is, no error
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

#define POK_ERROR_MAX_MSG_SIZE                  250

typedef struct
{
   uint8_t        error_kind;
   uint32_t       failed_thread;
   uint32_t       failed_addr;
   char           msg[POK_ERROR_MAX_MSG_SIZE];
   uint32_t       msg_size;
} pok_error_status_t;

/*
 * Creates an error-handler thread for the current partition.
 *
 * It's created in stopped state.
 */
pok_ret_t   pok_error_thread_create (uint32_t stack_size, void* entry);

/*
 * Raises a flag that specified thread is in error.
 *
 * This automatically resets the context of error handler
 * and marks it as runnable.
 *
 * Caller is expected to call pok_sched() afterwards, which will
 * then switch to the error handler.
 *
 * Note: cannot be called when there's another error being
 * handled.
 */
void        pok_error_declare2 (uint8_t error, pok_thread_id_t thread_id);

/*
 * Same as pok_error_declare2(error, POK_SCHED_CURRENT_THREAD).
 */
void        pok_error_declare (const uint8_t error);

/*
 * Returns: POK_ERRNO_OK, if current thread is the error handler for the current partition.
 *          POK_ERRNO_UNAVAILABLE, otherwise.
 */
pok_ret_t pok_error_is_handler(void);

void        pok_partition_error (uint8_t partition, uint32_t error);
void        pok_kernel_error (uint32_t error);
void        pok_error_partition_callback (uint32_t partition);
void        pok_error_kernel_callback ();

void        pok_error_raise_application_error (char* msg, uint32_t msg_size);
pok_ret_t   pok_error_get (pok_error_status_t* status);
#define POK_ERROR_CURRENT_PARTITION(error) pok_partition_error(pok_current_partition, error);

#endif

#endif
