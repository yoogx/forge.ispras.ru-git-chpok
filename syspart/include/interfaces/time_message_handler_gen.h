/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/afdx_net.yaml).
 */
#ifndef __INTERFACES_TIME_MESSAGE_HANDLER_H__
#define __INTERFACES_TIME_MESSAGE_HANDLER_H__

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


#include <lib/common.h>
    #include <ret_type.h>
    #include <arinc653/time.h>

typedef struct {
    ret_t (*handle)(self_t *, const uint8_t *, size_t, SYSTEM_TIME_TYPE);
} time_message_handler;


#endif

