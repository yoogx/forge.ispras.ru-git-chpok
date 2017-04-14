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
        channel->recv.max_nb_message + channel->send.max_nb_message;

    channel->border = channel->recv.next_message = channel->send.next_message = 0;

    const unsigned int message_alignment = __alignof__(int);

    channel->message_stride = ALIGN_VAL(channel->max_message_size,
        message_alignment);

    channel->messages = ja_mem_alloc_aligned(
        channel->message_stride * channel->max_nb_message,
        message_alignment);

    channel->message_sizes = ja_mem_alloc_aligned(
        sizeof(*channel->message_sizes) * channel->max_nb_message,
        __alignof__(*channel->message_sizes));

    channel->send.generation = 0;
    channel->send.is_notify = FALSE;

    channel->recv.generation = 0;
    channel->recv.is_notify = FALSE;

    channel->message_discarded = FALSE;
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
static inline char* channel_queuing_message_at(
    pok_channel_queuing_t* channel, pok_message_range_t pos)
{
    assert(pos < channel->max_nb_message);

    return &channel->messages[channel->message_stride * pos];
}

/* 
 * Helper: notify side of the channel.
 * 
 * Should be executed with global preemption disabled.
 * 
 * Should be called only after space/messages *become* available.
 * That is, the side should be initialized at this stage.
 */
static inline void channel_queuing_side_notify(struct pok_channel_queuing_side* side,
    enum jet_partition_event_type event_type)
{
    if(side->is_notify)
    {
        side->is_notify = FALSE;
        pok_partition_add_event(side->part, event_type, side->handler_id);
    }
}

void pok_channel_queuing_side_init(pok_channel_queuing_t* channel,
    struct pok_channel_queuing_side* side,
    uint16_t handler_id)
{
    pok_preemption_disable();
    side->next_message = channel->border;
    side->is_notify = FALSE;
    side->generation = side->part->partition_generation;
    side->handler_id = handler_id;

    __pok_preemption_enable();
}

pok_message_range_t pok_channel_queuing_r_n_messages(pok_channel_queuing_t* channel)
{
    pok_preemption_disable();

    size_t n_messages = channel_queuing_cyclic_sub(channel,
        channel->border,
        channel->recv.next_message);

    __pok_preemption_enable();

    return n_messages;
}

const char* pok_channel_queuing_r_get_message(
    pok_channel_queuing_t* channel,
    pok_message_size_t* size,
    pok_bool_t subscribe)
{
    const char* message;

    pok_preemption_disable();

    if(channel->recv.next_message != channel->border)
    {
        message = channel_queuing_message_at(channel, channel->recv.next_message);
        *size = channel->message_sizes[channel->recv.next_message];
    }
    else
    {
        message = NULL;
        if(subscribe) channel->recv.is_notify = TRUE;
    }

    __pok_preemption_enable();

    return message;
}


void pok_channel_queuing_r_consume_message(
    pok_channel_queuing_t* channel,
    pok_bool_t* message_discarded)
{
    assert(channel->recv.next_message != channel->border);

    pok_preemption_disable();

    channel->recv.next_message = channel_queuing_cyclic_add(channel,
        channel->recv.next_message, 1);

    if(channel->send.part->partition_generation == channel->send.generation
        && channel->border != channel->send.next_message)
    {
        channel->border = channel_queuing_cyclic_add(channel,
            channel->border, 1);
        channel_queuing_side_notify(&channel->send,
            JET_PARTITION_EVENT_TYPE_PORT_SEND_AVAILABLE);
    }

    *message_discarded = channel->message_discarded;
    channel->message_discarded = FALSE;
    pok_preemption_enable();
}

char* pok_channel_queuing_s_get_message(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe)
{
    pok_message_range_t n_used;
    char* message;

    pok_preemption_disable();

    n_used = channel_queuing_cyclic_sub(channel,
        channel->send.next_message, channel->border);

    if(n_used < channel->send.max_nb_message)
    {
        message = channel_queuing_message_at(channel, channel->send.next_message);
    }
    else
    {
        message = NULL;
        if(subscribe)
            channel->send.is_notify = TRUE;
    }

    __pok_preemption_enable();

    return message;
}

