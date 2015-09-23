#include <sysconfig.h>

enum {
 overhead_virtio = 10,
 overhead_udp = 42,
};

static struct {
    MESSAGE_SIZE_TYPE message_size;
    enum q_status status;
    uint32_t buffer_idx;
    char data[overhead_virtio + overhead_udp + 64];
} qp_0_data[10];

unsigned sysconfig_queues_nb = 1;
queue_t queues[] = {
    {
        .max_message_size = 64,
        .max_nb_messages = 10,
        
        .data = (void *) qp_0_data,
        .data_stride = sizeof(qp_0_data[0]),
    },
};

//pok_device_t devices[] = {
//    {
//        .driver = DRIVER_VIRTIO,
//        .kind = ETHERNET,
//        .ethernet_data = {
//            .ip = 0xa0000001
//        }
//    },
//}

const uint32_t pok_network_ip_address = 0xa000002;

unsigned sysconfig_links_nb = 1;
link_t links[] = {
    {
        .device_id = 0,
        .buffer_idx = 0,
        
        .linked_port_info = {
            .kind = POK_PORT_KIND_QUEUEING,
            .direction = SOURCE,
            .name = "QP2",
            .queueing_data = {
                .max_message_size = 64,
                .max_nb_messages = 10,
            },
        },

        .protocol = UDP,
        .udp_data = { //destination (our) ip
            .ip = 0xa000002,
            .port = 10001,
        }
    },
};
