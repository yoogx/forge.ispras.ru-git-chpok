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

#include <core/port.h>
#include <core/partition_arinc.h>
#include <core/thread.h>
#include "thread_internal.h"
#include <uaccess.h>
#include <core/sched_arinc.h>
#include <message.h>

/* 
 * Find *configured* queuing port by name, which comes from user space.
 * 
 * NOTE: Name should be checked before!
 */
static pok_port_queuing_t* find_port_queuing(const char* __user name)
{
    char kernel_name[MAX_NAME_LENGTH];
    
    pok_port_queuing_t* port_queuing =
        current_partition_arinc->ports_queuing;
        
    pok_port_queuing_t* ports_queuing_end =
        port_queuing + current_partition_arinc->nports_queuing;
        
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
    for(; port_queuing < ports_queuing_end; port_queuing++)
    {
        if(!pok_compare_names(port_queuing->name, kernel_name))
            return port_queuing;
    }
    
    return NULL;
}


/* 
 * Get *created* queuing port by id.
 */
static pok_port_queuing_t* get_port_queuing(pok_port_id_t id)
{
    pok_port_queuing_t* port_queuing;
    
    if(id < 0 || id > current_partition_arinc->nports_queuing) return NULL;
    
    port_queuing = &current_partition_arinc->ports_queuing[id];
    
    return port_queuing->is_created ? port_queuing : NULL;
}


void port_queuing_receive(pok_port_queuing_t* port, pok_thread_t* t)
{
    void* __user data = t->wait_private;
    pok_message_t* m = pok_channel_queuing_r_get_message(port->channel, 0);
    
    __copy_to_user(data, m->content, m->size);
    t->wait_private = (void*)(unsigned long)m->size;
    
    pok_channel_queuing_r_consume_messages(port->channel, 1);
}

void port_queuing_send(pok_port_queuing_t* port, pok_thread_t* t)
{
    pok_message_send_t* m_send = t->wait_private;
    pok_message_t* m = pok_channel_queuing_s_get_message(port->channel, FALSE);
    
    __copy_from_user(m->content, m_send->data, m_send->size);
    m->size = m_send->size;

    t->wait_private = 0;
    
    pok_channel_queuing_s_produce_message(port->channel);
}

// Callbacks for receive and send ports.
static void port_on_message_sent(pok_channel_queuing_reciever_t* receiver)
{
    pok_port_queuing_t* port_queuing
        = container_of(receiver, pok_port_queuing_t, receiver);

    barrier();
    port_queuing->is_notified = TRUE; // Release semantic

    pok_sched_local_invalidate();
}

static void port_on_message_received(pok_channel_queuing_sender_t* sender)
{
    pok_port_queuing_t* port_queuing
        = container_of(sender, pok_port_queuing_t, sender);

    barrier();
    port_queuing->is_notified = TRUE; // Release semantic

    pok_sched_local_invalidate();
}


void pok_port_queuing_init(pok_port_queuing_t* port_queuing)
{
    pok_thread_wq_init(&port_queuing->waiters);
    
    port_queuing->is_created = FALSE;

    if(port_queuing->direction == POK_PORT_DIRECTION_IN) {
        port_queuing->receiver.on_message_sent = &port_on_message_sent;
    }
    else {
        /* port_queuing->direction == POK_PORT_DIRECTION_OUT */
        port_queuing->sender.on_message_received = &port_on_message_received;
    }
}


