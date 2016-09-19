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

#include "ARP_ANSWERER_gen.h"

struct arp_packet_t {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[ETH_ALEN];
    uint32_t spa;
    uint8_t tha[ETH_ALEN];
    uint32_t tpa;
} __attribute__((packed));

static struct {
    struct ether_hdr ether_hdr;
    struct arp_packet_t arp_answer;
} __attribute__((packed)) arp_answer_buffer;


void arp_received(void* data, size_t payload_len)
{
    struct arp_packet_t *arp_packet = (void *) data;

    if (arp_packet->htype != hton16(1)) {
        return; // We support only Ethernet hardware type.
    }
    if (arp_packet->ptype != hton16(ETH_P_IP)) {
        return; // We support only IPv4 protocol type.
    }
    if (arp_packet->hlen != ETH_ALEN || arp_packet->plen != 4) {
        return; // We support Ethernet MAC and IPv4 addresses only.
    }
    if (arp_packet->oper != hton16(1)) {
        return; // This is not an ARP request.
    }
    if (arp_packet->tpa != hton32(pok_network_ip_address)) {
        return; // This ARP request is not for us.
    }
    printf("ARP: we have received a request for our MAC.\n");

    int i;
    for (i = 0; i < ETH_ALEN; i++) {
        arp_answer_buffer.arp_answer.sha[i] = NETDEVICE_PTR->mac[i];
        arp_answer_buffer.arp_answer.tha[i] = arp_packet->sha[i];
    }

    arp_answer_buffer.arp_answer.htype = arp_packet->htype;
    arp_answer_buffer.arp_answer.ptype = arp_packet->ptype;
    arp_answer_buffer.arp_answer.hlen = arp_packet->hlen;
    arp_answer_buffer.arp_answer.plen = arp_packet->plen;
    arp_answer_buffer.arp_answer.oper = hton16(2); // This is an ARP answer.
    arp_answer_buffer.arp_answer.spa = hton32(pok_network_ip_address);
    arp_answer_buffer.arp_answer.tpa = arp_packet->spa;

    mac_send((char *) &arp_answer_buffer.arp_answer,
                sizeof(arp_answer_buffer.arp_answer),
                sizeof(struct ether_hdr),
                arp_answer_buffer.arp_answer.tha,
                ETH_P_ARP);

    //if (!sent) {
    //    printf("ARP: unable to send an answer.\n");
    //}
    //change_to mac_flush
    mac_flush();
}
