/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2014, 2016 ISPRAS
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

/*
 *  ARINC thread.
 */

#ifndef __POK_THREAD_H__
#define __POK_THREAD_H__

#include <config.h>

// Note: Should come before possible inclusion of <core/partition.h>

#include <uapi/thread_types.h>

#include <types.h>
#include <errno.h>

#include <core/delayed_event.h>
#include <list.h>

#include <asp/cswitch.h>

#include <core/space.h>

/* Default stack size for main process */
#define DEFAULT_STACK_SIZE	4096

#ifdef POK_NEEDS_ERROR_HANDLING

/* 
 * Bits signalled about different *sources* of error.
 * 
 * NOTE: Different bits may be mapped to the same error id.
 * This is needed, because errors may be set async, while error handler
 * is executed.
 */
typedef uint8_t pok_thread_error_bits_t;

/* 
 * Synchronous error has been generated for thread.
 * 
 * Because synchronous errors cannot be generated while thread is
 * inactive, and after each of them error handler will be launched,
 * at most one error can be active for given thread.
 * 
 * Moreover, as error handler preempts all other threads, at most
 * one syncrhonouse error can be active for *whole partition*.
 * 
 * Because of that, sync error id is stored in `partition.sync_error`.
 * 
 * Fault address is stored in partition.sync_error_fault_address.
 *
 * In case of application error thread.wait_buffer.src points to the
 * message and thread.wait_len contains length of that message.
 * 
 * NOTE: Thread's fields 'wait_buffer' and 'wait_len' are not reseted on
 * 'thread_stop()'/'thread_start()' calls.
 * So, information about error remains while error handler is executed.
 */
#define POK_THREAD_ERROR_BIT_SYNC 1
/*
 * Deadline triggered.
 * 
 * Error id is POK_ERROR_ID_DEADLINE_MISSED.
 * 
 * Fault address is NULL.
 */
#define POK_THREAD_ERROR_BIT_DEADLINE 2

/*
 * While calculating next deadline, resulted time is out-of-range.
 * 
 * Error id is POK_ERROR_ID_ILLEGAL_REQUEST.
 * 
 * Fault address is NULL.
 * 
 * Note: While this event is incompatible with previous one
 * (Deadline missed), we need different bits for distinguish them.
 */
#define POK_THREAD_ERROR_BIT_DEADLINE_OOR 4


#endif /* POK_NEEDS_ERROR_HANDLING */

