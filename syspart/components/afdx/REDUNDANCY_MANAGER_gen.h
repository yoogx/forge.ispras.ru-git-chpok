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

#ifndef __REDUNDANCY_MANAGER_GEN_H__
#define __REDUNDANCY_MANAGER_GEN_H__

    #include <types.h>
    #include "redundancy_manager_struct.h"

    #include <interfaces/time_message_handler_gen.h>
    #include <interfaces/time_message_handler_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct REDUNDANCY_MANAGER_state {
    vl_data_t virtual_link_data[VIRTUAL_LINKS_COUNT];
    SYSTEM_TIME_TYPE arrival_time[SUBNETWORKS_COUNT][MAX_SEQUENCE_NUMBER + 1];
}REDUNDANCY_MANAGER_state;

typedef struct {
    REDUNDANCY_MANAGER_state state;
    struct {
            struct {
                time_message_handler ops;
            } portA;
            struct {
                time_message_handler ops;
            } portB;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } portC;
    } out;
} REDUNDANCY_MANAGER;



      ret_t redundancy_manager_receive_packet_net_a(REDUNDANCY_MANAGER *, const char *, size_t, SYSTEM_TIME_TYPE);
      ret_t redundancy_manager_receive_packet_net_b(REDUNDANCY_MANAGER *, const char *, size_t, SYSTEM_TIME_TYPE);

      ret_t REDUNDANCY_MANAGER_call_portC_handle(REDUNDANCY_MANAGER *, const char *, size_t);



    void redundancy_manager_init(REDUNDANCY_MANAGER *);



#endif
