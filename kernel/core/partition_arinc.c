#include <config.h>

#include <partition_arinc.h>

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
    pok_dstack_alloc(&t->initial_sp, KERNEL_STACK_SIZE);
    pok_thread_reset(t);
}


/*
 * Create thread.
 * 
 * After this operation thread can be accessed from any thread
 * of partition.
 * 
 * Should be executed with local preemption disabled.
 * 
 * Before given function, next fields should be initialized manually:
 * 
 * - entry [ENTRY_POINT]
 * - base_priority [BASE_PRIORITY]
 * - period [PERIOD]
 * - time_capacity [TIME_CAPACITY]
 * - deadline [DEADLINE]
 * 
 */
pok_bool_t thread_create(pok_thread_t* t,
    const char name[MAX_NAME_LENGTH],
    uint32_t user_stack_size)
{
    t->init_stack_addr = pok_thread_stack_addr(
        pok_partition_get_space(current_partition_arinc),
        user_stack_size,
        &current_partition_arinc->user_stack_state);
    
    if(t->init_stack_addr == 0) return FALSE;
    
    memcpy(t->name, name, MAX_NAME_LENGTH);
    
    t->sp = pok_context_init(
        pok_dstack_get_stack(&t->initial_sp),
        &thread_start);
    
    t->priority = t->base_priority;
    t->state = POK_STATE_STOPPED;
    
    t->suspended = FALSE;
    
    delayed_event_init(&t->thread_deadline_event);
    delayed_event_init(&t->thread_delayed_event);
    INIT_LIST_HEAD(&t->wait_elem);
    INIT_LIST_HEAD(&t->eligible_elem);

    return TRUE;
}


/* 
 * Process thread as its deadline occures.
 * 
 * Should be executed with local preemption disabled.
 */
static void thread_deadline(pok_thread_t* t)
{
	//TODO
}

/* 
 * Return true if thread is eligible.
 * 
 * Note: Main thread and error thread are NEVER eligible.
 */
static pok_bool_t thread_is_eligible(pok_thread_t* t)
{
	return !list_empty(&t->eligible_elem);
}

/* 
 * Set thread eligible for running.
 * 
 * Thread shouldn't be eligible already.
 * 
 * Should be executed with local preemption disabled.
 */
static void thread_set_eligible(pok_thread_t* t)
{
	pok_thread_t* other_thread;
	
	assert(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL);
	assert(!thread_is_eligible(t));
	
	list_for_each_entry(other_thread,
		&current_partition_arinc->eligible_threads,
		eligible_elem)
	{
		if(other_thread->priority < t->priority)
		{
			list_add_tail(&t->eligible_elem, &other_thread->eligible_elem);
		}

	}
	list_add_tail(&t->eligible_elem, &current_partition->eligible_threads);
	
	pok_sched_local_invalidate();
}

/* 
 * Move(re-add) thread in the eligible queue.
 * 
 * Thread should be eligible already.
 * 
 * Should be executed with local preemption disabled.
 */
static void thread_move_eligible(pok_thread_t* t)
{
	assert(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL);
	assert(thread_is_eligible(t));
	
	// TODO: Improve performance
	list_del_init(&thread->eligible_elem);
	thread_set_eligible(t);
}



/* 
 * Set thread not eligible for running, if it was.
 * 
 * Should be executed with local preemption disabled.
 */
static void thread_set_uneligible(pok_thread_t* t)
{
	if(!list_empty(&t->eligible_elem))
	{
		list_del_init(&t->eligible_elem);
		pok_sched_local_invalidate();
	}
}


/* 
 * Mark given thread as runnable in NORMAL mode.
 * 
 * Called with local preemption disabled.
 * 
 * TODO: When to use it???
 */
static void thread_start(pok_thread_t* t)
{
	assert(t->state == POK_STATE_WAITING);
	
	t->state = POK_STATE_RUNNABLE;
	
	if(!t->suspended)
	{
		thread_set_eligible(t);
	}
}

/* 
 * Resume given thread in NORMAL mode.
 * 
 * Called with local preemption disabled.
 * 
 * May be used as callback.
 */
static void thread_resume(pok_thread_t* t)
{
	assert(t->suspended);
	
	t->suspended = TRUE;
	
	if(t->state == POK_STATE_RUNNABLE)
	{
		thread_set_eligible(t);
	}
}

