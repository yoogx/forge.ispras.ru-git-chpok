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
#include <core/uaccess.h>
#include <core/sched_arinc.h>

/*
 * Find *configured* queuing port by name, which comes from user space.
 *
 * NOTE: Name should be checked before!
 */
static pok_port_queuing_t* find_port_queuing(const char* __kuser k_name)
{
    char kernel_name[MAX_NAME_LENGTH];

    pok_port_queuing_t* port_queuing =
        current_partition_arinc->ports_queuing;

    pok_port_queuing_t* ports_queuing_end =
        port_queuing + current_partition_arinc->nports_queuing;

    memcpy(kernel_name, k_name, MAX_NAME_LENGTH);

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
    pok_partition_arinc_t* part = current_partition_arinc;

    pok_port_queuing_t* port_queuing;

    if(id > part->nports_queuing) return NULL;

    port_queuing = &part->ports_queuing[id];

    return port_queuing->is_created ? port_queuing : NULL;
}


void port_queuing_receive(pok_port_queuing_t* port, pok_thread_t* t)
{
    pok_message_size_t message_size;
    const char* message = pok_channel_queuing_r_get_message(
        port->channel, &message_size, FALSE);

    assert(message);

    memcpy(t->wait_buffer.dest, message, message_size);
    t->wait_len = message_size;

    pok_bool_t message_discarded;
    pok_channel_queuing_r_consume_message(port->channel, &message_discarded);

    t->wait_result = message_discarded? JET_INVALID_CONFIG : EOK;
}

void port_queuing_send(pok_port_queuing_t* port, pok_thread_t* t)
{
    char* message = pok_channel_queuing_s_get_message(port->channel, FALSE);
    assert(message);

    memcpy(message, t->wait_buffer.src, t->wait_len);

    pok_channel_queuing_s_produce_message(port->channel, t->wait_len);

    t->wait_result = EOK;
}

void pok_port_queuing_init(pok_port_queuing_t* port_queuing)
{
    pok_thread_wq_init(&port_queuing->waiters);

    port_queuing->is_created = FALSE;
}


/*
 * Create queuing port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'name' or 'id' point to inaccessible memory
 *  - EEXIST - port with given name is already created.
 *  - JET_INVALID_CONFIG
 *    -- there is no queuing port with given name
 *    -- 'message_size' is incorrect or doesn't corresponded to configuration value.
 *    -- 'max_nb_message' is incorrect or doesn't corresponded to configuration value.
 *    -- 'direction' is incorrect or doesn't corresponded to configuration value.
 *    -- 'discipline' is incorrect or doesn't corresponded to configuration value.
 *  - JET_INVALID_MODE - mode is NORMAL
 */
jet_ret_t pok_port_queuing_create(
    const char* __user              name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_message,
    pok_port_direction_t            direction,
    pok_queuing_discipline_t        discipline,
    pok_port_id_t* __user           id)
{
    pok_port_queuing_t* port_queuing;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    pok_port_id_t* __kuser k_id = jet_user_to_kernel_typed(id);
    if(!k_id) return EFAULT;

    port_queuing = find_port_queuing(k_name);

    if(!port_queuing) return JET_INVALID_CONFIG;

    if(port_queuing->is_created) return EEXIST;

    if(message_size != port_queuing->channel->max_message_size)
        return JET_INVALID_CONFIG;
    if(direction != port_queuing->direction)
        return JET_INVALID_CONFIG;

    if(direction == POK_PORT_DIRECTION_IN)
    {
        if(max_nb_message != port_queuing->channel->recv.max_nb_message)
            return JET_INVALID_CONFIG;
    }
    else // (direction == POK_PORT_DIRECTION_OUT)
    {
        if(max_nb_message != port_queuing->channel->send.max_nb_message)
            return JET_INVALID_CONFIG;
    }


    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL)
        return JET_INVALID_MODE;

    port_queuing->is_created = TRUE;
    port_queuing->discipline = discipline;

    if(direction == POK_PORT_DIRECTION_IN) {
        pok_channel_queuing_side_init(port_queuing->channel,
            &port_queuing->channel->recv,
            port_queuing - current_partition_arinc->ports_queuing);
    }
    else {
        /* direction == POK_PORT_DIRECTION_OUT */
        pok_channel_queuing_side_init(port_queuing->channel,
            &port_queuing->channel->send,
            port_queuing - current_partition_arinc->ports_queuing);
    }

    *k_id = port_queuing - current_partition_arinc->ports_queuing;

    return EOK;
}

