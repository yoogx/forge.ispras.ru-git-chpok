#include "../../../libpok/test_framework/tester.h"

typedef int (*TestProcType)(void);
static TestProcType MasterErrorHandlerTestProc;

static PROCESS_ID_TYPE ControlProcessId;

static uint8_t  pre_created_id;
static char*    pre_created_name;

void first_process(void)
{
    
    //uint32_t* result = 0x80000000;
    
    //int param = 0;
    char* param = "text";
    
    //pok_ret_t 
    int ret = 0;
    
    // THREAD NP
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             NULL_POINTER, param, 2, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             NULL_POINTER, param, 3, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_SLEEP,              NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    // here should be sleep_until
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_SUSPEND,            NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_ID,                 NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             NULL_POINTER, param, 2, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             NULL_POINTER, param, 3, pre_created_id, pre_created_name);
    
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_DELAYED_START,      NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_REPLENISH,          NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_FIND,               NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_FIND,               NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_GET_STATUS,      NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_INC_LOCK_LEVEL,  NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL,  NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    
    
    // THREAD OPR
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_CREATE,             OUT_OF_PARTITION_RANGE, param, 3, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_SLEEP,              OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    // here should be sleep_until
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_SUSPEND,            OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_ID,                 OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_STATUS,             OUT_OF_PARTITION_RANGE, param, 3, pre_created_id, pre_created_name);
    
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_DELAYED_START,      OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_REPLENISH,          OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_FIND,               OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_THREAD_FIND,               OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_GET_STATUS,      OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_INC_LOCK_LEVEL,  OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    //ret = pointer_error_tester(POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL,  OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    
    
    // BUFFER NP
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_CREATE,       NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_CREATE,       NULL_POINTER, param, 4, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_SEND,       NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_SEND,       NULL_POINTER, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    NULL_POINTER, param, 2, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    NULL_POINTER, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_ID,         NULL_POINTER, param, 0, pre_created_id, pre_created_name); // 11
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_ID,         NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 11
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_STATUS,     NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    // BUFFER OPR
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_CREATE,       OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_CREATE,       OUT_OF_PARTITION_RANGE, param, 4, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_SEND,       OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_SEND,       OUT_OF_PARTITION_RANGE, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    OUT_OF_PARTITION_RANGE, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_ID,         OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_ID,         OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // TEST FAILED
    
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_STATUS,     OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    // BB NP
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       NULL_POINTER, param, 2, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_DISPLAY,    NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       NULL_POINTER, param, 2, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       NULL_POINTER, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CLEAR,      NULL_POINTER, param, 0, pre_created_id, pre_created_name); // TEST FAILED
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_ID,         NULL_POINTER, param, 0, pre_created_id, pre_created_name); // 11
    ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_ID,         NULL_POINTER, param, 1, pre_created_id, pre_created_name); // FATAL ERROR - FIX
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_STATUS,     NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    
    // BB OPR
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_DISPLAY,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_READ,       OUT_OF_PARTITION_RANGE, param, 3, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_CLEAR,      OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name); // TEST FAILED
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_ID,         OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name); // 2
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_ID,         OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // TEST FAILED
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_BLACKBOARD_STATUS,     OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    
    
    // SEM NP
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    NULL_POINTER, param, 4, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_WAIT,    NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_ID,      NULL_POINTER, param, 0, pre_created_id, pre_created_name); // 11
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_ID,      NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 11
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_STATUS,  NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    // SEM OPR
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    OUT_OF_PARTITION_RANGE, param, 4, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_WAIT,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_ID,      OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name); // 11
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_ID,      OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // TEST FAILED
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_SEMAPHORE_STATUS,  OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    
    // EVENT NP
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_CREATE,    NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_CREATE,    NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_WAIT,    NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_ID,      NULL_POINTER, param, 0, pre_created_id, pre_created_name); // 11
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_ID,      NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 11
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_STATUS,  NULL_POINTER, param, 1, pre_created_id, pre_created_name); // 2
    
    // EVENT OPR
    
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_CREATE,    OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_CREATE,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_WAIT,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_ID,      OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name); // 11
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_ID,      OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // TEST FAILED
    
    //~ ret = pointer_error_tester(POK_SYSCALL_INTRA_EVENT_STATUS,  OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name); // 2
    
    status = EXECUTED;
    TEST_RESULT = ret;
    
    STOP_SELF();
}

 void ControlProc()
 {
	printf("ControlProc - ENTERED\n");
	RETURN_CODE_TYPE ReturnCode;
	
    while (status == NOT_EXECUTED) {
       TIMED_WAIT(1000000000LL, &ReturnCode);
    }
    
    if (TEST_RESULT == 0)
    {
		printf("\n\nVERDICT: TEST FAILED (ERRNO_OK after bad param)\n\n\n");
	} 
	else
	{
		printf("\n\nVERDICT: TEST PASSED, code = %d\n\n\n", TEST_RESULT);
	}
	
	STOP_SELF();
 }
 
 int T_MYTEST_Error_Handler()
{
	int verdict = 0;
	
	printf("\n\nMY OWN ERROR HANDLER\n\n");
	
	status = EXECUTED;
	TEST_RESULT = 1000;
	
	return verdict;
}

 int MasterErrorHandler()
 {
	 int result = 0;
	 
	 printf("Master process Error_Handler thread started.\n");
	 
	 if (MasterErrorHandlerTestProc != NULL)
	 {
		 result = (*MasterErrorHandlerTestProc)();
	 }
	 
	 printf("Master process Error_Handler thread finished.\n");
	 
	 return result;
 }

