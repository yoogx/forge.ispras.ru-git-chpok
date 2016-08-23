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

#include <net/network.h>
#include <middleware/port.h>
#include <arinc653/types.h>

typedef struct
{
    //pok_port_kinds_t            kind;
    NAME_TYPE                   name;
    PORT_DIRECTION_TYPE         direction;
    unsigned                    overhead;
} sys_port_header_t;

typedef struct
{
    MESSAGE_SIZE_TYPE           message_size;
    char                        data[];
} sys_port_data_t;

enum QUEUING_STATUS {
    QUEUING_STATUS_NONE, // message hasn't been touched by network code at all
    QUEUING_STATUS_PENDING, // message has been sent to the driver, and its buffer is still in use by that driver
    QUEUING_STATUS_SENT, // message sent, buffer is free to use, but place is still occupied (it will be reclaimed soon)
};

typedef struct
{
    sys_port_header_t           header;

    pok_queuing_discipline_t discipline;

    pok_port_size_t             max_message_size;
    pok_port_size_t             max_nb_messages;

    pok_port_size_t             nb_message;
    pok_port_size_t             queue_head;

    sys_port_data_t             *data;
    APEX_INTEGER                id;

} sys_queuing_port_t;

typedef struct
{
    sys_port_header_t           header;

    pok_port_size_t             max_message_size;
    uint64_t                    refresh;
    uint64_t                    last_receive;
    pok_bool_t                  last_validity;
    pok_bool_t                  not_empty;
    pok_bool_t                  is_new;//TRUE if data hasn't been read yet
    sys_port_data_t             *data;
    APEX_INTEGER                id;
} sys_sampling_port_t;


#endif
