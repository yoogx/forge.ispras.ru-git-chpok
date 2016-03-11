#include <core/intra_arinc.h>
#include <core/partition_arinc.h>
#include <uaccess.h>
#include <core/sched_arinc.h>
#include "thread_internal.h"

/***************************** Buffer *********************************/

/*
 * While we have single .waiters field for senders and receivers,
 * we can distinguish between them using .nb_message field:
 * 
 * 0 - receivers
 * max_nb_message - senders
 * 
 * But in case of 0-capacity of the buffer, .max_nb_message equals to 0.
 * So, it cannot longer be used for distinguish usage of .waiters field.
 * 
 * Instead, we store special constants in .message_stride field:
 * 
 * WAITERS_ARE_RECEIVERS - receivers
 * WAITERS_ARE_SENDERS - senders
 * 
 * Note, that .waiters queue may be emptied outside (because of timeout),
 * so we check .message_stride field only if queue is not empty.
 */
#define WAITERS_ARE_RECEIVERS 0
#define WAITERS_ARE_SENDERS 1

/* 
 * Return sum of two message indecies.
 * 
 * Take into account circular nature of the messages array.
 */
static pok_message_range_t buffer_add_index(pok_buffer_t* buffer,
    pok_message_range_t base, pok_message_range_t offset)
{
    pok_message_range_t index = base + offset;
    assert(buffer->max_nb_message);
    
    if(index >= buffer->max_nb_message) index -= buffer->max_nb_message;
    
    return index;
}

/* Return message at given index. */
static pok_message_t* buffer_get_message(pok_buffer_t* buffer,
    pok_message_range_t index)
{
    assert(index < buffer->max_nb_message);
    return (pok_message_t*)(&buffer->messages[buffer->message_stride * index]);
}

pok_buffer_t* find_buffer(const char* name)
{
    pok_buffer_t* b = current_partition_arinc->buffers;
    pok_buffer_t* b_end = b + current_partition_arinc->nbuffers_used;
    
    for(; b != b_end; b++)
    {
        if(!pok_compare_names(b->name, name)) return b;
    }
    
    return NULL;
}

pok_buffer_t* get_buffer(pok_buffer_id_t id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    if(id < 0 || id >= part->nbuffers_used) return NULL;
    
    return &part->buffers[id];
}


pok_ret_t pok_buffer_create(char* __user name,
    pok_message_size_t max_message_size,
    pok_message_range_t max_nb_message,
    pok_queuing_discipline_t discipline,
    pok_buffer_id_t* __user id)
{
    pok_buffer_t* b;
    char kernel_name[MAX_NAME_LENGTH];
    pok_partition_arinc_t* part = current_partition_arinc;
    
    void* messages;
    
    pok_message_size_t message_stride;
    
    if(part->mode == POK_PARTITION_MODE_NORMAL) return POK_ERRNO_PARTITION_MODE;
    // No needs in critical section - init process cannot be preempted.

    if(discipline != POK_QUEUEING_DISCIPLINE_FIFO
        && discipline != POK_QUEUEING_DISCIPLINE_PRIORITY) return POK_ERRNO_EINVAL;
    
    if(max_message_size == 0) return POK_ERRNO_EINVAL;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    __copy_from_user(name, kernel_name, MAX_NAME_LENGTH);
    
    if(find_buffer(kernel_name)) return POK_ERRNO_EXISTS;
    
    if(part->nbuffers_used == part->nbuffers) return POK_ERRNO_TOOMANY;

    if(max_nb_message)
    {
        message_stride = POK_MESSAGE_STRUCT_SIZE(max_message_size);
        messages = partition_arinc_im_get(message_stride * max_nb_message, sizeof(unsigned long));
        
        if(!messages) return POK_ERRNO_UNAVAILABLE;
    }
    else
    {
        /* 
         * Buffer with 0 capacity.
         * 
         * Buffer is not used, messages are transmitted directly from
         * the sender to the receiver.
         * 
         * .message_stride has special usage here.
         * .messages field isn't used at all, but we initialize it
         * in spite of this.
         */
        message_stride = WAITERS_ARE_RECEIVERS;
        messages = NULL;
    }
    
    b = &part->buffers[part->nbuffers_used];
    
    memcpy(b->name, kernel_name, MAX_NAME_LENGTH);
    b->max_message_size = max_message_size;
    b->max_nb_message = max_nb_message;
    b->message_stride = message_stride;
    b->messages = messages;
    b->discipline = discipline;
    
    b->nb_message = 0;
    b->base_offset = 0;
    pok_thread_wq_init(&b->waiters);
    
    __put_user(id, part->nbuffers_used);
    
    part->nbuffers_used++;
    
    return POK_ERRNO_OK;
}


pok_ret_t pok_buffer_send(pok_buffer_id_t id,
    const void* __user data,
    pok_message_size_t length,
    pok_time_t timeout)
{
    pok_ret_t ret;
    long wait_result;    

    pok_buffer_t* buffer = get_buffer(id);
    
    if(!buffer) return POK_ERRNO_UNAVAILABLE;

    if(length <= 0 || length > buffer->max_message_size) return POK_ERRNO_EINVAL;
    if(!check_access_read(data, length)) return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    if(buffer->nb_message == 0
        && !pok_thread_wq_is_empty(&buffer->waiters)
        && (buffer->max_nb_message || buffer->message_stride == WAITERS_ARE_RECEIVERS))
    {
        // Directly copy message to waiter process.
        pok_thread_t* t = pok_thread_wq_wake_up(&buffer->waiters);
        assert(t);
        
        void* __user receiver_addr = (void* __user)t->wait_private;
        
        __copy_user(data, receiver_addr, length);
        t->wait_private = NULL;
        
        ret = POK_ERRNO_OK;
    }
    else if(buffer->nb_message == buffer->max_nb_message)
    {
        // Need to wait
        pok_thread_t* t = current_thread;
        if(timeout == 0)
        {
            ret = POK_ERRNO_UNAVAILABLE;
            goto out;
        }
        else if(!thread_is_waiting_allowed())
        {
            ret = POK_ERRNO_MODE;
            goto out;
        }
        
        pok_message_send_t message_send = {
            .size = length,
            .data = data
        };

        t->wait_private = &message_send;
        pok_thread_wq_add_common(&buffer->waiters, t,
            buffer->discipline);

        if(!buffer->max_nb_message)
            buffer->message_stride = WAITERS_ARE_SENDERS;
        
        thread_wait_common(t, timeout);
        
        goto out_wait;
    }
    else
    {
        // Store message in the buffer and continue.
        int index = buffer_add_index(buffer, buffer->base_offset,
            buffer->nb_message);
        pok_message_t* message = buffer_get_message(buffer, index);
        
        __copy_from_user(data, message->content, length);
        message->size = length;
        
        buffer->nb_message++;
        
        ret = POK_ERRNO_OK;
    }

out:
    pok_preemption_local_enable();

    return ret;

out_wait:
    pok_preemption_local_enable();
    
    wait_result = (unsigned long)current_thread->wait_private;
    if(wait_result < 0) return -wait_result;

    return POK_ERRNO_OK;
}

pok_ret_t pok_buffer_receive(pok_buffer_id_t* id,
    pok_time_t timeout,
    void* __user data,
    pok_message_size_t* __user length);

pok_ret_t pok_buffer_get_id(char* __user name,
    pok_buffer_id_t* __user id);

