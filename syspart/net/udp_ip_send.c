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

#define UDP_IP_HEADER_SIZE (20+8)
struct udp_ip_packet{
    struct ip_hdr ip_hdr;
    struct udp_hdr udp_hdr;
    char payload[];
} __attribute__((packed));

static void fill_in_udp_ip_header(
        struct udp_ip_packet *packet,
        size_t size, // size of UDP data
        uint32_t dst_ip,
        uint16_t dst_port)
{
    // ...next, IP heaader
    packet->ip_hdr.version_len = (4 << 4) | 5;
    packet->ip_hdr.dscp = 0;
    packet->ip_hdr.length = hton16(
            sizeof(struct ip_hdr) +
            sizeof(struct udp_hdr) +
            size
    );
    packet->ip_hdr.checksum = 0; // it's filled in just below
    packet->ip_hdr.id = 0;
    packet->ip_hdr.offset = 0;
    packet->ip_hdr.ttl = 32;
    packet->ip_hdr.proto = IPPROTO_UDP;
    packet->ip_hdr.src = hton32(pok_network_ip_address);
    packet->ip_hdr.dst = hton32(dst_ip);

    packet->ip_hdr.checksum = ip_hdr_checksum(&packet->ip_hdr);

    // ... and UDP header
    packet->udp_hdr.src_port = hton16(dst_port);
    packet->udp_hdr.dst_port = hton16(dst_port);
    packet->udp_hdr.length = hton16(size + sizeof(struct udp_hdr));
    packet->udp_hdr.checksum = 0; // no checksum
}

pok_bool_t udp_ip_send(
        char *payload,
        size_t payload_size,
        size_t max_backstep,
        void *driver_data
    )
{
    if (max_backstep < UDP_IP_HEADER_SIZE)
        return 0;

    void *udp_packet = payload - UDP_IP_HEADER_SIZE;
    udp_data_t *udp_data = driver_data;
    fill_in_udp_ip_header(
        udp_packet,
        payload_size,
        udp_data->ip,
        udp_data->port
    );

    uint8_t *dst_mac = find_mac_by_ip(udp_data->ip);

    mac_send(udp_packet,
            payload_size + UDP_IP_HEADER_SIZE,
            max_backstep - UDP_IP_HEADER_SIZE,
            dst_mac,
            ETH_P_IP);

    return 1;
}

void udp_ip_flush(void) {
    mac_flush();
}
