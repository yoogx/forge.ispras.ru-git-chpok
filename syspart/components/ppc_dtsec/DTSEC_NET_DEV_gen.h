/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/ppc_dtsec/config.yaml).
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

#ifndef __DTSEC_NET_DEV_GEN_H__
#define __DTSEC_NET_DEV_GEN_H__

#include <memblocks.h>
    #include "dtsec_state.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct DTSEC_NET_DEV_state {
    uint8_t dtsec_num;
    struct dev_state dev_state;
}DTSEC_NET_DEV_state;

typedef struct {
    char instance_name[16];
    DTSEC_NET_DEV_state state;
    struct {
            struct {
                preallocated_sender ops;
            } portA;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } portB;
    } out;
} DTSEC_NET_DEV;



      ret_t dtsec_send_frame(DTSEC_NET_DEV *, char *, size_t, size_t, size_t);
      ret_t dtsec_flush_send(DTSEC_NET_DEV *);

      ret_t DTSEC_NET_DEV_call_portB_handle(DTSEC_NET_DEV *, const char *, size_t);


 pok_ret_t DTSEC_NET_DEV_get_memory_block_status(
         DTSEC_NET_DEV *self,
         const char *name,
         jet_memory_block_status_t *mb_status);

    void dtsec_component_init(DTSEC_NET_DEV *);

    void dtsec_receive_activity(DTSEC_NET_DEV *);


#endif
