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

#ifndef __UDP_RECEIVER_GEN_H__
#define __UDP_RECEIVER_GEN_H__

#include <memblocks.h>
    #include "state_structs.h"
    #include "ip_addr.h"

    #include <interfaces/message_handler_gen.h>

    #include <interfaces/udp_message_handler_gen.h>

typedef struct UDP_RECEIVER_state {
}UDP_RECEIVER_state;

typedef struct {
    char instance_name[16];
    UDP_RECEIVER_state state;
    struct {
            struct {
                message_handler ops;
            } portA;
    } in;
    struct {
            struct {
                udp_message_handler *ops;
                self_t *owner;
            } portB;
    } out;
} UDP_RECEIVER;



      ret_t udp_receive(UDP_RECEIVER *, const char *, size_t);

      ret_t UDP_RECEIVER_call_portB_udp_message_handle(UDP_RECEIVER *, const char *, size_t, uint32_t, uint16_t);






#endif
