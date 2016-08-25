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


static void fill_in_udp_ip_header(
        char *buffer,
        size_t size, // size of UDP data
        uint32_t dst_ip,
        uint16_t dst_port)
{
    struct {
        struct ip_hdr ip_hdr;
        struct udp_hdr udp_hdr;
        char data[];
    } __attribute__((packed)) *real_buffer = (void*) (buffer + sizeof(struct ether_hdr));

    // ...next, IP heaader
    real_buffer->ip_hdr.version_len = (4 << 4) | 5;
    real_buffer->ip_hdr.dscp = 0;
    real_buffer->ip_hdr.length = hton16(
            sizeof(struct ip_hdr) +
            sizeof(struct udp_hdr) +
            size
    );
    real_buffer->ip_hdr.checksum = 0; // it's filled in just below
    real_buffer->ip_hdr.id = 0;
    real_buffer->ip_hdr.offset = 0;
    real_buffer->ip_hdr.ttl = 32;
    real_buffer->ip_hdr.proto = IPPROTO_UDP;
    real_buffer->ip_hdr.src = hton32(pok_network_ip_address);
    real_buffer->ip_hdr.dst = hton32(dst_ip);

    real_buffer->ip_hdr.checksum = ip_hdr_checksum(&real_buffer->ip_hdr);

    // ... and UDP header
    real_buffer->udp_hdr.src_port = hton16(dst_port);
    real_buffer->udp_hdr.dst_port = hton16(dst_port);
    real_buffer->udp_hdr.length = hton16(size + sizeof(struct udp_hdr));
    real_buffer->udp_hdr.checksum = 0; // no checksum
}

pok_bool_t udp_ip_send(
        char *buffer,
        size_t payload_size,
        void *driver_data
    )
{
    udp_data_t *udp_data = driver_data;
    fill_in_udp_ip_header(
        buffer,
        payload_size,
        udp_data->ip,
        udp_data->port
    );
    mac_send(buffer, payload_size, driver_data);
}

void udp_ip_flush(void) {
    mac_flush();
}