jet_ret_t pok_port_queuing_create_packed(
    const char* __user              name,
    const pok_port_queuing_create_arg_t* __user arg,
    pok_port_id_t* __user           id)
{
    const pok_port_queuing_create_arg_t* __kuser k_arg =
        jet_user_to_kernel_typed_ro(arg);
    if(!k_arg) return EFAULT;

    return pok_port_queuing_create(
       name,
       k_arg->message_size,
       k_arg->max_nb_message,
       k_arg->direction,
       k_arg->discipline,
       id);
}


/*
 * Receive message from the queuing port.
 *
 * On success return EOK or INVALID_CONFIG if some message has been lost
 * in the port because of overflow.
 *
 * Possible error codes:
 *
 *  - EINVAL - id is incorrect
 *  - EFAULT - 'timeout', 'data' or 'len' refers to inaccessible memory
 *  - JET_INVALID_MODE_TARGET - attempt to read from OUT port.
 *  - EAGAIN - no message in the port message queue and timeout is 0.
 *  - JET_INVALID_MODE
 *     -- timeout is non-zero, waiting is required, but it is not allowed in given mode.
 *     -- [IPPC] timeout is non-zero, waiting is required, but thread is an IPPC handler.
 *  - ETIMEDOUT - timeout has been expired while waiting message in the port message queue.
 */
jet_ret_t pok_port_queuing_receive(
    pok_port_id_t               id,
    const pok_time_t* __user    timeout,
    void* __user                data,
    pok_port_size_t* __user     len)
{
    pok_port_queuing_t* port_queuing;
    jet_ret_t ret;
    pok_thread_t* t;

    port_queuing = get_port_queuing(id);
    if(!port_queuing) return EINVAL;

    void* __kuser k_data = jet_user_to_kernel(data, port_queuing->channel->max_message_size);
    if(!k_data) return EFAULT;

    pok_port_size_t* __kuser k_len = jet_user_to_kernel_typed(len);
    if(!k_len) return EFAULT;

    const pok_time_t* __kuser k_timeout = jet_user_to_kernel_typed_ro(timeout);
    if(!k_timeout) return EFAULT;
    pok_time_t kernel_timeout = *k_timeout;

    if(port_queuing->direction != POK_PORT_DIRECTION_IN)
        return JET_INVALID_MODE_TARGET;

    pok_preemption_local_disable();

    t = current_thread;

    /*
     * We need to specify notification flag when trying get message
     * from the channel.
     *
     * One possible way is to not specify flag first time, and,
     * if receive fails, calculate flag for the second receive attempt.
     *
     * But here we calculate flag for the first (and the only) receive attempt.
     *
     * If ret is not EOK, it contains error code to be returned
     * if channel is currently empty. Otherwise waiting is allowed.
     */

    if(kernel_timeout == 0)
        ret = EAGAIN;
    else if(!thread_is_waiting_allowed())
        ret = JET_INVALID_MODE;
    else if(t->ippc_server_connection)
        ret = JET_INVALID_MODE; // Server threads may not wait on ports.
    else
        ret = EOK;

    pok_message_size_t message_size; // Only for call r_get_message().
    if(!pok_thread_wq_is_empty(&port_queuing->waiters) ||
        !pok_channel_queuing_r_get_message(port_queuing->channel,
            &message_size,
            ret == EOK))
    {
        if(ret)
        {
            // Waiting is not allowed, but it is needed.
            *k_len = 0;
            goto err;
        }

        // Prepare to wait.
        t->wait_buffer.dest = k_data;

        pok_thread_wq_add_common(&port_queuing->waiters, t,
            port_queuing->discipline);

        thread_wait_common(t, kernel_timeout);

        goto out;
    }

    /* Message is ready in the buffer. */
    t->wait_buffer.dest = k_data;
    port_queuing_receive(port_queuing, t);

out:
    pok_preemption_local_enable(); // Possible wait here

    *k_len = (t->wait_result == EOK || t->wait_result == JET_INVALID_CONFIG)?
        t->wait_len: // Success.
        0; // Fail

    return t->wait_result;

err:
    pok_preemption_local_enable();

    return ret;
}


/*
 * Send message into queuing port.
 *
 * Return EOF on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- 'id' is incorrect.
 *    -- 'len' is 0
 *  - JET_INVALID_MODE_TARGET - attempt to send message into IN port.
 *  - JET_INVALID_CONFIG - 'len' is more than max_message_size for given port.
 *  - EFAULT - 'data' or 'timeout' points to inaccessible memory.
 *  - EAGAIN - no space in the port message queue and timeout is 0.
 *  - JET_INVALID_MODE
 *     -- timeout is non-zero, waiting is required, but it is not allowed in given mode.
 *     -- [IPPC] timeout is non-zero, waiting is required, but thread is an IPPC handler.
 *  - ETIMEDOUT - timeout has been expired while waiting for space in the port message queue.
 */
