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
*
*=======================================================================
*
*                  AFDX Function that fill the frame 
*
* The following file is a part of the AFDX project. Any modification
* should made according to the AFDX standard.
*
*
* Created by ....
*/

 
#include <stdint.h>
#include <afdx/AFDX_ES_config.h>
#include "afdx.h"
#include <net/byteorder.h>
#include <string.h>
#include <stdio.h>

// include generated files
#include "AFDX_FILLER_gen.h"

#define C_NAME "AFDX_FILLER: "

#define MAX(A, B) (((A) > (B)) ? (A) : (B))

#define MAC_CONST_DST       0x3000000
#define MAC_CONST_SRC_1     0x2
#define MAC_CONST_SRC_2     0
#define MAC_CONST_SRC_3     0
#define MAC_NETWORK_ID      0
#define MAC_INTERFACE_ID_A  0x1    // (001 b) for the network A
#define MAC_INTERFACE_ID_B  0x2    // (010 b) for the network B
#define MAC_INTERFACE_ID_2  0

#define CONNECTION_TYPE     0x0800
#define IP_CONST_SRC        0xA
#define IP_NETWORK_ID       0x8
#define IP_PARTITION_ID_1   0x0
#define IP_CONST_DST        0xE0E0
#define IP_VERSION          0x4
#define TYPE_OF_SERVICE     0x00    //in most IP protocols 0

#define IPPROTO_UDP         0x11    // UDP
#define FRAGMENT_ID         0
#define DO_NOT_FRAGMENT     0x2
#define FRAGMENT_OFFSET     0
#define IHL                 0x5     // 5 words by 32 bits = 20 bytes IP Header

#define HEADER_LENGTH       42
#define UDP_H_LENGTH        8
#define IP_H_LENGTH         (IHL * 4) + UDP_H_LENGTH
#define FCS_LENGTH          4
#define SUFFIX_LENGTH       1

#define MIN_AFDX_PAYLOAD_SIZE    17

/* basic functions for filling the package */

/*
 * UDP (User Datagram Protocol) packet type.
 * filling depends on bytes order
 * param x: Array of three elements
*/
void set_mac_addr_const_part(uint8_t x[3])
{
    x[0] = MAC_CONST_SRC_1;
    x[1] = MAC_CONST_SRC_2;
    x[2] = MAC_CONST_SRC_3;
}

/* Calculate the UDP checksum (calculated with the whole packet).
 * param buff: The UDP packet.
 * param len: The UDP packet length. (header + payload)
 * param src_addr: The IP source address (in network format).
 * param dest_addr: The IP destination address (in network format).
 *
 * return: The result of the checksum.
 */
uint16_t udp_checksum(void *buff, uint16_t len, uint32_t src_addr, uint32_t dest_addr)
{

    uint16_t *buf;
    buf = (uint16_t *) buff;
    uint16_t *ip_src = (uint16_t *)&src_addr;
    uint16_t *ip_dst = (uint16_t *)&dest_addr;
    uint32_t sum;
    uint16_t length = ntoh16(len);

    // Calculate the sum
    sum = 0;
    //~ while (length > 1)
    for (sum = 0; length > 1; length -= 2) {
        sum += *(buf++);
    }

    if ( length > 0 ) {
#ifdef BIG_ENDIAN
         /* Add the last byte as the high byte. */
        sum += ((uint8_t) *(uint8_t *)buf) << 8;
#else
         /* Add the last byte as the low byte. */
        sum += *(uint8_t *)buf;
#endif
    }
    // Add the pseudo-header
    sum += *(ip_src++);
    sum += *(ip_src);
    sum += *(ip_dst++);
    sum += *(ip_dst);
    sum += ntoh16(IPPROTO_UDP);
    sum += len;

    // Add the carries
    CARRY_ADD(sum, sum, 0);
    CARRY_ADD(sum, sum, 0);

    // Return the one's complement of sum
    return (uint16_t)(~sum);
}

/*
 * brief Calculate the IP header checksum.
 * param: pointer to IP header.
 *
 * return: The result of the checksum.
 */
uint16_t ip_hdr_checksum_2(const struct ip_header *ip_hdr)
{
    uint32_t acc = 0;

    // TODO does it break aliasing?
    const uint16_t *words = (const uint16_t*) ip_hdr;

    int i;
    for (i = 0; i < (ip_hdr->version_and_ihl & 0xf) * 2; i++) {
        acc += words[i];
    }

    return ~((acc & 0xFFFF) + (acc >> 16));
}

/*
 * This function fill the afdx frame without information of interface id.
 * The information of interface id will fill another function, which will send
 * packets to the network cards
 */
