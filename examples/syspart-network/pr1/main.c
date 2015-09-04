#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

SAMPLING_PORT_ID_TYPE SP1;
#define SECOND 1000000000LL

static void first_process(void)
{
    STOP_SELF();
    RETURN_CODE_TYPE ret;

    struct {
        unsigned x;
        char message[32];
        unsigned y;
    } __attribute__((packed)) msg;

    msg.x = 0;
    strcpy(msg.message, "test sp message");
    msg.y = -1;
    while (1) {
        printf("PR1: sending message ... \n");
        WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &msg, sizeof(msg), &ret);
        if (ret != NO_ERROR) {
            printf("error: %u\n", ret);
        }

        msg.x++;
        msg.y--;
        
        //TIMED_WAIT(1LL * 1000 * 1000 * 1000 / 20, &ret);
        //TIMED_WAIT(10*SECOND, &ret);
        TIMED_WAIT(SECOND, &ret);
    }
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
    //CREATE_QUEUING_PORT("QP1", 64, 10, DESTINATION, FIFO, &QP1, &ret);
    CREATE_SAMPLING_PORT("SP1", 64, SOURCE, 5*SECOND, &SP1, &ret);


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
