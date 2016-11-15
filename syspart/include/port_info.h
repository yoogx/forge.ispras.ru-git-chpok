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

#ifndef __POK_SYSPART_PORTS_H__
#define __POK_SYSPART_PORTS_H__

#include <middleware/port.h>
#include <arinc653/types.h>

typedef struct
{
    NAME_TYPE                   name;
    PORT_DIRECTION_TYPE         direction;
    size_t                      overhead;
} sys_port_header_t;

typedef struct
{
    MESSAGE_SIZE_TYPE           message_size;
    char                        data[];
} sys_port_data_t;

typedef struct
{
    sys_port_header_t           header;

    pok_port_size_t             max_message_size;
    pok_port_size_t             max_nb_messages;

    sys_port_data_t             *data;
    APEX_INTEGER                id;

} sys_queuing_port_t;

typedef struct
{
    sys_port_header_t           header;

    pok_port_size_t             max_message_size;

    sys_port_data_t             *data;
    APEX_INTEGER                id;
} sys_sampling_port_t;


#endif
