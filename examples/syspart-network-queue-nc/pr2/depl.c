#include <sysconfig.h>
#include <channel_driver.h>

#define MY_IP IP_ADDR(192, 168, 160, 200)

const uint32_t pok_network_ip_address = MY_IP;

udp_data_t ipnet_data_0 = { //destination module ip
    .ip = IP_ADDR(192, 168, 160, 99),
    .port = 10000,
};

udp_data_t ipnet_data_1 = {
    .ip = MY_IP,
    .port = 10000,
};

//TODO
extern channel_driver_t ipnet_channel_driver;

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
