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

#ifndef __AFDX_TO_ARINC_ROUTER_GEN_H__
#define __AFDX_TO_ARINC_ROUTER_GEN_H__

#include <memblocks.h>
    #include <types.h>

    #include <interfaces/message_handler_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct AFDX_TO_ARINC_ROUTER_state {
    uint16_t map_afdx_dst_port_to_idx[10];
    size_t map_afdx_dst_port_to_idx_len;
}AFDX_TO_ARINC_ROUTER_state;

typedef struct {
    char instance_name[16];
    AFDX_TO_ARINC_ROUTER_state state;
    struct {
            struct {
                message_handler ops;
            } portC;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } *portArray;
    } out;
} AFDX_TO_ARINC_ROUTER;



      ret_t afdx_to_arinc_router_receive(AFDX_TO_ARINC_ROUTER *, const char *, size_t);

      ret_t AFDX_TO_ARINC_ROUTER_call_portArray_handle_by_index(int, AFDX_TO_ARINC_ROUTER *, const char *, size_t);






#endif
