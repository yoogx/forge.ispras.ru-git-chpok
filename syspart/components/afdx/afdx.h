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
*                   Function that fill the frame
*
* The following file is a part of the AFDX project. Any modification should
* made according to the AFDX standard.
*
*
* Created by ....
*/

#ifndef __AFDX_H_
#define __AFDX_H_

#define SECOND 1000000000LL

#define MAX_AFDX_FRAME_SIZE     1518

/*
 * This define for filling function
 */
#define CARRY_ADD(Result, Value1, Value2) {             \
    uint32_t Sum;                                       \
                                                        \
    Sum = (uint32_t)(Value1) + (uint32_t)(Value2);      \
    (Result) = (Sum & 0xFFFF) + (Sum >> 16);            \
}

/*
 * This structure describes a buffer to store the queue
 */
typedef struct {
    char    data[MAX_AFDX_FRAME_SIZE];
    size_t  size;
    unsigned prepend_max_size;
    unsigned append_max_size;
} afdx_buffer;

/*
 * This structure  describes packeges types
 * The packet may be unicast or multicast
 */
typedef enum
{
   UNICAST_PACKET       = 0,
   MULTICAST_PACKET     = 1,
} PACKET_TYPE;

/*
 * This structure describes IP Unicast Addressing format
 * (source or destination)
 */
 struct ip_unicast_addr {
            uint8_t     ip_const_src;       // 8 bits constant (0000 1010 b)
            uint8_t     network_id;         // 4 bits constant (1000 b) & 4 bits Domain_ID
            uint8_t     equipment_id;       // 3 bits side_ID & 5 bits Location_ID
            uint8_t     partition_id;       // 3 bits (000 b) & 5 bits Partition_ID
        }__attribute__((packed));

/*
 * This structure describes IP Multicast Addressing format
 * (destination)
 */
 struct ip_multicast_addr {
            uint16_t    ip_const_dst;       // 16 bits constant (1110 0000 1110 0000 b)
            uint16_t    vl_id;
        }__attribute__((packed));



/*
 * This stucture describes AFDX packege
 */
struct frame_data {
    //int64_t preamble;                           // 7 bytes
    //uint8_t start_delimiter;                    // 1 byte

    struct mac_header {                           // 14 bytes
        //MAC Destination address begins          6 bytes
        struct mac_dst_addr {
            uint32_t    mac_const_dst;            // 32 bits (0000 0011 0000 0000 0000 0000 0000 0000 b)
            uint16_t    vl_id;                    // 16 bits
        } __attribute__((packed)) mac_dst_addr;

        //MAC source address begins             6 bytes
        struct mac_src_addr {
            uint8_t     mac_const_src[3];       // 24 bits (0000 0010 0000 0000 0000 0000 b)
            uint8_t     network_id;             // 4 bits (0000 b) + 4 bits Domain_ID
            uint8_t     equipment_id;           // 3 bits Side_ID + 5 bits Location_ID
            uint8_t     interface_id;           // 3 bits Interface_ID
                                                // (001 b) for the network A
                                                // (010 b) for the network B
                                                // + 5 bits (00000 b)
        } __attribute__((packed)) mac_src_addr;
        //MAC source address ends
        uint16_t    connection_type;            // 16 bits (0x0800) IPv4
    } __attribute__((packed)) mac_header;
    //IP header                                 size 20 bytes
    struct ip_header {
        uint8_t     version_and_ihl;            // 4 + 4 bits
        uint8_t     type_of_service;            // 8 bits
        uint16_t    total_length;               // 16 bits
        uint16_t    fragment_id;                // 16 bits
        uint16_t    flag_and_fragment_offset;   // 3 bits, bit 0: Reserved must be zero
                                                //         bit 1: Don't fragment (DF)
                                                //         bit 2: More fragments (MF)
                                                //  + 13 bits Fragment offset
        uint8_t     ttl;
        uint8_t     protocol;                   // 8 bits (0x11) UDP, according to AFDX,
                                                // ARINC 664 Tutorial, AFDX Protocol Stack Section
        uint16_t    header_checksum;

        //IP source address
        union u_src_addr {
            struct      ip_unicast_addr ip_src_addr;
            uint32_t    ip_general_src_addr;
        } __attribute__((packed)) u_src_addr;

        //IP destination address Unicast or Multicast
        union u_dst_addr {
            struct      ip_unicast_addr ip_unicast_dst_addr;
            struct      ip_multicast_addr ip_multicast_dst_addr;
            uint32_t    ip_general_dst_addr;
        } __attribute__((packed)) u_dst_addr;
    } __attribute__((packed)) ip_header;

    // UDP Header                               size 8 bytes
    struct udp_header {
        uint16_t    afdx_src_port;
        uint16_t    afdx_dst_port;
        uint16_t    udp_length;
        uint16_t    udp_checksum;
    }__attribute__((packed)) udp_header;

    char afdx_payload[];                        // shows the place in memory where payload begins (minimum size is 17 bytes)
                                                // payload can be from 1 byte to 17 bytes + padding (from 0 to 16)
                                                // the maximum size of payload is 1471 byte without sequence number and fcs
                                                // afdx_sn (sequence number) (the size is 1 byte) must be writed after payload
                                                // after that fcs (the size is 4 bytes) must be writed
    //struct IFG;

}__attribute__((packed));

typedef struct frame_data frame_data_t;

#endif