jet_ret_t pok_port_queuing_send(
    pok_port_id_t               id,
    const void* __user          data,
    pok_port_size_t             len,
    const pok_time_t* __user    timeout)
{
    pok_port_queuing_t* port_queuing;
    jet_ret_t ret;
    pok_thread_t* t = current_thread;

    port_queuing = get_port_queuing(id);

    if(!port_queuing) return EINVAL;

    if(port_queuing->direction != POK_PORT_DIRECTION_OUT)
        return JET_INVALID_MODE_TARGET;

    if(len == 0) return EINVAL;

    if(len > port_queuing->channel->max_message_size)
        return JET_INVALID_CONFIG;

    const void* __kuser k_data = jet_user_to_kernel_ro(data, len);
    if(!k_data) return EFAULT;

    const pok_time_t* __kuser k_timeout = jet_user_to_kernel_typed_ro(timeout);
    if(!k_timeout) return EFAULT;

    pok_time_t kernel_timeout = *k_timeout;

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
     * If ret is not EOK, it contains error code to be returned
     * if channel is currently full. Otherwise waiting is allowed.
     */

    if(kernel_timeout == 0)
        ret = EAGAIN;
    else if(!thread_is_waiting_allowed())
        ret = JET_INVALID_MODE;
    else if(t->ippc_server_connection)
        ret = JET_INVALID_MODE; // Server threads may not wait on ports.
    else
        ret = EOK;

    if(!pok_thread_wq_is_empty(&port_queuing->waiters)
        || !pok_channel_queuing_s_get_message(port_queuing->channel, ret == EOK))
    {
        if(ret)
        {
            // Waiting is not allowed, but it is needed.
            goto err;
        }

        // Prepare to wait.
        t->wait_len = len;
        t->wait_buffer.src = k_data;

        pok_thread_wq_add_common(&port_queuing->waiters, t,
            port_queuing->discipline);

        thread_wait_common(t, kernel_timeout);

        goto out;
    }
    /* There is place for message in the buffer. */
    t->wait_len = len;
    t->wait_buffer.src = k_data;

    port_queuing_send(port_queuing, t);

out:
    pok_preemption_local_enable(); // Possible wait here

    return t->wait_result;

err:
    pok_preemption_local_enable();

    return ret;
}


/*
 * Extract status of the queuing port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'status' points to inaccessible memory.
 *  - EINVAL - 'id' is incorrect.
 */
