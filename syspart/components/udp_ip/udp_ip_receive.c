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
#include <net/ip.h>
#include <net/udp.h>
#include <stdio.h>

#include "UDP_RECEIVER_FILTER_gen.h"
#include "udp_ip_pair.h"

#define C_NAME "UDP_RECEIVER_FILTER: "

#define IP_PRINT(X) X>>24&0xff,X>>16&0xff,X>>8&0xff, X&0xff

static int find_ip_port_in_state(UDP_RECEIVER_FILTER_state *state, uint32_t ip, uint16_t udp_port)
{
    for (int i = 0; i < state->good_ip_port_len; i++) {
        struct udp_ip_pair *current_pair = &state->good_ip_port[i];
        if (ip == current_pair->ip && udp_port == current_pair->port)
            return 1;
    }
    return 0;
}


ret_t udp_receive_and_filter(UDP_RECEIVER_FILTER *self, char *data, size_t len)
{
    const struct ip_hdr *ip_hdr = (const struct ip_hdr *) data;

    if (len < sizeof(struct ip_hdr) || len < (size_t) (ip_hdr->version_len & 0xf) * 4) {
        printf(C_NAME"Received packet is too small (IP header doesn't fit).\n");
        return EINVAL;
    }

    if (len != ntoh16(ip_hdr->length)) {
        if (len > ntoh16(ip_hdr->length)) {
            len = ntoh16(ip_hdr->length);
        } else {
            printf(C_NAME"Packet length mismatch (received buffer size vs. specified in IP header).\n");
            return EINVAL;
        }
    }

    data += (ip_hdr->version_len & 0xf) * 4;
    len -= (ip_hdr->version_len & 0xf) * 4;

    if (ip_hdr_checksum(ip_hdr) != 0) {
        printf(C_NAME"Discarded IP packet with incorrect header checksum.\n");
        return EINVAL;
    }


    if (ip_hdr->proto != IPPROTO_UDP) {
        printf(C_NAME"it is not UDP\n");
        return EINVAL;
    }

    if (len < sizeof(struct udp_hdr)) {
        printf(C_NAME"Received IP packet is too small (UDP header doesn't fit).\n");
        return EINVAL;
    }

    const struct udp_hdr *udp_hdr = (const struct udp_hdr *) data;

    if (ntoh16(udp_hdr->length) != len) {
        printf(C_NAME"Packet length mismatch (received buffer size vs. specified in UDP header).\n");
        return EINVAL;
    }

    if (find_ip_port_in_state(&self->state, ntoh32(ip_hdr->dst), ntoh16(udp_hdr->dst_port))) {
        printf(C_NAME"GOOD UDP PACKET\n");
        hexdump(udp_hdr->payload, len-sizeof(struct udp_hdr));
        return EOK;
    } else {
        printf(C_NAME"packet not for us %ld.%ld.%ld.%ld:%d\n", IP_PRINT(ntoh32(ip_hdr->dst)), ntoh16(udp_hdr->dst_port));
        return EINVAL;
    }


    //received_callback(
    //        ntoh32(ip_hdr->dst),
    //        ntoh16(udp_hdr->dst_port),
    //        udp_hdr->payload,
    //        len);
}