uint16_t fill_afdx_frame(AFDX_FILLER_state *state,
                         frame_data_t *p,
                         char afdx_payload[],
                         uint16_t payload_size,
                         uint8_t seq_num
                         )
{
//MAC
    p->mac_header.mac_dst_addr.mac_const_dst = hton32(MAC_CONST_DST);
    p->mac_header.mac_dst_addr.vl_id = hton16(state->vl_id);
    set_mac_addr_const_part(p->mac_header.mac_src_addr.mac_const_src);

    p->mac_header.mac_src_addr.network_id = (MAC_NETWORK_ID << 4 | DOMAIN_ID);
    p->mac_header.mac_src_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
    p->mac_header.mac_src_addr.interface_id = 0;
    p->mac_header.connection_type = hton16(CONNECTION_TYPE);
//IP
    p->ip_header.version_and_ihl = (IP_VERSION << 4 | IHL);
    p->ip_header.type_of_service = TYPE_OF_SERVICE;
    p->ip_header.total_length = hton16(payload_size + IP_H_LENGTH);    //total length = (payload size + IP header size)
    p->ip_header.fragment_id = hton16(FRAGMENT_ID);
    p->ip_header.flag_and_fragment_offset = hton16(DO_NOT_FRAGMENT << 13 | FRAGMENT_OFFSET);    // flag = 0x2 -DO not fragment
                                                                                                //fragment_offset = 0
    p->ip_header.ttl = state->ttl;
    p->ip_header.protocol = IPPROTO_UDP;

    p->ip_header.header_checksum = 0;
    //src
    p->ip_header.u_src_addr.ip_src_addr.ip_const_src = IP_CONST_SRC;
    p->ip_header.u_src_addr.ip_src_addr.network_id = (IP_NETWORK_ID << 4 | DOMAIN_ID);
    p->ip_header.u_src_addr.ip_src_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
    //
    p->ip_header.u_src_addr.ip_src_addr.partition_id = (IP_PARTITION_ID_1 << 3 | state->src_partition_id);
    //dst !
    if (state->type_of_packet == UNICAST_PACKET) {
        p->ip_header.u_dst_addr.ip_unicast_dst_addr.ip_const_src = IP_CONST_SRC;
        p->ip_header.u_dst_addr.ip_unicast_dst_addr.network_id = (IP_NETWORK_ID << 4 | DOMAIN_ID);
        p->ip_header.u_dst_addr.ip_unicast_dst_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
        //
        p->ip_header.u_dst_addr.ip_unicast_dst_addr.partition_id = (IP_PARTITION_ID_1 << 3 | state->dst_partition_id);
    } else {
        p->ip_header.u_dst_addr.ip_multicast_dst_addr.ip_const_dst = hton16(IP_CONST_DST);
        p->ip_header.u_dst_addr.ip_multicast_dst_addr.vl_id =  hton16(state->vl_id);
    }
    p->ip_header.header_checksum = ip_hdr_checksum_2(&p->ip_header);
//UDP
    p->udp_header.afdx_src_port = hton16(state->src_afdx_port);
    p->udp_header.afdx_dst_port = hton16(state->dst_afdx_port);
    p->udp_header.udp_length = hton16(payload_size + UDP_H_LENGTH); //udp_length = (payload size + udp header size)
    p->udp_header.udp_checksum = 0;

//PAYLOAD
    memcpy(p->afdx_payload, afdx_payload, payload_size);

    // add pad if it's necessary and SN after that
    uint8_t pad_size = 0;
    if (payload_size < MIN_AFDX_PAYLOAD_SIZE) {
        pad_size = MIN_AFDX_PAYLOAD_SIZE - payload_size;
        memset((p->afdx_payload + payload_size), 0, pad_size);

        p->afdx_payload[MIN_AFDX_PAYLOAD_SIZE] = seq_num;
    } else {
        p->afdx_payload[payload_size] = seq_num;
    }

//UDP Checksum
    p->udp_header.udp_checksum = udp_checksum(&p->udp_header,
                                             hton16(payload_size + UDP_H_LENGTH),
                                             (p->ip_header.u_src_addr.ip_general_src_addr),
                                             (p->ip_header.u_dst_addr.ip_general_dst_addr)
                                             );
    if (pad_size == 0)
        return (payload_size + HEADER_LENGTH + SUFFIX_LENGTH);
    else
        return (HEADER_LENGTH + payload_size + pad_size + SUFFIX_LENGTH);

}

void afdx_filler_init(AFDX_FILLER *self)
{
    self->state.sn = 0;
}

ret_t afdx_filler_send(
        AFDX_FILLER *self,
        char *payload,
        const size_t payload_size,
        const size_t prepend_max_size,
        const size_t append_max_size
        )
{
    if (prepend_max_size < HEADER_LENGTH)
        return EINVAL;

    if (append_max_size < 1)
        return EINVAL;

    frame_data_t *afdx_frame = (frame_data_t *)(payload - HEADER_LENGTH);
    printf(C_NAME"SN= %u\n", self->state.sn);
    fill_afdx_frame(&self->state, afdx_frame, payload, payload_size, self->state.sn);
    self->state.sn = (self->state.sn % 255) + 1;
    ret_t res = AFDX_FILLER_call_portB_send(self,
                    (char *) afdx_frame,
                    HEADER_LENGTH + SUFFIX_LENGTH + MAX(payload_size, MIN_AFDX_PAYLOAD_SIZE),
                    prepend_max_size - HEADER_LENGTH,
                    append_max_size - SUFFIX_LENGTH
                    );

    if (res != EOK)
        printf(C_NAME"Error in send\n");

    return res;
}

ret_t afdx_filler_flush(AFDX_FILLER *self) {

    return AFDX_FILLER_call_portB_flush(self);
}
