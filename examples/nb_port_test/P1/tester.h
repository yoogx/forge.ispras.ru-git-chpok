#include "wrappers.h"

pok_ret_t pointer_error_tester(int syscall_id, int error_type, void* param, int pos);
//pok_ret_t null_pointer_tester(int syscall_id, int pos);
//pok_ret_t tester1 (int syscall_id, void* param, int pos);
//pok_ret_t pok_syscall_thread_create_wrapper(void* param, int pos);
//void entry_point_method(void);
//void first_process(void);

// 0 - not executed	
// 1 - executed
typedef enum {
	NOT_EXECUTED = 0,
	EXECUTED = 1
} TEST_STATUS;

TEST_STATUS status;

// 0 						  - verdict: test failed (nothing happened)
// 1000						  - verdict: test passed (ERROR HANDLER)
// ERROR_CODE	     		  - verdict: test passed
// PARTITION START not NORMAL - verdict is rendered another way)
static int TEST_RESULT = 0;


typedef enum {
	
	NULL_POINTER = 0,
	OUT_OF_PARTITION_RANGE = 1,
	ENTRY_POINT = 2
} ERROR_TYPE;
