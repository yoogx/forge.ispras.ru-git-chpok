#include <stdio.h>
#include <string.h>
//#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#include <net/network.h>

SAMPLING_PORT_ID_TYPE QP2;
#define SECOND 1000000000LL


const uint32_t pok_network_ip_address = 0xa000002;

static pok_bool_t udp_received_callback(
        uint32_t ip, 
        uint16_t port, 
        const char *payload, 
        size_t length) 
{
    //TODO check ip

    {
//        printf("PR2: sending message received from network... \n");

        RETURN_CODE_TYPE ret;

        SEND_QUEUING_MESSAGE(
                QP2,
                (MESSAGE_ADDR_TYPE) payload,
                length,
                0,
                &ret);
        if (ret == NOT_AVAILABLE) {
            printf("Buffer is full, drop packet\n");
            return TRUE;
        }
        if (ret != NO_ERROR) {
            printf("error: %u\n", ret);
        }
    }
    return TRUE;
}

static void first_process(void)
{
    while(1)
        pok_network_reclaim_receive_buffers();
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
    CREATE_QUEUING_PORT("QP2", 64, 10, SOURCE, FIFO, &QP2, &ret);

    // network init
    pok_network_init();

    static pok_network_udp_receive_callback_t udp_callback = {udp_received_callback, NULL};
    pok_network_register_udp_receive_callback(&udp_callback);

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
