#include <config.h>

#include <core/partition_arinc.h>
#include <core/sched_arinc.h>
#include "thread_internal.h"
#include <common.h>
#include <arch.h>
#include <uaccess.h>

/* Helpers */

/*
 * Reset thread object as it is not used.
 */
static void thread_reset(pok_thread_t* t)
{
    t->name[0] = '\0';
    // Everything else will be set at thread creation time.
}


/* 
 * Initialize thread object. 
 * 
 * Executed once during partition initialization.
 */
static void thread_init(pok_thread_t* t)
{
    pok_dstack_alloc(&t->initial_sp, KERNEL_STACK_SIZE_DEFAULT);
}

void pok_partition_arinc_reset(pok_partition_arinc_t* part,
	pok_partition_mode_t mode)
{
	part->mode = mode;
	
	part->intra_memory_size_used = 0;

	pok_partition_reset(&part->base_part);
}

// This name is not accessible for user space
static char main_thread_name[MAX_NAME_LENGTH] = "main";

static void partition_arinc_start(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

	part->preempt_local_counter = 1;
	
	INIT_LIST_HEAD(&part->eligible_threads);
	delayed_event_queue_init(&part->queue_deadline);
	delayed_event_queue_init(&part->queue_delayed);
    
	for(int i = 0; i < part->nthreads; i++)
	{
		thread_reset(&part->threads[i]);
	}
	
	part->nthreads_used = 0;
	part->user_stack_state = 0;

	part->thread_current = NULL;
#ifdef POK_NEEDS_ERROR_HANDLING
	part->thread_error = NULL;
	INIT_LIST_HEAD(&part->error_list);
#endif

    pok_thread_t* thread_main = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
    
	thread_main->entry = (void* __user)part->main_entry;
	thread_main->base_priority = 0;
	thread_main->period = POK_TIME_INFINITY;
	thread_main->time_capacity = POK_TIME_INFINITY;
	thread_main->deadline = DEADLINE_SOFT;
	strncpy(thread_main->name, main_thread_name, MAX_NAME_LENGTH);
	thread_main->user_stack_size = part->main_user_stack_size;

    thread_create(thread_main);
	
	part->nthreads_used = POK_PARTITION_ARINC_MAIN_THREAD_ID;
	
	part->lock_level = 1;
	part->thread_locked = thread_main;
	
	thread_main->state = POK_STATE_RUNNABLE;

	pok_sched_local_invalidate();

	pok_preemption_local_enable();
	// Idle thread now.
	wait_infinitely();
}


static const struct pok_partition_operations arinc_ops = {
	.start = &partition_arinc_start,
	.on_time_changed = &pok_sched_arinc_on_time_changed,
	.on_control_returned = &pok_sched_arinc_on_control_returned,
	.process_partition_error = &pok_partition_arinc_process_error,
};

void pok_partition_arinc_init(pok_partition_arinc_t* part)
{
	int i;
	size_t size = part->size;

	pok_dstack_alloc(&part->base_part.initial_sp, DEFAULT_STACK_SIZE);

	uintptr_t base_addr = (uintptr_t) pok_bsp_alloc_partition(part->size);
	uintptr_t base_vaddr = pok_space_base_vaddr(base_addr);
	
    /* 
	 * Memory.
	 */
	part->base_addr   = base_addr;
	part->base_vaddr  = base_vaddr;

	pok_create_space (part->base_part.space_id, base_addr, size);

	pok_arch_load_partition(part,
		part->partition_id, /* elf_id*/
		part->base_part.space_id,
		&part->main_entry);

	for(int i = 0; i < part->nthreads; i++)
	{
		thread_init(&part->threads[i]);
	}
	
	part->base_part.part_ops = &arinc_ops;
	
	part->intra_memory = part->intra_memory_size
		? pok_bsp_mem_alloc(part->intra_memory_size)
		: NULL;

	for(i = 0; i < part->nports_queuing; i++)
	{
		pok_port_queuing_init(&part->ports_queuing[i]);
	}

	for(i = 0; i < part->nports_sampling; i++)
	{
		pok_port_sampling_init(&part->ports_sampling[i]);
	}

	pok_partition_arinc_reset(part, POK_PARTITION_MODE_INIT_COLD);
}

void* partition_arinc_im_get(size_t size, size_t alignment)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	size_t start = ALIGN_SIZE(part->intra_memory_size_used, alignment);
	
	// TODO: revisit boundary check
	if(start <= part->intra_memory_size
		&&	start + size <= part->intra_memory_size) {
		
		part->intra_memory_size_used = start + size;
		return part->intra_memory + start;
	}
	
	return NULL;
}


/* 
 * Return current state of partition's intra memory.
 * 
 * Value return may be used in partition_arinc_im_rollback().
 */
void* partition_arinc_im_current(void)
{
	return (void*)current_partition_arinc->intra_memory_size_used;
}

/*
 * Revert all intra memory usage requests since given state.
 * 
 * `state` should be obtains with partition_arinc_im_current().
 */
void partition_arinc_im_rollback(void* prev_state)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	size_t size_used = (size_t)prev_state;
	
	assert(size_used <= part->intra_memory_size);
	
	part->intra_memory_size_used = size_used;
}


/*
 * Transition from INIT_* mode to NORMAL.
 * 
 * Executed with local preemption disabled.
 */
