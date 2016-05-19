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


#include <core/error.h>
#include <core/sched.h>

// TODO: this should be modified somewhere
pok_system_state_t kernel_state = POK_SYSTEM_STATE_INIT_PARTOS;

static pok_error_module_action_t get_module_action(
    const pok_error_module_action_table_t* table,
    pok_system_state_t system_state,
    pok_error_id_t error_id)
{
    return table->actions[system_state][error_id];
}

static void perform_module_action(pok_error_module_action_t action)
{
/*<<<<<<< HEAD
    assert(POK_CURRENT_PARTITION.thread_error_created);

    pok_thread_t *thread = &pok_threads[POK_CURRENT_PARTITION.thread_error];

    assert(thread->state == POK_STATE_STOPPED);

    // TODO this code repeats in pok_thread_delayed_start
    // I guess I should refactor it somewhere

    thread->sp = thread->initial_sp;
    pok_space_context_restart(
        thread->sp,
        thread->partition,
        (uintptr_t) thread->entry,
        thread->init_stack_addr,
        0xdead,
        0xbeef
    );

    // XXX hack hack hack - force context switch instead of returning from interrupt
    thread->force_restart = TRUE;

    thread->state  = POK_STATE_RUNNABLE;
}

static void take_fixed_action(pok_error_action_t action)
{
    // TODO in case of restart, fill in restart reason (to be inspected later)

    switch (action) {
        case POK_ERROR_ACTION_IGNORE:
            // TODO not all kinds of errors can be ignored just like that
            return;
        case POK_ERROR_ACTION_IDLE:
            pok_partition_set_mode_current(POK_PARTITION_MODE_IDLE);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_COLD_START:
            pok_partition_set_mode_current(POK_PARTITION_MODE_INIT_COLD);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_WARM_START:
            pok_partition_set_mode_current(POK_PARTITION_MODE_INIT_WARM);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        default:
            assert(FALSE && "invalid HM action");
=======*/
    if(action == POK_ERROR_MODULE_ACTION_IGNORE) return;
    
    pok_arch_preempt_disable();
    
    switch(action) {
    
    case POK_ERROR_MODULE_ACTION_RESET:
        pok_sched_restart();
        // TODO: Reset module
        break;

    case POK_ERROR_MODULE_ACTION_SHUTDOWN:
        // TODO: Shutdown module
        break;

    //default:
        /*
         * Incorrect configuration table.
         * 
         * We do not trust module HM table anymore.
         * 
         * Forse shutdown.
         */
        // TODO: Force shutdown
    }
}

static pok_system_state_t get_current_state(pok_bool_t user_space)
{
//<<<<<<< HEAD
    /*
     * This might be either process or partition level error,
     * which depends on HM table.
     *
     * So, we need to consult the table first.
     */
/*    const pok_error_hm_partition_t *entry;
    for (entry = pok_partition_hm_tables[POK_SCHED_CURRENT_PARTITION]; entry->kind != POK_ERROR_KIND_INVALID; entry++) {
        if (entry->kind == error) {
            break;
        }
    }
    if (entry->kind == POK_ERROR_KIND_INVALID) {
        // couldn't find error in the table, oops.
        // XXX what should we do? restart partition, or what?
        assert(FALSE && "missing error code in partition HM table");
    }

    if (entry->level == POK_ERROR_LEVEL_PARTITION || 
        !POK_CURRENT_PARTITION.thread_error_created || 
        POK_CURRENT_PARTITION.thread_error == thread_id)
    {
        // take fixed action
        take_fixed_action(entry->action);
        // TODO if action is ignore, sometimes we need to do something else (to prevent raising it again)
    } else {
        // otherwise, pass it to the error handler process

        // TODO add real error queue
        pok_error_status_t* status = &POK_CURRENT_PARTITION.error_status;
        assert(status->error_kind == POK_ERROR_KIND_INVALID);

        status->error_kind = entry->target_error_code;
        status->failed_thread = thread_id;
        if (msg_size > 0) {
            memcpy(status->msg, message, msg_size);
        }
        status->msg_size = msg_size;
        status->failed_addr = (uintptr_t) 0; // TODO somehow find interrupt frame and extract EIP from there
        
        // reset it's stack and other stuff
        pok_error_enable();
        pok_sched();
    }

    return POK_ERRNO_OK;
=======*/
    pok_partition_t* part;
    
    if(kernel_state != POK_SYSTEM_STATE_OS_PART)
        return kernel_state;
    
    part = current_partition;
    
    if(!user_space)
        return POK_SYSTEM_STATE_OS_PART;
    
    return part->is_error_handler
        ? POK_SYSTEM_STATE_ERROR_HANDLER
        : POK_SYSTEM_STATE_USER;
}


/* 
 * Check if error is module-level error and process it in that case.
 * 
 * Return FALSE if error is partition-level error.
 */
static pok_bool_t process_error_module(pok_system_state_t system_state,
    pok_error_id_t error_id)
{
    pok_error_module_action_t action;
    
    if(pok_error_level_select(&pok_hm_module_selector, system_state, error_id))
    {
        // Partition level error
        pok_partition_t* part = current_partition;
        
        if(pok_error_level_select(part->multi_partition_hm_selector,
            system_state, error_id)) {
            
            return FALSE; // Error should be processed in partition-specific way.
        }
        
        action = get_module_action(part->multi_partition_hm_table,
            system_state, error_id);
    }
    else
    {
        action = get_module_action(&pok_hm_module_table,
            system_state, error_id);
    }
    
    perform_module_action(action);
    
    return TRUE;
}

void pok_raise_error(pok_error_id_t error_id, pok_bool_t user_space, void* failed_address)
{
    pok_system_state_t system_state;
    
    pok_partition_t* part;
    pok_bool_t need_call_process_partition_error = FALSE;
    uint8_t preempt_local_disabled_old = 1;
    
    pok_preemption_disable();
    
    system_state = get_current_state(user_space);
    
    if(!process_error_module(system_state, error_id))
    {
        part = current_partition;
        assert(part);
        if(part->part_ops && part->part_ops->process_partition_error)
        {
            need_call_process_partition_error = TRUE;
            preempt_local_disabled_old = part->preempt_local_disabled;
            part->preempt_local_disabled = 0;
        }
    }
    
    pok_preemption_disable();

    if(need_call_process_partition_error)
        part->part_ops->process_partition_error(system_state, error_id,
            preempt_local_disabled_old, failed_address);
}

pok_bool_t pok_raise_error_by_partition(pok_system_state_t system_state,
    pok_error_id_t error_id)
{
    assert(system_state >= POK_SYSTEM_STATE_MIN_PARTITION);
    
    return process_error_module(system_state, error_id);
}
