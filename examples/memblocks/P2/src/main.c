#include <stdio.h>
#include <string.h>
#include <arinc653/queueing.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/memblocks.h>

QUEUING_PORT_ID_TYPE QP2;
void *mb0addr;

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    int i = 0;
    int status;
    while (1) {
        MESSAGE_SIZE_TYPE msg_len;
        RECEIVE_QUEUING_MESSAGE(QP2, INFINITE_TIME_VALUE,
                (MESSAGE_ADDR_TYPE) &status, &msg_len, &ret);
        if (ret != NO_ERROR) {
            printf("P2 error: %d\n", (int) ret);
        }
        if (!status) {
            printf("P2: status == 0!\n");
            continue;
        }

        printf("P2: string in memory block '%s'\n", (char *)mb0addr);
        i++;
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


    CREATE_QUEUING_PORT("QP2", 64, 10, DESTINATION, FIFO, &QP2, &ret);

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
