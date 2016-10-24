#include "wrappers.h"


// TODO: seems that it's just not apropriate place for this function
void entry_point_method(void)
{
	// add some output (print log)
    
    
    // actually this prints only
    // if partition state is in INIT mode
    printf("the created process is working");  
    
	return;	
}
 
// TODO: fix all parameter types + default values

/*
 * TODO: add parameter for IDs      of pre-created stuff
 * 
 * thread               in progress
 * 
 * buffer               +
 * blackboard           +
 * semaphore            +
 * event                +
 * 
 * health monitoring    in progress
 * 
 * port                 -
 */

// TODO: add parameter for names    of pre-created stuff
// TODO: handle wrong position of parameters via default case and more pre-checks 

// TODO: find why delay (time) is pointer

/////////////////////////// THREADS ///////////////////////////

// call before partition becomes NORMAL
pok_ret_t pok_thread_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name = "process1";
    void* entry = entry_point_method;
			
	pok_thread_attr_t attr = {
		.priority = 0,
		//.entry = entry_point_method,
		.period = INFINITE_TIME_VALUE,
		.deadline = SOFT,
		.time_capacity = INFINITE_TIME_VALUE,
		.stack_size = 8096,
	};
		
	pok_thread_id_t* id;
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_thread_create(param, entry, &attr, id); 
			break;
		case 1:
            ;
            ret = pok_thread_create(name, param, &attr, id);
			break;
        case 2:
            ;
            ret = pok_thread_create(name, entry, param, id);
			break;
        case 3:
            ;
            ret = pok_thread_create(name, entry, &attr, param);
			break;
	}
	
	return ret;
}


// process STOP service
// parameter is time pointer
pok_ret_t pok_thread_sleep_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_sleep(param); 
	
	return ret;
}

/*
// TODO: find definition
pok_ret_t pok_thread_sleep_until_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_sleep_until(param);
    
    return ret;
}
**/

// parameter is process id pointer
pok_ret_t pok_thread_suspend_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_suspend(param); 
	
	return ret;
}

// parameter is process id pointer
pok_ret_t pok_sched_get_current_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_sched_get_current(param); 
	
	return ret;
}

// TODO: find what is ARINC analogue (service) of this syscall
pok_ret_t pok_thread_get_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_thread_id_t thread_id = pre_created_id;
    char* name = pre_created_name;      // TODO: check whether it is name of the process
    
    void* entry = entry_point_method;   // TODO: check the right value
    
    pok_thread_status_t* status;        // out
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_thread_get_status(thread_id, param, entry, status); 
			break;
		
		case 2:
            ret = pok_thread_get_status(thread_id, name, param, status);
			break;
        case 3:
            ret = pok_thread_get_status(thread_id, name, entry, param);
			break;
	}
	
	return ret;
}

// why delay (time) is pointer?
pok_ret_t pok_thread_delayed_start_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_thread_id_t thread_id = pre_created_id; // TODO: try case with started and not started process
    
    ret = pok_thread_delayed_start(thread_id, param);
    
    return ret;
}

// why delay (time) is pointer? 
// does it make sense to call from aperiodic process?
pok_ret_t pok_sched_replenish_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_sched_replenish(param); 
	
	return ret;
}

// 
pok_ret_t pok_thread_find_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    char* name = pre_created_name; // TODO: check also without existing process
		
	pok_thread_id_t* id; // out
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_thread_find(param, id); 
			break;
		
		case 1:
            ret = pok_thread_find(name, param);
			break;
	}
	
	return ret;                     
}

// parameter is status pointer (out)
pok_ret_t pok_current_partition_get_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_get_status(param); 
	
	return ret;
}

// why param is lock_level? 
pok_ret_t pok_current_partition_inc_lock_level_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_inc_lock_level(param); 
	
	return ret;
}

// why param is lock_level? 
pok_ret_t pok_current_partition_dec_lock_level_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_dec_lock_level(param); 
	  
	return ret;
}
 

/////////////////////////// BUFFERS ///////////////////////////

