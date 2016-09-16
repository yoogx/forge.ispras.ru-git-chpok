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

#include "MAC_SENDER_gen.h"


#define MAC_HEADER_SIZE 14

struct mac_packet {
    struct ether_hdr ether_hdr;
    char payload[];
} __attribute__((packed));

static void fill_in_mac_header(
        MAC_SENDER_state *state,
        struct mac_packet *packet,
        uint8_t *dst_mac,
        enum ethertype type)
{
    struct ether_hdr *ether_hdr = &packet->ether_hdr;

    for (int i = 0; i < ETH_ALEN; i++) {
        ether_hdr->src[i] = state->src_mac[i];
        ether_hdr->dst[i] = dst_mac[i];
    }
    ether_hdr->ethertype = hton16(type);

}

ret_t mac_send(MAC_SENDER *self,
        char *payload,
        size_t payload_size,
        size_t max_backstep,
        uint8_t *dst_mac_addr,
        enum ethertype ethertype
        )
{
    if (max_backstep < MAC_HEADER_SIZE)
        return EINVAL;

    void *mac_packet = payload - MAC_HEADER_SIZE;
    fill_in_mac_header(
        &self->state,
        mac_packet,
        dst_mac_addr,
        ethertype
        );

    NETWORK_DRIVER_OPS->send_frame(
        NETDEVICE_PTR,
        mac_packet,
        payload_size + MAC_HEADER_SIZE
    );
    return EOK;
}

ret_t mac_flush(MAC_SENDER *self) {
    NETWORK_DRIVER_OPS->flush_send(NETDEVICE_PTR);
    return EOK;
}
