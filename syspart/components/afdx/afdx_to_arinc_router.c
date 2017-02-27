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

// ???
#define HEADER_LENGTH       42
#define SUFFIX_LENGTH       1
  
void afdx_to_arinc_router_init(AFDX_TO_ARINC_ROUTER *self)
{

}
/* Return index of afdx_dst_port or -1 if not found */
static int get_afdx_dst_port_index(AFDX_TO_ARINC_ROUTER_state *state, uint16_t afdx_dst_port)
{
    for (int i = 0; i < state->map_afdx_dst_port_to_idx_len; i++) {
        if (state->map_afdx_dst_port_to_idx[i] == afdx_dst_port) {
            return i;
        }
    }
    return -1;
}

ret_t afdx_to_arinc_router_receive(AFDX_TO_ARINC_ROUTER *self,
                                   const char * payload,
                                   const size_t frame_size
                                  )
{
    frame_data_t *afdx_frame = (frame_data_t *)payload;
    
    int idx = get_afdx_dst_port_index(&self->state, afdx_frame->udp_header.afdx_dst_port);

    if (idx < 0) {
        printf(C_NAME"Error, there is no afdx_dst_port %d\n", afdx_frame->udp_header.afdx_dst_port);
        return EINVAL;
    }

    //~ printf(C_NAME"message: %s\n", afdx_frame->afdx_payload);

    ret_t res = AFDX_TO_ARINC_ROUTER_call_portArray_handle_by_index(idx, self, afdx_frame->afdx_payload, frame_size - HEADER_LENGTH - SUFFIX_LENGTH);
    if (res != EOK)
            printf(C_NAME"Error in sending to afdx_dst_port %d\n",  afdx_frame->udp_header.afdx_dst_port);
    
    return EOK;
}
