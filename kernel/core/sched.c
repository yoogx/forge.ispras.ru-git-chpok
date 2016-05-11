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

#include <config.h>

#include <types.h>
#include <arch.h>

#include <core/time.h>
#include <core/sched.h>
#include <core/thread.h>

#include <core/partition.h>

#include <dependencies.h>

#include <core/debug.h>
#include <core/instrumentation.h>
#include <core/error.h>

#include <assert.h>

static pok_time_t first_frame_starts; // Time when first major frame is started.

static pok_time_t            pok_sched_next_deadline;
static pok_time_t            pok_sched_next_major_frame;
static uint8_t               pok_sched_current_slot = 0; /* Which slot are we executing at this time ?*/

pok_partition_t* current_partition = NULL;

volatile pok_bool_t pok_in_user_space = 0;

uintptr_t global_thread_stack = 0;

#ifdef POK_NEEDS_MONITOR
/* 
 * Whether current partition has `.is_paused` flag set.
 * 
 * This flag affects on the place, where currently used context should
 * be stored on context switch.
 * From the other side, the flag can be changed *outside* of scheduler.
 */
pok_bool_t current_partition_is_paused;

uint32_t idle_sp;
uint32_t idle_stack;

static void idle_function(void)
{
    pok_preemption_enable();
    
    wait_infinitely();
}

#endif

// Reset partition state, so scheduler may restart.
static void pok_partition_reset(pok_partition_t* part)
{
    part->sp = 0;
    part->restarted_externally = TRUE;
    part->partition_generation++;
    if(part->partition_generation == 0)
    {
        part->partition_generation = 1;
    }
}


static pok_bool_t sched_need_recheck;

static void start_partition(void)
{
    // Initialize state for started partition.
    current_partition->state.bytes_all = 0;
    
    current_partition->preempt_local_disabled = 1;
    
    /* It is safe to enable preemption on new stack. */
    pok_preemption_enable();
    
    if(current_partition->part_ops && current_partition->part_ops->start)
        current_partition->part_ops->start();
        
    wait_infinitely();
}

/* Switch within current partition, if needed. */
static void intra_partition_switch(void)
{
    uint32_t* old_sp = &current_partition->sp;
    
#ifdef POK_NEEDS_MONITOR
    uint32_t* new_sp = old_sp;
    if(current_partition_is_paused) old_sp = &idle_sp;
    if(current_partition->is_paused) new_sp = &idle_sp;
    
    if(old_sp != new_sp)
    {
        /* Need to switch context */
        current_partition_is_paused = current_partition->is_paused;
        
        if(*old_sp == 0)
        {
            /* 
             * Restart is requested by currently executed context.
             * (new context is idle, so it doesn't need restart.)
             * 
             * Perform jump instead of switch.
             */ 
             pok_context_jump(*new_sp);
             return;
        }
        else if(*new_sp == 0)
        {
            /* 
             * Restart is requested by new context.
             * (current context is idle, so it doesn't need restart.)
             * 
             * Need to initialize new context before switch.
             */ 
            *new_sp = pok_context_init(
                pok_dstack_get_stack(&current_partition->initial_sp),
                &start_partition);
        }
        pok_context_switch(old_sp, *new_sp);
        return;
    }
    
#endif /* POK_NEEDS_MONITOR */
    // old_sp == new_sp
    if(*old_sp == 0) /* Same context, restart requested. */
    {
        pok_context_restart(&current_partition->initial_sp,
            &start_partition,
            old_sp);
    }
}