static void partition_set_mode_normal(void)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	// Cached value of current time.
	pok_time_t current_time = POK_GETTICK();
	/*
	 * Cached value of first release point.
	 * 
	 * NOTE: Initially it is not cached. Such implementation allows to
	 * not calculate release point for partition which have no periodic
	 * processes.
	 */
	pok_time_t periodic_release_point = POK_TIME_INFINITY;
	
	for(int i = POK_PARTITION_ARINC_MAIN_THREAD_ID + 1; i < part->nthreads_used; i++)
	{
		pok_thread_t* t = &part->threads[i];
		pok_time_t thread_start_time;
		
		if(t->state == POK_STATE_STOPPED) continue;
		
		/*
		 * The only thing thread can wait in `INIT_*` mode is
		 * NORMAL mode switch.
		 */
		
		if(pok_time_is_infinity(t->period))
		{
			// Aperiodic process.
			thread_start_time = current_time + t->delayed_time;
		}
		else
		{
			// Periodic process
			if(pok_time_is_infinity(periodic_release_point))
				periodic_release_point = get_next_periodic_processing_start();
			thread_start_time = periodic_release_point + t->delayed_time;
		}
		
		if(thread_start_time <= current_time)
			thread_wake_up(t);
		else
			thread_delay_event(t, thread_start_time, &thread_wake_up);
		
		if(!pok_time_is_infinity(t->time_capacity))
		{
			thread_set_deadline(t, thread_start_time + t->time_capacity);
		}
	}
	
	pok_sched_local_invalidate();

	part->mode = POK_PARTITION_MODE_NORMAL;
}


// Executed with local preemption disabled.
static pok_ret_t partition_set_mode_internal (const pok_partition_mode_t mode)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	switch(mode)
	{
	case POK_PARTITION_MODE_INIT_WARM:
		if(part->mode == POK_PARTITION_MODE_INIT_COLD)
			return POK_ERRNO_PARTITION_MODE;
	// Walkthrough
	case POK_PARTITION_MODE_INIT_COLD:
		pok_partition_arinc_reset(part, mode);
	break;
	case POK_PARTITION_MODE_IDLE:
		part->mode = POK_PARTITION_MODE_IDLE;
	break;
	case POK_PARTITION_MODE_NORMAL:
		if(part->mode == POK_PARTITION_MODE_NORMAL)
			return POK_ERRNO_UNAVAILABLE; //TODO: revise error code
		partition_set_mode_normal();
	break;
	default:
		return POK_ERRNO_EINVAL;
	}
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_partition_set_mode_current (const pok_partition_mode_t mode)
{
	pok_ret_t res;
	
	pok_preemption_local_disable();
	res = partition_set_mode_internal(mode);
	pok_preemption_local_enable();
	
	return res;
}

/**
 * Get partition information. Used for ARINC GET_PARTITION_STATUS function.
 */
pok_ret_t pok_current_partition_get_id(pok_partition_id_t *id)
{
    *id = current_partition_arinc->partition_id;
    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_period (uint64_t *period)
{
    *period = current_partition_arinc->period;
    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_duration (uint64_t *duration)
{
    *duration = current_partition_arinc->duration;
    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_operating_mode (pok_partition_mode_t *op_mode)
{
	// Looks like disabling preemption is not needed here. But do that for safety.
	pok_preemption_local_disable();
    *op_mode = current_partition_arinc->mode;
    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_lock_level (uint32_t *lock_level)
{
	// Looks like disabling preemption is not needed here. But do that for safety.
	pok_preemption_local_disable();
    *lock_level = current_partition_arinc->lock_level;
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_get_start_condition (pok_start_condition_t *start_condition)
{
	// Looks like disabling preemption is not needed here. But do that for safety.
	pok_preemption_local_disable();
    *start_condition = current_partition_arinc->base_part.start_condition;
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

/* 
 * Whether lock level cannot be changed now. 
 * 
 * NOTE: Doesn't require disabled local preemption.
 */
pok_bool_t is_lock_level_blocked(void)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	return part->mode != POK_PARTITION_MODE_NORMAL ||
      part->thread_current == part->thread_error;
}

pok_ret_t pok_current_partition_inc_lock_level(uint32_t * __user lock_level)
{
    if(!is_lock_level_blocked())
		return POK_ERRNO_PARTITION_MODE;
    if(current_partition_arinc->lock_level == 16)
		return POK_ERRNO_EINVAL;
	if(!check_user_write(lock_level))
		return POK_ERRNO_EFAULT;

    
    pok_preemption_local_disable();
	++current_partition_arinc->lock_level;
	// Note: this doesn't invalidate any scheduling event.
	pok_preemption_local_enable();
	
	__put_user(lock_level, current_partition_arinc->lock_level);
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_dec_lock_level(uint32_t * __user lock_level)
{
    if(is_lock_level_blocked())
		return POK_ERRNO_PARTITION_MODE;
    if(current_partition_arinc->lock_level == 0)
		return POK_ERRNO_EINVAL;
	if(!check_user_write(lock_level))
		return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    if(--current_partition_arinc->lock_level == 0)
    {
		if(current_thread->eligible_elem.prev != &current_partition_arinc->eligible_threads)
		{
			// We are not the first thread in eligible queue
			pok_sched_local_invalidate();
		}
	}
	pok_preemption_local_enable();
	
	__put_user(lock_level, current_partition_arinc->lock_level);
	
	return POK_ERRNO_OK;
}


void pok_partition_arinc_init_all(void)
{
	for(int i = 0; i < pok_partitions_arinc_n; i++)
	{
		pok_partition_arinc_init(&pok_partitions_arinc[i]);

#ifdef POK_NEEDS_INSTRUMENTATION
		pok_instrumentation_partition_archi (i);
#endif
	}
}
