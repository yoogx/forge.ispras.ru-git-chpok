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

#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>
#include <pci.h>

#include <stdio.h>
#include <string.h>

#include "MAC_RECEIVER_gen.h"

#define MAC_HEADER_SIZE 14

#define C_NAME "MAC_RECEIVER: "
ret_t mac_receive(MAC_RECEIVER *self, char *data, size_t len)
{
    // TODO validate checksums, TTL, and all that stuff

    if (len < sizeof(struct ether_hdr)) {
        printf(C_NAME"Received packet is too small (even Ethernet header doesn't fit).");
        return EINVAL;
    }

    struct ether_hdr *ether_hdr = (struct ether_hdr *) data;
    data += sizeof(*ether_hdr);
    len -= sizeof(*ether_hdr);

    if (!ether_is_multicast(ether_hdr->dst) &&
        memcmp(ether_hdr->dst, self->state.my_mac, ETH_ALEN) != 0)
    {
        printf(C_NAME"packet NOT for us: dst= %x %x %x %x %x %x\n",
            ether_hdr->dst[0],
            ether_hdr->dst[1],
            ether_hdr->dst[2],
            ether_hdr->dst[3],
            ether_hdr->dst[4],
            ether_hdr->dst[5]);
        // it's not for us
        return EINVAL;
    }

    if (ether_hdr->ethertype == hton16(ETH_P_ARP)) {
        return MAC_RECEIVER_call_port_ARP_send(self, ether_hdr->payload, len);
    } else if(ether_hdr->ethertype == hton16(ETH_P_IP)){
        return MAC_RECEIVER_call_port_UDP_send(self, ether_hdr->payload, len);
    } else {
        // we don't know anything except IPv4
        return EINVAL;
    }



}