/*
 * Set deadline for given thread.
 * 
 * Called with local preemption disabled.
 */
static void thread_set_deadline(pok_thread_t* t, pok_time_t deadline_time)
{
	delayed_event_add(&t->thread_deadline_event, deadline_time,
		&current_partition->queue_deadline);
}

/*
 * Postpone event for given thread.
 * 
 * Called with local preemption disabled.
 */
static void thread_delay_event(pok_thread_t* t, pok_time_t delay_time,
	void (*thread_delayed_func)(pok_thread_t* t))
{
	t->thread_delayed_func = thread_delayed_func;

	delayed_event_add(&t->thread_delayed_event, delay_time,
		&current_partition->queue_delayed);
}


/*
 * Cancel postponed event for given thread, if it was.
 * 
 * Called with local preemption disabled.
 */
static void thread_delay_event_cancel(pok_thread_t* t)
{
	delayed_event_remove(&t->thread_delayed_event);
}

/* 
 * Mark thread as waiting on any condition except suspencion.
 * 
 * Called with local preemption disabled.
 */
static void thread_wait(pok_thread_t* t)
{
	assert(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL);

	thread->state = POK_STATE_WAITING;
	thread_set_uneligible(t);
}


/*
 * Awoke given thread.
 * 
 * This function can be used when:
 *   1. Thread sleeps (on `thread_delayed_event`),
 *      and sleeping time is over.
 *   2. Thread waits (on `wait_elem`), and waited object become
 *      accesible.
 *   3. Thread waits (on `wait_elem`), and timeout
 *      (on `thread_delayed_event`) is fired.
 * 
 *   4. Thread waits in INIT_* mode, and mode has been set to NORMAL.
 *      (All previouse cases related to NORMAL mode).
 * 
 * NOE: It is assumed that thread is alredy delete itself from queue
 * which is fired.
 * 
 * Called with local preemption disabled.
 * 
 * May be used as callback.
 */
static void thread_wake_up(pok_thread_t* t)
{
	assert(t->state == POK_STATE_WAITING);
	
	t->state = POK_STATE_RUNNABLE;
	
    // For the case 3. Cancel possible timeout.
	thread_delay_event_cancel(t);
	
	if(!list_empty(&t->wait_elem))
	{
		// Case 2.
		
		// Cancel wait on object.
		list_del(&t->wait_elem);
		// Set flag that we has been interrupted by timeout.
		t->wait_private = ERR_PTR(POK_ERRNO_TIMEOUT);
	}
	
	if(!t->suspended)
	{
		thread_set_eligible(t);
	}
}



/*
 * Stop given thread.
 * 
 * Called with local preemption disabled.
 */
static void thread_stop(pok_thread_t* t)
{
	pok_partition_arinc_t* part = current_partition_arinc;

	assert(t->state != POK_STATE_STOPPED);
	t->state = POK_STATE_STOPPED;
	
	thread_set_uneligible(t);
	
	// Remove thread from all queues.
	thread_delay_event_cancel(t);
	
	delayed_event_remove(&t->thread_deadline_event);

	pok_thread_wq_remove(t);
	
	if(part->lock_level)
	{
		if(part->thread_locked == t)
			current_partition_arinc->lock_level = 0;
	}
}


void pok_partition_arinc_reset(pok_partition_arinc_t* part,
	pok_partition_mode_t mode)
{
	part->mode = mode;

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
		pok_thread_reset(part->threads[i]);
	}
	
	part->nthreads_used = 0;
	part->user_stack_state = 0;

	part->thread_current = NULL;
#ifdef POK_NEEDS_ERROR_HANDLING
	part->thread_error = NULL;
