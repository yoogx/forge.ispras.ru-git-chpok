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
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by julien on Thu Jan 15 23:34:13 2009
 */


#ifndef __POK_THREAD_H__
#define __POK_THREAD_H__

#ifdef POK_NEEDS_THREADS

#include <types.h>
#include <errno.h>
#include <core/sched.h>


/*
 * In POK, we add a kernel thread and an idle thread. The kernel
 * thread is used to execute kernel code while the idle thread
 * is used to save processor resources.
 */

#define KERNEL_THREAD		POK_CONFIG_NB_THREADS -2
#define IDLE_THREAD        POK_CONFIG_NB_THREADS -1

#define POK_THREAD_DEFAULT_TIME_CAPACITY 10

/*
#define KERNEL_THREAD		POK_CONFIG_NB_THREADS
#define IDLE_THREAD        POK_CONFIG_NB_THREADS + 1
*/

#define POK_THREAD_MAX_PRIORITY  200


/*
 * IDLE_STACK_SIZE is the stack size of the idle thread
 * DEFAULT_STACK_SIZE if the stack size of regulard threads
 */
#define IDLE_STACK_SIZE		1024
#define DEFAULT_STACK_SIZE	4096

#ifndef POK_USER_STACK_SIZE
#define POK_USER_STACK_SIZE 8096
#endif

typedef enum {
    DEADLINE_SOFT,
    DEADLINE_HARD
} pok_deadline_t;

typedef struct
{
    /*
     * The priority given at process creation.
     */
    uint8_t             base_priority; 
    
    /*
     * Current priority (can be adjusted with SET_PRIORITY).
     *
     * Initialized to base_priority when process is created, reset, etc.
     */
    uint8_t             priority;

    /*
     * Process period.
     *
     * Positive when process is periodic, negative - if aperiodic.
     * Is never zero.
     */
    int64_t             period;

    /*
     * The time when process became ready.
     *
     * Is used by the scheduler to elect the process
     * that has been in the ready state for the longest time.
     *
     * Undefined when process is not ready 
     * (may contain stale value, or simply be unitialized at all).
     */
    uint64_t            ready_state_since;

    /*
     * Deadline type (soft or hard). As per ARINC-653, it's only used only by
     * error handling process, and the interpretation is up to programmer.
     */
    pok_deadline_t      deadline; 
    
    /*
     * Time capacity, as given at the process creation.
     *
     * If it's less than zero, time capacity is infinite.
     *
     * For periodic process, this's time capacity per each activation.
     */
    int64_t             time_capacity; 

    /*
     * Deadline time (called DEADLINE_TIME in ARINC-653).
     *
     * When this time hits, HM event is generated (error handling),
     * and process becomes... TODO what it becomes?
     *
     * Undefined if process time capacity is infinite.
     */
    int64_t             end_time;
    
    /*
     * Next activation for periodic process (called "release point" in ARINC-653).
     *
     * Is not defined for aperiodic processes.
     */
    uint64_t            next_activation; 

    /*
     * Process state (see core/sched.h).
     */
    pok_state_t         state;

    /*
     * The flag is set if process is suspended.
     *
     * It cannot be implemented as a separate state because 
     * process can be suspended in any state, and it must return
     * to that state when it's resumed.
     *
     * If suspension was implemented with states, it would require 
     * something like "state stack", which would be overkill.
     */
    pok_bool_t          suspended;

    /*
     * Suspension timeout (absolute time).
     *
     * Set to (uint64_t)-1 when it's supposed to be infinite.
     * This obviously assumes that uint64_t ms clock won't overflow
     * in foreseeable feature.
     *
     * Undefined if process is not suspended.
     */
    uint64_t             suspend_timeout;

    /*
     * Wakeup time, in case process is in POK_STATE_WAITING.
     *
     * If process is not in that state, the value is undefined.
     */
    uint64_t            wakeup_time; 

    /*
     * Process entry point.
     */
    void	        *entry;

    /*
     * Corresponding parition id.
     */
    pok_partition_id_t  partition;

    /*
     * Kernel stack addres.
     *
     * It's used to implement context switch.
     *
     * It's initially set in pok_partition_thread_create,
     * and updated by pok_context_switch. 
     *
     */
    uint32_t	        sp; 

    /*
     * Initial value of kernel stack (when it was allocated).
     *
     * Used for restarting thread.
     */
    uintptr_t           initial_sp;

    /*
     * ???
     *
     * Apparently, it's initial virtual address of user stack. 
     *
     * It's supposed to be used when thread is restarted (I think).
     */
    uint32_t            init_stack_addr; 
} pok_thread_t;

/*
 * Attributes given to create a thread
 */
typedef struct
{
	 uint8_t      priority;         /* Priority is from 0 to 255 */
	 void*        entry;            /* entrypoint of the thread  */
	 int64_t      period;
	 pok_deadline_t deadline;
	 int64_t     time_capacity;
	 uint32_t     stack_size;
} pok_thread_attr_t;

typedef struct 
{
        pok_thread_attr_t   attributes;
        uint64_t            deadline_time;
	pok_state_t         state;
        pok_bool_t          suspended;
        uint8_t             current_priority;
} pok_thread_status_t;


void            pok_thread_init(void);
pok_ret_t       pok_thread_create (pok_thread_id_t* thread_id, const pok_thread_attr_t* attr);
pok_ret_t       pok_thread_sleep(int64_t ms);
pok_ret_t       pok_thread_sleep_until(uint64_t ms);
void            pok_thread_start(void (*entry)(), unsigned int id);
pok_ret_t       pok_thread_suspend(int64_t ms);
pok_ret_t       pok_thread_suspend_target(pok_thread_id_t id);
pok_ret_t       pok_thread_stop(void);
pok_ret_t       pok_thread_stop_target(pok_thread_id_t);
pok_ret_t       pok_thread_delayed_start (pok_thread_id_t id, int64_t ms);
pok_ret_t       pok_thread_get_status(pok_thread_id_t id, pok_thread_status_t *attr);
pok_ret_t       pok_thread_set_priority(pok_thread_id_t id, const uint32_t priority);
pok_ret_t       pok_thread_resume(pok_thread_id_t id);
pok_ret_t       pok_thread_yield(void);

#ifdef POK_NEEDS_PARTITIONS
pok_ret_t       pok_partition_thread_create(
        pok_thread_id_t         *thread_id,
        const pok_thread_attr_t *attr,
	pok_partition_id_t      partition_id);
#endif

extern pok_thread_t              pok_threads[POK_CONFIG_NB_THREADS];

#define POK_CURRENT_THREAD pok_threads[POK_SCHED_CURRENT_THREAD]

// macro-like utitility functions

static inline pok_thread_id_t
pok_thread_get_id(const pok_thread_t *thread)
{
    return (pok_thread_id_t) (thread - &pok_threads[0]);
}   

static inline pok_bool_t
pok_thread_is_periodic(const pok_thread_t *thread)
{
    return thread->period > 0;
}

static inline pok_bool_t
pok_thread_is_runnable(const pok_thread_t *thread)
{
    return thread->state == POK_STATE_RUNNABLE && !thread->suspended;
}

#endif /* __POK_NEEDS_THREADS */

#endif /* __POK_THREAD_H__ */
