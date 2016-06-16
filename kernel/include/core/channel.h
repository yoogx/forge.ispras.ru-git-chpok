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

/* 
 * Generic channels between partitions.
 */

#ifndef __POK_KERNEL_CHANNEL_H__
#define __POK_KERNEL_CHANNEL_H__

#include <types.h>
#include <message.h>

#include <core/partition.h>

/*********************** Queuing channel ******************************/

/* 
 * Queuing channel between partitions.
 * 
 * Receiver has buffer of received messages.
 * Sender has buffer of messages ready to send.
 * 
 * Mesages in that channel are transmitted *instantly* unless receiver
 * is not ready or its buffer is full. In that case messages are
 * accumulated on sender side until it is possible to transmit them.
 */
typedef struct {
    pok_message_size_t max_message_size; // Maximum size of single message.
    pok_message_size_t message_stride; //Size of single message structure.
    
    pok_message_range_t max_nb_message_receive; // Buffer limit for receiver
    pok_message_range_t max_nb_message_send; // Buffer limit for sender
    
    pok_message_range_t max_nb_message; // Total buffer capasity.
    char* buffer; // Array of messages, max_nb_message * message_stride

    /*
     * Beginning of receiver buffer.
     * 
     * Advanced when messages is consumed (on receiver side).
     */
    pok_message_range_t r_start;
    /*
     * Border between receiver and sender buffers.
     * 
     * Advanced when message is transmitted from sender to receiver.
     */
    pok_message_range_t border;
    /*
     * End of sender buffer.
     * 
     * Advanced when messages is produced (on sender side).
     */
    pok_message_range_t s_end;

    /* Whether it is needed to notify receiver */
    pok_bool_t notify_receiver;
    /* Whether it is needed to notify sender */
    pok_bool_t notify_sender;

    /* Receiver partition. */
    pok_partition_t* receiver;
    /* Receiver's generation when it has its connection initialized. */
    pok_partition_generation_t receiver_generation;
    /* Sender partition. */
    pok_partition_t* sender;
    /* Sender's generation when it has its connection initialized. */
    pok_partition_generation_t sender_generation;

    
} pok_channel_queuing_t;

/* 
 * Initialize queuing channel.
 * 
 * Fields should be set before calling this function:
 * 
 *   - max_message_size
 *   - max_nb_messages_receive 
 *   - max_nb_messages_send
 */
void pok_channel_queuing_init(pok_channel_queuing_t* channel);

/**** Operations for receiver. Should be serialized wrt themselves ****/
/* (Re)initialize receiver connection to the channel. */
void pok_channel_queuing_r_init(pok_channel_queuing_t* channel);

/*
 * Return number of messages on the receiver side.
 */
size_t pok_channel_queuing_r_n_messages(pok_channel_queuing_t* channel);

/* 
 * Return the first message at receiver side.
 * 
 * If no message is available, return NULL. In that case, if 'subscribe'
 * is non-zero, receiver partition will be notified
 * (via `.outer_notification` flag) when new message will be available.
 */
pok_message_t* pok_channel_queuing_r_get_message(
    pok_channel_queuing_t* channel,
    pok_bool_t subscribe);

/* Consume the first message at receiver side. */
void pok_channel_queuing_r_consume_message(
    pok_channel_queuing_t* channel);

/***** Operations for sender. Should be serialized wrt themselves *****/
/* (Re)initialize sender connection to the channel. */
void pok_channel_queuing_s_init(pok_channel_queuing_t* channel);

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
    pok_bool_t subscribe);

/*
 * Mark (already filled) message as produced.
 * 
 * The message is ready for being sent.
 */
void pok_channel_queuing_s_produce_message(
    pok_channel_queuing_t* channel);

/*
 * Return number of messages on the sender side.
 * 
 * NOTE: Returning value is always 0 until receiver is not ready or
 * its buffer is full.
 */
pok_message_range_t pok_channel_queuing_s_n_messages(pok_channel_queuing_t* channel);

/*********************** Sampling channel *****************************/

/* 
 * Sampling channel.
 * 
 * Receiver has one message slot for (possibly) currently processed
 * message, and one message slot for newest message received.
 * 
 * Sender has one message slot for currently formed message.
 * 
 * Mesages in that channel are transmitted *instantly*.
 * 
 * Actually, ARINC distinguish message which is *sent* or *received*:
 * message is treated as sent until it is received.
 */
typedef struct {
    pok_message_size_t max_message_size;
    
    pok_message_size_t message_stride; //Size of of one message structure.
    char* buffer; // Buffer of messages, 3 * message_stride
    
    // Positions in range 0..2
    uint8_t read_pos;
    uint8_t read_pos_next;
    uint8_t write_pos;
    
    /* The simplest implementation: timestamp per message. */
    pok_time_t timestamps[3];
} pok_channel_sampling_t;

/* 
 * Initialize sampling channel.
 * 
 * Fields should be set before calling this function:
 * 
 *   - max_message_size
 */
void pok_channel_sampling_init(pok_channel_sampling_t* channel);


/**** Operations for receiver. Should be serialized wrt themselves ****/

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
    pok_channel_sampling_t* channel, pok_time_t* timestamp);

/*
 * Clear message received.
 */
void pok_channel_sampling_r_clear_message(pok_channel_sampling_t* channel);

/*
 * Return POK_ERRNO_OK if new message has been arrive since we check(read).
 * 
 * Return POK_ERRNO_EMPTY otherwise.
 */
pok_bool_t pok_channel_sampling_r_check_new_message(pok_channel_sampling_t* channel);

/***** Operations for sender. Should be serialized wrt themselves *****/

/*
 * Get pointer to the message for fill it.
 */
pok_message_t* pok_channel_sampling_s_get_message(
    pok_channel_sampling_t* channel);

/*
 * Send message which has been fully filled.
 */
void pok_channel_sampling_send_message(
    pok_channel_sampling_t* channel);

/*
 * Clear message sent.
 */
void pok_channel_sampling_s_clear_message(pok_channel_sampling_t* channel);

/**********************************************************************/
/* 
 * Array of queuing channels.
 * 
 * Should be set in module's config.
 */
extern pok_channel_queuing_t* pok_channels_queuing;

/*
 * Number of queuing channels.
 * 
 * Should be set in module's config.
 */
extern uint8_t pok_channels_queuing_n;

/* 
 * Array of sampling channels.
 * 
 * Should be set in module's config.
 */
extern pok_channel_sampling_t* pok_channels_sampling;

/*
 * Number of sampling channels.
 * 
 * Should be set in module's config.
 */
extern uint8_t pok_channels_sampling_n;

void pok_channels_init_all(void);

#endif /* __POK_KERNEL_CHANNEL_H__ */