pok_ret_t pok_port_queuing_create(
    const char* __user              name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_message,
    pok_port_direction_t            direction,
    pok_queuing_discipline_t        discipline,
    pok_port_id_t* __user           id)
{
    pok_port_queuing_t* port_queuing;
    
    if(!check_access_read(name, MAX_NAME_LENGTH))
        return POK_ERRNO_EFAULT;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;
    
    port_queuing = find_port_queuing(name);
    
    if(!port_queuing) return POK_ERRNO_UNAVAILABLE;
    
    if(port_queuing->is_created) return POK_ERRNO_EXISTS;
    
    if(message_size != port_queuing->channel->max_message_size)
        return POK_ERRNO_EINVAL;
    if(direction != port_queuing->direction)
        return POK_ERRNO_EINVAL;
    
    if(direction == POK_PORT_DIRECTION_IN)
    {
        if(max_nb_message != port_queuing->channel->max_nb_message_receive)
            return POK_ERRNO_EINVAL;
    }
    else // (direction == POK_PORT_DIRECTION_OUT)
    {
        if(max_nb_message != port_queuing->channel->max_nb_message_send)
            return POK_ERRNO_EINVAL;
    }


    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL)
        return POK_ERRNO_MODE;

    port_queuing->is_created = TRUE;
    port_queuing->is_notified = FALSE;
    port_queuing->partition = current_partition_arinc;
    port_queuing->discipline = discipline;
    
    if(direction == POK_PORT_DIRECTION_IN) {
        pok_channel_queuing_set_receiver(
            port_queuing->channel, &port_queuing->receiver);
    }
    else {
        /* direction == POK_PORT_DIRECTION_OUT */
        pok_channel_queuing_set_sender(
            port_queuing->channel, &port_queuing->sender);
    }
    
    __put_user(id, port_queuing - current_partition_arinc->ports_queuing);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_port_queuing_create_packed(
    const char* __user              name,
    const pok_port_queuing_create_arg_t* __user arg,
    pok_port_id_t* __user           id)
{
    if(!check_user_read(arg)) return POK_ERRNO_EFAULT;

    return pok_port_queuing_create(
       name,
       __get_user_f(arg, message_size),
       __get_user_f(arg, max_nb_message),
       __get_user_f(arg, direction),
       __get_user_f(arg, discipline),
       id);

}


pok_ret_t pok_port_queuing_receive(
    pok_port_id_t               id, 
    const pok_time_t* __user    timeout, 
    void* __user                data, 
    pok_port_size_t* __user     len)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    pok_thread_t* t;

    long wait_result;
        
    port_queuing = get_port_queuing(id);
    if(!port_queuing) return POK_ERRNO_PORT;

    if(!check_access_write(data, port_queuing->channel->max_message_size))
        return POK_ERRNO_EFAULT;
    if(!check_user_write(len)) return POK_ERRNO_EFAULT;

    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    pok_time_t kernel_timeout = __get_user(timeout);

    if(port_queuing->direction != POK_PORT_DIRECTION_IN)
        return POK_ERRNO_MODE;


    pok_preemption_local_disable();

    t = current_thread;
    
    if(!pok_channel_queuing_r_n_messages(port_queuing->channel))
    {
        /*
         * We need to specify notification flag when trying receive message
         * from the channel.
         * 
         * One possible way is to not specify flag first time, and,
         * if receive fails, calculate flag for the second receive attempt.
         * 
         * But here we calculate flag for the first (and the only) receive attempt.
         * 
         * If ret is not POK_ERRNO_OK, it contains error code to be returned
         * if channel is currently empty. Otherwise waiting is allowed.
         */

        if(kernel_timeout == 0)
            ret = POK_ERRNO_EMPTY;
        else if(!thread_is_waiting_allowed())
            ret = POK_ERRNO_MODE;
        else
            ret = POK_ERRNO_OK;


        if(!pok_thread_wq_is_empty(&port_queuing->waiters) ||
            !pok_channel_queuing_receive(port_queuing->channel, ret == POK_ERRNO_OK))
        {
            if(ret)
            {
                // Waiting is not allowed, but it is needed.
                __put_user(len, 0);
                goto err;
            }
                
            // Prepare to wait.
            t->wait_private = data;
            
            pok_thread_wq_add_common(&port_queuing->waiters, t,
                port_queuing->discipline);
            
            thread_wait_common(t, kernel_timeout);
            
            goto out;
        }
    }
    /* Message is ready in the buffer. */
    t->wait_private = data;
    port_queuing_receive(port_queuing, t);

