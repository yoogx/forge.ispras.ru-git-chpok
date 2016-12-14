/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
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

#ifndef __ARINC_PORT_READER_GEN_H__
#define __ARINC_PORT_READER_GEN_H__

    #include <arinc653/queueing.h>
    #include <arinc653/sampling.h>
    #include <port_info.h>


    #include <interfaces/preallocated_sender_gen.h>

typedef struct ARINC_PORT_READER_state {
    PORT_DIRECTION_TYPE port_direction;
    MESSAGE_RANGE_TYPE q_port_max_nb_messages;
    char * port_buffer;
    MESSAGE_SIZE_TYPE message_size;
    MESSAGE_SIZE_TYPE port_max_message_size;
    NAME_TYPE port_name;
    size_t prepend_overhead;
    size_t append_overhead;
    int is_queuing_port;
    APEX_INTEGER port_id;
}ARINC_PORT_READER_state;

typedef struct {
    ARINC_PORT_READER_state state;
    struct {
    } in;
    struct {
            struct {
                preallocated_sender *ops;
                self_t *owner;
            } portA;
    } out;
} ARINC_PORT_READER;




      ret_t ARINC_PORT_READER_call_portA_send(ARINC_PORT_READER *, char *, size_t, size_t, size_t);
      ret_t ARINC_PORT_READER_call_portA_flush(ARINC_PORT_READER *);



    void arinc_port_reader_init(ARINC_PORT_READER *);

    void arinc_port_reader_activity(ARINC_PORT_READER *);


#endif