// call before partition becomes NORMAL
pok_ret_t pok_buffer_create_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = "buffer1";
    
    // check allowed values at kernel/include/uapi/types.h
    
    pok_message_size_t max_message_size = 8192; 
    pok_message_range_t max_nb_message  = 512;
    pok_queuing_discipline_t discipline = POK_QUEUING_DISCIPLINE_FIFO;
    
    pok_buffer_id_t id; // out
    
	switch (pos)
	{
		case 0: // const char*, name of the buffer
			;
			ret = pok_buffer_create(param, max_message_size, max_nb_message, discipline, &id); 
			break;
		
		case 4: // pok_buffer_id_t*, id of new buffer
            ;
            ret = pok_buffer_create(name, max_message_size, max_nb_message, discipline, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_send_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id = pre_created_id; // pre-created buffer needed (check also with no buffer)
    
    int data = 57;                      // message to be written to the buffer (which is some variable of any type, basically)
                                        // but the method wants address (pointer) of the message
                                        // so here data is type of int, but it can be changed in order to test different cases
                                        //
                                        // TODO: try different types and values
                                      
    // check allowed values at kernel/include/uapi/types.h
    // TODO: check whether it matches the actual size of sent message (does it actually matter?)
    pok_message_size_t length = 8192;
    
    const pok_time_t timeout  = 100; // TODO: try another values
    
	switch (pos)
	{
		case 1: // any pointer (check whether it matches type of received message), address of the message
                // also check if the message size matches "length"
			;
			ret = pok_buffer_send(id, param, length, &timeout); 
			break;
		
		case 3: // any number (in range of type), timeout
            ;
            ret = pok_buffer_send(id, &data, length, &param);
			break;
	}
	
	return ret;
}

pok_ret_t pok_buffer_receive_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id = pre_created_id; // pre-created buffer needed (check also with no buffer)
    
    const pok_time_t timeout = 100; // TODO: try another values
    
    const void* data;          // out, address of received message (we don't know the exact type in advance)
    pok_message_size_t length; // out
    
    // TODO: check two cases: whether there was pre-sent message to read or not
    // TODO: for the second case add call of pok_buffer_send
    
	switch (pos)
	{
		case 1: // any number (in range of type), timeout
			;
			ret = pok_buffer_receive(id, &param, data, &length); 
			break;
		
		case 2: // pointer of type of message that we expect to receive (for example: if int sent, so it should be int*)
			;
			ret = pok_buffer_receive(id, &timeout, param, &length);     
			break;
            
        case 3: // pok_message_size_t*
			;
			ret = pok_buffer_receive(id, &timeout, data, param); 
			break;
	}
	
	return ret;
}

