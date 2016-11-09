/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */
#include <depl.h>
#include <channel_driver.h>
#include <net/network.h>


#define MY_IP IP_ADDR(192, 168, 56, 101)

const uint32_t pok_network_ip_address = MY_IP;

udp_data_t ipnet_data_0 = { //destination module ip
    .ip = IP_ADDR(192, 168, 56, 1),
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
        .ip = IP_ADDR(192, 168, 56, 1),
        //.mac = {0x6e, 0x80, 0xfb, 0x46, 0xd4, 0x92},
        .mac = {0x08, 0x00, 0x27, 0x00, 0x88, 0xAD},
    }
};

unsigned mac_addr_mapping_nb = ARRAY_SIZE(mac_addr_mapping);

channel_driver_t * channel_drivers[] = {
    &ipnet_channel_driver
};
unsigned channel_drivers_nb = ARRAY_SIZE(channel_drivers);

char *ipnet_netdev_name = "virtio-net0";
//char *ipnet_netdev_name = "ne2000-net1";
//char *ipnet_netdev_name = "dtsec3";

#include <drivers/virtio/virtio_network.h>
#include <drivers/p3041/p3041.h>
#include <drivers/ne2000/ne2000.h>
void drivers_init()
{
    virtio_net_init();
    ne2000_net_init();
    pci_init();
    //TODO add next string if BSP=P3041
    //dtsec_init();
    pok_network_init();
}

// for afdx
// the number of queuing ports on sys_part 
uint16_t config_queuing_port_list_size = 2;

queuing_port_config_t config_queuing_port_list[] = {
    {
        .name = "QP2",
        .msg_size = MAX_AFDX_PAYLOAD_SIZE,
        .max_nb_msg = MAX_NB_MESSAGE,
        .port_dir = DESTINATION,
        .que_disc = FIFO,
    },
    {
        .name = "QP4",
        .msg_size = MAX_AFDX_PAYLOAD_SIZE,
        .max_nb_msg = MAX_NB_MESSAGE,
        .port_dir = DESTINATION,
        .que_disc = FIFO,
    }
};
    