void pok_channel_queuing_s_produce_message(
    pok_channel_queuing_t* channel,
    pok_message_size_t size)
{
    assert(channel_queuing_cyclic_sub(channel, channel->send.next_message,
        channel->border) < channel->send.max_nb_message);

    assert(size > 0);
    assert(size <= channel->max_message_size);

    pok_preemption_disable();

    channel->message_sizes[channel->send.next_message] = size;
    channel->send.next_message = channel_queuing_cyclic_add(channel,
        channel->send.next_message, 1);

    if(channel->recv.part->partition_generation == channel->recv.generation)
    {
        // Receiver is ready.
        pok_message_range_t nb_message_receive = channel_queuing_cyclic_sub(
            channel, channel->border, channel->recv.next_message);

        if(nb_message_receive < channel->recv.max_nb_message)
        {
            // Move message to the receiver buffer
            channel->border = channel->send.next_message;
            // And notify receiver, if requested.
            channel_queuing_side_notify(&channel->recv,
                JET_PARTITION_EVENT_TYPE_PORT_RECEIVE_AVAILABLE);
        }
        else if(channel->overflow_strategy == JET_CHANNEL_QUEUING_RECEIVER_DISCARD)
        {
            /* Discard message and store note about that.*/
            channel->send.next_message = channel->border;
            channel->message_discarded = TRUE;
        }
    }
    else
    {
        // Just drop the message and all previous ones which have been received.
        channel->border = channel->send.next_message;
    }

    pok_preemption_enable();
}

pok_message_range_t pok_channel_queuing_s_n_messages(pok_channel_queuing_t* channel)
{
    pok_preemption_disable();

    pok_message_range_t n_messages = channel_queuing_cyclic_sub(channel,
        channel->send.next_message, channel->border);

    __pok_preemption_enable();

    return n_messages;
}

/*********************** Sampling channel *****************************/

void pok_channel_sampling_init(pok_channel_sampling_t* channel)
{
    const unsigned int message_alignment = __alignof__(int);

    channel->message_stride = ALIGN_VAL(channel->max_message_size,
        message_alignment);
    channel->messages = ja_mem_alloc_aligned(3 * channel->message_stride,
        message_alignment);

    channel->read_pos_next = 0;
    channel->read_pos = 0;

    // Mark the first message as empty.
    channel->message_sizes[0] = 0;

    channel->write_pos = 1;
}

char* channel_sampling_message_at(
    pok_channel_sampling_t* channel, int pos)
{
    assert(pos <= 2);

    return &channel->messages[pos * channel->message_stride];
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
const char* pok_channel_sampling_r_get_message(
    pok_channel_sampling_t* channel,
    pok_message_size_t* size,
    pok_time_t* timestamp)
{
    uint8_t read_pos;

    pok_preemption_disable();
    read_pos = channel->read_pos = channel->read_pos_next;
    __pok_preemption_enable();

    pok_message_size_t message_size = channel->message_sizes[read_pos];
    if(!message_size) return NULL;

    *size = message_size;
    *timestamp = channel->timestamps[read_pos];

    return channel_sampling_message_at(channel, read_pos);;
}

void pok_channel_sampling_r_clear_message(pok_channel_sampling_t* channel)
{
    pok_preemption_disable();
    channel->message_sizes[channel->read_pos] = 0;
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

char* pok_channel_sampling_s_get_message(
    pok_channel_sampling_t* channel)
{
    return channel_sampling_message_at(channel, channel->write_pos);
}

void pok_channel_sampling_send_message(
    pok_channel_sampling_t* channel,
    pok_message_size_t size)
{
    uint8_t read_pos_next;

    assert(size);

    pok_preemption_disable();
    channel->read_pos_next = read_pos_next = channel->write_pos;
    channel->timestamps[read_pos_next] = jet_system_time();
    channel->message_sizes[read_pos_next] = size;

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