pok_ret_t pok_buffer_get_id_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    char* name = (char*) pre_created_name; // pre-created buffer needed (check also with no buffer)
                        // TODO: check this dangerous cast from 'const char*' to 'char*'
                        //       maybe it would be better to change the syscall signature?
    
    pok_buffer_id_t id; // out
    
	switch (pos)
	{
		case 0: // char*, name of the buffer
			;
			ret = pok_buffer_get_id(param, &id); 
			break;
		
		case 1: // pok_buffer_id_t*, id of buffer
            ret = pok_buffer_get_id(name, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id = pre_created_id; // pre-created buffer needed (check also with no buffer)
    
	switch (pos)
	{
		case 1: // pok_buffer_status_t* (out param, actually)
			;
			ret = pok_buffer_status(id, param); 
			break;
	}
	
	return ret;                     
}


/////////////////////////// BLACKBOARDS ///////////////////////////

// call before partition becomes NORMAL
pok_ret_t pok_blackboard_create_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = "blackboard1";
    
    // check correct values at kernel/include/uapi/types.h
    
    pok_message_size_t max_message_size = 8192;
    
    pok_blackboard_id_t id; // out
    
	switch (pos)
	{
		case 0: // const char*, name of the blackboard
			;
			ret = pok_blackboard_create(param, max_message_size, &id); 
			break;
		
		case 2: // pok_blackboard_id_t*, id of new blackboard
            ;
            ret = pok_blackboard_create(name, max_message_size, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_read_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id = pre_created_id; // pre-created blackboard needed (check also with no blackboard)
    
    const pok_time_t timeout = 100; // TODO: try another values
    
    void* data;                 // out, address of received message (we don't know the exact type in advance)
    pok_message_size_t len;     // out
    
    // TODO: check two cases: whether there was pre-sent message to read or not
    // TODO: for the second case add call of pok_blackboard_display
    
	switch (pos)
	{
		case 1: // any number (in range of type), timeout
			;
			ret = pok_blackboard_read(id, &param, data, &len); 
			break;
        
        case 2: // pointer of type of message that we expect to receive (for example: if int sent, so it should be int*)
			;
			ret = pok_blackboard_read(id, &timeout, param, &len); 
			break;
		
		case 3: // pok_message_size_t*
            ;
            ret = pok_blackboard_read(id, &timeout, data, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_display_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id = pre_created_id; // pre-created blackboard needed (check also with no blackboard)
    
    // check allowed values at kernel/include/uapi/types.h
    // TODO: check whether it matches the actual size of sent message (does it actually matter?)
    pok_message_size_t len = 8192;
    
	switch (pos)
	{
		case 1: // any pointer (check whether it matches type of received message), address of the message
                // also check if the message size matches "len"
			;
			ret = pok_blackboard_display(id, param, len); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_clear_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    // TODO: check case with existing messages on blackboard (not empty)
    
	switch (pos)
	{
		case 0: // pok_blackboard_id_t, id of blackboard (check whether it exists) = two cases
			;
			ret = pok_blackboard_clear(param); 
			break;
	}
	
	return ret;         
}

pok_ret_t pok_blackboard_id_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = pre_created_name; // pre-created blackboard needed (check also with no blackboard)
    
    pok_blackboard_id_t id; // out
    
	switch (pos)
	{
		case 0: // const char*, name of the blackboard
			;
			ret = pok_blackboard_id(param, &id); 
			break;
		
		case 1: // pok_buffer_id_t*, id of buffer
            ;
            ret = pok_blackboard_id(name, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id = pre_created_id; // pre-created blackboard needed (check also with no blackboard)
    
	switch (pos)
	{
		case 1: // pok_blackboard_status_t* (out param, actually)
			;
			ret = pok_blackboard_status(id, param); 
			break;
	}
	
	return ret;                     
}


/////////////////////////// SEMAPHORES ///////////////////////////

pok_ret_t pok_semaphore_create_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = "semaphore1";
    
    pok_sem_value_t value               = 0;       // TODO: try other values
    pok_sem_value_t max_value           = 10;
    pok_queuing_discipline_t discipline = POK_QUEUING_DISCIPLINE_FIFO;
    
    pok_sem_id_t* id; // out-param
      
	switch (pos)
	{
		case 0: // const char*, name of the semaphore
			;
			ret = pok_semaphore_create(param, value, max_value, discipline, id); 
			break;
		
		case 4: // pok_blackboard_id_t*, id of new blackboard
            ; 
            ret = pok_semaphore_create(name, value, max_value, discipline, param);
			break;        
	}
	
	return ret;                     
}


pok_ret_t pok_semaphore_wait_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id = pre_created_id; // pre-created sem needed (check also with no sem)
    
	switch (pos)
	{
		case 1: // const pok_time_t*, timeout
			;
			ret = pok_semaphore_wait(id, param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_semaphore_id_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = pre_created_name; // pre-created sem needed (check also with no sem)
    
    pok_sem_id_t* id; // out param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_semaphore_id(param, id); 
			break;
		
		case 1:
            ;
            ret = pok_semaphore_id(name, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_semaphore_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id = pre_created_id; // pre-created sem needed (check also with no sem)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_semaphore_status(id, param); 
			break;
	}
	
	return ret;                     
}



/////////////////////////// EVENTS ///////////////////////////

// call before partition becomes NORMAL
pok_ret_t pok_event_create_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = "event1";
    
    pok_event_id_t* id; // out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_event_create(param, id); 
			break;
		
		case 1:
            ret = pok_event_create(name, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_wait_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id = pre_created_id; // pre-created event needed (check without event)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_event_wait(id, param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_id_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    const char* name = pre_created_name; // pre-created event needed (check also with no event)
    
    pok_event_id_t* id; // out param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_event_id(param, id); 
			break;
		
		case 1:
            ret = pok_event_id(name, param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_status_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_event_id_t id = pre_created_id; // pre-created event needed (check also with no event)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_event_status(id, param); 
			break;
	}
	
	return ret;                     
}




/////////////////////////// ERROR HANDLING (HEALTH MONITORING ///////////////////////////

// call before partition becomes NORMAL
pok_ret_t pok_error_thread_create_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    uint32_t stack_size = 8096; // value from examples (TODO: 8096 of what?)
    
	switch (pos)
	{
		case 1:
            ret = pok_error_thread_create(stack_size, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_error_raise_application_error_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    size_t msg_size = 64; // value from include/arinc653/error.h (0 .. 128)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_error_raise_application_error(&param, msg_size); 
			break;
	}
	
	return ret;                     
}

// TODO: check what msg is for
pok_ret_t pok_error_get_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name)
{
	pok_ret_t ret = 0;
    
    pok_error_status_t* status; // out
    void* msg;                  // out
    
	switch (pos)
	{
		case 0: // pok_error_status_t*, status, out
			;
			ret = pok_error_get(param, msg); 
			break;
		
		case 1: // ???
            ;
            ret = pok_error_get(status, param);
			break;
	}
	
	return ret;                     
}


/*

//////////---////////--- MIDDLEWARE SYSCALLS ---//////////---////////////

/////////////////////////// PORT SAMPLING ///////////////////////////

pok_ret_t pok_port_sampling_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "port1";
    
    pok_port_size_t size;           // TODO: check right value
    pok_port_direction_t direction;
    const pok_time_t* refresh;
    
    pok_port_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_sampling_create(&param, &size, &direction, &refresh, &id); 
			break;
            
        case 3:
            ret = pok_port_sampling_create(&name, &size, &direction, &param, &id);
			break;
        
		case 4:
            ret = pok_port_sampling_create(&name, &size, &direction, &refresh, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_write_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    pok_port_size_t len; // TODO: check the right value
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_write(&id, &param, &len); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_read_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    void* data;      // TODO: check the right values
    pok_port_size_t* len; // out??
    pok_bool_t* valid;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_read(&id, &param, &len, &valid); 
			break;
        
        case 2:
			;
			ret = pok_port_sampling_read(&id, &data, &param, &valid); 
			break;
		
		case 3:
            ret = pok_blackboard_read(&id, &data, &len, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created port needed (check also with no port)
    
    pok_port_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_sampling_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_port_sampling_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed (check also with no port)
    pok_port_sampling_status_t* status; // out param (?)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_status(&id, &param); 
			break;
	}
	
	return ret;                     
}


/////////////////////////// PORT QUEUEING ///////////////////////////


pok_ret_t pok_port_queuing_create_packed_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "port1";
    
    const pok_port_queuing_create_arg_t* arg;  // TODO: check right value
    
    pok_port_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_queuing_create_packed(&param, &arg, &id); 
			break;
            
        case 1:
            ret = pok_port_queuing_create_packed(&name, &param, &id);
			break;
        
		case 2:
            ret = pok_port_queuing_create_packed(&name, &arg, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_send_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    const void* data;   // TODO: check the right value
    pok_port_size_t len; 
    const pok_time_t* timeout;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_send(&id, &param, &len, &timeout); 
			break;
        
        case 3:
			;
			ret = pok_port_queuing_send(&id, &data, &len, &param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_receive_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    const pok_time_t* timeout; // TODO: check the right value
    void* data;   
    pok_port_size_t* len; 
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_receive(&id, &param, &data, &len); 
			break;
        
        case 2:
			;
			ret = pok_port_queuing_receive(&id, &timeout, &param, &len); 
			break;
		
		case 3:
            ret = pok_port_queuing_receive(&id, &timeout, &data, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created port needed (check also with no port)
    
    pok_port_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_queuing_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_port_queuing_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed (check also with no port)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

*/
