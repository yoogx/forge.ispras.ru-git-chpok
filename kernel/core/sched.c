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

pok_time_t first_frame_starts; // Time when first major frame is started.

pok_time_t            pok_sched_next_deadline;
pok_time_t            pok_sched_next_major_frame;
uint8_t             pok_sched_current_slot = 0; /* Which slot are we executing at this time ?*/

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
    
    while(true)
        yield();
}

#endif

static pok_bool_t sched_need_recheck;
static int preempt_counter;


static void start_partition(void)
{
    /* It is safe to enable preemtion on new stack. */
    pok_preemption_enable();
    
    current_partition->start();
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
                pok_dstack_get_stack(&currentPartition->initial_sp),
                &start_partition);
        }
        pok_context_switch(old_sp, *new_sp);
        return;
    }
    
#endif /* POK_NEEDS_MONITOR */
    // old_sp == new_sp
    if(*old_sp == 0) /* Same context, restart requested. */
    {
        pok_context_restart(&currentPartition->initial_sp,
            &start_partition);
    }
}

/* Switch to the new partition. */
static void inter_partition_switch(pok_partition_t* part)
{
    uint32_t* old_sp = &current_partition->sp;
    uint32_t* new_sp = &part->sp;
    
    current_partition = part;

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
        pok_contex_init(
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
    current_partition->is_slot_started = TRUE;
    current_partition->sched_local_recheck_needed = TRUE;
    
    
    new_sp = &current_partition->sp;
#ifdef POK_NEEDS_MONITOR
    if(current_partition->is_paused) new_sp = &idle_sp;
    current_partition_is_paused = current_partition->is_paused;    
#endif /*POK_NEEDS_MONITOR */
    if(*new_sp == 0)
    {
        *new_sp = pok_context_init(
            pok_dstack_get_stack(&currentPartition->initial_sp),
            &start_partition);
    }
    pok_context_jump(*new_sp);
   
   
   
   current_thread = KERNEL_THREAD;
   pok_sched_next_major_frame    = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
   pok_sched_next_deadline       = pok_module_sched[0].duration;
   if (pok_module_sched[0].type == POK_SLOT_PARTITION) {
      pok_current_partition      = pok_module_sched[0].partition.id;
   } else {
      // otherwise, we simply don't care
   }
}

void pok_sched_start (void)
{
#ifdef POK_NEEDS_MONITOR
    idle_stack = pok_stack_alloc(KERNEL_STACK_SIZE);
#endif /*POK_NEEDS_MONITOR */
    pok_sched_restart();
}

void pok_sched(void)
{
    pok_partition_t* new_partition;
    uint64_t now;
    
    uint32_t* old_sp;
    uint32_t* new_sp;

    if(!sched_need_recheck) return;
    sched_need_recheck = FALSE; // Acquire semantic
    barrier();
    
    now = POK_GETTICK();
    if(pok_sched_next_deadline > now) goto same_partition;
    
    pok_sched_current_slot = (pok_sched_current_slot + 1) % POK_CONFIG_SCHEDULING_NBSLOTS;

    new_partition = pok_module_sched[pok_sched_current_slot];
    
    new_partition->is_slot_started = TRUE;
    new_partition->sched_local_recheck_needed = TRUE;

    if(new_partition == current_partition) goto same_partition;
    
    inter_partition_switch(new_partition);
    
    return;

same_partition:
    intra_partition_switch();
}

void pok_preemption_disable(void)
{
    if(preempt_counter) {
        /* Preemption is already disabled. Not need special sync.*/
        preempt_counter++;
        return;
    }
    preempt_counter = 1; // Acquire semantic
    barrier();
}

void pok_preemption_enable(void)
{
    if(preempt_counter != 1) {
        /* Preemption remains disabled. Not need special sync.*/
        preempt_counter--;
        return;
    }
    
    pok_sched();
    barrier();
    preempt_counter = 0; // Release semantic
    
    while(sched_need_recheck) { // Possibly miss event affecting on scheduler.
        preempt_counter = 1; // Acquire semantic
        barrier();
        
        pok_sched();
        
        barrier();
        preempt_counter = 0; // Release semantic
    }
    
    if(currentPartition && !currentPartition->is_paused)
        currentPartition->sched();
}

void pok_sched_invalidate(void)
{
    barrier();
    sched_need_recheck = TRUE; // Release semantic
}

pok_bool_t pok_sched_local_check_invalidated(void)
{
    if(!currentPartition->sched_local_recheck_needed) return FALSE;

    currentPartition->sched_local_recheck_needed = FALSE; // Acquire semantic
    barrier();
    return TRUE;
}


pok_bool_t pok_sched_local_check_slot_started(void)
{
    if(!currentPartition->is_slot_started) return FALSE;

    currentPartition->is_slot_started = FALSE; // Acquire semantic
    barrier();
    return TRUE;
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
    for (i = 0; i < POK_CONFIG_SCHEDULING_NBSLOTS; i++) {
        // check all time slots
        // note that we ignore current activation of _this_ slot
        // e.g. if we're currently in periodic processing window,
        // and it's the only one in schedule, we say that next one
        // will be major frame time units later

        int time_slot_index = (i + 1 + pok_sched_current_slot) % POK_CONFIG_SCHEDULING_NBSLOTS;
        const pok_sched_slot_t *slot = &pok_module_sched[time_slot_index];

        if (slot->periodic_processing_start && slot->partition == current_partition) {
            return offset;
        }

        offset += slot->duration;
    }

    assert(FALSE && "Couldn't find next periodic processing window (configurator shouldn't have allowed that)");
}



/*
 * Context-switch function to switch from one thread to another
 * Rely on architecture-dependent functionnalities (must include arch.h)
 */
void pok_sched_context_switch(pok_thread_id_t elected_id)
{
   uint32_t *current_sp;
   uint32_t new_sp;
   

   if (POK_SCHED_CURRENT_THREAD == elected_id && !POK_CURRENT_THREAD.force_restart)
   {
      return;
   }

   current_sp = &POK_CURRENT_THREAD.sp;
   new_sp = pok_threads[elected_id].sp;

   pok_space_switch(POK_CURRENT_THREAD.partition,
		    pok_threads[elected_id].partition);

   current_thread = elected_id;
   
   POK_CURRENT_THREAD.force_restart = FALSE;

   pok_context_switch(current_sp, new_sp);
}

#endif /* __POK_NEEDS_SCHED */