out:
    pok_preemption_local_enable(); // Possible wait here

    // Decode result in t->wait_private.
    wait_result = (unsigned long)t->wait_private;
    
    if(wait_result > 0)
    {
        // Success. Result determines length of message read.
        __put_user(len, (pok_port_size_t)wait_result);
        return POK_ERRNO_OK;
    }
    else
    {
        // Fail(timeout). Result is negated error code.
        __put_user(len, 0);
        return (pok_ret_t)(-wait_result);
    }

err:
    pok_preemption_local_enable();
    
    return ret;
}



pok_ret_t pok_port_queuing_send(
    pok_port_id_t               id, 
    const void* __user          data,
    pok_port_size_t             len,
    const pok_time_t* __user    timeout)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    pok_thread_t* t = current_thread;

    long wait_result;
    pok_message_send_t message_send;
    
    port_queuing = get_port_queuing(id);
    
    if(!port_queuing) return POK_ERRNO_PORT;

    if(port_queuing->direction != POK_PORT_DIRECTION_OUT)
        return POK_ERRNO_MODE;

    if(len == 0) return POK_ERRNO_EINVAL;

    // error should be INVALID_CONFIG
    if(len > port_queuing->channel->max_message_size)
        return POK_ERRNO_EINVAL;

    if(!check_access_read(data, len)) return POK_ERRNO_EFAULT;
    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    
    pok_time_t kernel_timeout = __get_user(timeout);
    
    pok_preemption_local_disable();
    
    /*
     * We need to specify notification flag when trying to send message
     * into the channel.
     * 
     * One possible way is to not specify flag first time, and,
     * if send fails, calculate flag for the second receive attempt.
     * 
     * But here we calculate flag for the first (and the only) receive attempt.
     * 
     * If ret is not POK_ERRNO_OK, it contains error code to be returned
     * if channel is currently full. Otherwise waiting is allowed.
     */

    if(kernel_timeout == 0)
        ret = POK_ERRNO_EMPTY;
    else if(!thread_is_waiting_allowed())
        ret = POK_ERRNO_MODE;
    else
        ret = POK_ERRNO_OK;

    if(!pok_thread_wq_is_empty(&port_queuing->waiters)
        || !pok_channel_queuing_s_get_message(port_queuing->channel, ret == POK_ERRNO_OK))
    {
        if(ret)
        {
            // Waiting is not allowed, but it is needed.
            goto err;
        }
        
        // Prepare to wait.
        message_send.size = len;
        message_send.data = data;
        
        t->wait_private = &message_send;
        
        pok_thread_wq_add_common(&port_queuing->waiters, t,
            port_queuing->discipline);
        
        thread_wait_common(t, kernel_timeout);
        
        goto out;
    }
    /* There is place for message in the buffer. */
    message_send.size = len;
    message_send.data = data;

    t->wait_private = &message_send;

    port_queuing_send(port_queuing, t);

out:
    pok_preemption_local_enable(); // Possible wait here

    // Decode result in t->wait_private.
    wait_result = (unsigned long)t->wait_private;
    
    if(wait_result == 0)
    {
        // Success.
        return POK_ERRNO_OK;
    }
    else
    {
        // Fail(timeout). Result is negated error code.
        return (pok_ret_t)(-wait_result);
    }
    
err:
    pok_preemption_local_enable();
    return ret;
}


