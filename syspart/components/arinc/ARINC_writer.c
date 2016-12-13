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

#include <arinc653/queueing.h>
#include <arinc653/sampling.h>
#include <stdio.h>

#include <port_info.h>

#include "ARINC_PORT_WRITER_gen.h"

#define C_NAME "ARINC_PORT_WRITER: "

static ret_t send_msg_to_user_partition_queuing(ARINC_PORT_WRITER *self, const char *payload, size_t length)
{
    RETURN_CODE_TYPE ret;

    SEND_QUEUING_MESSAGE(
            self->state.port_id,
            (MESSAGE_ADDR_TYPE) payload,
            length,
            0,
            &ret);

    if (ret != NO_ERROR) {
        if (ret == NOT_AVAILABLE) {
            printf(C_NAME"%s port queue is full, drop packet\n", self->state.port_name);
            return EAGAIN; //what should be returned???
        } else {
            printf(C_NAME"%s port error: %d\n", self->state.port_name, ret);
            return EINVAL;
        }
    }
    return EOK;
}

static ret_t send_msg_to_user_partition_sampling(ARINC_PORT_WRITER *self, const char *payload, size_t length)
{
    RETURN_CODE_TYPE ret;

    WRITE_SAMPLING_MESSAGE(
            self->state.port_id,
            (MESSAGE_ADDR_TYPE) payload,
            length,
            &ret);

    if (ret != NO_ERROR) {
        printf(C_NAME"%s port error: %d\n", self->state.port_name, ret);
        return EINVAL;
    }
    return EOK;

}

void arinc_port_writer_init(ARINC_PORT_WRITER *self)
{
    RETURN_CODE_TYPE ret;
    if (self->state.is_queuing_port) {
        CREATE_QUEUING_PORT(
                self->state.port_name,
                self->state.port_max_message_size,
                self->state.q_port_max_nb_messages,
                self->state.port_direction,
                FIFO,
                &self->state.port_id,
                &ret);
    } else {
        CREATE_SAMPLING_PORT(
                self->state.port_name,
                self->state.port_max_message_size,
                self->state.port_direction,
                0, //in future should be any positive number
                &self->state.port_id,
                &ret);
    }
}

ret_t arinc_receive_message(ARINC_PORT_WRITER *self, const char *payload, size_t payload_size)
{
    //printf(C_NAME"%s got message\n", self->state.tmp_name);
    if (self->state.is_queuing_port)
        return send_msg_to_user_partition_queuing(self, payload, payload_size);
    else
        return send_msg_to_user_partition_sampling(self, payload, payload_size);
}