typedef struct _pok_thread
{
    /*
     * The priority given at process creation.
     * 
     * Final after create_process().
     */
    uint8_t             base_priority;

    /*
     * Current priority (can be adjusted with SET_PRIORITY).
     *
     * Initialized to base_priority when process is started.
     */
    uint8_t             priority;

    /*
     * If process is periodic, this is (positive) process's period.
     * If process aperiodic, this is POK_TIME_INFINITY.
     * 
     * Final after create_process().
     */
    pok_time_t          period;

    /*
     * Deadline type (soft or hard).
     * 
     * As per ARINC-653, it's only used only by error handling process,
     * and the interpretation is up to programmer.
     */
    pok_deadline_t      deadline;

    /*
     * If process has time is limited, this is (positive) time capacity.
     * Otherwise this is POK_TIME_INFINITY.
     *
     * Final after create_process().
     */
    pok_time_t          time_capacity;

    /*
     * Deadline event (called DEADLINE_TIME in ARINC-653).
     *
     * When this time hits, HM event is generated (error handling),
     * and process becomes... TODO what it becomes?
     *
     * Empty list if the process (currently) has no deadline.
     */
    struct delayed_event thread_deadline_event;

    /* 
     * Any other event delayed for a time.
     * 
     * Empty list if the process currently has no delayed event.
     */
    struct delayed_event thread_delayed_event;

    /* When process is started in INIT_* mode, it sets this field
     * to the delay.
     * 
     * TODO: This field can be combined(union) with @thread_delayed_event.
     */
    pok_time_t delayed_time;

    /* 
     * Function, processing delayed event in @thread_deadline_event.
     * 
     * Used only in conjunction with that field.
     */
    void (*thread_delayed_func)(struct _pok_thread* t);

    /**
     * Element in the wait queue on some object.
     * 
     * Empty list if the process currently doesn't wait on object.
     */
    struct list_head       wait_elem;

    /**
     * If wait queue uses priority/FIFO order, this field stores
     * priority at the moment when thread has been added to the queue.
     * 
     * Used only in conjunction with @wait_elem.
     */
    uint8_t wait_priority;

    /**
     * If wait on queuing port, this is pointer to the message which
     * should be sent/received from it.
     * 
     * Used only in conjunction with @wait_elem.
     */
    union {
        const void* src;
        void* dest;
    } wait_buffer;

    /**
     * If wait on port, this is length of the message.
     * This is OUT parameter for receive port and IN - for send port.
     * 
     * Used only in conjunction with @wait_elem.
     */
    size_t wait_len;

    /**
     * If wait on something, here will be stored result of this wait.
     */
    pok_ret_t wait_result;

    /*
     * Next activation for periodic process (called "release point" in ARINC-653).
     *
     * It's calculated when:
     *  - START/DELAYED_START is called in NORMAL mode
     *  - SET_PARTITION_MODE is called, and START/DELAYED start was
     *    already called before
     *  - TIMED_WAIT (pok_sched_end_period) is called
     * 
     * This also implies that for currently active periodic process,
     * next_activation actually refers to time of _current_ activation.
     *
     * Is not defined for aperiodic processes,
     * and when process is not yet started (or not in normal mode).
     */
    uint64_t            next_activation;

    /*
     * Process state.
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
     * If preempted process entering the msection, it is a pointer to it.
     * 
     * Otherwise NULL.
     * 
     * When process continues to execute, it enters msection automatically.
     * 
     * This field is also set when the process waits via 'msection_wait()'.
     */
    struct msection* __kuser msection_entering;

    /*
     * Relations between callers and targets of STOP() function.
     * 
     * Relation is initiated when the target of STOP() function needs to
     * execute code (until it leaves the last msection) before terminating.
     * 
     * Such execution may be performed on *donating* basis:
     * thread, which needs some other one to terminate, donates its time for
     * other thread.
     * 
     * 
     * General form of such relation is a list of threads (STOP() *callers*),
     * each of them may continue to work, when a single thread
     * (STOP() *target*) is stopped.
     * 
     * If any thread in the list is a target for some STOP() call, then
     * caller of that function should be *after* given thread in the list.
     * 
     * Such list organization is possible, because:
     * 1. After being STOP() *target*, thread cannot call STOP().
     *     (calling STOP() within msection is prohibited).
     */
    struct {
        /* 
         * First element in the list, which needs to donate execution time
         * to us for being able to do any progress.
         * 
         * Invariant (if not NULL):
         *   t->relations_stop.first_donator->donate_target == t
         * 
         * NULL if no one STOP()'s us.
         */
        struct _pok_thread* first_donator;
        /* 
         * Next element in the list, which needs to donate execution time
         * to the same '.donate_target' as we have.
         * 
         * Has a sence only if '.donate_target' is not NULL.
         * 
         * Invariant(if not NULL):
         *   t->relations_stop.donate_target ==
         *   t->relations_stop.next_donator->relations_stop.donate_target
         * 
         * NULL if there is no other threads.
         */
        struct _pok_thread* next_donator;
        /* 
         * Target for our donation.
         * 
         * Whenever this field is cleared for part->thread_selected,
         * pok_sched_local_invalidate() should be called.
         * 
         * NULL if we need no one to terminate.
         */
        struct _pok_thread* donate_target;
    } relations_stop;

    /*
     * Process entry point.
     * 
     * Final after create_process().
     */
    void* __user        entry;

    /*
     * Kernel stack address.
     *
     * It's used to implement context switch.
     *
     * It's initially set in pok_thread_create,
     * and updated by pok_context_switch.
     *
     */
    struct jet_context*	        sp;

    /*
     * Initial value of kernel stack (when it was allocated).
     *
     * Used for restarting thread.
     * 
     * Final after thread's initialization.
     */
    jet_stack_t         initial_sp;

    /*
     * Pointer to area for save floating point registers for given thread.
     * It is allocated at partition's initialization.
     */

    struct jet_fp_store*    fp_store;

    /*
     * ???
     *
     * Apparently, it's initial virtual address of user stack.
     *
     * It's supposed to be used when thread is restarted (I think).
     * 
     * Final after create_process().
     */
    jet_ustack_t        init_stack_addr;

    /*
     * Size of the user space stack.
     */
    uint32_t            user_stack_size;

    /**
     *  Linkage in the `eligible_threads` in partition.
     * 
     * If the process is not eligible, then empty list.
     */
    struct list_head       eligible_elem;

#ifdef POK_NEEDS_ERROR_HANDLING
    struct list_head       error_elem;       /** Linkage for partition's `.error_list`. */
    pok_thread_error_bits_t error_bits;
#endif

    // Whether thread is in unrecoverable error state.
    pok_bool_t is_unrecoverable;

#ifdef POK_NEEDS_GDB
    /*
     * Interrupt context where all user space registers have been saved.
     * 
     * If user space has never been called yet, this is 0.
     */
    struct jet_interrupt_context* entry_sp_user;
#endif /* POK_NEEDS_GDB */
    /* 
     * Name of the process.
     * 
     * Empty ("") name means that process is not created.
     */
    char 		    name [MAX_NAME_LENGTH];
} pok_thread_t;

