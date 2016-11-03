#include "tester.h"

pok_ret_t tester_create(int syscall_id, int error_type, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
    pok_ret_t ret = 0;
    
    ret = pointer_error_tester(syscall_id, error_type, param, pos, pre_created_id, pre_created_name);
    
    status = EXECUTED;
    TEST_RESULT = ret;
    
    return ret;
}

pok_ret_t tester_read(int entity_type, void* message, uint8_t pre_created_id, const char* pre_created_name)
{
    pok_ret_t ret = 0;
    
    switch (entity_type)
    {
      
        case BUFFER:
            ;
            
            // write wrapper
            blackboard_write_wrapper(message, pre_created_id);
            
            // read wrapper
            ret = blackboard_read_wrapper(pre_created_id);
            
            break;
        
        case BLACKBOARD:
            ;
            
            // write wrapper
            blackboard_write_wrapper(message, pre_created_id);
            
            // read wrapper
            ret = blackboard_read_wrapper(pre_created_id);
            
            break;
    }
    
    return ret;
}

/*
 * one param is tested, others - autocompleted
 * 
 **/
pok_ret_t tester1 (int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
    
    pok_ret_t ret = 0;
    
    //test_eom[0] = 1; // TODO: set it to the right place
    
	switch (syscall_id)
	{
        
        // ids for each method are from syscall_map_arinc.h
        // for each method one case (example below)
        
        /*
        case POK_SYSCALL_THREAD_CREATE:
			;
			ret = pok_thread_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        */
        
        // threads (includes 3 different sections actually)
        
        case POK_SYSCALL_THREAD_CREATE:
			;
			ret = pok_thread_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_SLEEP:
			;
			ret = pok_thread_sleep_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_SUSPEND:
			;
			ret = pok_thread_suspend_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_ID:
			;
			ret = pok_sched_get_current_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_STATUS:
			;
			ret = pok_thread_get_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_DELAYED_START:
			;
			ret = pok_thread_delayed_start_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_REPLENISH:
			;
			ret = pok_sched_replenish_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_THREAD_FIND:
			;
			ret = pok_thread_find_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
    
        case POK_SYSCALL_PARTITION_GET_STATUS:
			;
			ret = pok_current_partition_get_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        case POK_SYSCALL_PARTITION_INC_LOCK_LEVEL:
			;
			ret = pok_current_partition_inc_lock_level_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL:
			;
			ret = pok_current_partition_dec_lock_level_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        // buffers
        
        case POK_SYSCALL_INTRA_BUFFER_CREATE:
			;
			ret = pok_buffer_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_BUFFER_SEND:
			;
			ret = pok_buffer_send_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_RECEIVE:
			;
			ret = pok_buffer_receive_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_ID:
			;
			ret = pok_buffer_get_id_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_STATUS:
			;
			ret = pok_buffer_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        // blackboards
        
        case POK_SYSCALL_INTRA_BLACKBOARD_CREATE:
			;
			ret = pok_blackboard_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_BLACKBOARD_READ:
			;
			ret = pok_blackboard_read_wrapper (param, pos, pre_created_id, pre_created_name);
			break;

        case POK_SYSCALL_INTRA_BLACKBOARD_DISPLAY:
			;
			ret = pok_blackboard_display_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_BLACKBOARD_CLEAR:
			;
			ret = pok_blackboard_clear_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_BLACKBOARD_ID:
			;
			ret = pok_blackboard_id_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_BLACKBOARD_STATUS:
			;
			ret = pok_blackboard_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        // semaphores
        
        case POK_SYSCALL_INTRA_SEMAPHORE_CREATE:
			;
			ret = pok_semaphore_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_SEMAPHORE_WAIT:
			;
			ret = pok_semaphore_wait_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_SEMAPHORE_ID:
			;
			ret = pok_semaphore_id_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_SEMAPHORE_STATUS:
			;
			ret = pok_semaphore_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        
        // events
        
        case POK_SYSCALL_INTRA_EVENT_CREATE:
			;
			ret = pok_event_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_EVENT_WAIT:
			;
			ret = pok_event_wait_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_EVENT_ID:
			;
			ret = pok_event_id_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_INTRA_EVENT_STATUS:
			;
			ret = pok_event_status_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        // health monitoring
        
        case POK_SYSCALL_ERROR_HANDLER_CREATE:
			;
			ret = pok_error_thread_create_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        case POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR:
            ;
			ret = pok_error_raise_application_error_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
            
        case POK_SYSCALL_ERROR_GET:
			;
			ret = pok_error_get_wrapper (param, pos, pre_created_id, pre_created_name);
			break;
        
        
        // middleware syscalls:
        //  1. port sampling
        //  2. port queueing
        
		// TODO: add cases for all wrappers
	}
    
    
    return ret;
}

pok_ret_t null_pointer_tester(int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
	ret = tester1 (syscall_id, NULL, pos, pre_created_id, pre_created_name);
	
	return ret;
}

pok_ret_t out_of_partition_range_tester(int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    //uint32_t* out_of_range_param = 0x80000000;
    
	ret = tester1 (syscall_id, 0x80000000, pos, pre_created_id, pre_created_name);
	
	return ret;
}

pok_ret_t read_wrong_memory_tester(int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
	//ret = tester1 (syscall_id, 0x80000000, pos, pre_created_id, pre_created_name);
	
	return ret;
}

/*
 * tests one pointer parameter for one kind of pointer error
 * 
 **/

pok_ret_t pointer_error_tester(int syscall_id, int error_type, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
    pok_ret_t ret = 0;
    
	switch (error_type)
	{
		case NULL_POINTER:
			;
			ret = null_pointer_tester(syscall_id, param, pos, pre_created_id, pre_created_name);
			break;
		
		case OUT_OF_PARTITION_RANGE: // check
			;
			ret = out_of_partition_range_tester(syscall_id, param, pos, pre_created_id, pre_created_name);
			break;
        
        /*
        case READ_WRONG_MEMORY:
            ;
            ret = read_wrong_memory_tester(syscall_id, param, pos, pre_created_id, pre_created_name);
			break;
        */
            
        // TODO: add more types of errors
        
        
	}
    
    return ret;
}