#endif

    pok_thread_t* thread_main = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];
    
	thread_main->entry_point = part->main_entry;
	thread_main->base_priority = 0;
	thread_main->period = POK_TIME_INFINITY;
	thread_main->time_capacity = POK_TIME_INFINITY;
	thread_main->deadline = DEADLINE_SOFT;

    pok_thread_create(thread_main,
		main_thread_name, 
		part->main_user_stack_size
	);
	
	part->nthreads_used = POK_PARTITION_ARINC_MAIN_THREAD_ID;
	
	part->lock_count = 1;
	part->thread_locked = thread_main;
	
	thread_start->state = POK_STATE_RUNNABLE;

	pok_sched_local_invalidate();

	pok_preemption_local_enable();
	// Idle thread now.
	while(1)
		yield();
}


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

	pok_create_space (pok_partition_get_space(part), base_addr, size);

	pok_arch_load_partition(part,
		part->partition_id, /* elf_id*/
		pok_partition_get_space(part),
		&part->start_entry);

	for(int i = 0; i < part->nthreads; i++)
	{
		pok_thread_init(part->threads[i]);
	}
	
	part->activation        = 0; // FIXME that can't be right
	part->period            = POK_CONFIG_SCHEDULING_MAJOR_FRAME;
	part->duration          = POK_CONFIG_SCHEDULING_MAJOR_FRAME;

	#ifdef POK_NEEDS_INSTRUMENTATION
	pok_instrumentation_partition_archi (i);
	#endif

	part->start = &partition_arinc_start;
	
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
		pok_thread_t* t = part->threads[i];
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
		part->mode = POK_PARTITION_MODE_NORMAL;
		
	}

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
    *start_condition = current_partition_arinc->start_condition;
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

pok_bool_t is_lock_level_blocked(void)
{
	return current_partition_arinc->mode != POK_PARTITION_MODE_NORMAL ||
      current_partition_arinc->current_thread == current_partition_arinc->thread_error;
}

pok_ret_t pok_current_partition_inc_lock_level(uint32_t *lock_level)
{
    pok_ret_t ret;
    pok_preemption_local_disable();
	if(!is_lock_level_blocked())
	{
		if(current_partition_arinc->lock_level < 16)
		{
			*lock_level = ++current_partition_arinc->lock_level;
			// Note: this doesn't invalidate any scheduling event.
		}
		else
			ret = POK_ERRNO_EINVAL;
	}
	else
	{
		ret = POK_ERRNO_PARTITION_MODE;
	}
	pok_preemption_local_enable();
	return ret;
}

pok_ret_t pok_current_partition_dec_lock_level(uint32_t *lock_level)
{
    pok_ret_t ret;
    pok_preemption_local_disable();
	if(!is_lock_level_blocked())
	{
		if(current_partition_arinc->lock_level > 0)
		{
			if((*lock_level = --current_partition_arinc->lock_level) == 0)
			{
				pok_sched_local_invalidate();
			}
		}
		else
			ret = POK_ERRNO_EINVAL;
	}
	else
	{
		ret = POK_ERRNO_PARTITION_MODE;
	}
	pok_preemption_local_enable();
	return ret;
}


/**
 * Create a thread inside a partition
 * Return POK_ERRNO_OK if no error.
 * Return POK_ERRNO_TOOMANY if the partition cannot contain
 * more threads.
 */
pok_ret_t pok_thread_create (pok_thread_id_t* thread_id,
							   const pok_thread_attr_t  *attr)
{
    pok_thread_id_t id;
    uintptr_t stack_vaddr;
    
    pok_partition_arinc_t* part = current_partition_arinc;

    /**
     * We can create a thread only if the partition is in INIT mode
     */
    if (  (part->mode != POK_PARTITION_MODE_INIT_COLD) &&
          (part->mode != POK_PARTITION_MODE_INIT_WARM) )
    {
        return POK_ERRNO_PARTITION_MODE;
    }

    if (part->nthreads_used == part->nthreads) {
        return POK_ERRNO_TOOMANY;
    }

    if (attr->period == 0) {
        return POK_ERRNO_PARAM;
    }
    if (attr->time_capacity == 0) {
        return POK_ERRNO_PARAM;
    }
   
    if(!pok_time_is_infinity(attr->period))
    {
        if(pok_time_is_infinity(attr->time_capacity)) {
            // periodic process must have definite time capacity
            return POK_ERRNO_PARAM;
        }
       
        if(!pok_time_is_infinity(attr->time_capacity)
            && attr->time_capacity > attr->period) {
            // for periodic process, time capacity <= period
            return POK_ERRNO_PARAM;
        }
   }

    // do at least basic check of entry point
    if (!POK_CHECK_VPTR_IN_PARTITION(partition_id, attr->entry)) {
        return POK_ERRNO_PARAM;
    }

    thread->entry = attr->entry;
    thread->base_priority = attr->priority;
    thread->period = attr->period;
    thread->time_capacity = attr->time_capacity;
    thread->deadline = attr->deadline;
   
    if(!pok_thread_create(part->threads[part->nthreads_used],
                        attr->process_name,
                        attr->stack_size)) {
       return POK_ERRNO_TOOMANY; // TODO: Change return code for this case.
    }
   
    part->nthreads_used++;

    return POK_ERRNO_OK;
}


