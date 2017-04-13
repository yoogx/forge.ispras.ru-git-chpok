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
 */

#ifndef __POK_PARTITION_ARINC_H__
#define __POK_PARTITION_ARINC_H__

#include <core/partition.h>
#include <core/ippc.h>
#include <core/memblocks.h>
#include <core/error_arinc.h>
#include <core/port.h>
#include <core/ippc_arinc.h>
#include <core/thread.h>

#include <uapi/partition_arinc_types.h>

/* One entry for initialize memory block(s) at PARTITION or PARTITION:COLD stage. */
struct jet_partition_memory_block_init_entry
{
    enum jet_memory_block_init_type init_type;
    const char* source_id;
    const struct memory_block* const* mblocks;
};


/* Entry for search memory block by address.*/
struct jet_partition_arinc_mb_addr_entry
{
    uintptr_t vaddr;
    size_t size;
    const struct memory_block* mblock;
};

/* Portal type provided by the partition. */
struct jet_partition_arinc_ippc_portal_type
{
    struct jet_ippc_portal_type* portal_type;
};

/* Portal used by the partition. */
struct jet_partition_arinc_ippc_portal
{
    /* Same as name for portal type. */
    const char* portal_name;
    struct jet_ippc_portal* portal;

    /* Whether portal is initialized from the sence of the partition. */
    pok_bool_t is_initialized;
};


/*
 * ARINC partition.
 */
typedef struct _pok_partition_arinc
{
    pok_partition_t        base_part;

    /*
     * Information which affects on scheduling in current execution mode
     * (.ippc_handled_prev).
     */
    pok_bool_t sched_local_recheck_needed;


    pok_partition_mode_t   mode;           /**< Current mode of the partition */

    pok_time_t             activation; // Not used now

    /*
     * Array of memory blocks for given partition.
     *
     * Set in deployment.c.
     */
    const struct memory_block* memory_blocks;
    int nmemory_blocks;

    /*
     * Array of memory block entries for search by address.
     *
     * Ordered by addresses (ascending).
     *
     * Set in deployment.c.
     */
    const struct jet_partition_arinc_mb_addr_entry* mb_addr_table;

    /* Array of entries for initialize memory blocks at 'PARTITION' stage. Set in deployment.c.*/
    const struct jet_partition_memory_block_init_entry* memory_block_init_entries;
    int memory_block_init_entries_n;

    /* Array of entries for initialize memory blocks at 'PARTITION:COLD' stage. Set in deployment.c.*/
    const struct jet_partition_memory_block_init_entry* memory_block_init_entries_cold;
    int memory_block_init_entries_cold_n;

    /* Array of IPPC portal types *provided* by partition. Set in deployment.c. */
    struct jet_partition_arinc_ippc_portal_type* portal_types;
    int portal_types_n;

    /* Array of IPPC portals *used* by partition. Set in deployment.c. */
    struct jet_partition_arinc_ippc_portal* portals;
    int portals_n;

    /*
     * Number of (allocated) threads inside the partition.
     *
     * nthreads = nthreads_normal + <nthreads_server> + 1 (main thread) + 1 (error thread).
     *
     * Set in deployment.c.
     */
    int               nthreads;
    /*
     * Array of (allocated) threads inside the partition.
     *
     * All threads are listed here: main one, normal ones and server ones.
     *
     * Set in deployment.c. (threads needn't to be initialized there).
     */
    pok_thread_t*          threads;

    /*
     * Number of threads which are currently in use (created).
     *
     * Threads are created continuously, that is 0...(nthreads_used - 1) are used.
     */
    int               nthreads_used;

    /*
     * Maximum number of threads for work in "normal" execution mode.
     *
     * Set in deployment.c.
     */
    int               nthreads_normal;

    /* Number of "main" threads which are currently in use (created). */
    int               nthreads_normal_used;

    /* Currently executed thread. */
    pok_thread_t*          thread_current;

    /*
     * Thread, selected according to scheduling algorithm.
     *
     * Selected thread may differ from current one, if selected
     * thread wants to enter into critical section, which already has an owner,
     * or selected thread kills a thread, which is currently in critical section.
     */
    pok_thread_t*          thread_selected;

    /*
     * When current thread releases given msection, it should be preempted.
     *
     * If thread_selected != thread_current but waiting_section is NULL,
     * then preemption should be occured when current thread leaves
     * all sections and terminates.
     */
    struct msection*        waiting_section;

    /* Size of the heap to be allocated. Set in the deployment.c */
    size_t heap_size;

    pok_port_queuing_t*    ports_queuing; /* List of queuing ports. Set in deployment.c. */
    size_t                 nports_queuing;

    pok_port_sampling_t*   ports_sampling; /* List of sampling ports. Set in deployment.c. */
    size_t                 nports_sampling;

/* Error and main threads are special in sence that they cannot be reffered by ID.*/

    pok_thread_t*          thread_error;     /**< Error thread. One of the @threads. */
    struct list_head       error_list;       /** List of threads in errorneus state. */

    pok_error_id_t         sync_error;
    void* __user           sync_error_failed_addr;

    /*
     * Select level for the partition error.
     *
     * Value 0 means partition level, 1 - thread(process) level.
     *
     * Should be set in deployment.c
     */
    const pok_error_level_selector_t* partition_hm_selector;

    /*
     * Map contains information about thread(process)-level errors.
     *
     * Should be set in deployment.c
     */
    const pok_thread_error_map_t* thread_error_info;
    struct jet_kernel_shared_data* kshd;

    /* Memory for allocate stacks. */
    uintptr_t stacks_start;
    uintptr_t stacks_end;

    /* Currently used memory for stacks. */
    uintptr_t stacks_current;

    /*
     * Pointer to partition HM table.
     */
    const pok_error_hm_partition_t* partition_hm_table;

    /*
     * Number of threads in unrecoverable state.
     *
     * This field is incremented every time a thread finds its error state
     * as unrecoverable. Error, emitted in this case, should be
     * processed at partition(or above) level or by error handler.
     * Otherwise, new error will be emitted for partition.
     *
     * The field is decremented every time unrecoverable thread is stopped.
     */
    uint32_t                nthreads_unrecoverable;


    void                    (*main_entry)(void);
    uint32_t                main_user_stack_size;


    uint32_t		        lock_level;
    pok_thread_t*           thread_locked; /* Thread which locks preemption. */

    /**
     * Priority/FIFO ordered queue of eligible threads.
     *
     * Used only in NORMAL mode.
     */
    struct list_head       eligible_threads;

    /**
     * Queue of all timed events.
     */
    struct delayed_event_queue partition_delayed_events;

    /*
     * After main partition's thread creates start thread,
     * it is used as idle thread.
     *
     * This is context pointer for switch to it.
     */
    struct jet_context*               idle_sp;
} pok_partition_arinc_t;

