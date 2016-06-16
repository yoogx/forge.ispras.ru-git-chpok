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
#include <core/error_arinc.h>
#include <core/port.h>
#include <core/intra_arinc.h>

#include <uapi/partition_arinc_types.h>

/*!
 * \struct pok_partition_t
 * \brief This structure contains all needed information for partition management
 */
typedef struct _pok_patition_arinc
{
    pok_partition_t        base_part;
    
    /* Information which affects on scheduling has been changed. */
    pok_bool_t sched_local_recheck_needed;

    pok_partition_mode_t   mode;           /**< Current mode of the partition */

    /* 
     * Size of the allocated memory segment.
     * 
     * Set in deployment.c.
     */
    uint32_t               size;           
    uint32_t               base_addr;    /**< The base address inside the whole memory (where the segment is in the whole memory ?) */
    uint32_t               base_vaddr;   /**< The virtual address of the partition. The address the threads sees when they are executed */
    
    uint32_t               user_stack_state; /* State of the use-space allocation. */

    pok_time_t             activation; // Not used now

    /*
     * Number of (allocated) threads inside the partition.
     * 
     * Set in deployment.c.
     */
    uint32_t               nthreads;       
    /*
     * Array of (allocated) threads inside the partition.
     * 
     * Set in deployment.c. (thread needn't to be initialized there).
     */
    pok_thread_t*          threads;
    uint32_t               nthreads_used;   /**< Number of threads which are currently in use (created). */


    pok_thread_t*          thread_current; // Normal thread or special thread. NULL if doing nothing.


    pok_port_queuing_t*    ports_queuing; /* List of queuing ports. Set in deployment.c. */
    size_t                 nports_queuing;

    pok_port_sampling_t*   ports_sampling; /* List of sampling ports. Set in deployment.c. */
    size_t                 nports_sampling;


   /*
    * Size of memory pre-allocated for intra-partition communicaton
    * objects.
    * 
    * This memory can be used by buffers and blackboard for allocate
    * queue of messages.
    * 
    * Set in deployment.c
    */
   size_t                  intra_memory_size; 
   
   // Memory allocated at init for intra-partition communicaton objects.
   void*                   intra_memory; 
   
   /* Size of currently used `intra_memory`. Reseted at partition restart. */
   size_t                  intra_memory_size_used;
   

   pok_buffer_t*           buffers; // List of 'slots' for buffers. Set in deployment.c
   size_t                  nbuffers; // Number of buffers allocated. Set in deployment.c
   size_t                  nbuffers_used; // Number of buffers which are currently created.
   
   pok_blackboard_t*       blackboards; // List of 'slots' for blackboards. Set in deployment.c
   size_t                  nblackboards; // Number of blackboards allocated. Set in deployment.c
   size_t                  nblackboards_used; // Number of blackboards which are currently created.

   pok_semaphore_t*        semaphores; // List of 'slots' for semaphores. Set in deployment.c
   size_t                  nsemaphores; // Number of semaphores allocated. Set in deployment.c
   size_t                  nsemaphores_used; // Number of semaphores which are currently created.

   pok_event_t*            events; // List of 'slots' for events. Set in deployment.c
   size_t                  nevents; // Number of events allocated. Set in deployment.c
   size_t                  nevents_used; // Number of events which are currently created.


/* Error and main threads are special in sence that they cannot be reffered by ID.*/

#ifdef POK_NEEDS_ERROR_HANDLING
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
#endif

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


    uintptr_t               main_entry;
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
     * Queue of threads with deadline events.
     * 
     * Corresponded thread's field is .thread_deadline_event.
     */
    struct delayed_event_queue queue_deadline;
    /** 
     * Queue of delayed events, which should awoke thread
     * (or make it closer to RUNNING state).
     * 
     * Corresponded thread's field is .thread_delayed_event.
     */
    struct delayed_event_queue queue_delayed;
    
    /* 
     * After main partition's thread creates start thread,
     * it is used as idle thread.
     * 
     * This is context pointer for switch to it.
     */
    uint32_t               idle_sp;
} pok_partition_arinc_t;

#define current_partition_arinc container_of(current_partition, pok_partition_arinc_t, base_part)
#define current_thread (current_partition_arinc->thread_current)

/* 
 * Array of ARINC partitions.
 * 
 * Set in module's config.
 */
extern pok_partition_arinc_t* pok_partitions_arinc;

/*
 * Number of ARINC partitions.
 * 
 * Set in module's config.
 */
extern uint8_t pok_partitions_arinc_n;

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

// utility macro-like functions


/**
 * Chech that pointer \a ptr is located in the address space of partition
 * \a pid
 */

/* TODO dirty as hell */

#ifdef __i386__
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= current_partition_arinc->base_addr && \
                                             ((uintptr_t)(ptr)) <  current_partition_arinc->base_addr + current_partition_arinc->size\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= current_partition_arinc->base_vaddr && \
                                             ((uintptr_t)(ptr)) <  current_partition_arinc->base_vaddr + current_partition_arinc->size\
                                             )
#elif defined(__PPC__)
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )
#else
#error "POK_CHECK_PTR macros are not implemented for this arch, do it now!"
#endif

#endif /* !__POK_PARTITION_ARINC_H__ */
