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

static void queuing_send_to_partition(unsigned channel_idx, MESSAGE_ADDR_TYPE payload, size_t length)
{
    RETURN_CODE_TYPE ret;
    sys_channel_t channel = sys_queuing_channels[channel_idx];
    sys_queuing_port_t *port = &sys_queuing_ports[channel.port_index];

    SEND_QUEUING_MESSAGE(
            port->id,
            payload,
            length,
            0,
            &ret);

    if (ret == NOT_AVAILABLE) {
        printf("Buffer is full, drop packet\n");
    } else if (ret != NO_ERROR) {
        printf("SYSNET %s port error: %u\n", port->header.name, ret);
    }
}

static void sampling_send_to_partition(unsigned channel_idx, MESSAGE_ADDR_TYPE payload, size_t length)
{
    sys_channel_t channel = sys_sampling_channels[channel_idx];
    sys_sampling_port_t *port = &sys_sampling_ports[channel.port_index];
    RETURN_CODE_TYPE ret;

    WRITE_SAMPLING_MESSAGE(
            port->id,
            payload, 
            length, 
            &ret);

    if (ret != NO_ERROR) {
        printf("error: %u\n", ret);
    }

}

static pok_bool_t udp_received_callback(
        uint32_t ip, 
        uint16_t udp_port, 
        const char *payload, 
        size_t length) 
{
    for (int i = 0; i<sys_sampling_channels_nb; i++) {
        sys_channel_t *s_channel = &sys_sampling_channels[i];
        //TODO
        //if (s_channel->protocol != UDP)
        //    continue;

        sys_sampling_port_t *port = &sys_sampling_ports[s_channel->port_index];
        udp_data_t *udp_data = s_channel->driver_data;

        if (port->header.direction != SOURCE)
            continue;
        if (udp_data->ip != ip || udp_data->port != udp_port)
            continue;

        sampling_send_to_partition(i, (MESSAGE_ADDR_TYPE) payload, length);
    }

    for (int i = 0; i<sys_queuing_channels_nb; i++) {
        sys_channel_t *q_channel = &sys_queuing_channels[i];
        //TODO
        //if (q_channel->protocol != UDP)
        //    continue;

        sys_queuing_port_t *port = &sys_queuing_ports[q_channel->port_index];
        udp_data_t *udp_data = q_channel->driver_data;

        if (port->header.direction != SOURCE)
            continue;
        if (udp_data->ip != ip || udp_data->port != udp_port)
            continue;

        queuing_send_to_partition(i, (MESSAGE_ADDR_TYPE) payload, length);
    }

    return FALSE;
}

void ARINC_recv_active()
{
    for (int i = 0; i < channel_drivers_nb; i++) {
        channel_driver_t *driver_ptr = channel_drivers[i];
        driver_ptr->receive();
    }
}

void ARINC_recv_init()
{
    return;
    RETURN_CODE_TYPE ret;
    for (int i = 0; i<sys_sampling_ports_nb; i++) {
        sys_sampling_port_t *port = &sys_sampling_ports[i];
        if (port->header.direction != SOURCE)
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
        if (port->header.direction != SOURCE)
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
            printf("RCV: error %d creating %s port\n", ret, port->header.name);

    }

    channel_drivers[0]->register_received_callback(udp_received_callback);
}
