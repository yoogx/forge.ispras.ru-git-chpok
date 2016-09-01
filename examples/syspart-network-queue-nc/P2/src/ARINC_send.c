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
static void queuing_send_outside(unsigned channel_idx)
{
    sys_channel_t channel = sys_queuing_channels[channel_idx];
    RETURN_CODE_TYPE ret;

    sys_queuing_port_t *port = &sys_queuing_ports[channel.port_index];

    sys_port_data_t *dst_place = port->data;
    RECEIVE_QUEUING_MESSAGE(
            port->id,
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


void ARINC_send_init()
{
    RETURN_CODE_TYPE ret;
    for (int i = 0; i<sys_sampling_ports_nb; i++) {
        sys_sampling_port_t *port = &sys_sampling_ports[i];

        if (port->header.direction != DESTINATION)
            continue;
        CREATE_SAMPLING_PORT(
                port->header.name,
                port->max_message_size,
                port->header.direction,
                0,
                &port->id,
                &ret);

        if (ret != NO_ERROR)
            printf("error %d creating %s port\n", ret, port->header.name);
    }

    for (int i = 0; i<sys_queuing_ports_nb; i++) {
        sys_queuing_port_t *port = &sys_queuing_ports[i];
        if (port->header.direction != DESTINATION)
            continue;
        CREATE_QUEUING_PORT(
                port->header.name,
                port->max_message_size,
                port->max_nb_messages,
                port->header.direction,
                FIFO,
                &port->id,
                &ret);
        if (ret != NO_ERROR)
            printf("SND: error %d creating %s port\n", ret, port->header.name);

    }
}
