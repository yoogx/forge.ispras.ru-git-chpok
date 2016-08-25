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

#include "net.h"

void fill_in_mac_header(
        char *buffer,
        size_t size, // size of UDP data
        uint8_t *dst_mac)
{
    struct {
        struct ether_hdr ether_hdr;
        char data[];
    } __attribute__((packed)) *real_buffer = (void*) buffer;

    for (int i = 0; i < ETH_ALEN; i++) {
        real_buffer->ether_hdr.src[i] = NETDEVICE_PTR->mac[i];
        real_buffer->ether_hdr.dst[i] = dst_mac[i];
    }
    real_buffer->ether_hdr.ethertype = hton16(ETH_P_IP);

}

pok_bool_t mac_send(
        char *buffer,
        size_t payload_size,
        void *driver_data
    )
{
    udp_data_t *udp_data = driver_data;
    uint8_t *dst_mac = find_mac_by_ip(udp_data->ip);

    fill_in_mac_header(
        buffer,
        payload_size,
        dst_mac
    );

    return NETWORK_DRIVER_OPS->send_frame(
        NETDEVICE_PTR,
        buffer,
        payload_size + POK_NETWORK_OVERHEAD
    );

}

void mac_flush(void) {
    NETWORK_DRIVER_OPS->flush_send(NETDEVICE_PTR);
}
