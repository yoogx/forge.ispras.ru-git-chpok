/*                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <core/dependencies.h>

#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BUFFERS

#include <errno.h>
#include <types.h>
#include <core/event.h>
#include <core/thread.h>
#include <core/time.h>
#include <core/lockobj.h>
#include <core/partition.h>
#include <core/error.h>
#include <libc/string.h>
#include <middleware/buffer.h>
#include <utils.h>

#if 1
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

// data types

// must be at least MAX_NAME_LENGTH of ARINC653 (which is 30)
#define POK_BUFFER_MAX_NAME_LENGTH 30
#define POK_BUFFER_NAME_EQ(x, y) (strncmp((x), (y), POK_BUFFER_MAX_NAME_LENGTH) == 0)

typedef struct pok_buffer_wait_list_t
{
    struct pok_buffer_wait_list_t *next;

    pok_thread_id_t thread;
    int priority;
    int64_t timeout;
    pok_ret_t result;
    
    union {
        struct {
            const char *data_ptr;
            pok_port_size_t data_size;
        } sending;

        struct {
            char *data_ptr;
            pok_port_size_t *data_size_ptr;
        } receiving;
    };
} pok_buffer_wait_list_t;

typedef struct
{
   /* Flag indicating that buffer has been created.
    * 
    * If it's not, all other members are undefined.
    */
   pok_bool_t                   ready;

   /* 
    * Pointer to buffer (where messages are stored) 
    * As of now, it's allocated from 
    * static pok_buffers_data[POK_CONFIG_BUFFER_DATA_SIZE],
    * but it cab be changed to be allocated elsewhere.
    */
   void                         *buffer;

   /*
    * Maximum message size (as given when buffer was created).
    */
   pok_port_size_t              msg_size;

   /*
    * Index of the first message (that is going to be read next).
    */
   pok_port_size_t              head_index;

   /*
    * Number of messages currently stored in the buffer.
    */
   pok_port_size_t              number_of_messages;          

   /*
    * Maximum number of messages that can be queued in the buffer.
    */
   pok_port_size_t              max_number_of_messages;       

   /*
    * Queueing discipline. We need one here to properly
    * sort processes in the wait list.
    */
   pok_queueing_discipline_t    discipline;

   /*
    * The synchronization object, obviously.
    */
   pok_event_id_t               lock;

   /*
    * The wait list. 
    * When buffer is empty, processes attempting to receive message go here.
    * Likewise, when it's full, writing processes go here.
    *
    * Note that since buffer can't be full and empty at the same time,
    * we can use a single list.
    *
    * Wait list entries are allocated on the stack of 
    * the corresponding waiting process.
    */
   pok_buffer_wait_list_t       *wait_list;

   /*
    * Buffer name, as specified when it was created.
    */
   char                         name[POK_BUFFER_MAX_NAME_LENGTH];
} pok_buffer_t;

typedef struct
{
    pok_port_size_t             message_size;
    unsigned char               data[];
} pok_buffer_data_t;

// actual allocated data structures (as global static variables)
static pok_buffer_t pok_buffers[POK_CONFIG_NB_BUFFERS];

static char pok_buffers_data[POK_CONFIG_BUFFER_DATA_SIZE]; 
static pok_size_t pok_buffers_data_index = 0;

// misc. functions

/*
 * Returns stride between consecutive pok_buffer_data_t
 * structs.
 */
static size_t buffer_data_stride(pok_port_size_t msg_size)
{
    // XXX might want to round it up to ensure alignment
    return sizeof(pok_buffer_data_t) + msg_size;
}

/*
 * Returns pointer to the first message.
 * (assumes number_of_messages != 0)
 */
static pok_buffer_data_t* buffer_head(pok_buffer_t *buffer) 
{
    char *ptr = buffer->buffer;
    size_t stride = buffer_data_stride(buffer->msg_size);
    
    pok_port_size_t index = buffer->head_index;

    return (pok_buffer_data_t*) (ptr + stride * index);
}

/*
 * Returns pointer to the "after last" message.
 *
 * (assuming number_of_messages < max_number_of_messages,
 * it would be a free spot)
 */
static pok_buffer_data_t* buffer_tail(pok_buffer_t *buffer)
{
    char *ptr = buffer->buffer;
    size_t stride = buffer_data_stride(buffer->msg_size);

    pok_port_size_t index = (buffer->head_index + buffer->number_of_messages) % buffer->max_number_of_messages;

    return (pok_buffer_data_t*) (ptr + stride * index);
}

