#include <sysconfig.h>
#include <channel_driver.h>
#include <net/network.h>

#define MY_IP IP_ADDR(192, 168, 0, 2)

const uint32_t pok_network_ip_address = MY_IP;

udp_data_t ipnet_data_0 = { //destination module ip
    .ip = IP_ADDR(192, 168, 0, 1),
    .port = 10000,
};

udp_data_t ipnet_data_1 = {
    .ip = MY_IP,
    .port = 10001,
};

sys_channel_t sys_queuing_channels[] = {
    {
        .port_index = 1,
        .driver_data = &ipnet_data_0,
        .driver_ptr = &ipnet_channel_driver,
    },
    {
        .port_index = 0,
        .driver_data = &ipnet_data_1,
        .driver_ptr = &ipnet_channel_driver,
    },
};

unsigned sys_queuing_channels_nb = ARRAY_SIZE(sys_queuing_channels);

sys_channel_t sys_sampling_channels[] = {
};
unsigned sys_sampling_channels_nb = ARRAY_SIZE(sys_sampling_channels);

struct mac_ip mac_addr_mapping[] = {
    {
        .ip = IP_ADDR(192, 168, 0, 1),
        .mac = {0x6e, 0x80, 0xfb, 0x46, 0xd4, 0x92},
    }
};

unsigned mac_addr_mapping_nb = ARRAY_SIZE(sys_sampling_channels);

channel_driver_t * channel_drivers[] = {
    &ipnet_channel_driver
};
unsigned channel_drivers_nb = ARRAY_SIZE(channel_drivers);

#include <drivers/virtio/virtio_network.h>
void drivers_init()
{
    virtio_net_init();
    pci_init();
    pok_network_init();
}