/**
 * Queue of threads, waited for specific event.
 * 
 * All methods for the queue should be called with local preemption
 * disabled.
 */
typedef struct {
    /* Ordered list of wait threads (linkage it wait_elem). */
    struct list_head waits;
} pok_thread_wq_t;

/** Initialize wait queue with empty list of threads. */
void pok_thread_wq_init(pok_thread_wq_t* wq);

/**
 * Add thread into the queue with FIFO order.
 */
void pok_thread_wq_add(pok_thread_wq_t* wq, pok_thread_t* t);

/**
 * Add thread into the queue with priority/FIFO order.
 */
void pok_thread_wq_add_prio(pok_thread_wq_t* wq, pok_thread_t* t);

/**
 * Add thread into the queue with queuing order given as `discipline` parameter.
 */
static inline void pok_thread_wq_add_common(pok_thread_wq_t* wq,
    pok_thread_t* t, pok_queuing_discipline_t discipline)
{
    if(discipline == POK_QUEUING_DISCIPLINE_FIFO)
        pok_thread_wq_add(wq, t);
    else
        pok_thread_wq_add_prio(wq, t);
}

/**
 * Remove thread from wait queue, if it was.
 */
void pok_thread_wq_remove(pok_thread_t* t);

/**
 * Wake up thread in the queue.
 * 
 * Thread will be removed from the queue.
 * 
 * Return thread awoken or NULL is queue is empty.
 */
pok_thread_t* pok_thread_wq_wake_up(pok_thread_wq_t* wq);


/**
 * Return number of threads in the wait queue.
 * 
 * NOTE: complexity is linear.
 */
int pok_thread_wq_get_nwaits(pok_thread_wq_t* wq);


/**
 * Return TRUE if wait queue is empty.
 * 
 * Return FALSE is not empty.
 * 
 * NOTE: complexity is constant.
 */
pok_bool_t pok_thread_wq_is_empty(pok_thread_wq_t* wq);

// macro-like utitility functions
static inline
pok_bool_t pok_thread_is_periodic(const pok_thread_t *thread)
{
    return !pok_time_is_infinity(thread->period);
}

static inline
pok_bool_t pok_thread_is_runnable(const pok_thread_t *thread)
{
    return thread->state == POK_STATE_RUNNABLE && !thread->suspended;
}


/**
 * Create a thread inside a partition
 * Return POK_ERRNO_OK if no error.
 * Return POK_ERRNO_TOOMANY if the partition cannot contain
 * more threads.
 */
pok_ret_t pok_thread_create (const char* __user name,
    void* __user entry,
    const pok_thread_attr_t* __user attr,
    pok_thread_id_t* __user thread_id);

pok_ret_t pok_thread_start(pok_thread_id_t thread_id);
pok_ret_t pok_thread_suspend(const pok_time_t* time);
pok_ret_t pok_thread_suspend_target(pok_thread_id_t id);
pok_ret_t pok_thread_yield (void);
pok_ret_t pok_thread_stop(void);
pok_ret_t pok_thread_stop_target(pok_thread_id_t thread_id);
pok_ret_t pok_thread_delayed_start (pok_thread_id_t id, const pok_time_t* __user delay_time);
pok_ret_t pok_thread_get_id_self(pok_thread_id_t* __user thread_id);
pok_ret_t pok_thread_get_id(const char* __user name, pok_thread_id_t* __user thread_id);

pok_ret_t pok_thread_get_status(pok_thread_id_t id,
    char* __user name,
    void** __user entry,
    pok_thread_status_t* __user attr);

pok_ret_t pok_thread_set_priority(pok_thread_id_t id, const uint32_t priority);
pok_ret_t pok_thread_resume(pok_thread_id_t id);

#ifdef POK_NEEDS_THREAD_SLEEP
pok_ret_t pok_thread_sleep(const pok_time_t* __user time);
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
pok_ret_t pok_thread_sleep_until(const pok_time_t* __user time);
#endif

/* Find thread by its name. GET_PROCESS_ID in ARINC. */
pok_ret_t pok_thread_find(const char* __user name, pok_thread_id_t* id);

pok_ret_t pok_sched_end_period(void);
pok_ret_t pok_sched_replenish(const pok_time_t* __user budget);

pok_ret_t pok_sched_get_current(pok_thread_id_t* __user thread_id);

#endif /* __POK_THREAD_H__ */
