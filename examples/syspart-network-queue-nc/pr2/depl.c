#include <sysconfig.h>

#define MY_IP IP_ADDR(192, 168, 160, 200)

const uint32_t pok_network_ip_address = MY_IP;

sys_link_t sys_queuing_links[] = {
    {
        .port_id = 1,
        .protocol = UDP,
        .udp_data = { //destination module ip
            .ip = IP_ADDR(192, 168, 160, 99),
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
unsigned sys_queuing_links_nb = ARRAY_SIZE(sys_queuing_links);

sys_link_t sys_sampling_links[] = {
};
unsigned sys_sampling_links_nb = ARRAY_SIZE(sys_sampling_links);

struct mac_ip mac_addr_mapping[] = {
    {
        .ip  = MY_IP,
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} //NOT USED FOR NOW
    },
    {
        .ip = IP_ADDR(10, 0, 2, 2),
        .mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, //by default
    },
    {
        .ip = IP_ADDR(192, 168, 160, 99),
        .mac = {0x00, 0x80, 0x48, 0x6b, 0x53, 0xa9},
    }
};

unsigned mac_addr_mapping_nb = ARRAY_SIZE(sys_sampling_links);