/*
 * Pops one message from the buffer's circular buffer.
 *
 * (assumes there's one)
 */
static void buffer_pop(pok_buffer_t *buffer, void *data, pok_port_size_t *len)
{
    pok_buffer_data_t *place = buffer_head(buffer);

    memcpy(data, place->data, place->message_size);
    *len = place->message_size;

    buffer->head_index = (buffer->head_index + 1) % buffer->max_number_of_messages;
    buffer->number_of_messages--;
}

/*
 * Pushes one message to the buffer's circular buffer.
 *
 * (assumes there's space for one)
 */
static void buffer_push(pok_buffer_t *buffer, const void *data, pok_port_size_t len)
{
    pok_buffer_data_t *place = buffer_tail(buffer);
    
    place->message_size = len;
    memcpy(&place->data[0], data, len);

    buffer->number_of_messages++;
}

/*
 * Returns true if buffer is full.
 */
static pok_bool_t buffer_is_full(pok_buffer_t *buffer)
{
    return buffer->number_of_messages == buffer->max_number_of_messages;
}

/*
 * Returns true if buffer is empty.
 */
static pok_bool_t buffer_is_empty(pok_buffer_t *buffer)
{
    return buffer->number_of_messages == 0;
}

/*
 * Appends entry to buffer's process waiting list.
 *
 * This takes queueing discipline and process priority
 * into account automatically (caller doesn't need to do that).
 */
static void buffer_wait_list_append(
        pok_buffer_t *buffer, 
        pok_buffer_wait_list_t *entry)
{
    entry->priority = 0; 
    if (buffer->discipline == POK_QUEUEING_DISCIPLINE_PRIORITY) {
        pok_thread_status_t status;
        pok_thread_status(entry->thread, &status);

        entry->priority = status.current_priority;
    }
    
    pok_buffer_wait_list_t **list = &buffer->wait_list;

    while (*list != NULL && (*list)->priority >= entry->priority) {
        list = &((**list).next);
    }
    entry->next = *list;
    *list = entry;
}

/*
 * Returns lengths of the buffer's waiting list.
 */
static size_t buffer_wait_list_length(pok_buffer_t *buffer)
{
    size_t res = 0;
    pok_buffer_wait_list_t *list = buffer->wait_list;

    while (list != NULL) {
        res++;
        list = list->next;
    }
    return res;
}

static void buffer_wait_list_remove(
        pok_buffer_t *buffer, 
        pok_buffer_wait_list_t *entry)
{
    pok_buffer_wait_list_t **list = &buffer->wait_list;

    while (*list != NULL && *list != entry) {
        list = &((**list).next);
    }

    if (*list == NULL) return;

    *list = (**list).next;
}

// "public" functions (used by ARINC layer)

pok_ret_t pok_buffer_init(void)
{
    pok_buffer_id_t i;
    for (i = 0; i < POK_CONFIG_NB_BUFFERS; i++) {
        pok_buffers[i].ready = FALSE;
    }
    return POK_ERRNO_OK;
}

pok_ret_t pok_buffer_create(
        const char                  *name, 
        pok_port_size_t             num_messages, 
        pok_port_size_t             msg_size, 
        pok_queueing_discipline_t   discipline,
        pok_buffer_id_t             *id)
{
    pok_ret_t   ret; 
    pok_buffer_id_t i;

    pok_port_size_t size = num_messages * buffer_data_stride(msg_size);

    if (size > INT32_MAX - pok_buffers_data_index) {
      DEBUG_PRINT("size=%d + pok_buffers_data_index=%d will cause integer overflow\n", size, pok_buffers_data_index);
      return POK_ERRNO_EINVAL;
    }

    if (pok_buffers_data_index + size >= POK_CONFIG_BUFFER_DATA_SIZE) {
      DEBUG_PRINT("size=%d + pok_buffers_data_index=%d >= POK_CONFIG_BUFFER_DATA_SIZE=%d\n", size, pok_buffers_data_index, POK_CONFIG_BUFFER_DATA_SIZE);
      return POK_ERRNO_EINVAL;
    }

    // try to find existing buffer
    for (i = 0; i < POK_CONFIG_NB_BUFFERS; i++) {
        if (pok_buffers[i].ready && POK_BUFFER_NAME_EQ(name, pok_buffers[i].name)) {
            return POK_ERRNO_READY;
        }
    }

    // create a new one
    for (i = 0; i < POK_CONFIG_NB_BUFFERS; i++) {
        if (pok_buffers[i].ready == FALSE) {
            pok_buffer_t *buffer = &pok_buffers[i]; 

            ret = pok_event_create (&buffer->lock, discipline);

            if (ret != POK_ERRNO_OK) {
                DEBUG_PRINT("failed to create a lockobject for a buffer\n");
                return ret;
            }

            buffer->buffer = pok_buffers_data + pok_buffers_data_index;
            pok_buffers_data_index += size;

            buffer->msg_size = msg_size;
            buffer->discipline = discipline;
            buffer->wait_list = NULL;
            strncpy(buffer->name, name, POK_BUFFER_MAX_NAME_LENGTH);

            buffer->head_index = 0;
            buffer->number_of_messages = 0;
            buffer->max_number_of_messages = num_messages;

            
            buffer->ready = TRUE;

            *id = i;

            return POK_ERRNO_OK;
        }   

   }

   DEBUG_PRINT("no free buffers left\n");

   return POK_ERRNO_EINVAL;
}