/*
 * Return thread by id.
 * 
 * Return NULL if no such thread created.
 */
static pok_thread_t* get_thread_by_id(pok_thread_id_t id)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	if(id == 0 /* main thread have no id*/
		|| id >= part->nthreads_used /* thread is not created yet */
#ifdef POK_NEEDS_ERROR_HANDLING
		|| part->thread_error == &part->threads[id] /* error thread has no id */
#endif		
		)
		return NULL;
	
	return &part->threads[id];
}

/* Whether waiting is allowed in the current context. */
static pok_bool_t is_waiting_allowed(void)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	if(part->lock_level
#ifdef POK_NEEDS_ERROR_HANDLING
		|| part->thread_error == current_thread /* error thread cannot wait */
#endif		
      )
      return FALSE;

   return TRUE;
}

#ifdef POK_NEEDS_THREAD_SLEEP

/*
 * Turn current thread to wait until specified time.
 * 
 * Should be called with local preemption disabled.
 */
static void thread_wait_timed(pok_thread_t *thread, pok_time_t time)
{
    assert(thread);
    
    if (time == 0) {
        return POK_ERRNO_OK;
    }
    
    thread_wait(thread);
    
    if(!pok_time_is_infinity(time))
    {
		thread_delay_event(thread, POK_GETTICK() + time, &thread_wake_up);
	}
    
    return POK_ERRNO_OK;
}

/* 
 * Turn current thread into the sleep for a while.
 * 
 * NOTE: It is possible to sleep forever(ARINC prohibits that).
 */
pok_ret_t pok_thread_sleep(pok_time_t time)
{
	pok_ret_t ret;
	pok_preemption_local_disable();

    ret = POK_ERRNO_MODE;
    if(!is_waiting_allowed()) goto out;
	
	if(time == 0)
	{
		thread_move_eligible(current_thread);
	}
	else if(!pok_time_is_infinity(time))
	{
		ret = thread_wait_timed(current_thread, POK_GETTICK() + time);
	}
	else
	{
		// Wait forever
		thread_wait(current_thread);
	}
out:
	pok_preemption_local_enable();
	
	return ret;
}
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
pok_ret_t pok_thread_sleep_until(uint64_t time)
{
	pok_ret_t ret;
	pok_preemption_local_disable();

    ret = POK_ERRNO_MODE;
    if(!is_waiting_allowed()) goto out;
	
	ret = thread_wait_timed(current_thread, time);
out:
	pok_preemption_local_enable();
	
	return ret;
}
#endif