pok_ret_t pok_port_queuing_status(
    pok_port_id_t               id,
    pok_port_queuing_status_t * __user status)
{
    pok_port_queuing_t* port_queuing;
    pok_channel_queuing_t* channel;
    
    if(!check_access_write(status, sizeof(status)))
        return POK_ERRNO_EFAULT;
    
    port_queuing = get_port_queuing(id);
    
    if(!port_queuing) return POK_ERRNO_PORT;

    channel = port_queuing->channel;
    
    __put_user_f(status, max_message_size, channel->max_message_size);
    __put_user_f(status, direction, port_queuing->direction);

    pok_preemption_local_disable();

    __put_user_f(status, waiting_processes, pok_thread_wq_get_nwaits(&port_queuing->waiters));
    
    if(port_queuing->direction == POK_PORT_DIRECTION_IN) {
        __put_user_f(status, max_nb_message, channel->max_nb_message_receive);
        __put_user_f(status, nb_message, pok_channel_queuing_r_n_messages(channel));
    }
    else {
        /* port_queuing->direction == POK_PORT_DIRECTION_OUT */
        __put_user_f(status, max_nb_message, channel->max_nb_message_send);
        __put_user_f(status, nb_message, pok_channel_queuing_s_n_messages(channel));
    }
    
    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

pok_ret_t pok_port_queuing_id(
    const char* __user name,
    pok_port_id_t* __user id)
{
    pok_port_queuing_t* port_queuing;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;

    port_queuing = find_port_queuing(name);
    
    if(!port_queuing) return POK_ERRNO_UNAVAILABLE;
    if(!port_queuing->is_created) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, port_queuing - current_partition_arinc->ports_queuing);
    
    return POK_ERRNO_OK;
}

/**********************************************************************/
/* 
 * Find *configured* sampling port by name, which comes from user space.
 * 
 * NOTE: Name should be checked before!
 */
static pok_port_sampling_t* find_port_sampling(const char* __user name)
{
    char kernel_name[MAX_NAME_LENGTH];
    
    pok_port_sampling_t* port_sampling =
        current_partition_arinc->ports_sampling;
        
    pok_port_sampling_t* ports_sampling_end =
        port_sampling + current_partition_arinc->nports_sampling;
        
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
    for(; port_sampling < ports_sampling_end; port_sampling++)
    {
        if(!pok_compare_names(port_sampling->name, kernel_name))
            return port_sampling;
    }
    
    return NULL;
}


/* 
 * Get *created* sampling port by id.
 */
static pok_port_sampling_t* get_port_sampling(pok_port_id_t id)
{
    pok_port_sampling_t* port_sampling;
    
    if(id < 0 || id > current_partition_arinc->nports_sampling) return NULL;
    
    port_sampling = &current_partition_arinc->ports_sampling[id];
    
    return port_sampling->is_created ? port_sampling : NULL;
}

void pok_port_sampling_init(pok_port_sampling_t* port_sampling)
{
    port_sampling->is_created = FALSE;
}


