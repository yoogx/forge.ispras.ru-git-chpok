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

#include <core/channel.h>
#include <types.h>
#include <assert.h>
#include <core/time.h>

#include <core/sched.h>
#include <alloc.h>

/*********************** Queuing channel ******************************/

void pok_channel_queuing_init(pok_channel_queuing_t* channel)
{
    channel->max_nb_message =
        channel->max_nb_message_receive + channel->max_nb_message_send;
    
    channel->r_start = 0;
    channel->border = 0;
    channel->s_end = 0;
    
    channel->message_stride = POK_MESSAGE_STRUCT_SIZE(channel->max_message_size);
    
    channel->buffer = jet_mem_alloc(
        channel->message_stride * channel->max_nb_message);
    
    channel->notify_receiver = FALSE;
    channel->notify_sender = FALSE;
    
    channel->sender_generation = 0;
    channel->receiver_generation = 0;
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

/* 
 * Helper: notify sender if requested.
 * 
 * Should be executed with preemption disabled.
 * 
 * Should be called only after consuming messages from the sender.
 * That is, sender is initialized at this stage.
 */
static inline void channel_queuing_notify_sender(pok_channel_queuing_t* channel)
{
    if(channel->notify_sender)
    {
        channel->notify_sender = FALSE;
        channel->sender->state.bytes.outer_notification = 1;
    }
}

void pok_channel_queuing_r_init(pok_channel_queuing_t* channel)
{
    pok_preemption_disable();
    channel->r_start = channel->border;
    channel->notify_receiver = FALSE;
    channel->receiver_generation = channel->receiver->partition_generation;

    __pok_preemption_enable();
}

size_t pok_channel_queuing_r_n_messages(pok_channel_queuing_t* channel)
{
    // Both boundaries are modified by receiver itself. No needs critical section.
    return channel_queuing_cyclic_sub(channel,
        channel->border,
        channel->r_start);
}

pok_message_t* pok_channel_queuing_r_get_message(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe)
{
    pok_message_t* msg;

    if(!subscribe)
    {
        // fastpath without locks. TODO: revisit
        return (channel->border != channel->r_start)
            ? channel_queuing_message_at(channel, channel->r_start)
            : NULL;
    }
    
    pok_preemption_disable();
    

    if(channel->border != channel->r_start)
    {
        msg = channel_queuing_message_at(channel, channel->r_start);
    }
    else
    {
        msg = NULL;
        channel->notify_receiver = TRUE;
    }

    __pok_preemption_enable();
    
    return msg;
}


void pok_channel_queuing_r_consume_message(
    pok_channel_queuing_t* channel)
{
    assert(channel->r_start != channel->border);
    
    pok_preemption_disable();
    
    channel->r_start = channel_queuing_cyclic_add(channel,
        channel->r_start, 1);
    
    if(channel->sender->partition_generation == channel->sender_generation
        && channel->border != channel->s_end)
    {
        channel->border = channel_queuing_cyclic_add(channel,
            channel->border, 1);
        channel_queuing_notify_sender(channel);
    }
    
    __pok_preemption_enable();
}

void pok_channel_queuing_s_init(pok_channel_queuing_t* channel)
{
    pok_preemption_disable();
    channel->s_end = channel->border;
    channel->notify_sender = FALSE;
    channel->sender_generation = channel->sender->partition_generation;

    __pok_preemption_enable();
}

pok_message_t* pok_channel_queuing_s_get_message(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe)
{
    pok_message_range_t n_used;
    pok_message_t* msg = NULL;

    pok_preemption_disable();

    n_used = channel_queuing_cyclic_sub(channel,
        channel->s_end, ACCESS_ONCE(channel->border));

    if(n_used < channel->max_nb_message_send)
    {
        msg = channel_queuing_message_at(channel, channel->s_end);
    }
    else if(subscribe)
    {
        channel->notify_sender = TRUE;
    }

    __pok_preemption_enable();
    
    return msg;
}

void pok_channel_queuing_s_produce_message(
    pok_channel_queuing_t* channel)
{
    assert(channel_queuing_cyclic_sub(channel, channel->s_end,
        ACCESS_ONCE(channel->border)) < channel->max_nb_message_send);
    
    assert(channel_queuing_message_at(channel, channel->s_end)->size > 0);

    assert(channel_queuing_message_at(channel, channel->s_end)->size
        <= channel->max_message_size);

    pok_preemption_disable();
    
    channel->s_end = channel_queuing_cyclic_add(channel, channel->s_end, 1);
    
    if(channel->receiver->partition_generation == channel->receiver_generation)
    {
        // Receiver is ready.
        pok_message_range_t nb_message_receive = channel_queuing_cyclic_sub(
            channel, channel->border, channel->r_start);
        
        if(nb_message_receive < channel->max_nb_message_receive)
        {
            // Move message to the receiver buffer
            channel->border = channel->s_end;
            // And notify receiver, if requested.
            if(channel->notify_receiver)
            {
                channel->notify_receiver = FALSE;
                channel->receiver->state.bytes.outer_notification = 1;
            }
        }
    }
    else
    {
        // Just drop the message and all previous ones which have been received.
        channel->border = channel->s_end;
    }

    __pok_preemption_enable();
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
    
    channel->buffer = jet_mem_alloc(3 * channel->message_stride);
    
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
