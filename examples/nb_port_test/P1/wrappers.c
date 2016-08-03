#include "wrappers.h"


void entry_point_method(void)
{
	// add some output (print log)
    
    printf("the created process is working");
    
	return;	
}


/*
 * 
 * pos - position of tested param
 * 
 * other _n params are autocompleted
 * 
 **/
pok_ret_t pok_syscall_thread_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
	
	switch (pos)
	{
		case 0:
			//char* name = "process1";
			;
			void* entry = entry_point_method;
			
			pok_thread_attr_t attr = {
				.priority = 0,
				//.entry = entry_point_method,
				.period = INFINITE_TIME_VALUE,
				.deadline = SOFT,
				.time_capacity = INFINITE_TIME_VALUE,
				.stack_size = 8096,
			};
			
			pok_thread_id_t id;
			
			
			//strncpy(attrs.process_name, "process1", sizeof(PROCESS_NAME_TYPE));
			
            //ret = pok_thread_create("process 2", &entry, &attr, &id); // TODO: UNCOMMENT
			//ret = pok_thread_create(&param, &entry, &attr, &id);
			
			break;
		
		case 1:
		
			//pok_thread_id_t id;
			
			//pok_thread_create(&id, param);
			
			break;
	}
	
	return ret;
}
