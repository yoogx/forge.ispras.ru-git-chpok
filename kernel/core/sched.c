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
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
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

/**
 **\\file   sched.c
 **\\brief  Function for partitions and kernel scheduling
 **\\author Julien Delange
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

static pok_bool_t sched_need_recheck;

static void start_partition(void)
{
    // Initialize state for started partition.
    current_partition->state.bytes.time_changed = 0;
    current_partition->state.bytes.control_returned = 0;
    current_partition->state.bytes.unused1 = 0;
    current_partition->state.bytes.unused2 = 0;
    
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
    uint32_t* new_sp = old_sp;
    
#ifdef POK_NEEDS_MONITOR
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
            &start_partition);
    }
}

/* Switch to the new partition. */
static void inter_partition_switch(pok_partition_t* part)
{
    uint32_t* old_sp = &current_partition->sp;
    uint32_t* new_sp = &part->sp;
    
    current_partition = part;

    if(part->space_id != 0xff)
        pok_space_switch(part->space_id);
    else
        pok_space_switch(0xff); // TODO: This should disable all user space tables

#ifdef POK_NEEDS_MONITOR
    if(current_partition_is_paused) old_sp = &idle_sp;
    if(part->is_paused) new_sp = &idle_sp;

    if(old_sp == new_sp)
    {
        // Idle thread is continued. Nothing to do.
        return;
    }
    
    current_partition_is_paused = current_partition->is_paused;
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
        pok_context_init(
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

void pok_sched(void)
{
    pok_partition_t* new_partition;
    pok_time_t now;
    
    if(!flag_test_and_reset(sched_need_recheck)) return;
    
    now = POK_GETTICK();
    if(pok_sched_next_deadline > now) goto same_partition;
    
    pok_sched_current_slot = (pok_sched_current_slot + 1);
    if(pok_sched_current_slot == pok_module_sched_n)
        pok_sched_current_slot = 0;

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
    uint8_t preempt_local_disabled_old;
    pok_bool_t need_call_time_changed = FALSE;
    pok_bool_t need_call_control_returned = FALSE;
    
    assert(pok_arch_preempt_enabled());
    
    pok_sched();
    
    preempt_local_disabled_old = current_partition
        ? current_partition->preempt_local_disabled
        : 1; // Idle thread has no preemption mechanism.
    
    if(preempt_local_disabled_old)
    {
        // Do nothing in case of disabled local preemption.
    }
    else if(current_partition->state.bytes.time_changed
        && current_partition->part_ops
        && current_partition->part_ops->on_time_changed)
    {
        // Time has been changed and partition has operation for process that.
        current_partition->preempt_local_disabled = 1;
        current_partition->state.bytes.time_changed = 0;
        need_call_time_changed = TRUE;
    }
    else if(current_partition->state.bytes.control_returned
        && current_partition->part_ops
        && current_partition->part_ops->on_time_changed)
    {
        // Control has been returned and partition has operation for process that.
        current_partition->preempt_local_disabled = 1;
        current_partition->state.bytes.control_returned = 0;
        need_call_control_returned = TRUE;
    }
    
    pok_arch_preempt_enable();
    
    if(need_call_time_changed)
    {
        current_partition->part_ops->on_time_changed();
    }
    else if(need_call_control_returned)
    {
        current_partition->part_ops->on_control_returned();
    }
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
    if(current_partition)
        current_partition->state.bytes.time_changed = 1;
    
    sched_need_recheck = TRUE;
    pok_preemption_enable();
}
