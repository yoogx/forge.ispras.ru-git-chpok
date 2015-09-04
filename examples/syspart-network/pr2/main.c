#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>
#include <memory.h>

#include <net/network.h>

#include "drivers/virtio/mem.h"

SAMPLING_PORT_ID_TYPE SP2;
#define SECOND 1000000000LL

static void pok_sampling_channel_udp_buffer_callback(void *arg) {
    printf("callback\n");
    pok_bool_t *var = (pok_bool_t*) arg;
    *var = FALSE;
}

static void first_process(void)
{
    struct {
        unsigned x;
        char message[32];
        unsigned y;
    } __attribute__((packed)) msg;

    unsigned last_x = 0;

//    while (1) {
//        RETURN_CODE_TYPE ret;
//        MESSAGE_SIZE_TYPE len;
//        VALIDITY_TYPE validity;
//
//        READ_SAMPLING_MESSAGE(SP2, (MESSAGE_ADDR_TYPE) &msg, &len, &validity, &ret);
//
//        if (ret == NO_ERROR) {
//            printf("PR2: %u %s %u\n", msg.x, msg.message, msg.y);
//        } else {
//            printf("PR2: sp error: %u\n", ret);
//        }
//
//        if (msg.x < last_x) {
//            printf("PR2: warning: received SP message out of order\n");
//        } else if (msg.x > last_x + 1) {
//            printf("PR2: warning: possible SP packet loss\n");
//        } else if (msg.x == last_x && last_x != 0) {
//            printf("PR2: warning: possible SP duplicate message\n");
//        }
//        last_x = msg.x;
//
//        TIMED_WAIT(SECOND/2, &ret);
////TIMED_WAIT(1000000000LL, &ret);
//    }

    {
        msg.x = 5;
        strcpy(msg.message, "test sp message");
        msg.y = 7;


        pok_bool_t buffer_being_used;
        char message_buffer[256];
        memcpy(message_buffer + POK_NETWORK_OVERHEAD, &msg, sizeof(msg));

        if (!pok_network_send_udp(
                            message_buffer,
                            sizeof(msg),
                            0xa000002,
                            10000,
                            pok_sampling_channel_udp_buffer_callback,
                            &buffer_being_used)) 
        {
            printf("Error in send_udp\n");
        }


        pok_network_flush_send();
        pok_network_reclaim_buffers();

        STOP_SELF();
    }
}


const uint32_t pok_network_ip_address = 0xa000001;

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


    CREATE_SAMPLING_PORT("SP2", 64, DESTINATION, 5*SECOND, &SP2, &ret);

    
    //----- NETWORK
    //
    pok_network_init();




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
