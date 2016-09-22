#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

QUEUING_PORT_ID_TYPE QP1;
QUEUING_PORT_ID_TYPE QP2;
#define SECOND 1000000000LL

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    char msg[30];
    
    printf( "PR1:=== Test case SIMPLE_UDP_RECEIVE started ===\n" );
    printf( "PR1:Expect to receive \"helloworld\" message\n" );
    //TODO: Time init

    /* Wait a little until second partition is initialized */
    TIMED_WAIT(SECOND/5, &ret);

    MESSAGE_SIZE_TYPE len;
    RECEIVE_QUEUING_MESSAGE(QP2, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &msg, &len, &ret);
    if (ret == NO_ERROR) 
    {
        msg[len] = '\0';
        if( !strcmp(msg, "helloworld" ) )
            printf( "PR1:VERDICT: OK\n" );
        else
            printf( "PR1:VERDICT: NOT OK\n" ); //smth else mb
    }
    else
    {
        printf("PR1: qp error: %u\n", ret);
        printf( "PR1:VERDICT: NOT OK\n" ); //smth else mb
   }
    TIMED_WAIT(SECOND, &ret);
    printf( "=== Test case SIMPLE_UDP_RECEIVE finished, elapsed time ??? secs ===\n" );
    STOP_SELF();
}


static int real_main(void)
{
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
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }


    // create ports
    CREATE_QUEUING_PORT("QP1", 64, 10, SOURCE,      FIFO, &QP1, &ret);
    CREATE_QUEUING_PORT("QP2", 64, 10, DESTINATION, FIFO, &QP2, &ret);


    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    printf("going to NORMAL mode...\n");
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    } 

    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}
