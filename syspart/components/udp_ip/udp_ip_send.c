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

#include "UDP_IP_SENDER_gen.h"

#define UDP_IP_HEADER_SIZE (20+8)
struct udp_ip_packet{
    struct ip_hdr ip_hdr;
    struct udp_hdr udp_hdr;
    char payload[];
} __attribute__((packed));

static void fill_in_udp_ip_header(
        UDP_IP_SENDER_state *state,
        struct udp_ip_packet *packet,
        size_t payload_size
        )
{
    // ...next, IP heaader
    packet->ip_hdr.version_len = (4 << 4) | 5;
    packet->ip_hdr.dscp = 0;
    packet->ip_hdr.length = hton16(
            sizeof(struct ip_hdr) +
            sizeof(struct udp_hdr) +
            payload_size
    );
    packet->ip_hdr.checksum = 0; // it's filled in just below
    packet->ip_hdr.id = 0;
    packet->ip_hdr.offset = 0;
    packet->ip_hdr.ttl = 32;
    packet->ip_hdr.proto = IPPROTO_UDP;
    packet->ip_hdr.src = hton32(state->src_ip);
    packet->ip_hdr.dst = hton32(state->dst_ip);

    packet->ip_hdr.checksum = ip_hdr_checksum(&packet->ip_hdr);

    // ... and UDP header
    packet->udp_hdr.src_port = hton16(state->src_port);
    packet->udp_hdr.dst_port = hton16(state->dst_port);
    packet->udp_hdr.length = hton16(payload_size + sizeof(struct udp_hdr));
    packet->udp_hdr.checksum = 0; // no checksum
}

ret_t udp_ip_send(
        UDP_IP_SENDER *self,
        char *payload,
        size_t payload_size,
        size_t prepend_max_size,
        size_t append_max_size
        )
{
    if (prepend_max_size < UDP_IP_HEADER_SIZE)
        return EINVAL;

    void *udp_packet = payload - UDP_IP_HEADER_SIZE;
    fill_in_udp_ip_header(
        &self->state,
        udp_packet,
        payload_size
    );

    return UDP_IP_SENDER_call_portB_mac_send(self,
            udp_packet,
            payload_size + UDP_IP_HEADER_SIZE,
            prepend_max_size - UDP_IP_HEADER_SIZE,
            append_max_size,
            self->state.dst_mac,
            ETH_P_IP);
}

ret_t udp_ip_flush(UDP_IP_SENDER *self) {
    return UDP_IP_SENDER_call_portB_flush(self);
}
