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

static void fill_in_mac_header(
        char *buffer,
        uint8_t *dst_mac,
        uint8_t *my_mac,
        enum ethertype type)
{
    struct ether_hdr *ether_hdr = (struct ether_hdr *) buffer;

    for (int i = 0; i < ETH_ALEN; i++) {
        ether_hdr->src[i] = my_mac[i];
        ether_hdr->dst[i] = dst_mac[i];
    }
    ether_hdr->ethertype = hton16(type);

}


pok_bool_t mac_send(
        char *buffer,
        size_t payload_size,
        uint8_t *dst_mac_addr,
        enum ethertype ethertype
        )
{

    fill_in_mac_header(
        buffer,
        dst_mac_addr,
        NETDEVICE_PTR->mac,
        ethertype
        );

    size_t frame_size;
    //HACK
    if (ethertype == ETH_P_ARP)
        frame_size = payload_size;
    else
        frame_size = payload_size + POK_NETWORK_OVERHEAD;

    return NETWORK_DRIVER_OPS->send_frame(
        NETDEVICE_PTR,
        buffer,
        frame_size
    );

}

void mac_flush(void) {
    NETWORK_DRIVER_OPS->flush_send(NETDEVICE_PTR);
}
