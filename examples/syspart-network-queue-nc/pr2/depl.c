#include <sysconfig.h>

enum {
    overhead_p3041 = 0,
    overhead_virtio = 10,
    overhead_udp = 42,
};

static struct {
    MESSAGE_SIZE_TYPE message_size;
    enum q_status status;
    uint32_t buffer_idx;
    char data[overhead_virtio + overhead_udp + 64];
} qp_0_data[10];

static struct {
    MESSAGE_SIZE_TYPE message_size;
    enum q_status status;
    uint32_t buffer_idx;
    char data[overhead_virtio + overhead_udp + 64];
} qp_1_data[10];

unsigned sysconfig_samples_nb = 0;
sample_t samples[] = {
};
unsigned sysconfig_queues_nb = 2;
queue_t queues[] = {
    {
        .max_message_size = 64,
        .max_nb_messages = 10,

        .data = (void *) qp_0_data,
        .data_stride = sizeof(qp_0_data[0]),
    },
    {
        .max_message_size = 64,
        .max_nb_messages = 10,

        .data = (void *) qp_1_data,
        .data_stride = sizeof(qp_1_data[0]),
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

#define IP_ADDR(a,b,c,d) ((uint32_t)((a) & 0xff) << 24) | \
                         ((uint32_t)((b) & 0xff) << 16) | \
                         ((uint32_t)((c) & 0xff) << 8)  | \
                          (uint32_t)((d) & 0xff)

const uint32_t pok_network_ip_address = IP_ADDR(192, 168, 160, 200);

unsigned sysconfig_links_nb = 2;
link_t links[] = {
    {
        .device_id = 0,
        .buffer_idx = 0,
        .linked_port_info = {
            .kind = POK_PORT_KIND_QUEUEING,
            .direction = DESTINATION,
            .name = "UOUT",
            .queueing_data = {
                .max_message_size = 64,
                .max_nb_messages = 10,
            },
        },

        .protocol = UDP,
        .udp_data = { //destination module ip
            .ip = IP_ADDR(192, 168, 160, 99),
            .port = 10000,
        }
    },
    {
        .device_id = 0,
        .buffer_idx = 1,
        .linked_port_info = {
            .kind = POK_PORT_KIND_QUEUEING,
            .direction = SOURCE,
            .name = "UIN",
            .queueing_data = {
                .max_message_size = 64,
                .max_nb_messages = 10,
            },
        },

        .protocol = UDP,
        .udp_data = { //destination module ip
            .ip = IP_ADDR(192, 168, 160, 200),
            .port = 10000,
        }
    },
};
