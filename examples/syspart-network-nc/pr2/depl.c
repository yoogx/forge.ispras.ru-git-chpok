#include <sysconfig.h>

#define MY_IP IP_ADDR(10, 0, 2, 15)
const uint32_t pok_network_ip_address = MY_IP;

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
