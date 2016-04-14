#include <core/channel.h>
#include <types.h>
#include <arch.h>
#include <assert.h>
#include <core/time.h>

#include <core/sched.h>

/*********************** Queuing channel ******************************/

void pok_channel_queuing_init(pok_channel_queuing_t* channel)
{
    channel->max_nb_message =
        channel->max_nb_message_receive + channel->max_nb_message_send;
    
    channel->r_start = 0;
    channel->border = 0;
    channel->s_end = 0;
    
    channel->message_stride = POK_MESSAGE_STRUCT_SIZE(channel->max_message_size);
    
    channel->buffer = pok_bsp_mem_alloc(
        channel->message_stride * channel->max_nb_message);
    
    channel->notify_receiver = FALSE;
    channel->notify_sender = FALSE;
    
    channel->sender = NULL;
    channel->receiver = NULL;
}

/*
 * Helper: Add index to given buffer's position in the channel.
 * 
 * Take into account cyclic nature of the channel's buffer.
 */
static inline
uint16_t channel_queuing_cyclic_add(pok_channel_queuing_t* channel,
    uint16_t pos, uint16_t index)
{
    uint16_t sum = pos + index;
    if(sum >= channel->max_nb_message) sum -= channel->max_nb_message;
    
    return sum;
}

/*
 * Helper: Get number of messages between given buffer's positions.
 * 
 * Take into account cyclic nature of the channel's buffer.
 */
static inline
pok_message_range_t channel_queuing_cyclic_sub(pok_channel_queuing_t* channel,
    pok_message_range_t pos_end, pok_message_range_t pos_start)
{
    return pos_end >= pos_start
        ? pos_end - pos_start
        : pos_end + channel->max_nb_message - pos_start;
}

/* Helper: Return message in the buffer at given index. */
static inline pok_message_t* channel_queuing_message_at(
    pok_channel_queuing_t* channel, pok_message_range_t pos)
{
    assert(pos < channel->max_nb_message);
    
    return (pok_message_t*)&channel->buffer[channel->message_stride * pos];
}


void pok_channel_queuing_set_receiver(pok_channel_queuing_t* channel,
    pok_channel_queuing_reciever_t* receiver)
{
    barrier();
    channel->receiver = receiver;
}

size_t pok_channel_queuing_r_n_messages(pok_channel_queuing_t* channel)
{
    // Both boundaries are modified by receiver itself. No need ACCESS_ONCE().
    return channel_queuing_cyclic_sub(channel,
        channel->border,
        channel->r_start);
}

pok_message_t* pok_channel_queuing_r_get_message(
    pok_channel_queuing_t* channel,
    pok_message_range_t index)
{
    pok_message_range_t pos;
    assert(index < pok_channel_queuing_r_n_messages(channel));
    
    pos = channel_queuing_cyclic_add(channel, channel->r_start, index);
    
    return channel_queuing_message_at(channel, pos);
}


void pok_channel_queuing_r_consume_messages(
    pok_channel_queuing_t* channel,
    pok_message_range_t n)
{
    assert(n < pok_channel_queuing_r_n_messages(channel));
    
    barrier();
    channel->r_start = channel_queuing_cyclic_add(channel,
        channel->r_start, n); //Release semantic
}

pok_message_range_t pok_channel_queuing_receive(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe)
{
    pok_message_range_t n, n_available;
    
    /* 
     * Because temporal subscription may be needed, we need
     * to rollback it on success, but only when subscription
     * initially was FALSE.
     */
    pok_bool_t unsubscribe_on_success = FALSE;
    
    assert(!pok_channel_queuing_r_n_messages(channel));

    /* 
     * Because s_produce() is asyncronous, we need to subscribe before
     * cheking emptiness.
     */
    if(subscribe && !ACCESS_ONCE(channel->notify_receiver)) {
        channel->notify_receiver = TRUE;
        barrier();
        unsubscribe_on_success = TRUE;
    }
    
    n_available = channel_queuing_cyclic_sub(channel,
        ACCESS_ONCE(channel->s_end), channel->border);
    
    if(n_available)
    {
        // Reverting temporal subscription doesn't require barriers.
        if(unsubscribe_on_success) channel->notify_receiver = FALSE;
        
        n = n_available > channel->max_nb_message_receive
            ? channel->max_nb_message_receive
            : n_available;
        
        channel->border = channel_queuing_cyclic_add(channel,
            channel->border, n); // Acquire semantic

        barrier();
        
        if(channel->notify_sender)
        {
            ACCESS_ONCE(channel->notify_sender) = FALSE;
            channel->sender->on_message_received(channel->sender);
        }
    }
    else
    {
        n = 0;
    }
    
    return n;
}

void pok_channel_queuing_set_sender(pok_channel_queuing_t* channel,
    pok_channel_queuing_sender_t* sender)
{
    barrier();
    channel->sender = sender;
}

/* 
 * Return pointer to the message for being filled at sender side.
 * 
 * Return NULL if no space is left in the sender buffer.
 * 
 * If there is no space in the sender buffer and @subscribe parameter
 * is TRUE, subscribe to notifications when the space will be released.
 */
