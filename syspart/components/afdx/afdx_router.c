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

#include "AFDX_ROUTER_gen.h"
#include "afdx.h"

#define C_NAME "AFDX_ROUTER: "

/* Return index of vl_id. -1 if not found */
static int get_vl_id_index(AFDX_ROUTER_state *state, uint16_t vl_id)
{
    for (size_t i = 0; i < state->map_vl_id_to_idx_len; i++) {
        //~ struct vl_index *cur_state = &state->map_vl_id_to_idx[i];
        if (state->map_vl_id_to_idx[i].vl_id == vl_id) {
            return i;
        }
    }
    return -1;
}

ret_t afdx_router_receive_packet(AFDX_ROUTER *self, const char *payload, size_t payload_size, SYSTEM_TIME_TYPE arrival_time)
{
    frame_data_t *afdx_frame = (frame_data_t *)payload;
    
    int idx = get_vl_id_index(&self->state, afdx_frame->mac_header.mac_dst_addr.vl_id);

    if (idx < 0) {
        printf(C_NAME"Error, there is no vl_id %d\n", afdx_frame->mac_header.mac_dst_addr.vl_id);
        return EINVAL;
    }
    //~ printf(C_NAME"get message for vl_id %d: %s\n", afdx_frame->mac_header.mac_dst_addr.vl_id, afdx_frame->afdx_payload);
    ret_t res = AFDX_ROUTER_call_portArray_handle_by_index(idx, self, payload, payload_size, arrival_time);
    
    if (res != EOK)
            printf(C_NAME"Error in sending to vl_id %d",  afdx_frame->mac_header.mac_dst_addr.vl_id);
    //~ printf(C_NAME" send message\n");
    return EOK;
}