static int real_main(void)
{
	STACK_SIZE_TYPE VALID_STACK_SIZE = 8192;
	RETURN_CODE_TYPE ReturnCode;
    BUFFER_ID_TYPE id;
    pre_created_id = -1;
    pre_created_name = "foo";
    
    
    
    
    pre_created_id = create_entity(BLACKBOARD, pre_created_name);
    
    
    //ret = tester_create(POK_SYSCALL_INTRA_BUFFER_CREATE,       NULL_POINTER, param, 4, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_BUFFER_CREATE,       OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_BUFFER_CREATE,       OUT_OF_PARTITION_RANGE, param, 4, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       NULL_POINTER, param, 2, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_BLACKBOARD_CREATE,       OUT_OF_PARTITION_RANGE, param, 2, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    NULL_POINTER, param, 4, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_SEMAPHORE_CREATE,    OUT_OF_PARTITION_RANGE, param, 4, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_EVENT_CREATE,    NULL_POINTER, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_EVENT_CREATE,    NULL_POINTER, param, 1, pre_created_id, pre_created_name);
    
    //ret = tester_create(POK_SYSCALL_INTRA_EVENT_CREATE,    OUT_OF_PARTITION_RANGE, param, 0, pre_created_id, pre_created_name);
    //ret = tester_create(POK_SYSCALL_INTRA_EVENT_CREATE,    OUT_OF_PARTITION_RANGE, param, 1, pre_created_id, pre_created_name);
	
	MasterErrorHandlerTestProc = &T_MYTEST_Error_Handler;
	
	// create error handler
	CREATE_ERROR_HANDLER(&MasterErrorHandler, VALID_STACK_SIZE, &ReturnCode);
	if (ReturnCode != NO_ERROR) {
        printf("error creating ERROR HANDLER: %d\n", (int) ReturnCode);
        return 1;
    } else {
        printf("ERROR HANDLER successfully created\n");
    }
    
    RETURN_CODE_TYPE ret;
    
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 created\n");
    }
    // start process 1
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }
    
    // create control process
	PROCESS_ATTRIBUTE_TYPE ControlProcessAttributes = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

	ControlProcessAttributes.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE) &ControlProc;
	strncpy(ControlProcessAttributes.NAME, "Control process", sizeof(PROCESS_NAME_TYPE));
	CREATE_PROCESS (&ControlProcessAttributes, &ControlProcessId, &ReturnCode);
	if (ReturnCode != NO_ERROR) {
        printf("couldn't create ControlProc process: %d\n", (int) ReturnCode);
        return 1;
    } else {
        printf("ControlProc process created\n");
    }
	
	// start control process
	START (ControlProcessId, &ReturnCode);
    if (ReturnCode != NO_ERROR) {
        printf("couldn't start ControlProc: %d\n", (int) ReturnCode);
        return 1;
    } else {
        printf("ControlProc \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    } 

    //STOP_SELF();
    return 0;
}

void main(void) {
	
	status = NOT_EXECUTED;

	
	PARTITION_STATUS_TYPE part_status;
	RETURN_CODE_TYPE ret;
	
	GET_PARTITION_STATUS(&part_status, &ret);
	
	if (part_status.START_CONDITION != 0) 
	{
		printf("VERDICT: TEST PASSED - PARTITION RESTART WAS CALLED\n");
		printf("partition (re)start mode: %d\n", (int) part_status.START_CONDITION);
		STOP_SELF();
		return;
	}
	
    real_main();  
    STOP_SELF();
}  
