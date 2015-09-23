#include <sysconfig.h>

enum {
 overhead_virtio = 10,
 overhead_udp = 42,
};

static struct {
    MESSAGE_SIZE_TYPE message_size;
    pok_bool_t busy;
    char data[overhead_virtio + overhead_udp + 64];
} sp_0_data;

unsigned sysconfig_samples_nb = 1;
sample_t samples[] = {
    {
        .max_message_size = 64,
        .data = (void *) &sp_0_data,
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

const uint32_t pok_network_ip_address = 0xa000002;

unsigned sysconfig_links_nb = 1;
link_t links[] = {
    {
        .device_id = 0,
        .buffer_idx = 0,
        
        .linked_port_info = {
            .kind = POK_PORT_KIND_SAMPLING,
            .direction = DESTINATION,
            .name = "SP2",
            .sampling_data = {
                .max_message_size = 64,
            },
        },

        .protocol = UDP,
        .udp_data = { //destination (our) ip
            .ip = 0xa000002,
            .port = 10000,
        }
    },
};
