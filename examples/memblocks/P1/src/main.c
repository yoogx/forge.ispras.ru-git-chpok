#include <stdio.h>
#include <string.h>
#include <arinc653/queueing.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/memblocks.h>

QUEUING_PORT_ID_TYPE QP1;
void *mb0addr;

static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    int i = 0;
    int status = 1; //non-zero means 'ready'
    while (1) {
        snprintf(mb0addr, 1000, "hello %d", i);

        SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) &status, sizeof(status),
                0, &ret);
        if (ret != NO_ERROR) {
                printf("P1 error: %d\n", (int) ret);
        }

        i++;
        TIMED_WAIT(1000000000LL, &ret);
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

    // create ports
    CREATE_QUEUING_PORT("QP1", 64, 10, SOURCE, FIFO, &QP1, &ret);

    if (ret != NO_ERROR) {
        printf("error creating a port: %d\n", (int) ret);
        return 1;
    }

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

    MEMORY_BLOCK_STATUS_TYPE status;
    GET_MEMORY_BLOCK_STATUS("shared_mb0", &status, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't get Memory Block: %d\n", (int) ret);
        return 1;
    }
    printf("P1 memblock: %s %p [0x%lx]\n",
            status.MEMORY_BLOCK_MODE == MB_READ? "MB_READ": "MB_READ_WRITE",
            status.MEMORY_BLOCK_ADDR,
            status.MEMORY_BLOCK_SIZE);
    mb0addr = status.MEMORY_BLOCK_ADDR;


    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
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
