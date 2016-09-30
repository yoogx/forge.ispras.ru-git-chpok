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

#include <net/byteorder.h>
#include <net/ip.h>
#include <net/udp.h>
#include <stdio.h>

#include "ROUTER_gen.h"

#define C_NAME "ROUTER: "

/* Return index of (ip, port) pair in array in state. -1 if not found */
static int get_ip_port_index(ROUTER_state *state, uint32_t ip, uint16_t port)
{
    for (int i=0; i<state->map_ip_port_to_idx_len; i++) {
        struct udp_ip_pair *cur_pair = &state->map_ip_port_to_idx[i];
        if (cur_pair->ip == ip && cur_pair->port == port)
            return i;
    }
    return -1;
}

ret_t receive_packet(ROUTER *self, char *payload, size_t payload_size, uint32_t ip, uint16_t port)
{
    int idx = get_ip_port_index(&self->state, ip, port);

    if (idx < 0) {
        printf(C_NAME"packet not for us (from %ld.%ld.%ld.%ld:%d)\n", IP_PRINT(ip), port);
        return EINVAL;
    }
    ROUTER_call_portArray_send_by_index(idx, self, payload, payload_size);

    return EOK;
}
