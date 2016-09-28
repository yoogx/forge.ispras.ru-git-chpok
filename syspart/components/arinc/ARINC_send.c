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

#include <mem.h>

#include "ARINC_SENDER_gen.h"
#define C_NAME "ARINC_SENDER: "

static int receive_msg_queuing(ARINC_SENDER *self)
{
    sys_port_data_t *dst_place = self->state.port_buffer;

    RETURN_CODE_TYPE ret;
    RECEIVE_QUEUING_MESSAGE(
            self->state.port_id,
            0,
            (MESSAGE_ADDR_TYPE ) (dst_place->data + self->state.overhead),
            &dst_place->message_size,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf(C_NAME"%s port error: %u\n", self->state.port_name, ret);
        return -1;
    }
    return 0;
}

static int receive_msg_samping(ARINC_SENDER *self)
{
    RETURN_CODE_TYPE ret;
    sys_port_data_t *dst_place = self->state.port_buffer;

    if (!SYS_SAMPLING_PORT_CHECK_IS_NEW_DATA(self->state.port_id))
        return -1;

    READ_SAMPLING_MESSAGE(
            self->state.port_id,
            (MESSAGE_ADDR_TYPE ) (dst_place->data + self->state.overhead),
            &dst_place->message_size,
            NULL,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf(C_NAME"%s port error: %u\n", self->state.port_name, ret);
        return -1;
    }

    return 0;
}

void arinc_sender_activity(ARINC_SENDER *self)
{
    int receive_error;
    if (self->state.is_queuing_port)
        receive_error = receive_msg_queuing(self);
    else
        receive_error = receive_msg_samping(self);

    if (receive_error != 0)
        return;

    sys_port_data_t *dst_place = self->state.port_buffer;
    ret_t res = ARINC_SENDER_call_portA_send(self,
            dst_place->data + self->state.overhead,
            dst_place->message_size,
            self->state.overhead
            );

    if (res != EOK)
        printf(C_NAME"Error in send_udp\n");

    ARINC_SENDER_call_portA_flush(self);
}

void arinc_sender_init(ARINC_SENDER *self)
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

    if (ret != NO_ERROR) {
        printf(C_NAME"error %d during creation %s port\n", ret, self->state.port_name);
        return;
    }

    printf(C_NAME"successfuly create %s port\n", self->state.port_name);

    self->state.port_buffer = smalloc(
            sizeof(*self->state.port_buffer) +
            self->state.overhead +
            self->state.port_max_message_size);
}
