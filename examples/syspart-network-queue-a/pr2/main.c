#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#include <net/network.h>
#include <sysconfig.h>

#define SECOND 1000000000LL

static void queueing_send_to_partition(unsigned link_idx, MESSAGE_ADDR_TYPE payload, size_t length)
{
    port_info_t *port_info = &links[link_idx].linked_port_info;
    RETURN_CODE_TYPE ret;

    SEND_QUEUING_MESSAGE(
            port_info->id,
            payload,
            length,
            0,
            &ret);

    if (ret == NOT_AVAILABLE) {
        printf("Buffer is full, drop packet\n");
    } else if (ret != NO_ERROR) {
        printf("SYSNET %ld port error: %u\n", port_info->id, ret);
    }
}

static void sampling_send_to_partition(unsigned link_idx, MESSAGE_ADDR_TYPE payload, size_t length)
{
    port_info_t *port_info = &links[link_idx].linked_port_info;
    RETURN_CODE_TYPE ret;

    WRITE_SAMPLING_MESSAGE(
            port_info->id,
            payload, 
            length, 
            &ret);

    if (ret != NO_ERROR) {
        printf("error: %u\n", ret);
    }
        
}
static pok_bool_t udp_received_callback(
        uint32_t ip, 
        uint16_t port, 
        const char *payload, 
        size_t length) 
{
    for (int i = 0; i<sysconfig_links_nb; i++) {
        if (links[i].protocol != UDP)
            continue;

        port_info_t port_info = links[i].linked_port_info;
        udp_data_t udp_data = links[i].udp_data;

        if (port_info.direction != SOURCE)
            continue;
        if (udp_data.ip != ip || udp_data.port != port) {
            continue;
        }

        if (port_info.kind == POK_PORT_KIND_QUEUEING)
            queueing_send_to_partition(i, (MESSAGE_ADDR_TYPE) payload, length);
        else 
            sampling_send_to_partition(i, (MESSAGE_ADDR_TYPE) payload, length);
    }

    return FALSE;
}

static void udp_sent_queueing_callback(void *arg)
{
    q_data_t *qdata = arg;
    qdata->status = QUEUEING_STATUS_SENT;
    queue_t *queue = &queues[qdata->queue_idx];

    for (int i = 0; i < queue->nb_message; i++) {
        q_data_t *cur_data = utils_queue_head(queue);

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

static void udp_sent_sampling_callback(void *arg) {
    pok_bool_t *var = (pok_bool_t*) arg;
    if (!*var)
        printf("error: buffer is no busy in callback\n");
    *var = FALSE;
}

static void queueing_send_outside(unsigned link_idx)
{
    link_t link = links[link_idx];
    unsigned queue_idx = link.buffer_idx;
    RETURN_CODE_TYPE ret;
    queue_t *queue = &queues[queue_idx];

    while (!utils_queue_full(queue)) {

        if (queue->nb_message > queue->max_nb_messages/2) {
            pok_network_reclaim_send_buffers();
        }

        q_data_t *dst_place = utils_queue_tail(queue);
        if (dst_place->status != QUEUEING_STATUS_NONE) {
            printf("SYSNET error: status is not NONE\n");
            STOP_SELF();
        }
        //TODO fix OVERHEAD
        RECEIVE_QUEUING_MESSAGE(
                link.linked_port_info.id,
                0,
                (MESSAGE_ADDR_TYPE ) (dst_place->data + POK_NETWORK_OVERHEAD),
                &dst_place->message_size,
                &ret
                );

        if (ret != NO_ERROR) {
            if (ret != NOT_AVAILABLE)
                printf("SYSNET: %s port error: %u\n", 
                        link.linked_port_info.name, ret);

            break;
        } 

        queue->nb_message++;

        if (link.protocol == UDP) {
            pok_bool_t res = pok_network_send_udp(
                    dst_place->data,
                    dst_place->message_size,
                    link.udp_data.ip,
                    link.udp_data.port,
                    udp_sent_queueing_callback,
                    (void*) dst_place
                    );

            if (!res)
                printf("SYSNET: Error in send_udp\n");
        }

        dst_place->status = QUEUEING_STATUS_PENDING;
        dst_place->queue_idx = queue_idx;
        pok_network_flush_send();
    }
}

static void sampling_send_outside(unsigned link_idx)
{
    link_t link = links[link_idx];
    RETURN_CODE_TYPE ret;
    VALIDITY_TYPE validity;
    sample_t *sample = &samples[link.buffer_idx];
    s_data_t *dst_place = sample->data;

    if (!SYS_SAMPLING_PORT_CHECK_IS_NEW_DATA(link.linked_port_info.id))
        return;

    if (dst_place->busy) {
        printf("SYSNET error: sampling buffer is busy\n");
        STOP_SELF();
        return;
    }

    //TODO fix OVERHEAD
    READ_SAMPLING_MESSAGE(
            link.linked_port_info.id,
            (MESSAGE_ADDR_TYPE ) (dst_place->data + POK_NETWORK_OVERHEAD),
            &dst_place->message_size,
            &validity,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf("SYSNET: %s port error: %u\n", 
                    link.linked_port_info.name, ret);

        return;
    }

    dst_place->busy = TRUE;

    if (link.protocol == UDP) {
        pok_bool_t res = pok_network_send_udp(
                dst_place->data,
                dst_place->message_size,
                link.udp_data.ip,
                link.udp_data.port,
                udp_sent_sampling_callback,
                &dst_place->busy
                );

        if (!res)
            printf("SYSNET: Error in send_udp\n");
    }

    pok_network_flush_send();
}

static void first_process(void)
{
    while (1) {
        for (int i = 0; i<sysconfig_links_nb; i++) {
            port_info_t port_info = links[i].linked_port_info;

            if (port_info.direction != DESTINATION)
                break;
            if (port_info.kind == POK_PORT_KIND_QUEUEING)
                queueing_send_outside(i);
            else 
                sampling_send_outside(i);

            pok_network_reclaim_send_buffers();
        }
        pok_network_reclaim_receive_buffers();
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

    for (int i = 0; i<sysconfig_links_nb; i++) {
        port_info_t *port_info = &links[i].linked_port_info;
        // create ports
        if (port_info->kind == POK_PORT_KIND_QUEUEING) {
            CREATE_QUEUING_PORT(
                    port_info->name, 
                    port_info->queueing_data.max_message_size, 
                    port_info->queueing_data.max_nb_messages, 
                    port_info->direction,
                    FIFO, 
                    &port_info->id, 
                    &ret);
        } else { 
            CREATE_SAMPLING_PORT(
                    port_info->name, 
                    port_info->sampling_data.max_message_size, 
                    port_info->direction,
                    0,
                    &port_info->id, 
                    &ret);
        }
       
        if (ret != NO_ERROR)
            printf("error %d creating %s port\n", ret, port_info->name);
    }

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
