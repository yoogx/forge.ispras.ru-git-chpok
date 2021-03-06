#include <stdio.h>
#include <string.h>
//#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

SAMPLING_PORT_ID_TYPE SP2;
#define SECOND 1000000000LL

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    struct {
        unsigned x;
        char message[32];
        unsigned y;
    } __attribute__((packed)) msg;

    unsigned last_x = 0;

    while (1) {
        MESSAGE_SIZE_TYPE len;
        VALIDITY_TYPE validity;

        READ_SAMPLING_MESSAGE(SP2, (MESSAGE_ADDR_TYPE) &msg, &len, &validity, &ret);

        if (ret == NO_ERROR) {
            printf("PR2: got message {%u, \"%s\", %u}\n", msg.x, msg.message, msg.y);
        } else {
            printf("PR2: sp error: %u\n", ret);
        }

        if (msg.x < last_x) {
            printf("PR2: warning: received SP message out of order\n");
        } else if (msg.x > last_x + 1) {
            printf("PR2: warning: possible SP packet loss\n");
        } else if (msg.x == last_x && last_x != 0) {
            printf("PR2: warning: possible SP duplicate message\n");
        }
        last_x = msg.x;

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
