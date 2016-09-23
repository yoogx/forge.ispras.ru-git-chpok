
#include "tester.h"



/*
 * one param is tested, others - autocompleted
 * 
 **/
pok_ret_t tester1 (int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
    
    pok_ret_t ret = 0;
    
    test_eom[0] = 1; // TODO: set it to the right place
    
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
        
        // threads
        
        
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
        // events
        
        // error handling
        
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
			ret = null_pointer_tester(syscall_id, param, pos, pre_created_id, pre_created_name);
			break;
            
        // TODO: add more types of errors
        
        
	}
    
    return ret;
}

//pok_ret_t pre_create(int syscall_id, int error_type, void* param, int pos, int pre_created_id_type)
//{
//    switch(pre_created_id_type)
//}