/* Switch to the new partition. */
static void inter_partition_switch(pok_partition_t* part)
{
    uint32_t* old_sp = &current_partition->sp;
    uint32_t* new_sp = &part->sp;
    
    current_partition->entry_sp = global_thread_stack;

    current_partition = part;

    if(part->space_id != 0xff)
        pok_space_switch(part->space_id);
    else
        pok_space_switch(0xff); // TODO: This should disable all user space tables
#ifdef POK_NEEDS_MONITOR
    if(current_partition_is_paused) old_sp = &idle_sp;
    if(part->is_paused)
    {
        assert(idle_sp); // Idle sp shouldn't be 0.
        new_sp = &idle_sp;
    }

    if(old_sp == new_sp)
    {
        // Idle thread is continued. Nothing to do.
        return;
    }
    
    current_partition_is_paused = part->is_paused;
#endif /* POK_NEEDS_MONITOR */
    // old_sp != new_sp

    if(*new_sp == 0)
    {
        /* 
         * Restart is requested by new context.
         * (current context is different)
         * 
         * Need to initialize new context before switch.
         */
        *new_sp = pok_context_init(
            pok_dstack_get_stack(&part->initial_sp),
            &start_partition);
    }
    
    if(*old_sp == 0)
    {
        pok_context_jump(*new_sp);
    }
    else
    {
        pok_context_switch(old_sp, *new_sp);
    }
}


void pok_sched_restart (void)
{
    uint32_t* new_sp;
    
    first_frame_starts = POK_GETTICK();
#ifdef POK_NEEDS_MONITOR
    idle_sp = pok_context_init(idle_stack, &idle_function);
#endif /*POK_NEEDS_MONITOR */

    for_each_partition(&pok_partition_reset);

    sched_need_recheck = 0; // Acquire semantic
    barrier();
   
    // Navigate to the first slot
    pok_sched_current_slot = 0;
    pok_sched_next_major_frame = first_frame_starts + pok_config_scheduling_major_frame;
    pok_sched_next_deadline = pok_module_sched[0].duration + first_frame_starts;
    
    current_partition = pok_module_sched[0].partition;
    
    new_sp = &current_partition->sp;
#ifdef POK_NEEDS_MONITOR
    if(current_partition->is_paused) new_sp = &idle_sp;
    current_partition_is_paused = current_partition->is_paused;    
#endif /*POK_NEEDS_MONITOR */
    if(*new_sp == 0)
    {
        *new_sp = pok_context_init(
            pok_dstack_get_stack(&current_partition->initial_sp),
            &start_partition);
    }

    if(current_partition->space_id != 0xff)
        pok_space_switch(current_partition->space_id);
    else
        pok_space_switch(0xff); // TODO: This should disable all user space tables

    pok_context_jump(*new_sp);
}

void pok_sched_start (void)
{
#ifdef POK_NEEDS_MONITOR
    idle_stack = pok_stack_alloc(KERNEL_STACK_SIZE_DEFAULT);
#endif /*POK_NEEDS_MONITOR */
    pok_sched_restart();
}

/* 
 * Perform scheduling.
 * 
 * Should be called with preemption disabled.
 */
static void pok_sched(void)
{
    pok_partition_t* new_partition;
    pok_time_t now;
    
    if(!flag_test_and_reset(sched_need_recheck)) return;
    
    now = POK_GETTICK();
    if(pok_sched_next_deadline > now) goto same_partition;
    
    pok_sched_current_slot = (pok_sched_current_slot + 1);
    if(pok_sched_current_slot == pok_module_sched_n)
    {
        pok_sched_next_major_frame += pok_config_scheduling_major_frame;
        pok_sched_current_slot = 0;
    }
    pok_sched_next_deadline += pok_module_sched[pok_sched_current_slot].duration;

    new_partition = pok_module_sched[pok_sched_current_slot].partition;
    
    if(new_partition == current_partition) goto same_partition;
    
    inter_partition_switch(new_partition);
    
    // After interpartition switch we return back.
    flag_set(current_partition->state.bytes.control_returned);
    
    return;

same_partition:
    intra_partition_switch();
}

void pok_preemption_disable(void)
{
    assert(pok_arch_preempt_enabled());

    pok_arch_preempt_disable();
}


