/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
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

#ifndef __UDP_IP_SENDER_GEN_H__
#define __UDP_IP_SENDER_GEN_H__

#include <memblocks.h>
    #include "ip_addr.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/ethernet_packet_sender_gen.h>

typedef struct UDP_IP_SENDER_state {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t dst_mac[6];
}UDP_IP_SENDER_state;

typedef struct {
    char instance_name[16];
    UDP_IP_SENDER_state state;
    struct {
            struct {
                preallocated_sender ops;
            } portA;
    } in;
    struct {
            struct {
                ethernet_packet_sender *ops;
                self_t *owner;
            } portB;
    } out;
} UDP_IP_SENDER;



      ret_t udp_ip_send(UDP_IP_SENDER *, char *, size_t, size_t);
      ret_t udp_ip_flush(UDP_IP_SENDER *);

      ret_t UDP_IP_SENDER_call_portB_mac_send(UDP_IP_SENDER *, char *, size_t, size_t, uint8_t *, enum ethertype);
      ret_t UDP_IP_SENDER_call_portB_flush(UDP_IP_SENDER *);






#endif
