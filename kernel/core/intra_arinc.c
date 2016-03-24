#include <core/intra_arinc.h>
#include <core/partition_arinc.h>
#include <uaccess.h>
#include <core/sched_arinc.h>
#include "thread_internal.h"

/*
 * Find object of given type with given name.
 * 
 * TYPE should be either: buffer, blackboard, semaphore, event.
 * name is `const char*` variable(in kernel space).
 */
#define find_object(TYPE, name) ({                                          \
    pok_ ## TYPE ## _t* obj = current_partition_arinc->TYPE ## s;          \
    pok_ ## TYPE ## _t* obj_end = obj + current_partition_arinc->n ## TYPE ## s_used; \
    for(; obj != obj_end; obj++) {                                     \
        if(!pok_compare_names(obj->name, name)) break;                      \
    }                                                                       \
    obj != obj_end ? obj : NULL;                                            \
})

/*
 * Get object of given type with given id.
 * 
 * TYPE should be either: buffer, blackboard, semaphore, event.
 * id should have identificator type corresponded for the object's type.
 */
#define get_object(TYPE, id) ({                                                 \
    pok_partition_arinc_t* part = current_partition_arinc;                      \
    (id < 0 || id >= part->n ## TYPE ## s_used) ? NULL : &part->TYPE ## s[id];  \
})

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

static pok_buffer_t* find_buffer(const char* name)
{
    return find_object(buffer, name);
}

