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


static pok_bool_t udp_ip_send(
        char *buffer,
        size_t buffer_size,
        void *driver_data
    );

pok_bool_t initialized = FALSE;

pok_netdevice_t *current_netdevice;
#define NETDEVICE_PTR current_netdevice
#define NETWORK_DRIVER_OPS (NETDEVICE_PTR->ops)

static pok_bool_t (*received_callback)(
        uint32_t ip,
        uint16_t port,
        const char *payload,
        size_t length);


static void flush_send();

#define POK_NEEDS_ARP_ANSWER

#ifdef POK_NEEDS_ARP_ANSWER
// ---- ARP support - begin ----

#define ETH_P_ARP 0x0806

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

//static pok_bool_t ether_is_broadcast(const uint8_t addr[ETH_ALEN]) {
//    for (int i = 0; i < ETH_ALEN; ++i) {
//        if (addr[i] != 0xFF) {
//            return 0;
//        }
//    }
//    return 1;
//}


static struct {
    struct ether_hdr ether_hdr;
    struct arp_packet_t arp_answer;
} __attribute__((packed)) arp_answer_buffer;

static void try_arp(const struct ether_hdr *ether_hdr, size_t payload_len) {

    //TODO check that dst is either broadcast or our mac address
    //if (!ether_is_broadcast(ether_hdr->dst)) {
    //    return; // This is not an ARP request.
    //}

    if (ether_hdr->ethertype != hton16(ETH_P_ARP)) {
        return; // This is not an ARP request.
    }
    struct arp_packet_t *arp_packet = (void *) ether_hdr->payload;
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
        arp_answer_buffer.arp_answer.sha[i] =
            arp_answer_buffer.ether_hdr.src[i] = NETDEVICE_PTR->mac[i];
        arp_answer_buffer.arp_answer.tha[i] =
            arp_answer_buffer.ether_hdr.dst[i] = arp_packet->sha[i];
    }
    arp_answer_buffer.ether_hdr.ethertype = hton16(ETH_P_ARP);
    arp_answer_buffer.arp_answer.htype = arp_packet->htype;
    arp_answer_buffer.arp_answer.ptype = arp_packet->ptype;
    arp_answer_buffer.arp_answer.hlen = arp_packet->hlen;
    arp_answer_buffer.arp_answer.plen = arp_packet->plen;
    arp_answer_buffer.arp_answer.oper = hton16(2); // This is an ARP answer.
    arp_answer_buffer.arp_answer.spa = hton32(pok_network_ip_address);
    arp_answer_buffer.arp_answer.tpa = arp_packet->spa;
    pok_bool_t sent =
        NETWORK_DRIVER_OPS->send_frame(
                NETDEVICE_PTR,
                (char *) &arp_answer_buffer,
                sizeof(arp_answer_buffer)
                );
    if (!sent) {
        printf("ARP: unable to send an answer.\n");
    }
    flush_send();
}
// ---- ARP support -  end  ----
#endif // POK_NEEDS_ARP_ANSWER

static void packet_received_callback(const char *data, size_t len)
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
        // it's not for us
        return;
    }

    if (ether_hdr->ethertype != hton16(ETH_P_IP)) {

#ifdef POK_NEEDS_ARP_ANSWER
        try_arp(ether_hdr, len); // Manage if it is an ARP packet
#endif // POK_NEEDS_ARP_ANSWER

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

    received_callback(
            ntoh32(ip_hdr->dst),
            ntoh16(udp_hdr->dst_port),
            udp_hdr->payload, 
            len);
}

uint8_t default_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

extern struct mac_ip mac_addr_mapping[];
uint8_t* find_mac_by_ip(uint32_t dst_ip)
{
    for (int i=0; i < mac_addr_mapping_nb; i++) {
        if (mac_addr_mapping[i].ip == dst_ip)
            return mac_addr_mapping[i].mac;
    }
    return default_mac;
}

void pok_network_init(void)
{
    current_netdevice = get_netdevice(ipnet_netdev_name);

    if (current_netdevice == NULL) {
        printf("ipnet ERROR: netdevice '%s' not found\n", ipnet_netdev_name);
        return;
    }

    NETWORK_DRIVER_OPS->set_packet_received_callback(NETDEVICE_PTR, packet_received_callback);
    initialized = TRUE;
    printf("INITED\n");
}


static void receive(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_receive_buffers(NETDEVICE_PTR);
    }
}

static void flush_send(void) {
    if (initialized) {
        NETWORK_DRIVER_OPS->flush_send(NETDEVICE_PTR);
    }
}

void register_received_callback(
            pok_bool_t (*callback)(
                uint32_t ip,
                uint16_t port,
                const char *payload,
                size_t length
                )
            )
{
    received_callback = callback;
}

struct channel_driver ipnet_channel_driver = {
    .send = udp_ip_send,
    .receive = receive,
    .flush_send = flush_send,
    .register_received_callback = register_received_callback
};