void pok_preemption_enable(void)
{
    assert(!pok_arch_preempt_enabled());
    
    pok_sched();
    if(current_partition->preempt_local_disabled
        || !current_partition->state.bytes_all)
    {
        // Partition doesn't require notifications. Common case.
        pok_arch_preempt_enable();
        return;
    }

    current_partition->preempt_local_disabled = 1;
    
    // Until partition "consume" all state bits or enables preemption.
    do    
    {
        pok_arch_preempt_enable();
    
        current_partition->part_sched_ops->on_event();
        
        pok_arch_preempt_disable();
    } while(current_partition->preempt_local_disabled
        && current_partition->state.bytes_all);
    
    current_partition->preempt_local_disabled = 0;
    pok_arch_preempt_enable();
}

void __pok_preemption_enable(void)
{
    assert(!pok_arch_preempt_enabled());
    pok_arch_preempt_enable();
}


/*
 * Forward implementation, which iterates over all slots.
 * 
 * May be optimized in the future.
 */
pok_time_t get_next_periodic_processing_start(void)
{
    int i;

    pok_time_t offset = pok_sched_next_deadline;

    // check all time slots
    // note that we ignore current activation of _this_ slot
    // e.g. if we're currently in periodic processing window,
    // and it's the only one in schedule, we say that next one
    // will be major frame time units later
    int time_slot_index = pok_sched_current_slot;

    for (i = 0; i < pok_module_sched_n; i++) {

        time_slot_index++;
        if(time_slot_index == pok_module_sched_n) time_slot_index = 0;

        const pok_sched_slot_t *slot = &pok_module_sched[time_slot_index];

        if (slot->periodic_processing_start && slot->partition == current_partition) {
            return offset;
        }

        offset += slot->duration;
    }

    assert(FALSE && "Couldn't find next periodic processing window (configurator shouldn't have allowed that)");
}

void pok_sched_on_time_changed(void)
{
    pok_partition_t* part = current_partition;
    sched_need_recheck = TRUE;

    pok_bool_t in_user_space = pok_in_user_space;

    if(in_user_space)
    {
        part->entry_sp_user = global_thread_stack;
        pok_in_user_space = FALSE;
    }

    pok_sched();

    if(current_partition_is_paused) goto out;

    pok_bool_t preempt_local_disabled_old = current_partition->preempt_local_disabled;

    part->state.bytes.time_changed = 1;

    if(preempt_local_disabled_old) goto out;

    // Emit events for partition.
    do
    {
        part->preempt_local_disabled = 1;
        pok_arch_preempt_enable();
        part->part_sched_ops->on_event();
        pok_arch_preempt_disable();
    } while(part->preempt_local_disabled && part->state.bytes_all);

    part->preempt_local_disabled = 0;

    // Still with disabled preemption. It is needed for returning from interrupt.

out:
    // Restore user space indicator on return
    pok_in_user_space = in_user_space;
}

void pok_partition_return_user(void)
{
    pok_partition_t* part = current_partition;

    pok_preemption_disable();

    // Emit events for partition.
    while(part->preempt_local_disabled
        && part->state.bytes_all)
    {
        part->preempt_local_disabled = 1;
        pok_arch_preempt_enable();
        part->part_sched_ops->on_event();
        pok_arch_preempt_disable();
    }

    part->preempt_local_disabled = 0;

    pok_in_user_space = TRUE;
}


void pok_partition_jump_user(void* __user entry,
    void* __user stack_addr,
    struct dStack* stack_kernel)
{
    pok_partition_return_user();
    
    pok_partition_t* part = current_partition;

    pok_context_user_jump(
        stack_kernel,
        part->space_id,
        (unsigned long)entry,
        (unsigned long)stack_addr,
        0xdead,
        0xbeaf);
}

void pok_partition_restart(void)
{
	pok_partition_t* part = current_partition;
    assert(part);

    pok_preemption_disable();

    // Assign new generation for partition.
    part->partition_generation++;
    // For the case of overflow.
    if(part->partition_generation == 0) part->partition_generation = 1;

	part->sp = 0;
    sched_need_recheck = TRUE;

    pok_preemption_enable();

    unreachable();
}


void pok_sched_init(void)
{
    // TODO: It looks like nothing should be done there.
}
