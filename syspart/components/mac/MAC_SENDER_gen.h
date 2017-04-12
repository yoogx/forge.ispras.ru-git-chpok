/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/mac/config.yaml).
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

#ifndef __MAC_SENDER_GEN_H__
#define __MAC_SENDER_GEN_H__

#include <memblocks.h>

    #include <interfaces/ethernet_packet_sender_gen.h>

    #include <interfaces/preallocated_sender_gen.h>

typedef struct MAC_SENDER_state {
    uint8_t src_mac[6];
}MAC_SENDER_state;

typedef struct {
    char instance_name[16];
    MAC_SENDER_state state;
    struct {
            struct {
                ethernet_packet_sender ops;
            } portA;
    } in;
    struct {
            struct {
                preallocated_sender *ops;
                self_t *owner;
            } portB;
    } out;
} MAC_SENDER;



      ret_t mac_send(MAC_SENDER *, char *, size_t, size_t, size_t, uint8_t *, enum ethertype);
      ret_t mac_flush(MAC_SENDER *);

      ret_t MAC_SENDER_call_portB_send(MAC_SENDER *, char *, size_t, size_t, size_t);
      ret_t MAC_SENDER_call_portB_flush(MAC_SENDER *);






#endif
