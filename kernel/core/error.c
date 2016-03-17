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
