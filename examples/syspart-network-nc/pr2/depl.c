#include <sysconfig.h>

enum {
    overhead_udp = 42,
};

unsigned sysconfig_samples_nb = 2;
sample_t samples[] = {};
unsigned sysconfig_queues_nb = 0;
queue_t queues[] = {};

#define MY_IP IP_ADDR(10, 0, 2, 15)
const uint32_t pok_network_ip_address = MY_IP;

unsigned sysconfig_links_nb = 0;
link_t links[] = {};

sys_link_t sys_queuing_links[] = {};
unsigned sys_queuing_links_nb = ARRAY_SIZE(sys_queuing_links);

sys_link_t sys_sampling_links[] = {
    {
        .port_id = 1,
        .protocol = UDP,
        .udp_data = { //destination module ip
            .ip = IP_ADDR(10, 0, 2, 2),
            .port = 10000,
        }
    },
    {
        .port_id = 0,
        .protocol = UDP,
        .udp_data = {
            .ip = MY_IP,
            .port = 10000,
        }
    },
};
unsigned sys_sampling_links_nb = ARRAY_SIZE(sys_sampling_links);


struct mac_ip {
    uint32_t ip;
    uint8_t mac[6];
};

struct mac_ip mac_addr_mapping[] = {
    {
        .ip  = MY_IP,
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
    },
    {
        .ip = IP_ADDR(10, 0, 2, 2),
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    },
};
