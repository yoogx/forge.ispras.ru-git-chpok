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

#ifndef __AFDX_STOP_GEN_H__
#define __AFDX_STOP_GEN_H__

    #include <arinc653/time.h>

    #include <interfaces/message_handler_gen.h>


typedef struct AFDX_STOP_state {
    int st;
}AFDX_STOP_state;

typedef struct {
    AFDX_STOP_state state;
    struct {
            struct {
                message_handler ops;
            } portA;
    } in;
    struct {
    } out;
} AFDX_STOP;



      ret_t afdx_stop_func(AFDX_STOP *, const char *, size_t);







#endif