pok_ret_t pok_port_sampling_create(
    const char* __user          name,
    pok_port_size_t             size,
    pok_port_direction_t        direction,
    const pok_time_t* __user    refresh,
    pok_port_id_t               *id
)
{
    pok_port_sampling_t* port_sampling;

    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL)
        return POK_ERRNO_MODE;

    if(!check_access_read(name, MAX_NAME_LENGTH))
        return POK_ERRNO_EFAULT;

    if(!check_user_read(refresh)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;

    pok_time_t kernel_refresh = __get_user(refresh);
    
    port_sampling = find_port_sampling(name);
    
    if(!port_sampling) return POK_ERRNO_UNAVAILABLE;
    if(port_sampling->is_created) return POK_ERRNO_EXISTS;

    if(size != port_sampling->channel->max_message_size)
        return POK_ERRNO_EINVAL;
    if(direction != port_sampling->direction)
        return POK_ERRNO_EINVAL;
    // ARINC specifies to check refresh period for any direction.
    if(pok_time_is_infinity(kernel_refresh) || kernel_refresh == 0)
        return POK_ERRNO_EINVAL;

    port_sampling->is_created = TRUE;

    // Useless for OUT port.
    port_sampling->refresh_period = kernel_refresh;
    port_sampling->last_message_validity = FALSE;
    
    if(port_sampling->direction == POK_PORT_DIRECTION_OUT)
    {
        pok_channel_sampling_s_clear_message(port_sampling->channel);
    }
    else
    {
        pok_channel_sampling_r_clear_message(port_sampling->channel);
    }

    __put_user(id, port_sampling - current_partition_arinc->ports_sampling);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void __user       *data,
    pok_port_size_t         len)
{
    pok_port_sampling_t* port_sampling;

    pok_message_t* message;
    
    port_sampling = get_port_sampling(id);
    
    if(!port_sampling) return POK_ERRNO_PORT;
    
    if(len == 0 || len > port_sampling->channel->max_message_size)
        return POK_ERRNO_EINVAL;

    if(port_sampling->direction != POK_PORT_DIRECTION_OUT)
        return POK_ERRNO_MODE;

    if(!check_access_read(data, len))
        return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();

    message = pok_channel_sampling_s_get_message(port_sampling->channel);

    message->size = len;
    __copy_from_user(message->content, data, len);
    
    pok_channel_sampling_send_message(port_sampling->channel);

    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void __user             *data,
    pok_port_size_t __user  *len,
    bool_t __user           *valid)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;
    pok_time_t ts;

    pok_message_t* message;
    
    port_sampling = get_port_sampling(id);
    
    if(!port_sampling) return POK_ERRNO_PORT;
    
    if(port_sampling->direction != POK_PORT_DIRECTION_IN)
        return POK_ERRNO_DIRECTION;

    if(!check_access_write(data, port_sampling->channel->max_message_size))
        return POK_ERRNO_EFAULT;
    if(!check_user_write(len))
        return POK_ERRNO_EFAULT;
    if(!check_user_write(valid))
        return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();

    message = pok_channel_sampling_r_get_message(port_sampling->channel, &ts);
    
    if(message)
    {
        __copy_to_user(data, message->content, message->size);
        __put_user(len, (pok_port_size_t)message->size);
        
        pok_time_t current_time = POK_GETTICK();
        port_sampling->last_message_validity =
            ((ts + port_sampling->refresh_period) >= current_time)
            ? TRUE: FALSE;
        ret = POK_ERRNO_OK;
    }
    else
    {
        __put_user(len, 0);
        port_sampling->last_message_validity = FALSE;
        ret = POK_ERRNO_EMPTY;
    }

    __put_user(valid, port_sampling->last_message_validity);

    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_port_sampling_id(
    const char __user      *name,
    pok_port_id_t __user   *id
)
{
    pok_port_sampling_t* port_sampling;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    port_sampling = find_port_sampling(name);
    
    if(!port_sampling) return POK_ERRNO_UNAVAILABLE;
    if(!port_sampling->is_created) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, port_sampling - current_partition_arinc->ports_sampling);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_port_sampling_status (
    pok_port_id_t                 id,
    pok_port_sampling_status_t __user   *status
)
{
    pok_port_sampling_t* port_sampling;
    
    port_sampling = get_port_sampling(id);
    
    if(!port_sampling) return POK_ERRNO_PORT;

    if(!check_user_write(status)) return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    __put_user_f(status, size, port_sampling->channel->max_message_size);
    __put_user_f(status, direction, port_sampling->direction);
    __put_user_f(status, refresh, port_sampling->refresh_period);
    __put_user_f(status, validity, port_sampling->last_message_validity);
    
    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

// TODO: This should be transformed into READ_UPDATED_SAMPLING_MESSAGE eventually.
pok_ret_t pok_port_sampling_check(pok_port_id_t id)
{
    pok_ret_t ret;

    pok_port_sampling_t* port_sampling;

    port_sampling = get_port_sampling(id);

    if(!port_sampling) return POK_ERRNO_PORT;

    if(port_sampling->direction != POK_PORT_DIRECTION_IN)
        return POK_ERRNO_MODE;


    pok_preemption_local_disable();
    ret = pok_channel_sampling_r_check_new_message(port_sampling->channel)
        ? POK_ERRNO_OK
        : POK_ERRNO_EMPTY;
    pok_preemption_local_enable();

    return ret;
}
