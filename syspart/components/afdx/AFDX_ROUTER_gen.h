/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/afdx/config.yaml).
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

#ifndef __AFDX_ROUTER_GEN_H__
#define __AFDX_ROUTER_GEN_H__

#include <memblocks.h>
    #include "router_state_structs.h"

    #include <interfaces/time_message_handler_gen.h>

    #include <interfaces/time_message_handler_gen.h>

typedef struct AFDX_ROUTER_state {
    struct vl_index map_vl_id_to_idx[10];
    size_t map_vl_id_to_idx_len;
}AFDX_ROUTER_state;

typedef struct {
    char instance_name[16];
    AFDX_ROUTER_state state;
    struct {
            struct {
                time_message_handler ops;
            } portA;
    } in;
    struct {
            struct {
                time_message_handler *ops;
                self_t *owner;
            } *portArray;
    } out;
} AFDX_ROUTER;



      ret_t afdx_router_receive_packet(AFDX_ROUTER *, const char *, size_t, SYSTEM_TIME_TYPE);

      ret_t AFDX_ROUTER_call_portArray_handle_by_index(int, AFDX_ROUTER *, const char *, size_t, SYSTEM_TIME_TYPE);






#endif