pok_message_t* pok_channel_queuing_s_get_message(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe)
{
    pok_message_range_t n_used;
    /* 
     * Because temporal subscription may be needed, we need
     * to rollback it on success, but only when subscription
     * initially was FALSE.
     */
    pok_bool_t unsubscribe_on_success = FALSE;
    
    /* 
     * Because s_produce() is asyncronous, we need to subscribe before
     * cheking emptiness.
     */
    if(subscribe && !ACCESS_ONCE(channel->notify_sender)) {
        channel->notify_sender = TRUE;
        barrier();
        unsubscribe_on_success = TRUE;
    }
    
    n_used = channel_queuing_cyclic_sub(channel,
        channel->s_end, ACCESS_ONCE(channel->border));
    
    if(n_used < channel->max_nb_message_send)
    {
        // Reverting temporal subscription doesn't require barriers.
        if(unsubscribe_on_success) channel->notify_sender = FALSE;
        
        return channel_queuing_message_at(channel, channel->s_end);
    }
    
    return NULL;
}

void pok_channel_queuing_s_produce_message(
    pok_channel_queuing_t* channel)
{
    assert(channel_queuing_cyclic_sub(channel, channel->s_end,
        ACCESS_ONCE(channel->border)) < channel->max_nb_message_send);
    
    assert(channel_queuing_message_at(channel, channel->s_end)->size
        <= channel->max_message_size);
    
    barrier();
    channel->s_end = channel_queuing_cyclic_add(channel, channel->s_end, 1);
    
    if(ACCESS_ONCE(channel->notify_receiver))
    {
        ACCESS_ONCE(channel->notify_receiver) = FALSE;
        
        barrier();
        channel->receiver->on_message_sent(channel->receiver);
    }
}

pok_message_range_t pok_channel_queuing_s_n_messages(pok_channel_queuing_t* channel)
{
    return channel_queuing_cyclic_sub(channel, channel->s_end,
        ACCESS_ONCE(channel->border));
}

/*********************** Sampling channel *****************************/

void pok_channel_sampling_init(pok_channel_sampling_t* channel)
{
    channel->message_stride = POK_MESSAGE_STRUCT_SIZE(channel->max_message_size);
    
    channel->buffer = pok_bsp_mem_alloc(3 * channel->message_stride);
    
    channel->read_pos_next = 0;
    channel->read_pos = 0;
    
    // Mark the first message as empty.
    ((pok_message_t*)channel->buffer)->size = 0;
    
    channel->write_pos = 1;
}

static pok_message_t* channel_sampling_message_at(
    pok_channel_sampling_t* channel, int pos)
{
    assert(pos <= 2);
    
    return (pok_message_t*)&channel->buffer[pos * channel->message_stride];
}
    

/*
 * Get pointer to the message for read it.
 * 
 * `timestamp` will be filled with time, when message has been sent.
 * 
 * If there is no message available for read, return NULL.
 * 
 * NOTE: Every new function's call invalidates previous value.
 * Using this semantic, receiver doesn't need to mark message as consumed.
 */
pok_message_t* pok_channel_sampling_r_get_message(
    pok_channel_sampling_t* channel, pok_time_t* timestamp)
{
    pok_message_t* m;
    uint8_t read_pos;
    
    pok_preemption_disable();
    read_pos = channel->read_pos = channel->read_pos_next;
    __pok_preemption_enable();
    
    m = channel_sampling_message_at(channel, read_pos);
    
    if(m->size)
    {
        *timestamp = channel->timestamps[read_pos];
        return m;
    }
    
    return NULL;
}

void pok_channel_sampling_r_clear_message(pok_channel_sampling_t* channel)
{
    pok_preemption_disable();
    channel_sampling_message_at(channel, channel->read_pos)->size = 0;
    __pok_preemption_enable();
}

pok_bool_t pok_channel_sampling_r_check_new_message(pok_channel_sampling_t* channel)
{
    pok_bool_t ret = FALSE;

    pok_preemption_disable();
    if(channel->read_pos != channel->read_pos_next)
    {
        ret = TRUE;
        // TODO: This mark message as consumed. Do we need that?
        channel->read_pos = channel->read_pos_next;
    }
    __pok_preemption_enable();

    return ret;
}

pok_message_t* pok_channel_sampling_s_get_message(
    pok_channel_sampling_t* channel)
{
    return channel_sampling_message_at(channel, channel->write_pos);
}

void pok_channel_sampling_send_message(
    pok_channel_sampling_t* channel)
{
    uint8_t read_pos_next;

    assert(channel_sampling_message_at(channel, channel->write_pos)->size);
    
    pok_preemption_disable();
    channel->read_pos_next = read_pos_next = channel->write_pos;
    channel->timestamps[read_pos_next] = POK_GETTICK();

    /*
     * The only choice for `write_pos`, which can differ from both
     * `read_pos` and `read_pos_next`.
     * 
     * (read_pos + read_pos_next + write_pos = 0 + 1 + 2 = 3).
     */
    channel->write_pos = 3 - channel->read_pos - read_pos_next;
    __pok_preemption_enable();
}

void pok_channel_sampling_s_clear_message(pok_channel_sampling_t* channel)
{
    pok_preemption_disable();
    channel->read_pos_next = channel->read_pos;
    __pok_preemption_enable();
}

/**********************************************************************/
void pok_channels_init_all(void)
{
    int i;
    
    for(i = 0; i < pok_channels_queuing_n; i++)
    {
        pok_channel_queuing_init(&pok_channels_queuing[i]);
    }

    for(i = 0; i < pok_channels_sampling_n; i++)
    {
        pok_channel_sampling_init(&pok_channels_sampling[i]);
    }

}
