#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

SAMPLING_PORT_ID_TYPE SP1;
SAMPLING_PORT_ID_TYPE SP2;
#define SECOND 1000000000LL

static void __first_process(void)
{
    RETURN_CODE_TYPE ret;

    /* Wait a little until second partition is initialized */
    TIMED_WAIT(SECOND/5, &ret);

    char msg[64];

    int i = 0;
    while (1) {
        i++;
        snprintf(msg, 64, "test sp message %d\n", i);
        WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &msg, sizeof(msg), &ret);
        if (ret != NO_ERROR) {
            printf("error: %u\n", ret);
        }
        TIMED_WAIT(3*SECOND, &ret);
    }
}

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    char msg[64];

    /* Wait a little until second partition is initialized */
    TIMED_WAIT(SECOND/5, &ret);

    snprintf(msg, 64, "Hello \n");
    WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &msg, sizeof(msg), &ret);

    while (1) {
        MESSAGE_SIZE_TYPE len;
        VALIDITY_TYPE validity;

        READ_SAMPLING_MESSAGE(SP2, (MESSAGE_ADDR_TYPE) &msg, &len, &validity, &ret);

        if (ret == NO_ERROR) {
            msg[len] = '\0';
            printf("%s", msg);
        } else if (ret == NO_ACTION) {
        //    printf("waiting for packet\n");
        } else {
            printf("PR1: sp error: %u\n", ret);
        }

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
    CREATE_SAMPLING_PORT("SP1", 64, SOURCE, 5*SECOND, &SP1, &ret);
    CREATE_SAMPLING_PORT("SP2", 64, DESTINATION, 5*SECOND, &SP2, &ret);


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
