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

#ifndef __POK_KERNEL_PORT_H__
#define __POK_KERNEL_PORT_H__

/* ARINC ports. */

#include <core/channel.h>
#include <core/thread.h>
#include <common.h>
#include <uapi/port_types.h>

struct _pok_patition_arinc;

/* 
 * Queuing port. 
 */
typedef struct
{
    /* 
     * Name of the port.
     * 
     * Set in the deployment.c.
     */
    const char                  *name;
    /*
     * Queue of waited threads.
     */
    pok_thread_wq_t             waiters;
    
    /*
     * Channel to which port is connected.
     * 
     * Set in the deployment.c.
     */
    pok_channel_queuing_t       *channel;

    /*
     * Direction (IN or OUT).
     * 
     * Set in the deployment.c.
     */
    pok_port_directions_t       direction;
    
    /*
     * Queuing discipline.
     * 
     * Set on port creation.
     */
    pok_queuing_discipline_t    discipline;

    /* Whether port has been created (with CREATE_QUEUING_PORT)*/
    pok_bool_t                  is_created;
} pok_port_queuing_t;

// Initialize queuing port.
void pok_port_queuing_init(pok_port_queuing_t* port_queuing);


pok_ret_t pok_port_queuing_create(
    const char* __user              name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_message,
    pok_port_direction_t            direction,
    pok_queuing_discipline_t        discipline,
    pok_port_id_t* __user           id);


pok_ret_t pok_port_queuing_create_packed(
    const char* __user              name,
    const pok_port_queuing_create_arg_t* __user arg,
    pok_port_id_t* __user           id);
    

pok_ret_t pok_port_queuing_receive(
    pok_port_id_t               id, 
    const pok_time_t* __user    timeout, 
    void* __user                data, 
    pok_port_size_t* __user     len);


pok_ret_t pok_port_queuing_send(
    pok_port_id_t               id, 
    const void* __user          data,
    pok_port_size_t             len,
    const pok_time_t* __user    timeout);


pok_ret_t pok_port_queuing_status(
    pok_port_id_t               id,
    pok_port_queuing_status_t* __user status);

pok_ret_t pok_port_queuing_id(
    const char* __user name,
    pok_port_id_t* __user id);

pok_ret_t pok_port_queuing_clear(pok_port_id_t id);

/* 
 * Receive message from the port into specified process.
 * 
 * Port should be available for read.
 * 
 * Before call:
 * t->wait_buffer.dest - points to the message
 * 
 * After call:
 * t->wait_len - length of stored message
 * t->wait_result - result of wait (POK_ERRNO_OK or POK_ERRNO_TOOMANY).
 * 
 * Should be called with local preemption disabled.
 */
void port_queuing_receive(pok_port_queuing_t* port, pok_thread_t* t);

/* 
 * Send message into the port from specified process.
 * 
 * Port should be available for write.
 * 
 * Before call:
 * t->wait_buffer.src - points to the message,
 * t->wait_len - length of the message
 * 
 * After call:
 * t->wait_result - result of wait (POK_ERRNO_OK)
 * 
 * Should be called with local preemption disabled.
 */
void port_queuing_send(pok_port_queuing_t* port, pok_thread_t* t);


/* 
 * Sampling port.
 * 
 * Contain common parts for "in" and "out" ports +
 * refresh_period for "in" port.
 */
typedef struct
{
    /* 
     * Name of the port.
     * 
     * Should be set initially.
     */
    const char                  *name;
    
    /*
     * Channel to which port is connected.
     * 
     * Should be set initially.
     */
    pok_channel_sampling_t       *channel;

    /*
     * Direction (IN or OUT).
     * 
     * Should be set initially.
     */
    pok_port_directions_t       direction;

    /* Whether port has been created (with CREATE_SAMPLING_PORT)*/
    pok_bool_t                  is_created;

    /* Refresh period for IN port. */
    pok_time_t                  refresh_period;
    
    /* Validity of last message read from the port. */
    pok_bool_t                  last_message_validity;
} pok_port_sampling_t;

// Initialize sampling port
void pok_port_sampling_init(pok_port_sampling_t* port_sampling);

pok_ret_t pok_port_sampling_create(
    const char* __user          name,
    pok_port_size_t             size,
    pok_port_direction_t        direction,
    const pok_time_t* __user    refresh,
    pok_port_id_t               *id
);

pok_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void __user       *data,
    pok_port_size_t         len
);

pok_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void __user             *data,
    pok_port_size_t __user  *len,
    pok_bool_t __user       *valid
);

pok_ret_t pok_port_sampling_id(
    const char __user       *name,
    pok_port_id_t           *id
);

pok_ret_t pok_port_sampling_status (
    pok_port_id_t                      id,
    pok_port_sampling_status_t __user   *status
);

pok_ret_t pok_port_sampling_check(pok_port_id_t id);

#endif /* __POK_KERNEL_PORT_H__ */