jet_ret_t pok_port_queuing_status(
    pok_port_id_t               id,
    pok_port_queuing_status_t * __user status)
{
    pok_port_queuing_t* port_queuing;
    pok_channel_queuing_t* channel;

    pok_port_queuing_status_t* __kuser k_status = jet_user_to_kernel_typed(status);
    if(!k_status) return EFAULT;

    port_queuing = get_port_queuing(id);

    if(!port_queuing) return EINVAL;

    channel = port_queuing->channel;

    k_status->max_message_size = channel->max_message_size;
    k_status->direction = port_queuing->direction;

    pok_preemption_local_disable();

    k_status->waiting_processes = pok_thread_wq_get_nwaits(&port_queuing->waiters);

    if(port_queuing->direction == POK_PORT_DIRECTION_IN) {
        k_status->max_nb_message = channel->recv.max_nb_message;
        k_status->nb_message = pok_channel_queuing_r_n_messages(channel);
    }
    else {
        /* port_queuing->direction == POK_PORT_DIRECTION_OUT */
        k_status->max_nb_message = channel->send.max_nb_message;
        k_status->nb_message = pok_channel_queuing_s_n_messages(channel);
    }

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Find queuing port by name.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'name' or 'id' points to inaccessible memory.
 *  - JET_INVALID_CONFIG - 'name' doesn't corresponds to create queuing port.
 */
jet_ret_t pok_port_queuing_id(
    const char* __user name,
    pok_port_id_t* __user id)
{
    pok_port_queuing_t* port_queuing;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    pok_port_id_t* __kuser k_id = jet_user_to_kernel_typed(id);
    if(!k_id) return EFAULT;

    port_queuing = find_port_queuing(k_name);

    if(!port_queuing) return JET_INVALID_CONFIG;
    if(!port_queuing->is_created) return JET_INVALID_CONFIG;

    *k_id = port_queuing - current_partition_arinc->ports_queuing;

    return EOK;
}

/*
 * Clear queuing port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL - 'id' is incorrect.
 *  - JET_INVALID_MODE_TARGET - port is OUT.
 */
jet_ret_t pok_port_queuing_clear(pok_port_id_t id)
{
    pok_port_queuing_t* port_queuing = get_port_queuing(id);

    if(!port_queuing) return EINVAL;

    if(port_queuing->direction != POK_PORT_DIRECTION_IN)
        return JET_INVALID_MODE_TARGET;

    pok_preemption_local_disable();
    pok_channel_queuing_side_init(port_queuing->channel,
            &port_queuing->channel->recv,
            port_queuing - current_partition_arinc->ports_queuing);
    pok_preemption_local_enable();

    return EOK;
}

/**********************************************************************/
/*
 * Find *configured* sampling port by name, which comes from user space.
 */
static pok_port_sampling_t* find_port_sampling(const char* __kuser k_name)
{
    char kernel_name[MAX_NAME_LENGTH];

    pok_port_sampling_t* port_sampling =
        current_partition_arinc->ports_sampling;

    pok_port_sampling_t* ports_sampling_end =
        port_sampling + current_partition_arinc->nports_sampling;

    memcpy(kernel_name, k_name, MAX_NAME_LENGTH);

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

    if(id > current_partition_arinc->nports_sampling) return NULL;

    port_sampling = &current_partition_arinc->ports_sampling[id];

    return port_sampling->is_created ? port_sampling : NULL;
}

void pok_port_sampling_init(pok_port_sampling_t* port_sampling)
{
    port_sampling->is_created = FALSE;
}


/*
 * Create sampling port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - JET_INVALID_MODE - mode is NORMAL
 *  - EFAULT - 'name', 'refresh' or 'id' points to inaccessible memory
 *  - JET_INVALID_CONFIG
 *    -- there is not sampling port with given name
 *    -- 'direction' is invalid or doesn't correspond to configuration one
 *    -- 'size' is invalid or doesn't correspond to configuration one
 *  - EINVAL - refresh period is not positive.
 *  - EEXIST - port with given name is already created.
 */
jet_ret_t pok_port_sampling_create(
    const char* __user          name,
    pok_port_size_t             size,
    pok_port_direction_t        direction,
    const pok_time_t* __user    refresh,
    pok_port_id_t* __user       id
)
{
    pok_port_sampling_t* port_sampling;

    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL)
        return JET_INVALID_MODE;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    const pok_time_t* __kuser k_refresh = jet_user_to_kernel_typed_ro(refresh);
    if(!k_refresh) return EFAULT;

    pok_port_id_t* __kuser k_id = jet_user_to_kernel_typed(id);
    if(!k_id) return EFAULT;

    pok_time_t kernel_refresh = *k_refresh;

    port_sampling = find_port_sampling(k_name);

    if(!port_sampling) return JET_INVALID_CONFIG;
    if(port_sampling->is_created) return EEXIST;

    if(size != port_sampling->channel->max_message_size)
        return JET_INVALID_CONFIG;
    if(direction != port_sampling->direction)
        return JET_INVALID_CONFIG;
    // ARINC specifies to check refresh period for any direction.
    if(pok_time_is_infinity(kernel_refresh) || kernel_refresh == 0)
        return EINVAL;

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

    *k_id = port_sampling - current_partition_arinc->ports_sampling;

    return EOK;
}

/*
 * Write message into sampling port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL
 *    -- 'id' is incorrect
 *    -- len is 0
 *  - JET_INVALID_CONFIG - 'len' is more than max_message_size.
 *  - JET_INVALID_MODE_TARGET - attempt to write into IN port
 *  - EFAULT - 'data' points to inaccessible memory.
 */