static pok_buffer_t* get_buffer(pok_buffer_id_t id)
{
    return get_object(buffer, id);
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
    
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
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
    const pok_time_t* __user timeout)
{
    pok_ret_t ret;
    long wait_result;    

    pok_buffer_t* buffer = get_buffer(id);
    
    if(!buffer) return POK_ERRNO_UNAVAILABLE;

    if(length <= 0 || length > buffer->max_message_size) return POK_ERRNO_EINVAL;
    if(!check_access_read(data, length)) return POK_ERRNO_EFAULT;
    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    if(buffer->nb_message == 0
        && !pok_thread_wq_is_empty(&buffer->waiters)
        && (buffer->max_nb_message || buffer->message_stride == WAITERS_ARE_RECEIVERS))
    {
        // Directly copy message to waiter process.
        pok_thread_t* t = pok_thread_wq_wake_up(&buffer->waiters);
        assert(t);
        
        void* __user data_receive = (void* __user) t->wait_private;
        
        __copy_user(data_receive, data, length);
        t->wait_private = (void*)(unsigned long)length;
        
        ret = POK_ERRNO_OK;
    }
    else if(buffer->nb_message == buffer->max_nb_message)
    {
        // Need to wait
        pok_thread_t* t = current_thread;
        pok_time_t timeout_kernel = __get_user(timeout);
        if(timeout_kernel == 0)
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
        
        thread_wait_common(t, timeout_kernel);
        
        goto out_wait;
    }
    else
    {
        // Store message in the buffer and continue.
        int index = buffer_add_index(buffer, buffer->base_offset,
            buffer->nb_message);
        pok_message_t* message = buffer_get_message(buffer, index);
        
        __copy_from_user(message->content, data, length);
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

pok_ret_t pok_buffer_receive(pok_buffer_id_t id,
    const pok_time_t* __user timeout,
    void* __user data,
    pok_message_size_t* __user length)
{
    pok_ret_t ret;
    long wait_result;

    pok_buffer_t* buffer = get_buffer(id);
    
    if(!buffer) return POK_ERRNO_UNAVAILABLE;

    if(!check_access_write(data, buffer->max_message_size)) return POK_ERRNO_EFAULT;
    if(!check_user_write(length)) return POK_ERRNO_EFAULT;
    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    if(buffer->nb_message > 0)
    {
        // Copy message from the array of messages
        pok_message_t* message = buffer_get_message(buffer,
            buffer->base_offset);
        
        __copy_to_user(data, message->content, message->size);
        __put_user(length, message->size);
        
        // Move buffer's base position
        buffer->base_offset = buffer_add_index(buffer, buffer->base_offset, 1);
        
        if(!pok_thread_wq_is_empty(&buffer->waiters))
        {
            // Copy message from waited sender into available cell.
            pok_thread_t* sender_thread = pok_thread_wq_wake_up(&buffer->waiters);
            
            pok_message_send_t* message_send = sender_thread->wait_private;
            
            __copy_from_user(message->content, message_send->data, message_send->size);
            
            sender_thread->wait_private = NULL;
        }
        else
        {
            buffer->nb_message--; // No senders await
        }
        
        ret = POK_ERRNO_OK;
    }
    else if(buffer->max_nb_message
        || pok_thread_wq_is_empty(&buffer->waiters)
        || buffer->message_stride == WAITERS_ARE_RECEIVERS)
    {
        // Need to wait
        pok_thread_t* t = current_thread;
        pok_time_t timeout_kernel = __get_user(timeout);
        if(timeout_kernel == 0)
        {
            __put_user(length, 0);
            ret = POK_ERRNO_UNAVAILABLE;
            goto out;
        }
        else if(!thread_is_waiting_allowed())
        {
            __put_user(length, 0);
            ret = POK_ERRNO_MODE;
            goto out;
        }
        
        t->wait_private = (void*)data;
        
        pok_thread_wq_add_common(&buffer->waiters, t,
            buffer->discipline);

        if(!buffer->max_nb_message)
            buffer->message_stride = WAITERS_ARE_RECEIVERS;
        
        thread_wait_common(t, timeout_kernel);
        
        goto out_wait;
    }
    else
    {
        // 0-sise buffer and sender(s) waits. Directly copy message from sender thread.
        pok_thread_t* sender_thread = pok_thread_wq_wake_up(&buffer->waiters);
        
        pok_message_send_t* message_send = sender_thread->wait_private;
        
        __copy_user(data, message_send->data, message_send->size);
        __put_user(length, message_send->size);
        
        sender_thread->wait_private = NULL;
        ret = POK_ERRNO_OK;
    }

out:
    pok_preemption_local_enable();

    return ret;

out_wait:
    pok_preemption_local_enable();
    
    wait_result = (unsigned long)current_thread->wait_private;
    if(wait_result < 0) return -wait_result;

    __put_user(length, wait_result);

    return POK_ERRNO_OK;
}

pok_ret_t pok_buffer_get_id(char* __user name,
    pok_buffer_id_t* __user id)
{
    char kernel_name[MAX_NAME_LENGTH];
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
    pok_buffer_t* buffer = find_buffer(kernel_name);
    
    if(!buffer) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, buffer - current_partition_arinc->buffers);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_buffer_status(pok_buffer_id_t id,
    pok_buffer_status_t* __user status)
{
    pok_buffer_t* buffer = get_buffer(id);
    
    if(!buffer) return POK_ERRNO_UNAVAILABLE;
    
    if(!check_user_write(status)) return POK_ERRNO_EFAULT;
    
    __put_user_f(status, max_nb_message, buffer->max_nb_message);
    __put_user_f(status, max_message_size, buffer->max_message_size);
    
    pok_preemption_local_disable();
    __put_user_f(status, nb_message, buffer->nb_message);
    __put_user_f(status, waiting_processes, pok_thread_wq_get_nwaits(&buffer->waiters));
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

/************************ Blackboard **********************************/
static pok_blackboard_t* find_blackboard(const char* name)
{
    return find_object(blackboard, name);
}

static pok_blackboard_t* get_blackboard(pok_blackboard_id_t id)
{
    return get_object(blackboard, id);
}

pok_ret_t pok_blackboard_create (const char* __user             name,
                                 const pok_message_size_t       max_message_size,
                                 pok_blackboard_id_t* __user    id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    if(part->mode == POK_PARTITION_MODE_NORMAL) return POK_ERRNO_PARTITION_MODE;
    
    if(part->nblackboards_used == part->nblackboards) return POK_ERRNO_TOOMANY;
    
    pok_blackboard_t* blackboard = &part->blackboards[part->nblackboards_used];
    
    __copy_from_user(blackboard->name, name, MAX_NAME_LENGTH);
    
    if(find_blackboard(blackboard->name)) return POK_ERRNO_EXISTS;
    
    if(max_message_size == 0) return POK_ERRNO_EINVAL;
    
    // TODO: check upper limit of max_message_size.
    pok_message_size_t message_stride = POK_MESSAGE_STRUCT_SIZE(max_message_size);
    pok_message_t* message = partition_arinc_im_get(message_stride, sizeof(unsigned long));
    
    if(!message) return POK_ERRNO_UNAVAILABLE;
    
    blackboard->message_stride = message_stride;
    blackboard->max_message_size = max_message_size;
    blackboard->message = message;
    
    message->size = 0;
    
    pok_thread_wq_init(&blackboard->waiters);
    
    __put_user(id, part->nblackboards_used);
    
    part->nblackboards_used++;
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_blackboard_read (pok_blackboard_id_t          id,
                               const pok_time_t* __user     timeout,
                               void* __user                 data,
                               pok_message_size_t* __user   len)
{
    pok_ret_t ret;
    pok_thread_t* t;

    pok_blackboard_t* blackboard = get_blackboard(id);
    if(!blackboard) return POK_ERRNO_UNAVAILABLE;
    
    if(!check_access_write(data, blackboard->max_message_size)) return POK_ERRNO_EFAULT;
    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    if(!check_user_write(len)) return POK_ERRNO_EFAULT;
    
    pok_time_t kernel_timeout = __get_user(timeout);
    pok_message_t* message = blackboard->message;
    
    pok_preemption_local_disable();
    if(message->size)
    {
        __copy_to_user(data, message->content, message->size);
        __put_user(len, message->size);
        ret = POK_ERRNO_OK;
        goto out;
    }
    else
    {
        // Need to wait
        if(kernel_timeout == 0) {
            __put_user(len, 0);
            ret = POK_ERRNO_UNAVAILABLE;
            goto out;
        }
        else if(!thread_is_waiting_allowed()) {
            __put_user(len, 0);
            ret = POK_ERRNO_MODE;
            goto out;
        }
        
        t = current_thread;
        
        t->wait_private = data;
        pok_thread_wq_add(&blackboard->waiters, t);
        
        thread_wait_common(t, kernel_timeout);
        goto out_wait;
    }
out:
    pok_preemption_local_enable();
    
    return ret;

out_wait:
    pok_preemption_local_enable();
    
    unsigned long wait_result = (unsigned long)t->wait_private;
    if(wait_result < 0) return (pok_ret_t)(-wait_result);
    
    __put_user(len, wait_result);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_blackboard_display (pok_blackboard_id_t   id,
                                  const void* __user    message,
                                  pok_message_size_t    len)
{
    pok_blackboard_t* blackboard = get_blackboard(id);
    
    if(!blackboard) return POK_ERRNO_UNAVAILABLE;
    
    if(len > blackboard->max_message_size || len == 0) return POK_ERRNO_EINVAL;
    if(!check_access_read(message, len)) return POK_ERRNO_EFAULT;
    
    pok_preemption_local_disable();
    
    __copy_from_user(blackboard->message->content, message, len);
    blackboard->message->size = len;
    
    for(pok_thread_t* t = pok_thread_wq_wake_up(&blackboard->waiters);
        t;
        t = pok_thread_wq_wake_up(&blackboard->waiters))
    {
        char* __user recv_data = (char* __user)t->wait_private;
        
        __copy_user(recv_data, message, len);
        t->wait_private = (void*)(unsigned long)len;
    }
    
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_blackboard_clear (pok_blackboard_id_t id)
{
    pok_blackboard_t* blackboard = get_blackboard(id);
    
    if(!blackboard) return POK_ERRNO_UNAVAILABLE;
    
    pok_preemption_local_disable();
    blackboard->message->size = 0;
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_blackboard_id     (const char* __user             name,
                                 pok_blackboard_id_t* __user    id)
{
    char kernel_name[MAX_NAME_LENGTH];
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
    pok_blackboard_t* blackboard = find_blackboard(kernel_name);
    
    if(!blackboard) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, blackboard - current_partition_arinc->blackboards);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_blackboard_status (pok_blackboard_id_t                id,
                                 pok_blackboard_status_t* __user    status)
{
    pok_blackboard_t* blackboard = get_blackboard(id);
    
    if(!blackboard) return POK_ERRNO_UNAVAILABLE;
    
    if(!check_user_write(status)) return POK_ERRNO_EFAULT;

    __put_user_f(status, max_message_size, blackboard->max_message_size);

    pok_preemption_local_disable();
    __put_user_f(status, empty_indicator,
        blackboard->message->size? POK_BLACKBOARD_OCCUPIED : POK_BLACKBOARD_EMPTY);
    __put_user_f(status, waiting_processes,
        pok_thread_wq_get_nwaits(&blackboard->waiters));
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

/*********************** Semaphores ***********************************/
static pok_semaphore_t* find_semaphore(const char* name)
{
    return find_object(semaphore, name);
}

static pok_semaphore_t* get_semaphore(pok_sem_id_t id)
{
    return get_object(semaphore, id);
}


pok_ret_t pok_semaphore_create(const char* __user name,
    pok_sem_value_t value,
    pok_sem_value_t max_value,
    pok_queuing_discipline_t discipline,
    pok_sem_id_t* __user id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    if(part->mode == POK_PARTITION_MODE_NORMAL) return POK_ERRNO_PARTITION_MODE;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    if(value > max_value) return POK_ERRNO_EINVAL;
    
    // TODO: check whether max_value is out of range.
    
    if(discipline != POK_QUEUEING_DISCIPLINE_FIFO
        && discipline != POK_QUEUEING_DISCIPLINE_PRIORITY) return POK_ERRNO_EINVAL;

    if(part->nsemaphores_used == part->nsemaphores) return POK_ERRNO_TOOMANY;
    
    pok_semaphore_t* semaphore = &part->semaphores[part->nsemaphores_used];
    
    __copy_from_user(semaphore->name, name, MAX_NAME_LENGTH);
    
    if(find_semaphore(semaphore->name)) return POK_ERRNO_EXISTS;
    
    semaphore->value = value;
    semaphore->max_value = max_value;
    semaphore->discipline = discipline;
    pok_thread_wq_init(&semaphore->waiters);
    
    __put_user(id, semaphore - part->semaphores);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_semaphore_wait(pok_sem_id_t id,
    const pok_time_t* __user timeout)
{
    pok_semaphore_t* semaphore = get_semaphore(id);
    if(!semaphore) return POK_ERRNO_UNAVAILABLE;
    
    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    pok_time_t kernel_timeout = __get_user(timeout);
    
    pok_ret_t ret;
    pok_thread_t* t = current_thread;
    
    pok_preemption_local_disable();
    if(semaphore->value)
    {
        semaphore->value--;
        ret = POK_ERRNO_OK;
    }
    else if(kernel_timeout == 0)
    {
        ret = POK_ERRNO_EMPTY;
    }
    else if(!thread_is_waiting_allowed())
    {
        ret = POK_ERRNO_MODE;
    }
    else
    {
        // Wait
        pok_thread_wq_add_common(&semaphore->waiters, t, semaphore->discipline);
        thread_wait_common(t, kernel_timeout);
        goto out_wait;
    }
    
    pok_preemption_local_enable();

    return ret;
    
out_wait:
    pok_preemption_local_enable();

    unsigned long wait_result = (unsigned long)t->wait_private;
    if(wait_result) return (pok_ret_t)(-(long) wait_result);

    return POK_ERRNO_OK;
}


pok_ret_t pok_semaphore_signal(pok_sem_id_t id)
{
    pok_ret_t ret = POK_ERRNO_OK;
    pok_semaphore_t* semaphore = get_semaphore(id);
    if(!semaphore) return POK_ERRNO_UNAVAILABLE;

    pok_preemption_local_disable();
    // Having waiters check first we process semaphore with 0 as max_value
    if(!pok_thread_wq_is_empty(&semaphore->waiters))
    {
        pok_thread_t* t = pok_thread_wq_wake_up(&semaphore->waiters);
        
        t->wait_private = NULL;
    }
    else if(semaphore->value == semaphore->max_value)
    {
        ret = POK_ERRNO_EINVAL;
    }
    else
    {
        semaphore->value++;
    }

    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_semaphore_id(const char* __user name,
    pok_sem_id_t* __user id)
{
    char kernel_name[MAX_NAME_LENGTH];
    
    if(check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(check_user_read(id)) return POK_ERRNO_EFAULT;
    
    pok_semaphore_t* semaphore = find_semaphore(kernel_name);
    if(!semaphore) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, semaphore - current_partition_arinc->semaphores);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_semaphore_status(pok_sem_id_t id,
    pok_semaphore_status_t* __user status)
{
    pok_semaphore_t* semaphore = get_semaphore(id);
    if(!semaphore) return POK_ERRNO_UNAVAILABLE;

    if(!check_user_write(status)) return POK_ERRNO_EFAULT;
    
    __put_user_f(status, maximum_value, semaphore->max_value);
    
    pok_preemption_local_disable();
    __put_user_f(status, current_value, semaphore->value);
    __put_user_f(status, waiting_processes,
        pok_thread_wq_get_nwaits(&semaphore->waiters));
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}

/************************ Events **************************************/
static pok_event_t* find_event(const char* name)
{
    return find_object(event, name);
}

static pok_event_t* get_event(pok_event_id_t id)
{
    return get_object(event, id);
}


pok_ret_t pok_event_create(const char* __user name,
    pok_event_id_t* __user id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    if(part->mode == POK_PARTITION_MODE_NORMAL) return POK_ERRNO_PARTITION_MODE;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;

    if(part->nevents_used == part->nevents) return POK_ERRNO_TOOMANY;
    
    pok_event_t* event = &part->events[part->nevents_used];
    
    __copy_from_user(event->name, name, MAX_NAME_LENGTH);
    
    if(find_event(event->name)) return POK_ERRNO_EXISTS;
    
    event->value = POK_EVENT_DOWN;

    pok_thread_wq_init(&event->waiters);
    
    __put_user(id, event - part->events);
    
    return POK_ERRNO_OK;

}

pok_ret_t pok_event_set(pok_event_id_t id)
{
    pok_event_t* event = get_event(id);
    if(!event) return POK_ERRNO_UNAVAILABLE;
    
    pok_preemption_local_disable();
    event->value = POK_EVENT_UP;
    
    for(pok_thread_t* t = pok_thread_wq_wake_up(&event->waiters);
        t;
        t = pok_thread_wq_wake_up(&event->waiters))
    {
        t->wait_private = NULL;
    }
    
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}
pok_ret_t pok_event_reset(pok_event_id_t id)
{
    pok_event_t* event = get_event(id);
    if(!event) return POK_ERRNO_UNAVAILABLE;
    
    pok_preemption_local_disable();
    event->value = POK_EVENT_DOWN;
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;

}

pok_ret_t pok_event_wait(pok_event_id_t id,
    const pok_time_t* __user timeout)
{
    pok_ret_t ret;
    
    pok_event_t* event = get_event(id);
    if(!event) return POK_ERRNO_UNAVAILABLE;

    if(!check_user_read(timeout)) return POK_ERRNO_EFAULT;
    pok_time_t kernel_timeout = __get_user(timeout);

    pok_thread_t* t = current_thread;
    
    pok_preemption_local_disable();
    if(event->value == POK_EVENT_UP)
    {
        ret = POK_ERRNO_OK;
    }
    else if(kernel_timeout == 0)
    {
        ret = POK_ERRNO_UNAVAILABLE;
    }
    else if(!thread_is_waiting_allowed())
    {
        ret = POK_ERRNO_MODE;
    }
    else
    {
        // Wait
        pok_thread_wq_add(&event->waiters, t);
        thread_wait_common(t, kernel_timeout);
        goto out_wait;
    }
    
    pok_preemption_local_enable();

    return ret;
        
out_wait:
    pok_preemption_local_enable();
    
    long wait_result = (long)t->wait_private;
    if(wait_result) return (pok_ret_t)(-wait_result);

    return POK_ERRNO_OK;
}

pok_ret_t pok_event_id(const char* __user name,
    pok_event_id_t* __user id)
{
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    char kernel_name[MAX_NAME_LENGTH];
    
    __copy_from_user(kernel_name, name, MAX_NAME_LENGTH);
    
    pok_event_t* event = find_event(kernel_name);
    if(!event) return POK_ERRNO_UNAVAILABLE;
    
    __put_user(id, event - current_partition_arinc->events);
    
    return POK_ERRNO_OK;
}

pok_ret_t pok_event_status(pok_event_id_t id,
    pok_event_status_t* __user status)
{
    pok_event_t* event = get_event(id);
    if(!event) return POK_ERRNO_UNAVAILABLE;

    if(!check_user_write(status)) return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();
    __put_user_f(status, event_state, event->value);
    __put_user_f(status, waiting_processes,
        pok_thread_wq_get_nwaits(&event->waiters));
    pok_preemption_local_enable();
    
    return POK_ERRNO_OK;
}
