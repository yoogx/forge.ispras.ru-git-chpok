#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#include <net/network.h>

// ***************THIS WILL BE MOVED TO DEPLOYMENT.C**************
// START {{{
//
//
//

typedef struct
{
    uint32_t max_message_size;
    uint32_t max_nb_messages;
    uint32_t nb_message;
    uint32_t head;
    size_t   data_stride;
    char    *data;
} queue_t;

enum q_status{
    QUEUEING_STATUS_NONE, // message hasn't been touched by network code at all
    QUEUEING_STATUS_PENDING, // message has been sent to the driver, and its buffer is still in use by that driver
    QUEUEING_STATUS_SENT, // message sent, buffer is free to use, but place is still occupied (it will be reclaimed soon)
};

typedef struct
{
    MESSAGE_SIZE_TYPE message_size;
    enum q_status status;
    uint32_t queue_idx;
    char     data[];
} q_data_t;

static inline q_data_t * utils_queue_tail(queue_t *queue)
{
    uint32_t index = (queue->head + queue->nb_message) % queue->max_nb_messages;
    return (q_data_t*) (queue->data + queue->data_stride * index);
}

static inline q_data_t * utils_queue_head(queue_t *queue)
{
    uint32_t index = queue->head;
    return (q_data_t *) (queue->data + queue->data_stride * index);
}

static inline pok_bool_t utils_queue_empty(queue_t *queue)
{
    return queue->nb_message == 0;
}

static inline pok_bool_t utils_queue_full(queue_t *queue) 
{
    return queue->nb_message == queue->max_nb_messages;
}

static struct {
    MESSAGE_SIZE_TYPE message_size;
    enum q_status status;
    uint32_t queue_idx;
    char data[POK_NETWORK_OVERHEAD + 64];
} qp_0_data[10];


queue_t queues[] = {
    {
        .max_message_size = 64,
        .max_nb_messages = 10,
        
        .data = (void *) qp_0_data,
        .data_stride = sizeof(qp_0_data[0]),
    },

};

const uint32_t pok_network_ip_address = 0xa000001;

// END }}}
// ***************************************************************
//TODO DELETE IT
struct message {
    unsigned x;
    char message[32];
    unsigned y;
} __attribute__((packed));

// ***************************************************************

SAMPLING_PORT_ID_TYPE QP2;
#define SECOND 1000000000LL

static void udp_sent_queueing_callback(void *arg)
{
    printf("callback\n");
    q_data_t *qdata = arg;
    qdata->status = QUEUEING_STATUS_SENT;
    queue_t *queue = &queues[qdata->queue_idx];
    //queue_t *queue = &queues[0];

    //printf("%lu\n", queue->nb_message);
    for (int i = 0; i < queue->nb_message; i++) {
        q_data_t *cur_data = utils_queue_head(queue);

        //uint32_t index = (queue->head + queue->nb_message) % queue->max_nb_messages;

        if (cur_data->status == QUEUEING_STATUS_SENT) {
            queue->head = (queue->head + 1) % queue->max_nb_messages;
            queue->nb_message--;

            cur_data->status = QUEUEING_STATUS_NONE;

        } else {
            // ignore messages not at the head of the queue
            // even if they're already sent, and buffers are free to use
            break;
        }
    }

}

static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    while (1) {
        queue_t *queue = &queues[0];

        if (utils_queue_full(queue)){
            printf("FULLL\n");
            continue;
        }
        if(queue->nb_message>queue->max_nb_messages/2) {
            pok_network_reclaim_send_buffers();
        }

        q_data_t *dst_place = utils_queue_tail(queue);
        if (dst_place->status != QUEUEING_STATUS_NONE) {
            printf("ERROR. status is not NONE\n");
            STOP_SELF();
        }

        RECEIVE_QUEUING_MESSAGE(
                QP2, 
                0, 
                (MESSAGE_ADDR_TYPE ) (dst_place->data+POK_NETWORK_OVERHEAD), 
                &dst_place->message_size, 
                &ret
                );

        if (ret == NOT_AVAILABLE) {
            continue;
        }



        if (ret != NO_ERROR ) {
            printf("PR2: error: %u\n", ret);
        } else {
            queue->nb_message++;

            {
                struct message msg = *(struct message *)(dst_place->data+
                        POK_NETWORK_OVERHEAD);
                printf("PR2: Received queueing {%u \"%s\" %u}\n", 
                        msg.x, msg.message, msg.y);
            }

            if (!pok_network_send_udp(
                        dst_place->data,
                        dst_place->message_size,
                        0xa000002,
                        10000,
                        udp_sent_queueing_callback,
                        (void*) dst_place
                        )) {
                printf("Error in send_udp\n");
            }

            dst_place->status = QUEUEING_STATUS_PENDING;
            dst_place->queue_idx = 0; //TODO
            pok_network_flush_send();

        }
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
    CREATE_QUEUING_PORT("QP2", 64, 10, DESTINATION, FIFO, &QP2, &ret);

    // network init
    pok_network_init();


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
