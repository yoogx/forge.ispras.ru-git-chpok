/* 
 * Generic channels between partitions.
 */

#ifndef __POK_KERNEL_CHANNEL_H__
#define __POK_KERNEL_CHANNEL_H__

/*********************** Queuing channel ******************************/

struct _pok_channel_queuing;

/* Receiver for queing channel. */
typedef struct _pok_channel_queuing_reciever {
    /* 
     * This function is called when new message is appeared in the
     * channel and receiver have subscribed to notification.
     * 
     * Subscription is cancelled before calling this functon.
     * 
     * NOTE: function is called at time of SEND partition.
     * 
     * NOTE: There are can spurious notifications.
     */
    void (*on_message_sent)(struct _pok_channel_queuing_reciever*);
}pok_channel_queuing_reciever_t;

/* Sender for queing channel. */
typedef struct _pok_channel_queuing_sender {
    /* 
     * This function is called when some message are consumed by the
     * other side, so there is free space in the send buffer.
     * 
     * Sender should subscribe on notification before.
     * 
     * Subscription is cancelled before calling this functon.
     * 
     * NOTE: function is called at time of SEND partition.
     * 
     * NOTE: There are can spurious notifications.
     */
    void (*on_message_received)(struct _pok_channel_queuing_sender*);
}pok_channel_queuing_sender_t;


/* 
 * Queuing channel.
 * 
 * This channel is *receiver*-driven, this is receiver who decide when
 * transmission of the message occures.
 * 
 * Receiver has buffer of received messages.
 * Sender has buffer of messages ready to send.
 */
typedef struct _pok_channel_queuing {
    size_t max_message_size; // Maximum size of single message.
    size_t message_stride; //Size of single message structure.
    
    uint16_t max_nb_messages_receive; // Buffer limit for receiver
    uint16_t max_nb_messages_send; // Buffer limit for sender
    
    uint16_t max_nb_messages; // Total buffer capasity.
    char* buffer; // Array of messages, max_nb_messages * message_stride

    /*
     * Beginning of receiver buffer.
     * 
     * Advanced when messages is consumed (on receiver side).
     */
    uint16_t r_start;
    /*
     * Border between receiver and sender buffers.
     * 
     * Advanced when message is transmitted from sender to receiver.
     */
    uint16_t border;
    /*
     * End of sender buffer.
     * 
     * Advanced when messages is produced (on sender side).
     */
    uint16_t s_end;

    /* Whether it is needed to notify receiver */
    pok_bool_t notify_receiver;
    /* Whether it is needed to notify sender */
    pok_bool_t notify_sender;

    /* Receiver. NULL if there is no receiver currently. */
    pok_channel_queuing_reciever_t* receiver;
    /* Sender. NULL if there is no sender currently. */
    pok_channel_queuing_sender_t* sender;
    
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
/* 
 * Set receiver for the channel.
 * 
 * If receiver is already set, then no-op.
 */
void pok_channel_queuing_set_receiver(pok_channel_queuing_t* channel,
    pok_channel_queuing_reciever_t* receiver);

/*
 * Return number of messages on the receiver side.
 */
size_t pok_channel_queuing_r_n_messages(pok_channel_queuing_t* channel);

/* Return message at given index at receiver side. */
pok_message_t* pok_channel_queuing_r_get_message(
    pok_channel_queuing_t* channel,
    size_t index);

/* Consume n messages at receiver side. */
void pok_channel_queuing_r_consume_messages(
    pok_channel_queuing_t* channel,
    size_t n);

/* 
 * Receive messages.
 * 
 * Receive at most
 * 
 *   max_nb_messages_receive - pok_channel_queuing_r_n_messages()
 * 
 * messages.
 * 
 * Return number of messages received.
 * 
 * If there are no messages at the send side and @subscribe parameter
 * is TRUE, subscribe to notifications about new messages at the send side.
 * 
 * NOTE: Current implementation *requires* receive buffer to be empty
 * at the function's call.
 */
size_t pok_channel_queuing_receive(pok_channel_queuing_t* channel,
    pok_bool_t subscribe);

/***** Operations for sender. Should be serialized wrt themselves *****/
/* 
 * Set sender for the channel.
 * 
 * If sender is already set, then no-op.
 */
void pok_channel_queuing_set_sender(pok_channel_queuing_t* channel,
    pok_channel_queuing_sender_t* sender);

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
 * Return number of messages on the receiver side.
 */
size_t pok_channel_queuing_s_n_messages(pok_channel_queuing_t* channel)
{
    return pok_channel_queuing_cyclic_sub(channel,
        channel->s_end, ACCESS_ONCE(channel->border));
}


/*********************** Sampling channel *****************************/

/* 
 * Sampling channel.
 * 
 * This channel is *sender*-driven: sender decides when transmit the message.
 * 
 * Receiver has one message slot for (possibly) currently processed
 * message, and one message slot for newest message received.
 * 
 * Sender has one message slot for currently formed message.
 */
typedef struct {
    size_t max_message_size;
    
    size_t message_stride; //Size of of one message structure.
    char* buffer; // Buffer of messages, 3 * message_stride
    
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


#endif /* __POK_KERNEL_CHANNEL_H__ */
