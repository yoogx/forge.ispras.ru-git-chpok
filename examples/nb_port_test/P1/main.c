#include "tester.h"

typedef int (*TestProcType)(void);
static TestProcType MasterErrorHandlerTestProc;

static PROCESS_ID_TYPE ControlProcessId;

static uint8_t  pre_created_id;
static char*    pre_created_name;


void first_process(void)
{
    //const pok_port_id_t id = 1;
    //uint32_t* result = 0x80000000;
    //pok_ret_t r = pok_port_virtual_nb_destinations(id, result);
    
    //uint32_t* result = 0x80000000;
    
    //int param = 0;
    char* param = "text";
    int pos = 0;
    
    //pok_ret_t 
    int ret = 0;
    ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_CREATE,     NULL_POINTER, param, pos, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_SEND,       NULL_POINTER, param, pos, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_RECEIVE,    NULL_POINTER, param, pos, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_ID,         NULL_POINTER, param, pos, pre_created_id, pre_created_name);
    //ret = pointer_error_tester(POK_SYSCALL_INTRA_BUFFER_STATUS,     NULL_POINTER, param, pos, pre_created_id, pre_created_name);
    
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
		printf("VERDICT: TEST FAILED (nothing happened after bad param\n");
	} 
	else
	{
		printf("VERDICT: TEST PASSED, code = %d\n", TEST_RESULT);
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
    
    // create buffer
    CREATE_BUFFER(pre_created_name, sizeof(int), 10, FIFO, &id, &ReturnCode);
    if (ReturnCode != NO_ERROR) {
        printf("error creating a buffer: %d\n", (int) ReturnCode);
        return 1;
    } else {
        printf("buffer successfully created\n");
    }
    pre_created_id = id;
    
	
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
	
	printf("started\n");
	
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
