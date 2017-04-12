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

#ifndef __AFDX_TIME_ADDER_GEN_H__
#define __AFDX_TIME_ADDER_GEN_H__

#include <memblocks.h>

    #include <interfaces/message_handler_gen.h>

    #include <interfaces/time_message_handler_gen.h>

typedef struct AFDX_TIME_ADDER_state {
}AFDX_TIME_ADDER_state;

typedef struct {
    char instance_name[16];
    AFDX_TIME_ADDER_state state;
    struct {
            struct {
                message_handler ops;
            } portB;
    } in;
    struct {
            struct {
                time_message_handler *ops;
                self_t *owner;
            } portA;
    } out;
} AFDX_TIME_ADDER;



      ret_t time_adder_send(AFDX_TIME_ADDER *, const uint8_t *, size_t);

      ret_t AFDX_TIME_ADDER_call_portA_handle(AFDX_TIME_ADDER *, const uint8_t *, size_t, SYSTEM_TIME_TYPE);






#endif