#define current_partition_arinc container_of(current_partition, pok_partition_arinc_t, base_part)
#define current_thread (current_partition_arinc->thread_current)

/*
 * Array of ARINC partitions.
 *
 * Set in deployment.c.
 */
extern pok_partition_arinc_t pok_partitions_arinc[];

/*
 * Number of ARINC partitions.
 *
 * Set in deployment.c
 */
extern const uint8_t pok_partitions_arinc_n;

/*
 * Get chunk of `intra_memory` with given size and alignment.
 *
 * Note, that this memory is reseted at partition restart.
 */
void* partition_arinc_im_get(size_t size, size_t alignment);

/*
 * Return current state of partition's intra memory.
 *
 * Value return may be used in partition_arinc_im_rollback().
 */
void* partition_arinc_im_current(void);

/*
 * Revert all intra memory usage requests since given state.
 *
 * `state` should be obtains with partition_arinc_im_current().
 */
void partition_arinc_im_rollback(void* prev_state);


// Main thread is always first allocated (Do not change this!).
#define POK_PARTITION_ARINC_MAIN_THREAD_ID 0


/* Initialize ARINC partitions. */
void pok_partition_arinc_init_all(void);

/** Initialize arinc partition. */
void pok_partition_arinc_init(pok_partition_arinc_t* part);

/*
 * Find memory block by name.
 *
 * Return NULL if not found.
 */
const struct memory_block* jet_partition_arinc_find_memory_block(
    pok_partition_arinc_t* part, const char* name);


/*
 * Find memory block for given virtual(user) address range.
 *
 * Return NULL if none block contains the range.
 */
const struct memory_block* jet_partition_arinc_get_memory_block_for_addr(
    pok_partition_arinc_t* part, const void* __user addr, size_t size);


/**
 * Reset current arinc partition.
 *
 * May be called as reaction for partition-level error
 * or by the use space via SET_PARTITION_MODE.
 *
 * Switch between COLD_START and WARM_START generates error.
 *
 * Appropriate START_CONDITION should be set before.
 */
void pok_partition_arinc_reset(pok_partition_mode_t mode);

/*
 * Move current partition into IDLE state.
 *
 * May be called at any time of partition's execution.
 */
void pok_partition_arinc_idle(void);

pok_ret_t pok_partition_set_mode_current (const pok_partition_mode_t mode);

pok_ret_t pok_current_partition_get_status (pok_partition_status_t* status);

pok_ret_t pok_current_partition_inc_lock_level(int32_t *lock_level);

pok_ret_t pok_current_partition_dec_lock_level(int32_t *lock_level);


/*
 * Raise error about inconsistent state of OS.
 *
 * User space may modify part of OS state directly writting some memory
 * cells. This is faster than syscalls, but may lead to inconsistent state
 * of OS. When kernel code detects inconsistency, it should call this function.
 */
// TODO: this should raise partition-level error.
#define assert_os(expr) assert(expr)

#endif /* !__POK_PARTITION_ARINC_H__ */
