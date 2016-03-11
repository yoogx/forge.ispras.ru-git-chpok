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

#ifndef __POK_CORE_ERROR_ARINC_H__
#define __POK_CORE_ERROR_ARINC_H__

#include <config.h>

#ifdef POK_NEEDS_ERROR_HANDLING

#include <types.h>
#include <core/error.h>
#include <core/sched.h>
#include <common.h>
#include <core/thread.h>

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


/* Information about thread-level error. */
typedef struct {
    pok_error_kind_t error_kind;
    const char* error_description;
}pok_thread_error_info_t;

/* 
 * Mapping of errors in POK_SYSTEM_STATE_USER state
 * into thread-level errors.
 */
typedef struct
{
    pok_thread_error_info_t map[POK_ERROR_ID_MAX + 1];
}pok_thread_error_map_t;

typedef struct
{
   pok_error_kind_t     error_kind;
   pok_thread_id_t      failed_thread;
   uintptr_t            failed_addr;
   size_t               msg_size;
   char                 msg[POK_ERROR_MAX_MSG_SIZE];
} pok_error_status_t;

/*
 * Creates an error-handler thread for the current partition.
 *
 * It's created in stopped state.
 */
pok_ret_t   pok_error_thread_create (uint32_t stack_size, void* __user entry);

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


/* 
 * Check partition state after error handler is finished.
 * 
 * Should be called with local preemption disabled.
 */
void error_check_after_handler(void);

#endif

typedef uint8_t pok_error_action_t;
#define POK_ERROR_ACTION_IGNORE 1
#define POK_ERROR_ACTION_IDLE 2
#define POK_ERROR_ACTION_COLD_START 3
#define POK_ERROR_ACTION_WARM_START 4

/*
 * Table of actions for partition-level errors.
 */
typedef struct {
    /* 
     * actions[STATE][ID] give action, which should be taken on module level.
     * 
     * Only those [STATE][ID] cells are used, which corresponds to
     * partition-level errors.
     */
    pok_error_action_t actions[POK_SYSTEM_STATE_MAX + 1][POK_ERROR_ID_MAX + 1];
} pok_error_hm_partition_t ;

/* 
 * Process (synchronous) error detected via interrupt.
 * 
 * Callback for `pok_partition_operations.process_partition_error`.
 */
void pok_partition_arinc_process_error(
    pok_system_state_t partition_state,
    pok_error_id_t error_id,
    uint8_t preempt_local_disabled_old,
    void* failed_address);


/* 
 * Emit Deadline Missed event for given thread.
 * 
 * Should be called with local preemption disabled.
 * 
 * Function may be called at most once during single critical section.
 */
void pok_thread_emit_deadline_missed(pok_thread_t* thread);

/* 
 * Emit Deadline Out of range event for given thread.
 * 
 * Should be called with local preemption disabled.
 * 
 * Function may be called at most once during single critical section.
 * 
 * After subsequent pok_preemption_local_enable() given thread
 * should be stopped (or restarted).
 */
void pok_thread_emit_deadline_oor(pok_thread_t* thread);

#endif /* __POK_CORE_ERROR_ARINC_H__ */
