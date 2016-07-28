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


#ifndef __POK_SCHED_H__
#define __POK_SCHED_H__

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/partition.h>
#include <common.h>

/* 
 * Partition which does nothing.
 * 
 * It may be used in scheduling for time intervals which are not assigned
 * to other partitions.
 * 
 * This partition is used in deployment.c instead of special partition
 * (like GDB or monitor) which support is not enabled in the config.
 */
extern pok_partition_t partition_idle;


#ifdef POK_NEEDS_MONITOR
/*
 * Monitor partition.
 * 
 * It should be defined in core/libc/monitor.c.
 * 
 * For take effect, this partition should be assigned to a slot
 * in deployment.c.
 */
extern pok_partition_t partition_monitor;
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB

/*
 * GDB partition.
 * 
 * It should be defined in deployment.c and slot should be assigned to it.
 */
extern pok_partition_t partition_gdb;
#endif /* POK_NEEDS_GDB */

typedef struct
{
    uint64_t duration; // Set in deployment.c
    uint64_t offset; // Set in deployment.c

    pok_partition_t* partition; // Set in deployment.c

    pok_bool_t periodic_processing_start; // Set in deployment.c

    uint32_t id; // Set in deployment.c
} pok_sched_slot_t;

/*
 * Array of schedule slots.
 * 
 * Set in deployment.c.
 */
extern const pok_sched_slot_t pok_module_sched[];

/*
 * Number of schedule slots.
 * 
 * Set in deployment.c.
 */
extern const uint8_t pok_module_sched_n;

/*
 * Major time frame.
 * 
 * Set in deployment.c.
 */
extern const pok_time_t pok_config_scheduling_major_frame;

void pok_sched_init(void); /* Initialize scheduling stuff */


/**
 *  Starts critical section, when scheduler will not run.
 * 
 * Shouldn't be called from interrupt handler
 * (see pok_preemption_disable_from_interrupt below).
 * 
 * NOTE: Critical sections are NOT reentrant.
 */
void pok_preemption_disable(void);

/**
 *  Ends critical section, and runs scheduler.
 * 
 * Should be paired with pok_preemption_disable or
 * pok_preemption_disable_from_interrupt.
 */
void pok_preemption_enable(void);

/**
 *  Ends critical section without running scheduler.
 * 
 * Should be paired with pok_preemption_disable or
 * pok_preemption_disable_from_interrupt.
 * 
 * Should be used only if no action in the critical section affects
 * scheduler. Otherwise pok_preemption_enable() should be used.
 */
void __pok_preemption_enable(void);


/** 
 * Mark scheduler state as needed to be rechecked.
 * 
 * If called outside of critical section, action will be taken at next
 * scheduler run.
 */
void pok_sched_invalidate(void);

/**
 * Disables preemption in interrupt handler.
 * 
 * If preemption is not disabled by normal context, disable it
 * and return TRUE.
 * 
 * Otherwise return FALSE.
 * 
 * DEV: If preemption will be implemented via disabling interrupts,
 * this will return TRUE always.
 */
pok_bool_t pok_preemption_disable_from_interrupt(void);

/**
 * Set `invalidate` flag for scheduler.
 * 
 * Can be called outside of critical section. So scheduler will found
 * the flag set on the next call.
 */
void pok_sched_invalidate(void);

/**
 * Disable preemption before scheduler (re)start.
 * 
 * DEV: When kernel is initialized, this function should be called
 * before enabling interrupts. So, triggering timer interrupt will
 * not call scheduler until it will be ready.
 */
void pok_preemption_off(void);

/**
 * Start scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 * 
 * Assume partitions to be initialized at this step.
 */
void pok_sched_start(void);

/**
 * Retart scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 */
void pok_sched_restart(void);

/**
 * Should be called from interrupt handler after time is changed.
 * 
 * Called with disabled interrupts.
 */
void pok_sched_on_time_changed(void);

/**
 * Return next release point for periodic process in current partition.
 * 
 * May be used by partition-specific code.
 */
pok_time_t get_next_periodic_processing_start(void);


/*
 * Jump into user code from partition.
 * 
 * `entry` and `stack_addr` denotes pointers to initial user-space
 * entries and stack values correspondingly.
 * 
 * `stack_kernel` is kernel stack which will be used for interrupts
 * occured while user-space code is executed.
 * 
 * Some part of this functions is executed with global(!) preemption
 * disabled, because its architecture-specific code uses registers
 * which are not stored on common context switch.
 * 
 * Also, just before jumping into user space local preemption is
 * enabled. It is assumed that partition never trust to user in
 * preemption-related things. Because of enabled local preemption
 * all pending callbacks for partition are triggered.
 */
void pok_partition_jump_user(void (* __user entry)(void),
    jet_ustack_t stack_user,
    jet_stack_t stack_kernel);

/*
 * Return to the user space.
 *
 * Called from interrupt after partition has finished to process it.
 *
 * Just before jumping into user space local preemption is
 * enabled. It is assumed that partition never trust to user in
 * preemption-related things. Because of enabled local preemption
 * all pending callbacks for partition are triggered.
 */
void pok_partition_return_user(void);

/** 
 * Restart current partition.
 * 
 * Because this function is called by partition itself,
 * '.restarted_externally' flag is not set.
 */
void pok_partition_restart(void);


#endif /* !__POK_SCHED_H__ */
