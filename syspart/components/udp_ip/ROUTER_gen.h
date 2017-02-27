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

#ifndef __ROUTER_GEN_H__
#define __ROUTER_GEN_H__

#include <memblocks.h>
    #include "state_structs.h"
    #include "ip_addr.h"

    #include <interfaces/udp_message_handler_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct ROUTER_state {
    size_t map_ip_port_to_idx_len;
    struct udp_ip_pair map_ip_port_to_idx[10];
}ROUTER_state;

typedef struct {
    char instance_name[16];
    ROUTER_state state;
    struct {
            struct {
                udp_message_handler ops;
            } portA;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } *portArray;
    } out;
} ROUTER;



      ret_t receive_packet(ROUTER *, const char *, size_t, uint32_t, uint16_t);

      ret_t ROUTER_call_portArray_handle_by_index(int, ROUTER *, const char *, size_t);






#endif