pok_ret_t pok_buffer_id(
        const char                  *name, 
        pok_buffer_id_t             *id)
{
    pok_buffer_id_t i;

    for (i = 0; i < POK_CONFIG_NB_BUFFERS; i++) {
        if (POK_BUFFER_NAME_EQ(name, pok_buffers[i].name)) {
            *id = i;
            return POK_ERRNO_OK;
        }
    }

    return POK_ERRNO_EINVAL;
}

pok_ret_t pok_buffer_receive (
        pok_buffer_id_t             id, 
        int64_t                     timeout, 
        void                        *data, 
        pok_port_size_t             *len)
{
    if (id >= POK_CONFIG_NB_BUFFERS) {
        return POK_ERRNO_EINVAL;
    }

    if (pok_buffers[id].ready == FALSE) {
        return POK_ERRNO_EINVAL;
    }

    if (data == NULL) {
        // maybe it's not mandated, but let's be safe anyway
        return POK_ERRNO_EINVAL;
    }
   
    int64_t delay_ms = arinc_time_to_ms(timeout);

    pok_buffer_t *buffer = &pok_buffers[id];

    pok_event_lock(buffer->lock); 

    if (buffer_is_empty(buffer)) {
        if (delay_ms == 0) {
            pok_event_unlock(buffer->lock);
            *len = 0;
            return POK_ERRNO_EMPTY;
        } else {
            // bail out if preemption is disabled
            if (pok_current_partition_preemption_disabled() || pok_error_is_handler() == POK_ERRNO_OK) {
                pok_event_unlock(buffer->lock);
                return POK_ERRNO_MODE;
            }

            // buffer is empty
            // we're now waiting, and some other process
            // should give us a message _directly_,
            // without copying to circular buffer

            pok_buffer_wait_list_t wait_list_entry;

            if (delay_ms > 0) {
                uint64_t time;
                pok_time_get(&time);
                wait_list_entry.timeout = time + delay_ms;
                wait_list_entry.result = POK_ERRNO_TIMEOUT;
            } else {
                wait_list_entry.timeout = -1; 
            }
            pok_thread_id(&wait_list_entry.thread);

            *len = 0; // if it expires, it should be 0

            wait_list_entry.receiving.data_ptr = data;
            wait_list_entry.receiving.data_size_ptr = len;
            
            buffer_wait_list_append(buffer, &wait_list_entry);

            pok_event_wait(buffer->lock, delay_ms > 0 ? delay_ms : 0);

            // by now, we're either 
            // - timed out 
            // - someone else delivered us a message


            // in any case, we should remove ourselves from the queue
            // (if we weren't removed by other process)
            buffer_wait_list_remove(buffer, &wait_list_entry);

            pok_event_unlock(buffer->lock);
            return wait_list_entry.result;
        }
    }
    
    buffer_pop(buffer, data, len);

    // if someone was waiting for buffer to become not full,
    // move their messages to the queue on their behalf
    while (buffer->wait_list != NULL) {
        uint64_t time;
        pok_time_get(&time);

        if (time >= (uint64_t) buffer->wait_list->timeout) {
            buffer->wait_list = buffer->wait_list->next;
        } else {
            buffer_push(buffer, buffer->wait_list->sending.data_ptr, buffer->wait_list->sending.data_size);
            buffer->wait_list->result = POK_ERRNO_OK;
            pok_event_signal_thread(buffer->lock, buffer->wait_list->thread);
            buffer->wait_list = buffer->wait_list->next;
    
            pok_event_unlock (pok_buffers[id].lock);
            
            pok_thread_yield();

            return POK_ERRNO_OK;
        }
    }

    pok_event_unlock (pok_buffers[id].lock);
    return POK_ERRNO_OK;


}
pok_ret_t pok_buffer_send (
        pok_buffer_id_t             id, 
        const void                  *data, 
        pok_port_size_t             len, 
        int64_t                     timeout)
{
    if (id >= POK_CONFIG_NB_BUFFERS) {
        return POK_ERRNO_EINVAL;
    }
    
    pok_buffer_t *buffer = &pok_buffers[id];

    if (buffer->ready == FALSE) {
        return POK_ERRNO_EINVAL;
    }

    if (data == NULL) {
        return POK_ERRNO_EINVAL;
    }

    if (len <= 0) {
        return POK_ERRNO_EINVAL;
    }

    if (len > buffer->msg_size) {
        return POK_ERRNO_EINVAL;
    }

    int64_t delay_ms = arinc_time_to_ms(timeout);

    pok_event_lock (buffer->lock); 

    if (buffer_is_full(buffer)) {
        if (delay_ms == 0) {
            pok_event_unlock(buffer->lock);
            return POK_ERRNO_FULL;
        }
        
        // bail out if preemption is disabled
        if (pok_current_partition_preemption_disabled() || pok_error_is_handler() == POK_ERRNO_OK) {
            pok_event_unlock(buffer->lock);
            return POK_ERRNO_MODE;
        }

        // buffer is full
        // we are now waiting, and someone else should 
        // receive our message on our behalf
        // (unless it's timed out!)

        pok_buffer_wait_list_t wait_list_entry;
        
        if (delay_ms > 0) {
            uint64_t time;
            pok_time_get(&time);
            wait_list_entry.timeout = time + delay_ms;
            wait_list_entry.result = POK_ERRNO_TIMEOUT;
        } else {
            wait_list_entry.timeout = -1; 
        }

        pok_thread_id(&wait_list_entry.thread); // it's used by others to wake up us

        wait_list_entry.sending.data_ptr = data;
        wait_list_entry.sending.data_size = len;

        buffer_wait_list_append(buffer, &wait_list_entry);

        pok_event_wait(buffer->lock, delay_ms > 0 ? delay_ms : 0);
      
        // by now, we're either 
        // - timed out 
        // - someone else queued our message on our behalf
        //

        // in any case, we should remove ourselves from the queue
        // (if we weren't removed by other process)
        buffer_wait_list_remove(buffer, &wait_list_entry);

        pok_event_unlock(pok_buffers[id].lock);
        return wait_list_entry.result;
    }

    while (buffer->wait_list != NULL) {
        uint64_t time;
        pok_time_get(&time);

        if (time >= (uint64_t) buffer->wait_list->timeout) {
            buffer->wait_list = buffer->wait_list->next;
        } else {
            memcpy(buffer->wait_list->receiving.data_ptr, data, len);
            *buffer->wait_list->receiving.data_size_ptr = len;
            buffer->wait_list->result = POK_ERRNO_OK;
            pok_event_signal_thread(buffer->lock, buffer->wait_list->thread);
            buffer->wait_list = buffer->wait_list->next;

            pok_event_unlock(buffer->lock);
            
            pok_thread_yield();
            
            return POK_ERRNO_OK;
        }   
    }

    // queue it in the buffer
    buffer_push(buffer, data, len);

    pok_event_unlock (pok_buffers[id].lock);

    return POK_ERRNO_OK;
}

pok_ret_t pok_buffer_status (
        pok_buffer_id_t             id,
        pok_buffer_status_t         *status)
{

    if (id >= POK_CONFIG_NB_BUFFERS) {
        return POK_ERRNO_EINVAL;
    }

    if (pok_buffers[id].ready == FALSE) {
        return POK_ERRNO_EINVAL;
    }
    
    pok_buffer_t *buffer = &pok_buffers[id];

    // we have to do that in critical section,
    // otherwise, nb_messages might be inconsistent with
    // waiting_processes
    pok_event_lock(buffer->lock); 

    status->nb_messages = buffer->number_of_messages;
    status->max_messages = buffer->max_number_of_messages;
    status->message_size = buffer->msg_size;
    status->waiting_processes = buffer_wait_list_length(buffer);
    
    pok_event_unlock(buffer->lock); 

    return POK_ERRNO_OK;
}

#endif // POK_NEEDS_BUFFERS
#endif // POK_NEEDS_MIDDLEWARE
