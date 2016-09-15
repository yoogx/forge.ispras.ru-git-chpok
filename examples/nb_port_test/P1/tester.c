
#include "tester.h"



/*
 * one param is tested, others - autocompleted
 * 
 **/
pok_ret_t tester1 (int syscall_id, void* param, int pos)
{
    
    pok_ret_t ret = 0;
    
	switch (syscall_id)
	{
        
        // ids for each method are from syscall_map_arinc.h
        // for each method one case (example below)
        
        /*
        case POK_SYSCALL_THREAD_CREATE:
			;
			ret = pok_thread_create_wrapper (param, pos);
			break;
        */
        
        // threads
        
        
        // buffers
        
        case POK_SYSCALL_INTRA_BUFFER_CREATE:
			;
			ret = pok_buffer_create_wrapper (param, pos);
			break;
            
        case POK_SYSCALL_INTRA_BUFFER_SEND:
			;
			ret = pok_buffer_send_wrapper (param, pos);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_RECEIVE:
			;
			ret = pok_buffer_receive_wrapper (param, pos);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_ID:
			;
			ret = pok_buffer_get_id_wrapper (param, pos);
			break;
        
        case POK_SYSCALL_INTRA_BUFFER_STATUS:
			;
			ret = pok_buffer_status_wrapper (param, pos);
			break;
        
        // blackboards
        // semaphores
        // events
        // error handling
        // middleware syscalls:
        //  1. port sampling
        //  2. port queueing
        
		
	}
    
    
    return ret;
}

pok_ret_t null_pointer_tester(int syscall_id, int pos)
{
	pok_ret_t ret = 0;
	ret = tester1 (syscall_id, NULL, pos);
	
	return ret;
}

pok_ret_t out_of_partition_range_tester(int syscall_id, int pos)
{
	pok_ret_t ret = 0;
	ret = tester1 (syscall_id, NULL, pos);
	
	return ret;
}

/*
 * tests one pointer parameter for one kind of pointer error
 * 
 **/

pok_ret_t pointer_error_tester(int syscall_id, int error_type, void* param, int pos)
{
	switch (error_type)
	{
		case NULL_POINTER:
			;
			pok_ret_t ret = 0;
			ret = null_pointer_tester(syscall_id, pos);
			return ret;
			
			break;
		
		case OUT_OF_PARTITION_RANGE:
			
			
			
			break;
	}
}
