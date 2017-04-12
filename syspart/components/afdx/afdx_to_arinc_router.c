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

#include <stdio.h>
#include "AFDX_TO_ARINC_ROUTER_gen.h"
#include "afdx.h"

#define C_NAME "AFDX_TO_ARINC_ROUTER: "


#define HEADER_LENGTH       42
#define SUFFIX_LENGTH       1

/* Return index of (afdx_dst_port and vl_id) or -1 if not found */
static int get_afdx_dst_port_index(AFDX_TO_ARINC_ROUTER_state *state, uint16_t afdx_dst_port, uint16_t vl_id)
{
    for (size_t i = 0; i < state->map_afdx_dst_port_to_idx_len; i++) {
        if ((state->map_afdx_dst_port_vl_id_to_idx[i].afdx_dst_port == afdx_dst_port) &&
            (state->map_afdx_dst_port_vl_id_to_idx[i].vl_id == vl_id))
        {
            return i;
        }
    }
    return -1;
}

ret_t afdx_to_arinc_router_receive(AFDX_TO_ARINC_ROUTER *self,
                                   const uint8_t * afdx_packet,
                                   const size_t afdx_packet_size
                                  )
{
    ret_t ret = check_afdx_frame_size(afdx_packet_size);
    if (ret != EOK) {
        return ret;
    }

    afdx_frame_t *afdx_frame = (afdx_frame_t *)afdx_packet;

    int idx = get_afdx_dst_port_index(&self->state, afdx_frame->udp_header.afdx_dst_port, afdx_frame->mac_header.mac_dst_addr.vl_id);

    if (idx < 0) {
        printf(C_NAME"Error, there is no afdx_dst_port %d\n", afdx_frame->udp_header.afdx_dst_port);
        return EINVAL;
    }

    ret_t res = AFDX_TO_ARINC_ROUTER_call_portArray_handle_by_index(idx, self, (const uint8_t *)afdx_frame->afdx_payload, afdx_packet_size - HEADER_LENGTH - SUFFIX_LENGTH);
    if (res != EOK)
            printf(C_NAME"Error in sending to afdx_dst_port %d\n",  afdx_frame->udp_header.afdx_dst_port);

    return res;
}
