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
#include "MAC_RECEIVER_gen.h"

#define MAC_HEADER_SIZE 14

ret_t mac_receive(MAC_RECEIVER *self, char *data, size_t len)
{
    // TODO validate checksums, TTL, and all that stuff

    if (len < sizeof(struct ether_hdr)) {
        printf("Received packet is too small (even Ethernet header doesn't fit).");
        return;
    }

    const struct ether_hdr *ether_hdr = (const struct ether_hdr *) data;
    data += sizeof(*ether_hdr);
    len -= sizeof(*ether_hdr);

    if (!ether_is_multicast(ether_hdr->dst) &&
        memcmp(ether_hdr->dst, NETDEVICE_PTR->mac, ETH_ALEN) != 0)
    {
        printf("packet NOT for us: dst= %x %x %x %x %x %x\n",
            ether_hdr->dst[0],
            ether_hdr->dst[1],
            ether_hdr->dst[2],
            ether_hdr->dst[3],
            ether_hdr->dst[4],
            ether_hdr->dst[5]);
        // it's not for us
        return;
    }

    if (ether_hdr->ethertype != hton16(ETH_P_IP)) {
        if (ether_hdr->ethertype == hton16(ETH_P_ARP)) {
            MAC_RECEIVER_call_portB_send(self, ether_hdr->payload, len);
        }
        // we don't know anything except IPv4
        return;
    }

    const struct ip_hdr *ip_hdr = (const struct ip_hdr *) data;

    if (len < sizeof(struct ip_hdr) || len < (size_t) (ip_hdr->version_len & 0xf) * 4) {
        printf("Received packet is too small (IP header doesn't fit).\n");
        return;
    }

    if (len != ntoh16(ip_hdr->length)) {
        if (len > ntoh16(ip_hdr->length)) {
            len = ntoh16(ip_hdr->length);
        } else {
            printf("Packet length mismatch (received buffer size vs. specified in IP header).\n");
            return;
        }
    }

    data += (ip_hdr->version_len & 0xf) * 4;
    len -= (ip_hdr->version_len & 0xf) * 4;

    if (ip_hdr_checksum(ip_hdr) != 0) {
        printf("Discarded IP packet with incorrect header checksum.\n");
        return;
    }
    
    // TODO network broadcast address?
    if (ip_hdr->dst != hton32(0xFFFFFFFFUL) &&
        ip_hdr->dst != hton32(pok_network_ip_address))
    {
        // it's not for us
        return;
    }

    if (ip_hdr->proto != IPPROTO_UDP) {
        // we don't know anything except UDP
        return;
    }

    if (len < sizeof(struct udp_hdr)) {
        printf("Received IP packet is too small (UDP header doesn't fit).\n");
        return;
    }

    const struct udp_hdr *udp_hdr = (const struct udp_hdr *) data;

    if (ntoh16(udp_hdr->length) != len) {
        printf("Packet length mismatch (received buffer size vs. specified in UDP header).\n");
        return;
    }
    
    data += sizeof(struct udp_hdr);
    len -= sizeof(struct udp_hdr);

    //received_callback(
    //        ntoh32(ip_hdr->dst),
    //        ntoh16(udp_hdr->dst_port),
    //        udp_hdr->payload, 
    //        len);
}
