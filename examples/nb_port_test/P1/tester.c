
#include "tester.h"



/*
 * one param is tested, others - autocompleted
 * 
 **/
pok_ret_t tester1 (int syscall_id, void* param, int pos)
{
	switch (syscall_id)
	{
		case POK_SYSCALL_THREAD_CREATE:
			;
			pok_ret_t ret = 0;
			ret = pok_syscall_thread_create_wrapper (param, pos);
			return ret;
			
			break;
			
		// other methods - for each methods one case
	}	
}

pok_ret_t null_pointer_tester(int syscall_id, int pos)
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