pok_ret_t pok_thread_yield (void)
{
	pok_preemption_local_disable();
	thread_move_eligible(current_thread);
	pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

// Called with local preemption disabled
static pok_ret_t thread_delayed_start_internal (pok_thread_id_t id,
												pok_time_t ms)
{
    pok_thread_t *thread = get_thread_by_id(id);
    pok_partition_arinc_t* part = current_partition_arinc;
    
    pok_time_t thread_start_time;
    
    if (!thread) {
        return POK_ERRNO_THREADATTR;
    }

    if (ms < 0) {
        return POK_ERRNO_EINVAL;
    }
    
    if (thread->state != POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    }

    if (!pok_time_is_infinity(thread->period) && ms >= thread->period) {
        return POK_ERRNO_EINVAL;
    }
    
    thread->priority = thread->base_priority;
	thread->sp = 0;
  
	if(part->mode != POK_PARTITION_MODE_NORMAL)
	{
		/* Delay thread's starting until normal mode. */
		thread->delayed_time = ms;
		thread_wait(thread);
		
		return POK_ERRNO_OK;
	}

    // Normal mode.
    if (pok_time_is_infinity(thread->period)) {
        // aperiodic process
        thread_start_time = POK_GETTICK() + ms;
    }
    else {
		// periodic process
		thread_start_time = get_next_periodic_processing_start() + ms;
		thread->next_activation = thread_start_time + t->period;
	}
	
	/* Only non-delayed aperiodic process starts immediately */
	if(ms == 0 && pok_time_is_infinity(thread->period)) {
		thread->state = POK_STATE_RUNNABLE;
		// Thread cannot be suspended before the start.
		thread_set_eligible(thread);
	}
	else {
		thread_wait_timed(thread, thread_start_time);
	}
	
	if(!pok_time_is_infinity(thread->time_capacity))
	{
		thread_set_deadline(thread_start_time + thread->time_capacity);
	}
	
    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_delayed_start (pok_thread_id_t id, pok_time_t ms)
{
	pok_ret_t ret;
	
	pok_preemption_local_disable();
	ret = thread_delayed_start_internal(id, ms);
	pok_preemption_local_enable();
	
	return ret;
}

pok_ret_t pok_thread_start (pok_thread_id_t id)
{
	pok_ret_t ret;
	
	pok_preemption_local_disable();
	ret = thread_delayed_start_internal(id, 0);
	pok_preemption_local_enable();
	
	return ret;
}


pok_ret_t pok_thread_get_status (pok_thread_id_t id, pok_thread_status_t *status)
{
    pok_ret_t ret = POK_ERRNO_PARAM;
    pok_thread_t *t;
    
    pok_preemption_local_disable();

    t = get_thread_by_id(id);
	if(!t) goto out;

	status->attributes.priority       = t->base_priority;
	status->attributes.entry          = t->entry;
	status->attributes.period         = t->period;
	status->attributes.deadline       = t->deadline;
	status->attributes.time_capacity  = t->time_capacity;
	status->attributes.stack_size     = t->user_stack_state;

	status->suspended = t->suspended;
	status->current_priority = t->priority;

	if(t->state == POK_STATE_RUNNABLE && t->suspended)
		status->state = POK_STATE_WAITING;
	else
		status->state = t->state;
	
	if(pok_time_is_infinity(t->time_capacity))
		status->deadline_time = POK_TIME_INFINITY;
	else
		status->deadline_time = t->thread_deadline_event.timepoint;

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();
	
	return ret;
}

pok_ret_t pok_thread_set_priority(pok_thread_id_t id, uint32_t priority)
{
    pok_ret_t ret = POK_ERRNO_PARAM;
    pok_thread_t *t;
    
    pok_preemption_local_disable();

    t = get_thread_by_id(id);
	if(!t) goto out;

    ret = POK_ERRNO_UNAVAILABLE;
    if (t->state == POK_STATE_STOPPED) goto out;

    t->priority = priority;
    
    thread_move_eligible(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_resume(pok_thread_id_t id)
{
    pok_ret_t ret = POK_ERRNO_PARAM;
    pok_thread_t *t;
    
    pok_preemption_local_disable();

    t = get_thread_by_id(id);
	if(!t) goto out;

    // can't resume self, lol
    ret = POK_ERRNO_THREADATTR;
    if (t == current_thread) goto out;
    
	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    ret = POK_ERRNO_MODE;
    if (pok_thread_is_periodic(thread)) goto out;

    if (thread->state == POK_STATE_STOPPED) goto out;
    
	ret = POK_ERRNO_UNAVAILABLE;
    if (!thread->suspended) goto out;

    thread_resume(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_suspend_target(pok_thread_id_t id)
{
    pok_ret_t ret = POK_ERRNO_PARAM;
    pok_thread_t *t;
    
    pok_preemption_local_disable();

    t = get_thread_by_id(id);
	if(!t) goto out;

	// can't suspend current process
	// _using this function_
	// (use pok_thread_suspend instead)
    ret = POK_ERRNO_THREADATTR;
    if (t == current_thread) goto out;
    
	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    ret = POK_ERRNO_MODE;
    if (pok_thread_is_periodic(t)) goto out;

    // can't suspend stopped (dormant) process
    if (t->state == POK_STATE_STOPPED) goto out;

    // Cannot suspend process holded preemption lock.
    if (current_partition_arinc->lock_level > 0
		&& current_partition_arinc->thread_locked == t) goto out;

	ret = POK_ERRNO_UNAVAILABLE;
    if (t->suspended) goto out;

    t->suspended = TRUE;
    thread_set_uneligible(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_suspend(int64_t ms) // Time should be converted?
{
    pok_ret_t ret = POK_ERRNO_MODE;
    pok_thread_t *t = current_thread;
    
    pok_preemption_local_disable();

	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) goto out;

    if (!is_waiting_allowed()) goto out; 

	ret = POK_ERRNO_OK;
	if(ms == 0) goto out; // Nothing to do with 0 timeout.

    t->suspended = TRUE;
    thread_set_uneligible(t);
    
	if(!pok_time_is_infinity(ms))
		thread_delay_event(thread, POK_GETTICK() + ms, &thread_resume);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_stop_target(pok_thread_id_t id)
{
    pok_ret_t ret = POK_ERRNO_PARAM;
    pok_thread_t *t;
    
    pok_preemption_local_disable();

    t = get_thread_by_id(id);
	if(!t) goto out;

	// can's stop self
	// use pok_thread_stop to do that
    ret = POK_ERRNO_THREADATTR;
    if (t == current_thread) goto out;
    
    ret = POK_ERRNO_UNAVAILABLE;
    if (t->state == POK_STATE_STOPPED) goto out;

	thread_stop(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_stop(void)
{
    pok_preemption_local_disable();
	thread_stop(current_thread);
	pok_preemption_local_enable();
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_thread_find(const char name[MAX_NAME_LENGTH], pok_thread_id_t* id)
{
	pok_ret_t ret = POK_ERRNO_OK;
	pok_partition_arinc_t* part = current_partition_arinc;
	/* Length for compare names. */
	size_t cmp_len = strnlen(name, MAX_NAME_LENGTH);
	if(cmp_len != MAX_NAME_LENGTH) cmp_len++;
	
	pok_preemption_local_disable();
	for(int i = POK_PARTITION_ARINC_MAIN_THREAD_ID;
		i < part->nthreads_used;
		i++)
	{
		pok_thread_t* t = part->threads[i];
#ifdef POK_NEEDS_ERROR_HANDLING
		if(t == part->thread_error) continue;
#endif
		if(!memcmp(name, t->name, cmp_len))
		{
			*id = i;
			goto out;
		}
	}
	ret = POK_ERRNO_EINVAL; // TODO: INVALID_CONFIG for ARINC
	
out:
	pok_preemption_local_enable();
	
	return ret;
}

// called by periodic process when it's done its work
// ARINC-653 PERIODIC_WAIT
pok_ret_t pok_sched_end_period(void)
{
    pok_thread_t* t = current_thread;
    pok_ret_t ret = POK_ERRNO_MODE;
    
    pok_preemption_local_disable();
    
    if(!pok_thread_is_periodic(t)) goto out;

	if(is_waiting_allowed()) goto out;

	thread_wait_timed(t, t->next_activation);
	thread_set_deadline(t, t->next_activation + t->deadline_time);
	t->next_activation += t->period;

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}


// Executed with preemption disabled.
static pok_ret_t sched_replenish_internal(pok_time_t budget)
{
    pok_thread_t* t = current_thread;
    pok_time_t calculated_deadline;

#ifdef POK_NEEDS_ERROR_HANDLING
    if(t == part->thread_error) return POK_ERRNO_UNAVAILABLE;
#endif
    
    if(current_partition_arinc->mode != POK_PARTITION_MODE_NORMAL)
		return POK_ERRNO_UNAVAILABLE;
    
    if (budget > INT32_MAX) return POK_ERRNO_ERANGE;
    
    if(pok_time_is_infinity(t->time_capacity)) return POK_ERRNO_OK; //nothing to do
    
    calculated_deadline = POK_GETTICK() + budget;
    
    if(!pok_time_is_infinity(t->period)
		&& calculated_deadline >= t->next_activation)
		return POK_ERRNO_MODE;
	
	thread_set_deadline(t, calculated_deadline);
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_sched_replenish(pok_time_t budget)
{
    pok_ret_t ret;
    
    pok_preemption_local_disable();
    ret = sched_replenish_internal(budget);
    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_sched_get_current(pok_thread_id_t *thread_id)
{
	pok_thread_t* t = current_thread;
	
	// Current thread's id is fixed after creation, no needs in critical section.
	
	if(current_partition_arinc->mode != POK_PARTITION_MODE_NORMAL)
		return POK_ERRNO_THREAD;
	

#ifdef POK_NEEDS_ERROR_HANDLING
	if(t == current_partition_arinc->thread_error)
		return POK_ERRNO_THREAD;
#endif
	*thread_id = current_thread - current_partition_arinc->threads;

    return POK_ERRNO_OK;
}
