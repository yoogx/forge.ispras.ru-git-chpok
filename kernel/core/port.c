#include <core/port.h>

/* 
 * Find *configured* queuing port by name, which comes from user space.
 * 
 * Should be called with local preemption disabled.
 * 
 * NOTE: Name should be checked before!
 */
static pok_port_queuing_t* find_port_queuing(const char* __user name)
{
    char name_kernel[MAX_NAME_LENGTH];
    
    pok_port_queuing_t* port_queuing =
        current_partition_arinc->ports_queuing;
        
    pok_port_queuing_t* ports_queuing_end =
        ports_queuing + current_partition_arinc->nports_queuing;
        
    __copy_from_user(name, name_kernel, MAX_NAME_LENGTH);
    
    for(; port_queuing < ports_queuing_end; port_queuing++)
    {
        if(!pok_compare_names(port_queuing->name, name_kernel))
            return port_queuing;
    }
    
    return NULL;
}


/* 
 * Get *created* queuing port by id.
 * 
 * Should be called with local preemption disabled.
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
    
    __copy_to_user(m->content, data, m->size);
    t->wait_private = (void*)(unsigned long)m->size;
    
    pok_channel_queuing_r_consume_messages(port->channel, 1);
}

void port_queuing_send(pok_port_queuing_t* port, pok_thread_t* t)
{
    pok_message_send_t* m_send = t->wait_private;
    pok_message_t* m = pok_channel_queuing_s_get_message(port->channel, FALSE);
    
    __copy_from_user(m_send->data, m->content, m_send->len);
    t->wait_private = (void*)(unsigned long)m->size;
    
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

static void port_on_message_sent(pok_channel_queuing_sender_t* sender)
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
    pok_port_size_t                 max_nb_messages,
    pok_port_direction_t            direction,
    pok_port_queueing_discipline_t  discipline,
    pok_port_id_t* __user           id)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    
    if(!check_access_read(name, MAX_NAME_LENGTH))
        return POK_ERRNO_EFAULT;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    port_queuing = find_port_queuing(name);
    
    ret = POK_ERRNO_UNAVAILABLE;
    if(!port_queuing) goto out;
    
    ret = POK_ERRNO_EXISTS;
    if(port_queuing->is_created) goto out;
    
    ret = POK_ERRNO_EINVAL;
    
    if(message_size != port_queuing->channel->max_message_size) goto out;
    if(max_nb_messages != port_queuing->channel->max_nb_messages) goto out;
    if(direction != port_queuing->direction) goto out;
    if(discipline != port_queuing->discipline) goto out;
    
    ret = POK_ERRNO_MODE;
    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL) goto out;

    port_queuing->is_created = TRUE;
    port_queuing->is_notified = FALSE;
    port_queuing->partition = current_partition_arinc;
    
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
    
    ret = POK_ERRNO_OK;
    
out:
    pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_port_queuing_receive(
    pok_port_id_t           id, 
    pok_time_t              timeout, 
    pok_port_size_t         maxlen, 
    void* __user            data, 
    pok_port_size_t* __user len)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    pok_thread_t* t;

    long wait_result;
    pok_message_t* message;
    
    pok_preemption_local_disable();
    
    port_queuing = get_port_queuing(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_queuing) goto err;
    
    ret = POK_ERRNO_EFAULT;
    if(!check_access_write(data, port_queuing->channel->max_message_size)) goto err;
    if(!check_user_write(len)) goto err;
    
    ret = POK_ERRNO_MODE;
    if(port_queuing->direction != POK_PORT_DIRECTION_IN) goto err;

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

        if(timeout == 0)
            ret = POK_ERRNO_EMPTY;
        else if(!is_waiting_allowed())
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
            
            if(port_queuing->discipline == POK_QUEUEING_DISCIPLINE_FIFO)
                pok_thread_wq_add(port_queuing->waiters, t);
            else
                pok_thread_wq_add_prio(port_queuing->waiters, t);
            
            if(!pok_time_is_infinity(timeout))
                thread_wait_timed(t, POK_GETTICK() + timeout);
            else
                thread_wait(t);
            
            goto out;
        }
    }
    /* Message is ready in the buffer. */
    t->wait_private = data;
    port_queuing_receive(port_queuing, t);

