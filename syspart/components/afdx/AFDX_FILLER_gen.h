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

#ifndef __AFDX_FILLER_GEN_H__
#define __AFDX_FILLER_GEN_H__

    #include "afdx.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/afdx_queue_enqueuer_gen.h>

typedef struct AFDX_FILLER_state {
    uint16_t vl_id;
    uint16_t src_afdx_port;
    uint16_t dst_afdx_port;
    uint8_t sn;
    uint8_t dst_partition_id;
    uint8_t ttl;
    PACKET_TYPE type_of_packet;
    uint8_t src_partition_id;
}AFDX_FILLER_state;

typedef struct {
    AFDX_FILLER_state state;
    struct {
            struct {
                preallocated_sender ops;
            } portA;
    } in;
    struct {
            struct {
                afdx_queue_enqueuer *ops;
                self_t *owner;
            } portB;
    } out;
} AFDX_FILLER;



      ret_t afdx_filler_send(AFDX_FILLER *, char *, size_t, size_t, size_t);
      ret_t afdx_filler_flush(AFDX_FILLER *);

      ret_t AFDX_FILLER_call_portB_afdx_add_to_queue(AFDX_FILLER *, char *, size_t);



    void afdx_filler_init(AFDX_FILLER *);



#endif
