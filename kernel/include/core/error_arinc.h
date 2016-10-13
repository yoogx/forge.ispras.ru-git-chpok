/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
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

#include <uapi/error_arinc_types.h>

#define POK_ERROR_MAX_MSG_SIZE                  250


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
pok_ret_t   pok_error_raise_application_error (const char* __user msg, size_t msg_size);

/* 
 * Raise OS-error from user space.
 * 
 * Application never continues to work.
 */
pok_ret_t pok_error_raise_os_error (const char* __user msg, size_t msg_size);

/*
 * Pops an error from partition error queue.
 */
pok_ret_t   pok_error_get (pok_error_status_t* status,
    void* __user msg);


/* 
 * Check partition state after error handler is finished.
 * 
 * Should be called with local preemption disabled.
 */
void error_check_after_handler(void);

/*
 * If synchronous error is in the error list, clear it.
 * 
 * This function is called when error handler stops.
 * 
 * Because syncrhonous error cannot be generated while
 * error handler is executed, it always trigger error handler to start,
 * so it never can be missed because of race.
 */
void error_ignore_sync(void);

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