jet_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void* __user      data,
    pok_port_size_t         len)
{
    pok_port_sampling_t* port_sampling;

    char* message;

    port_sampling = get_port_sampling(id);

    if(!port_sampling) return EINVAL;

    if(len == 0) return EINVAL;
    if(len > port_sampling->channel->max_message_size) return JET_INVALID_CONFIG;

    if(port_sampling->direction != POK_PORT_DIRECTION_OUT)
        return JET_INVALID_MODE_TARGET;

    const void* __kuser k_data = jet_user_to_kernel_ro(data, len);
    if(!k_data) return EFAULT;

    pok_preemption_local_disable();

    message = pok_channel_sampling_s_get_message(port_sampling->channel);

    memcpy(message, k_data, len);

    pok_channel_sampling_send_message(port_sampling->channel, len);

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Read sampling message.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL - 'id' is incorrect
 *  - EFAULT - 'data', 'len' or 'valid' points to inaccessible memory.
 *  - JET_INVALID_MODE_TARGET - attempt to read from OUT port.
 *  - EAGAIN - no message in the port.
 */
jet_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void* __user            data,
    pok_port_size_t* __user len,
    pok_bool_t* __user      valid)
{
    pok_port_sampling_t* port_sampling;
    jet_ret_t ret;
    pok_time_t ts;

    const char* message;
    pok_message_size_t message_size;

    port_sampling = get_port_sampling(id);

    if(!port_sampling) return EINVAL;

    if(port_sampling->direction != POK_PORT_DIRECTION_IN)
        return JET_INVALID_MODE_TARGET;

    void* __kuser k_data = jet_user_to_kernel(data, port_sampling->channel->max_message_size);
    if(!k_data) return EFAULT;

    pok_port_size_t* __kuser k_len = jet_user_to_kernel_typed(len);
    if(!k_len) return EFAULT;

    pok_bool_t* __kuser k_valid = jet_user_to_kernel_typed(valid);
    if(!k_len) return EFAULT;

    pok_preemption_local_disable();

    message = pok_channel_sampling_r_get_message(port_sampling->channel,
        &message_size, &ts);

    if(message)
    {
        memcpy(k_data, message, message_size);
        *k_len = (pok_port_size_t)message_size;

        pok_time_t current_time = jet_system_time();
        port_sampling->last_message_validity =
            ((ts + port_sampling->refresh_period) >= current_time)
            ? TRUE: FALSE;
        ret = EOK;
    }
    else
    {
        *k_len = 0;
        port_sampling->last_message_validity = FALSE;
        ret = EAGAIN;
    }

    *k_valid = port_sampling->last_message_validity;

    pok_preemption_local_enable();

    return ret;
}

/*
 * Find sampling port by name.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EFAULT - 'name' or 'id' points to inaccessible memory.
 *  - JET_INVALID_CONFIG - there is no port with given name created.
 */
jet_ret_t pok_port_sampling_id(
    const char* __user     name,
    pok_port_id_t* __user  id
)
{
    pok_port_sampling_t* port_sampling;

    const char* __kuser k_name = jet_user_to_kernel_ro(name, MAX_NAME_LENGTH);
    if(!k_name) return EFAULT;

    pok_port_id_t* __kuser k_id = jet_user_to_kernel_typed(id);
    if(!k_id) return EFAULT;

    port_sampling = find_port_sampling(k_name);

    if(!port_sampling) return JET_INVALID_CONFIG;
    if(!port_sampling->is_created) return JET_INVALID_CONFIG;

    *k_id = port_sampling - current_partition_arinc->ports_sampling;

    return EOK;
}

/*
 * Extract status of the given sampling port.
 *
 * Return EOK on success.
 *
 * Possible error codes:
 *
 *  - EINVAL - 'id' is invalid
 *  - EFAULT - 'status' points to inaccessible memory.
 */
jet_ret_t pok_port_sampling_status (
    pok_port_id_t                       id,
    pok_port_sampling_status_t* __user  status
)
{
    pok_port_sampling_t* port_sampling;

    port_sampling = get_port_sampling(id);

    if(!port_sampling) return EINVAL;

    pok_port_sampling_status_t* __kuser k_status = jet_user_to_kernel_typed(status);
    if(!k_status) return EFAULT;

    pok_preemption_local_disable();

    k_status->size = port_sampling->channel->max_message_size;
    k_status->direction = port_sampling->direction;
    k_status->refresh = port_sampling->refresh_period;
    k_status->validity = port_sampling->last_message_validity;

    pok_preemption_local_enable();

    return EOK;
}

/*
 * Check whether new message in the port exist.
 *
 * Return EOK if message exist, EAGAIN if not.
 *
 * Possible error codes:
 *
 *  - EINVAL - 'id' is incorrect.
 *
 * TODO: This should be transformed into READ_UPDATED_SAMPLING_MESSAGE eventually.
 */
jet_ret_t pok_port_sampling_check(pok_port_id_t id)
{
    jet_ret_t ret;

    pok_port_sampling_t* port_sampling;

    port_sampling = get_port_sampling(id);

    if(!port_sampling) return EINVAL;

    if(port_sampling->direction != POK_PORT_DIRECTION_IN)
        return JET_INVALID_MODE_TARGET;


    pok_preemption_local_disable();
    ret = pok_channel_sampling_r_check_new_message(port_sampling->channel)
        ? EOK
        : EAGAIN;
    pok_preemption_local_enable();

    return ret;
}
