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

#include <net/network.h>
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>
#include <pci.h>
#include <depl.h>

#include <stdio.h>
#include <string.h>
#include <net/netdevices.h>
#include <channel_driver.h>


pok_bool_t udp_ip_send(
        char *payload,
        size_t payload_size,
        size_t max_backstep,
        void *driver_data
    );

pok_bool_t initialized = FALSE;

pok_netdevice_t *current_netdevice;
#define NETDEVICE_PTR current_netdevice
#define NETWORK_DRIVER_OPS (NETDEVICE_PTR->ops)

static pok_bool_t (*received_callback)(
        uint32_t ip,
        uint16_t port,
        const char *payload,
        size_t length);


void udp_ip_flush();


//static pok_bool_t ether_is_broadcast(const uint8_t addr[ETH_ALEN]) {
//    for (int i = 0; i < ETH_ALEN; ++i) {
//        if (addr[i] != 0xFF) {
//            return 0;
//        }
//    }
//    return 1;
//}


static void packet_received_callback(const char *data, size_t len)
{
}

uint8_t default_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

extern struct mac_ip mac_addr_mapping[];
uint8_t* find_mac_by_ip(uint32_t dst_ip)
{
    for (int i=0; i < mac_addr_mapping_nb; i++) {
        if (mac_addr_mapping[i].ip == dst_ip)
            return mac_addr_mapping[i].mac;
    }
    return default_mac;
}

void pok_network_init(void)
{
    current_netdevice = get_netdevice(ipnet_netdev_name);

    if (current_netdevice == NULL) {
        printf("ipnet ERROR: netdevice '%s' not found\n", ipnet_netdev_name);
        return;
    }

    NETWORK_DRIVER_OPS->set_packet_received_callback(NETDEVICE_PTR, packet_received_callback);
    initialized = TRUE;
    printf("INITED\n");
}


static void receive(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_receive_buffers(NETDEVICE_PTR);
    }
}

void register_received_callback(
            pok_bool_t (*callback)(
                uint32_t ip,
                uint16_t port,
                const char *payload,
                size_t length
                )
            )
{
    received_callback = callback;
}

struct channel_driver ipnet_channel_driver = {
    .send = udp_ip_send,
    .receive = receive,
    .flush_send = udp_ip_flush,
    .register_received_callback = register_received_callback
};

