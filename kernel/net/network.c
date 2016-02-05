/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#ifdef POK_NEEDS_NETWORKING

#include <assert.h>
#include <libc.h>
#include <arch.h>

#include <net/network.h>
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>

static pok_bool_t initialized = FALSE;

#ifdef POK_NEEDS_NETWORKING_VIRTIO
    #include <drivers/virtio/virtio_network.h>
    #define NETWORK_DRIVER pok_network_virtio_device
    #define NETWORK_DRIVER_OPS (pok_network_virtio_device.ops)
#endif

static pok_network_udp_receive_callback_t *receive_callback_list = NULL;

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

static int ether_is_broadcast(const uint8_t addr[ETH_ALEN]) {
    for (int i = 0; i < ETH_ALEN; ++i) {
        if (addr[i] != 0xFF) {
            return 0;
        }
    }

    return 1;
}

static void try_arp(const struct ether_hdr *ether_hdr, size_t payload_len) {
    if (!ether_is_broadcast(ether_hdr->dst)) {
        return; // This is not an ARP request.
    }

    if (ether_hdr->ethertype != hton16(ETH_P_ARP)) {
        return; // This is not an ARP request.
    }
}

// ---- ARP support -  end  ----

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
        memcmp(ether_hdr->dst, NETWORK_DRIVER.mac, ETH_ALEN) != 0)
    {
        // it's not for us
        return;
    }

    if (ether_hdr->ethertype != hton16(ETH_P_IP)) {
        try_arp(ether_hdr, len); // Manage if it is an ARP packet

        // we don't know anything except IPv4
        return;
    }

    const struct ip_hdr *ip_hdr = (const struct ip_hdr *) data;

    if (len < sizeof(struct ip_hdr) || len < (size_t) (ip_hdr->version_len & 0xf) * 4) {
        printf("Received packet is too small (IP header doesn't fit).\n");
        return;
    }

    if (len != ntoh16(ip_hdr->length)) {
        printf("Packet length mismatch (received buffer size vs. specified in IP header).\n");
        return;
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

    pok_network_udp_receive_callback_t *cb = receive_callback_list;
    while (cb != NULL) {
        if (cb->callback(ntoh32(ip_hdr->dst),
                         ntoh16(udp_hdr->dst_port), 
                         udp_hdr->payload, 
                         len))
        {
            break;
        }
        cb = cb->next;
    }
}

void pok_network_init(void)
{
    if (NETWORK_DRIVER_OPS->init()) {
        NETWORK_DRIVER_OPS->set_packet_received_callback(packet_received_callback);
        initialized = TRUE;
    }
}

static void fill_in_udp_header(
        char *buffer, 
        size_t size, // size of UDP data
        uint32_t dst_ip, 
        uint16_t dst_port)
{
    struct {
        struct ether_hdr ether_hdr;
        struct ip_hdr ip_hdr;
        struct udp_hdr udp_hdr;
        char data[];
    } __attribute__((packed)) *real_buffer = (void*) buffer;

    // fill in Ethernet header
    int i;
    for (i = 0; i < ETH_ALEN; i++) {
        real_buffer->ether_hdr.src[i] = NETWORK_DRIVER.mac[i];
        real_buffer->ether_hdr.dst[i] = 0xFF;
    }
    real_buffer->ether_hdr.ethertype = hton16(ETH_P_IP);

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

pok_bool_t pok_network_send_udp(
    char *buffer,
    size_t size,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg)
{
    if (!initialized) return FALSE;

    fill_in_udp_header(
        buffer + POK_NETWORK_OVERHEAD_DRIVER,
        size,
        dst_ip,
        dst_port
    );

    return NETWORK_DRIVER_OPS->send_frame(
        buffer,
        size + POK_NETWORK_OVERHEAD,
        callback,
        callback_arg
    );
}

pok_bool_t pok_network_send_udp_gather(
    const pok_network_sg_list_t *sg_list,
    size_t sg_list_len,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg)
{
    if (!initialized) return FALSE;

    assert(sg_list_len >= 1);
    assert(sg_list[0].size == POK_NETWORK_OVERHEAD);

    size_t payload_length = 0;
    size_t i;
    for (i = 1; i < sg_list_len; i++) {
        payload_length += sg_list[i].size;
    }

    fill_in_udp_header(
        sg_list[0].buffer + POK_NETWORK_OVERHEAD_DRIVER, 
        payload_length, 
        dst_ip, 
        dst_port
    );

    return NETWORK_DRIVER_OPS->send_frame_gather(
        sg_list,
        sg_list_len,
        callback,
        callback_arg
    );
}

void pok_network_register_udp_receive_callback(
    pok_network_udp_receive_callback_t *cb)
{
    cb->next = receive_callback_list;
    receive_callback_list = cb;
}

void pok_network_reclaim_send_buffers(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_send_buffers();
    }
}

void pok_network_reclaim_receive_buffers(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_receive_buffers();
    }
}

void pok_network_flush_send(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->flush_send();
    }
}

void pok_network_thread(void)
{
    pok_arch_preempt_enable();
    for (;;) {
        if (initialized) {
            pok_network_reclaim_send_buffers();
            pok_network_reclaim_receive_buffers();
        }

        /*
         * Conserve electricity (and CPU time).
         * It's actually very important in QEMU, 
         * where virtual machine competes for resources with host 
         * and even other guests.
         *
         * TODO Enable network card interrupt here, so we'd wake up
         *      as soon as there's work to be done.
         *      (Currently, we wake up on timer interrupt)
         */
        #ifdef i386
        asm("hlt");
        #endif
    }
}

#endif
