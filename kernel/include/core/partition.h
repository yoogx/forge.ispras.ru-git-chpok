/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2013-2014, 2016 ISPRAS
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


#ifndef __POK_PARTITION_H__
#define __POK_PARTITION_H__

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/error.h>
#include <arch.h>

#include <gdb.h>

typedef enum
{
  POK_START_CONDITION_NORMAL_START          = 0,
  POK_START_CONDITION_PARTITION_RESTART     = 1,
  POK_START_CONDITION_HM_MODULE_RESTART     = 2,
  POK_START_CONDITION_HM_PARTITION_RESTART  = 3
}pok_start_condition_t;

struct _pok_partition;

/* Scheduling operations specific for given partition. */
struct pok_partition_sched_operations
{
    /*
     * Called when some async event about partition occures.
     * Event is encoded into partition's `.state` bits(bytes).
     * 
     * During call to this handler, local preemption is disabled.
     * 
     * The handler should either clear `.state` bytes, or enable local
     * preemption. Otherwise the handler will be called again.
     * 
     * If local preemption is already disabled, handler is not called.
     * But corresponded `.state` bytes are set nevetheless.
     */
    void (*on_event)(void);

    /* 
     * Return number of threads for given partition.
     * 
     * NOTE: Called (possibly) outside of partition's timeslot, so
     * 'current_partition' doesn't correspond to given partition.
     */
    int (*get_number_of_threads)(struct _pok_partition* part);

    /*
     * Return index of current thread within partition.
     * 
     * Returning negative value means that no current thread(is it possible?).
     * 
     * If partition doesn't work with user space, never called.
     * 
     * NOTE: Called (possibly) outside of partition's timeslot, so
     * 'current_partition' doesn't correspond to given partition.
     */
    int (*get_current_thread_index)(struct _pok_partition* part);

    /* 
     * Return 0 if thread with given index exists. Fill 'private' with
     * value, which will be passed for callbacks below.
     */
    int (*get_thread_at_index)(struct _pok_partition* part,
        int index, void** private);

    /*
     * Get information about thread with given index.
     * 
     * Write some string into buffer 'buf' of maximum capacity 'size'.
     * Return actual number of characters written.
     * 
     * NOTE: Called (possibly) outside of partition's timeslot, so
     * 'current_partition' doesn't correspond to given partition.
     */
    size_t (*get_thread_info)(struct _pok_partition* part, int index, void* private,
        char* buf, size_t size);

    /*
     * Get (architecture-specific) registers for non-current thread.
     */
    void (*get_thread_registers)(struct _pok_partition* part, int index, void* private,
        uint32_t registers[NUMREGS]);

    /*
     * Set (architecture-specific) registers for non-current thread.
     */
    void (*set_thread_registers)(struct _pok_partition* part, int index, void* private,
        const uint32_t registers[NUMREGS]);
};

struct pok_partition_operations
{
    /* 
     * Called when partition is (re)started.
     * 
     * Local preemption is disabled.
     * 
     * Cannot be NULL.
     */
    void (*start)(void);  /**< The entry-point for the partition's thread. */

    /* 
     * Process sync error related to given partition.
     * 
     * Local preemption is disabled.
     * 
     * Previous local preemption state is passed as parameter.
     * 
     * Cannot be NULL.
     */
    void (*process_partition_error)(pok_system_state_t partition_state,
        pok_error_id_t error_id,
        uint8_t state_byte_preempt_local,
        void* failed_address);
};


/*!
 * \struct pok_partition_t
 * \brief This structure contains all needed information for partition management
 */
typedef struct _pok_partition
{
#ifdef POK_NEEDS_MONITOR
    pok_bool_t               is_paused;      /*Partition paused or not*/
#endif

    const struct pok_partition_sched_operations* part_sched_ops;
    const struct pok_partition_operations* part_ops;
    
    /* 
     * State of the partition.
     * 
     * This is like "state register" notion used for architectures.
     */
    union {
        /* 
         * State flags (bytes). Can be 0 or 1.
         * 
         * Because flags are bytes (not bits), they can be set atomically
         * without locked operations.
         */
        struct {
            /*
             * Set when time is changed.
             */
            uint8_t time_changed;
            /*
             * Set after time slot is changed from other partition to given one.
             */
            uint8_t control_returned;

            uint8_t unused1;
            uint8_t unused2;
        } bytes;
        /* 
         * All flags at once.
         * 
         * This value may be checked by partition after enabling preemption
         * for ensure, that it hasn't miss other flags.
         */
        uint32_t bytes_all;
    } state;

    /*
     * Whether local preemption is disabled.
     * 
     * Flag can be read, modified or cleared by the partition itself.
     * 
     * Flag is set and checked by the global scheduler,
     * when it need to call partition's callbacks.
     */
    uint8_t preempt_local_disabled;
    
    /* 
     * Whether currently executed *user space* process is error handler.
     * 
     * This field is used for determine system level in case when
     * error is catched via interrupt.
     * 
     * Reseted to false when partition starts.
     */
    pok_bool_t               is_error_handler;

    const char               *name;          /**< Name of the partition */

    uint8_t                  priority;       /**< Priority of the partition (unused at this time */
    uint32_t                 period;         /**< Period of the partition, unused at this time */
    uint32_t                 duration;       /**< Duration of the partition */

    /*
     * Kernel stack address which is used for enter into the partition.
     * 
     * 0 value in this field means that partition needs to be (re)started.
     *
     */
    uint32_t	        	 sp; 

    /*
     * Initial value of kernel stack (when it was allocated).
     *
     * Used for restarting partition.
     * 
     * Set by particular partition's implementation.
     */
    struct dStack            initial_sp;

    /* 
     * Identificator of (user) space, corresponded to given partition.
     * Special value 0xff means that no user space is used by this partition.
     * 
     * Set in deployment.c
     * 
     * Used by scheduler when it switch into partition.
     */
    uint8_t                  space_id;

    /*
     * Pointer to the user space registers array, stored for given partition.
     * 
     * Set by global scheduler, used (may be cleared) by partition.
     */
    uint32_t                entry_sp_user;

    /*
     * Pointer to the registers array, stored for given partition when switched off.
     * 
     * Used only by global scheduler.
     */
    uint32_t                entry_sp;

#ifdef POK_NEEDS_IO
  uint16_t		    io_min;             /**< If the partition is allowed to perform I/O, the lower bound of the I/O */
  uint16_t		    io_max;             /**< If the partition is allowed to perform I/O, the uppder bound of the I/O */
#endif

  pok_start_condition_t	    start_condition;
  
  /* 
   * Pointer to Multi partition HM selector.
   * 
   * Bit's value 0 means module level error, 1 - partition level error.
   * 
   * Set in deployment.c
   */
  const pok_error_level_selector_t* multi_partition_hm_selector;
  /*
   * Pointer to Multi partition HM table.
   * 
   * Set in deployment.c
   */
  const pok_error_module_action_table_t* multi_partition_hm_table;
} pok_partition_t;

/**
 * Pointer to the current partition.
 * 
 * DEV: Readonly for all except scheduler-related stuff.
 */
extern pok_partition_t* current_partition;

/**
 * Execute given function for each partition.
 */
void for_each_partition(void (*f)(pok_partition_t* part));

/* 
 * Ready-made scheduler operations for partition with single kernel thread.
 * 
 * '.start' function for such partition shouldn't enable local preemption.
 */
extern const struct pok_partition_sched_operations partition_sched_ops_kernel;

#endif /* __POK_PARTITION_H__ */