out:
    pok_preemption_local_enable(); // Possible wait here

    // Decode result in t->wait_private.
    wait_result = t->wait_private;
    
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
    pok_port_id_t       id, 
    const void* __user  data,
    pok_port_size_t     len,
    pok_time_t          timeout)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    pok_thread_t* t;

    long wait_result;
    struct pok_message_send_t message_send;
    
    pok_preemption_local_disable();
    
    port_queuing = get_port_queuing(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_queuing) goto err;
    
    ret = POK_ERRNO_EINVAL;
    if(len == 0) goto out;
    
    // error should be INVALID_CONFIG
    if(len > port_queuing->channel->max_message_size) goto err;
    
    ret = POK_ERRNO_EFAULT;
    if(!check_access_read(data, len)) goto err;
    
    ret = POK_ERRNO_MODE;
    if(port_queuing->direction != POK_PORT_DIRECTION_OUT) goto err;
    
    t = current_thread;
    
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

    if(timeout == 0)
        ret = POK_ERRNO_EMPTY;
    else if(!is_waiting_allowed())
        ret = POK_ERRNO_MODE;
    else
        ret = POK_ERRNO_OK;

    if(!pok_thread_wq_is_empty(&port_queuing->waiters)
        || !pok_channel_queuing_s_get_message(port_queuing->channel))
    {
        if(ret)
        {
            // Waiting is not allowed, but it is needed.
            __put_user(len, 0);
            goto err;
        }
        
        // Prepare to wait.
        message_send.size = len;
        message_send.data = data;
        
        t->wait_private = &message_send;
        
        if(port_queuing->discipline == POK_QUEUEING_DISCIPLINE_FIFO)
            pok_thread_wq_add(port_queuing->waiters, t);
        else
            pok_thread_wq_add_prio(port_queuing->waiters, t);
        
        if(!pok_time_is_infinity(timeout))
            thread_wait_timed(t, POK_GETTICK() + timeout);
        else
            thread_wait(t);
        
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
    wait_result = t->wait_private;
    
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
    pok_port_queueing_status_t * __user status)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    pok_channel_queuing_t* channel;
    
    if(!check_access_write(status, sizeof(status)))
        return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    port_queuing = get_port_queuing(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_queuing) goto out;
    
    channel = port_queuing->channel;
    
    __put_user_f(status, max_message_size, channel->max_message_size);
    __put_user_f(status, direction, port_queuing->direction);
    __put_user_f(status, waiting_processes, pok_thread_wq_get_nwaits(&port_queuing->waiters));
    
    
    if(port_queuing->direction == POK_PORT_DIRECTION_IN) {
        __put_user_f(status, max_nb_message, channel->max_nb_messages_receive);
        __put_user_f(status, nb_message, pok_channel_queuing_r_n_messages(channel));
    }
    else {
        /* port_queuing->direction == POK_PORT_DIRECTION_OUT */
        __put_user_f(status, max_nb_message, channel->max_nb_messages_send);
        __put_user_f(status, nb_message, pok_channel_queuing_s_n_messages(channel));
    }
    
    ret = POK_ERRNO_OK;
    
out:
    pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_port_queuing_id(
    const char* __user name,
    pok_port_id_t* __user id)
{
    pok_port_queuing_t* port_queuing;
    pok_ret_t ret;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();
    
    port_queuing = find_port_queuing(name);
    
    ret = POK_ERRNO_UNAVAILABLE;
    if(!port_queuing) goto out;
    if(!port_queuing->is_created) goto out;
    
    __put_user(id, port_queuing - current_partition_arinc->ports_queuing);
    ret = POK_ERRNO_OK;
out:
    pok_preemption_local_enable();
    
    return ret;
}

/**********************************************************************/
/* 
 * Find *configured* sampling port by name, which comes from user space.
 * 
 * Should be called with local preemption disabled.
 * 
 * NOTE: Name should be checked before!
 */
static pok_port_sampling_t* find_port_sampling(const char* __user name)
{
    char name_kernel[MAX_NAME_LENGTH];
    
    pok_port_sampling_t* port_sampling =
        current_partition_arinc->ports_sampling;
        
    pok_port_sampling_t* ports_sampling_end =
        ports_sampling + current_partition_arinc->nports_sampling;
        
    __copy_from_user(name, name_kernel, MAX_NAME_LENGTH);
    
    for(; port_sampling < ports_sampling_end; port_sampling++)
    {
        if(!pok_compare_names(port_sampling->name, name_kernel))
            return port_sampling;
    }
    
    return NULL;
}


