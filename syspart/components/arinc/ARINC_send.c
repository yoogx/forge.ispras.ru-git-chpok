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

#include <net/network.h>
#include <depl.h>
#include <port_info.h>

#include "ARINC_SENDER_gen.h"
#define C_NAME "ARIND_SENDER: "

int tmp_id;
static void queuing_send_outside(unsigned channel_idx)
{
    sys_channel_t channel = sys_queuing_channels[channel_idx];
    RETURN_CODE_TYPE ret;

    sys_queuing_port_t *port = &sys_queuing_ports[channel.port_index];

    sys_port_data_t *dst_place = port->data;
    RECEIVE_QUEUING_MESSAGE(
            tmp_id,
            0,
            (MESSAGE_ADDR_TYPE ) (dst_place->data + port->header.overhead),
            &dst_place->message_size,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf("SYSNET: %s port error: %u\n", port->header.name, ret);
        return;
    }

    pok_bool_t res = channel.driver_ptr->send(
            dst_place->data + port->header.overhead,
            dst_place->message_size,
            port->header.overhead,
            channel.driver_data
            );

    if (!res)
        printf("SYSNET: Error in send_udp\n");

    channel.driver_ptr->flush_send();
}

static void sampling_send_outside(unsigned channel_idx)
{
    RETURN_CODE_TYPE ret;
    VALIDITY_TYPE validity;

    sys_channel_t channel = sys_sampling_channels[channel_idx];
    sys_sampling_port_t *port = &sys_sampling_ports[channel.port_index];
    sys_port_data_t *dst_place = port->data;

    if (!SYS_SAMPLING_PORT_CHECK_IS_NEW_DATA(port->id))
        return;

    READ_SAMPLING_MESSAGE(
            port->id,
            (MESSAGE_ADDR_TYPE ) (dst_place->data + port->header.overhead),
            &dst_place->message_size,
            &validity,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf("SYSNET: %s port error: %u\n", port->header.name, ret);
        return;
    }

    pok_bool_t res = channel.driver_ptr->send(
            dst_place->data + port->header.overhead,
            dst_place->message_size,
            port->header.overhead,
            channel.driver_data
            );

    if (!res)
        printf("SYSNET: Error in send_udp\n");

    channel.driver_ptr->flush_send();
}
void ARINC_send_active()
{
    for (int i = 0; i<sys_sampling_channels_nb; i++) {
        sys_channel_t *channel = &sys_sampling_channels[i];
        sys_sampling_port_t *port = &sys_sampling_ports[channel->port_index];

        if (port->header.direction != DESTINATION)
            continue;

        sampling_send_outside(i);
    }

    for (int i = 0; i<sys_queuing_channels_nb; i++) {
        sys_channel_t *channel = &sys_queuing_channels[i];
        sys_queuing_port_t *port = &sys_queuing_ports[channel->port_index];

        if (port->header.direction != DESTINATION)
            continue;

        queuing_send_outside(i);
    }
}


void arinc_sender_activity(ARINC_SENDER *self)
{
    printf(C_NAME"activity\n");
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
    tmp_id = self->state.port_id;

    if (ret != NO_ERROR)
        printf(C_NAME"error %d during creation %s port\n", ret, self->state.port_name);
    else
        printf(C_NAME"successfuly create %s port\n", self->state.port_name);
}
