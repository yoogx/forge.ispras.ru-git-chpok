#include "wrappers.h"

// TODO: add 2 param case (tester2 and other methods to fit it)
//       comes in handy if the programmer wants to set more then one parameter.
//       for example,
//       one param is tested, other param is tuned (for instance, timeout of pok_buffer_read), and others are autocompleted => tuned1_tester1, tuned2_tester1
//       or
//       two params needed to setup test: [message + len of message] or [type of sent message + type of received message]

pok_ret_t tester1 (int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

pok_ret_t tester_read(int entity_type, void* message, uint8_t pre_created_id, const char* pre_created_name);

pok_ret_t pointer_error_tester(int syscall_id, int error_type, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

// for each type of error
pok_ret_t null_pointer_tester           (int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t out_of_partition_range_tester (int syscall_id, void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);



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

// 0 						  - verdict: test failed (nothing happened) - impossible scenario, actually. Because of a syscall signature
// 1000						  - verdict: test passed (ERROR HANDLER)
// ERROR_CODE	     		  - verdict: test passed
// PARTITION START not NORMAL - verdict is rendered another way
static int TEST_RESULT = 0;

// TODO: add other types of errors
typedef enum {
	NULL_POINTER = 0,
	OUT_OF_PARTITION_RANGE = 1,
	ENTRY_POINT = 2
} ERROR_TYPE;

extern char* test_eom; // TODO: copy format to other