/* 
 * Get *created* sampling port by id.
 * 
 * Should be called with local preemption disabled.
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
    const char*             name,
    pok_port_size_t         size,
    pok_port_direction_t    direction,
    uint64_t                refresh,
    pok_port_id_t           *id
)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;
    
    if(!check_access_read(name, MAX_NAME_LENGTH))
        return POK_ERRNO_EFAULT;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    port_sampling = find_port_sampling(name);
    
    ret = POK_ERRNO_UNAVAILABLE;
    if(!port_sampling) goto out;
    
    ret = POK_ERRNO_EXISTS;
    if(port_sampling->is_created) goto out;
    
    ret = POK_ERRNO_EINVAL;
    
    if(message_size != port_sampling->channel->max_message_size) goto out;
    if(direction != port_sampling->direction) goto out;
    // ARINC specifies to check refresh period for any direction.
    if(pok_time_is_infinity(refresh) || refresh == 0) goto out;

    ret = POK_ERRNO_MODE;
    if(current_partition_arinc->mode == POK_PARTITION_MODE_NORMAL) goto out;

    port_sampling->is_created = TRUE;
    
    // Useless for OUT port.
    port_sampling->refresh_period = refresh;
    port_sampling->last_message_validity = FALSE;
    
    __put_user(id, port_sampling - current_partition_arinc->ports_sampling);
    
    ret = POK_ERRNO_OK;
    
out:
    pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void              *data,
    pok_port_size_t         len
)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;

    pok_message_t* message;
    
    pok_preemption_local_disable();
    
    port_sampling = get_port_sampling(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_sampling) goto out;
    
    ret = POK_ERRNO_EINVAL;
    if(len == 0 || len > port_sampling->channel->max_message_size) goto out;

    ret = POK_ERRNO_MODE;
    if(port_sampling->direction != POK_PORT_DIRECTION_OUT) goto err;
    
    ret = POK_ERRNO_EFAULT;
    if(!check_access_read(data, len)) goto out;

    message = pok_channel_sampling_s_get_message(port_sampling->channel);

    message->size = len;
    __copy_from_user(data, message->content, len);
    
    pok_channel_sampling_send_message(port_sampling->channel);
    
    ret = POK_ERRNO_OK;

out:
    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void                    *data,
    pok_port_size_t         *len,
    bool_t                  *valid
)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;
    pok_time_t ts;

    pok_message_t* message;
    
    pok_preemption_local_disable();
    
    port_sampling = get_port_sampling(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_sampling) goto out;
    
    ret = POK_ERRNO_MODE;
    if(port_sampling->direction != POK_PORT_DIRECTION_IN) goto err;

    ret = POK_ERRNO_EFAULT;
    if(!check_access_write(data, port_sampling->channel->max_message_size)) goto out;
    if(!check_user_write(len)) goto out;
    if(!check_user_write(valid)) goto out;
    
    message = pok_channel_sampling_r_get_message(port_sampling->channel, &ts);
    
    if(message)
    {
        __copy_to_user(message->content, data, message->size);
        __put_user(len, (pok_port_size_t)message->size);
        
        port_sampling->last_message_validity =
            ((ts + port_sampling->refresh_period) <= POK_GETTICK())
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

out:
    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_port_sampling_id(
    const char              *name,
    pok_port_id_t           *id
)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;
    
    if(!check_user_write(id))
        return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();
    
    port_sampling = find_port_sampling(name);
    
    ret = POK_ERRNO_UNAVAILABLE;
    if(!port_sampling) goto out;
    if(!port_sampling->is_created) goto out;
    
    __put_user(id, port_sampling - current_partition_arinc->ports_sampling);
    ret = POK_ERRNO_OK;
out:
    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_port_sampling_status (
    const pok_port_id_t         id,
    pok_port_sampling_status_t  *status
)
{
    pok_port_sampling_t* port_sampling;
    pok_ret_t ret;
    
    if(!check_access_write(status, sizeof(status)))
        return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    port_sampling = get_port_sampling(id);
    
    ret = POK_ERRNO_PORT;
    if(!port_sampling) goto out;
    
    __put_user_f(status, max_message_size, port_sampling->channel->max_message_size);
    __put_user_f(status, direction, port_sampling->direction);
    __put_user_f(status, refresh, port_sampling->refresh_period);
    __put_user_f(status, validity, port_sampling->last_message_validity);
    
    ret = POK_ERRNO_OK;

out:
    pok_preemption_local_enable();

    return ret;
}
