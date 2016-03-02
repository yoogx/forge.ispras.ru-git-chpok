#include <sysconfig.h>

enum {
 overhead_p3041 = 0,
 overhead_virtio = 10,
 overhead_udp = 42,
};

static struct {
    MESSAGE_SIZE_TYPE message_size;
    pok_bool_t busy;
    char data[overhead_p3041 + overhead_udp + 64];
} sp_0_data;

static struct {
    MESSAGE_SIZE_TYPE message_size;
    pok_bool_t busy;
    char data[overhead_p3041 + overhead_udp + 64];
} sp_1_data;

unsigned sysconfig_samples_nb = 2;
sample_t samples[] = {
    {
        .max_message_size = 64,
        .data = (void *) &sp_0_data,
    },
    {
        .max_message_size = 64,
        .data = (void *) &sp_1_data,
    },
};
unsigned sysconfig_queues_nb = 0;
queue_t queues[] = {};

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
            .kind = POK_PORT_KIND_SAMPLING,
            .direction = DESTINATION,
            .name = "UOUT",
            .sampling_data = {
                .max_message_size = 64,
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
            .kind = POK_PORT_KIND_SAMPLING,
            .direction = SOURCE,
            .name = "UIN",
            .sampling_data = {
                .max_message_size = 64,
            },
        },

        .protocol = UDP,
        .udp_data = { //destination module ip
            .ip = IP_ADDR(192, 168, 160, 200),
            .port = 10000,
        }
    },
};
