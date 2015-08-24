/*
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
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

#include <config.h>

#ifdef POK_NEEDS_PORTS_QUEUEING

#include <libc.h>
#include <errno.h>
#include <types.h>
#include <core/lockobj.h>
#include <core/partition.h>
#include <core/time.h>
#include <middleware/port.h>
#include <middleware/port_utils.h>

#if 0
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

static void port_wait_list_append(
        pok_port_queueing_t *port, 
        pok_port_queueing_wait_list_t *entry)
{
    entry->priority = 0; 
    if (port->discipline == POK_PORT_QUEUEING_DISCIPLINE_PRIORITY) {
        entry->priority = POK_CURRENT_THREAD.priority;
    }
    
    pok_port_queueing_wait_list_t **list = &port->wait_list;

    while (*list != NULL && (*list)->priority >= entry->priority) {
        list = &((**list).next);
    }
    entry->next = *list;
    *list = entry;
}

static size_t port_wait_list_length(pok_port_queueing_t *port)
{
    size_t res = 0;
    pok_port_queueing_wait_list_t *list = port->wait_list;

    while (list != NULL) {
        res++;
        list = list->next;
    }
    return res;
}

static void port_wait_list_remove(
        pok_port_queueing_t *port, 
        pok_port_queueing_wait_list_t *entry)
{
    pok_port_queueing_wait_list_t **list = &port->wait_list;

    while (*list != NULL && *list != entry) {
        list = &((**list).next);
    }

    if (*list == NULL) return;

    *list = (**list).next;
}

/* This file contains entry points from system calls
 * (and most of the logic)
 */

// TODO revise return codes

static int find_queueing_port(const char *name, pok_partition_id_t partid) {
    int i;
    for (i = 0; i < POK_CONFIG_NB_QUEUEING_PORTS; i++) {
        if (pok_queueing_ports[i].header.partition != partid) continue;

        if (strcmp(pok_queueing_ports[i].header.name, name) == 0) {
            return i;
        }
    }
    return -1;
}


pok_ret_t pok_port_queueing_create(
    const char                      *name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_messages,
    pok_port_direction_t            direction,
    pok_port_queueing_discipline_t  discipline,
    pok_port_id_t                   *id)
{
    int index = find_queueing_port(name, POK_SCHED_CURRENT_PARTITION);
    
    if (index < 0) {
        DEBUG_PRINT("there's no port named %s\n", name);
        return POK_ERRNO_PORT;
    }
    
    pok_port_queueing_t *port = &pok_queueing_ports[index];

    if (port->header.created) {
        DEBUG_PRINT("port already created\n");
        return POK_ERRNO_EXISTS;
    }

    if (message_size <= 0 || message_size != port->max_message_size) {
        DEBUG_PRINT("size doesn't match (%u supplied, %u expected)\n", (unsigned) message_size, (unsigned) port->max_message_size);
        return POK_ERRNO_EINVAL;
    }
    if (max_nb_messages <= 0 || max_nb_messages != port->max_nb_messages) {
        DEBUG_PRINT("max_nb_messages doesn't match (%u supplied, %u expected)\n", (unsigned) max_nb_messages, (unsigned) port->max_nb_messages);
        return POK_ERRNO_EINVAL;
    }
    if (direction != POK_PORT_DIRECTION_IN && direction != POK_PORT_DIRECTION_OUT) {
        DEBUG_PRINT("direction is not recognized\n");
        return POK_ERRNO_EINVAL;
    }
    if (direction != port->header.direction) {
        DEBUG_PRINT("direction doesn't match\n");
        return POK_ERRNO_EINVAL;
    }
    if (discipline != POK_PORT_QUEUEING_DISCIPLINE_FIFO && discipline != POK_PORT_QUEUEING_DISCIPLINE_PRIORITY) {
        DEBUG_PRINT("queueing discipline is not recognized\n");
        return POK_ERRNO_EINVAL;
    }
    if (POK_CURRENT_PARTITION.mode == POK_PARTITION_MODE_NORMAL) {
        DEBUG_PRINT("partition mode is normal\n");
        return POK_ERRNO_MODE;
    }

    // everything is OK, initialize it
    pok_lockobj_attr_t lockattr;
    lockattr.kind = POK_LOCKOBJ_KIND_EVENT;
    lockattr.queueing_policy = POK_QUEUEING_DISCIPLINE_FIFO; // it doesn't matter

    pok_ret_t ret = pok_lockobj_create(&port->header.lock, &lockattr);
    if (ret != POK_ERRNO_OK) return ret;
    
    *id = index;
    port->header.created = TRUE;
    port->wait_list = NULL;
    port->discipline = discipline;

    DEBUG_PRINT("port %s (index=%d) created\n", name, index);

    return POK_ERRNO_OK;
}

pok_ret_t pok_port_queueing_receive(
    pok_port_id_t           id, 
    int64_t                 timeout, 
    const pok_port_size_t   maxlen, 
    void                    *data, 
    pok_port_size_t         *len)
{
    (void) maxlen; // XXX
    pok_ret_t ret;

    if (id >= POK_CONFIG_NB_QUEUEING_PORTS) {
        return POK_ERRNO_PORT;
    }
    pok_port_queueing_t *port = &pok_queueing_ports[id];
   
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        DEBUG_PRINT("port doesn't belong to this partition\n");
        return POK_ERRNO_EINVAL;
    }

    if (!port->header.created) {
        DEBUG_PRINT("port is not created yet\n");
        return POK_ERRNO_EINVAL;
    }

    if (port->header.direction != POK_PORT_DIRECTION_IN) {
        DEBUG_PRINT("port is for sending, not for receiving\n");
        return POK_ERRNO_DIRECTION;
    }

    pok_lockobj_lock(&port->header.lock, NULL);

    if (!pok_port_utils_queueing_empty(port)) {
        // everything is great, we can receive it right now
        pok_port_utils_queueing_read(port, data, len);
        ret = POK_ERRNO_OK;
        goto end;
    }
    
    *len = 0;

    // queue is empty...

    if (timeout == 0) {
        DEBUG_PRINT("port is empty (non-blocking read)\n");
        ret = POK_ERRNO_EMPTY;
        goto end;
    } 

    if (POK_CURRENT_PARTITION.lock_level > 0) {
        DEBUG_PRINT("can't wait with preemption locked\n");
        ret = POK_ERRNO_MODE;
        goto end;
    }

    if (pok_thread_is_error_handling(&POK_CURRENT_THREAD)) {
        DEBUG_PRINT("can't wait in error handler\n");
        ret = POK_ERRNO_MODE;
        goto end;
    }

    {
        pok_port_queueing_wait_list_t wait_list_entry;

        if (timeout < 0) {
            wait_list_entry.timeout = -1;
        } else {
            wait_list_entry.timeout = POK_GETTICK() + timeout;
            wait_list_entry.result = POK_ERRNO_TIMEOUT;
        }
        wait_list_entry.thread = POK_SCHED_CURRENT_THREAD;

        wait_list_entry.receiving.data_ptr = data;
        wait_list_entry.receiving.data_size_ptr = len;

        port_wait_list_append(port, &wait_list_entry);

        pok_lockobj_eventwait(&port->header.lock, timeout>0 ? timeout : 0);

        // by now, we're either 
        // - timed out 
        // - someone else delivered us a message
            
        port_wait_list_remove(port, &wait_list_entry);

        ret = wait_list_entry.result;
        goto end;
    }

end:
    pok_lockobj_unlock(&port->header.lock, NULL);
    return ret;
}

pok_ret_t pok_port_queueing_send(
    pok_port_id_t       id, 
    const void          *data,
    pok_port_size_t     len,
    int64_t             timeout)
{
    pok_ret_t ret;

    if (id >= POK_CONFIG_NB_QUEUEING_PORTS) {
        DEBUG_PRINT("port is out of range\n");
        return POK_ERRNO_PORT;
    }
    pok_port_queueing_t *port = &pok_queueing_ports[id];
   
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        DEBUG_PRINT("port doesn't belong to this partition\n");
        return POK_ERRNO_EINVAL;
    }

    if (!port->header.created) {
        DEBUG_PRINT("port is not created yet\n");
        return POK_ERRNO_EINVAL;
    }

    if (len <= 0) {
        DEBUG_PRINT("len <= 0\n");
        return POK_ERRNO_EINVAL;
    }

    if (len > port->max_message_size) {
        DEBUG_PRINT("invalid message length\n");
        return POK_ERRNO_PARAM;
    }

    if (port->header.direction != POK_PORT_DIRECTION_OUT) {
        DEBUG_PRINT("port is for receiving, not for sending");
        return POK_ERRNO_DIRECTION;
    }

    pok_lockobj_lock(&port->header.lock, NULL);

    if (!pok_port_utils_queueing_full(port)) {
        // everything is great, we can send right now
        pok_port_utils_queueing_write(port, data, len);
        ret = POK_ERRNO_OK;
        goto end;
    }
    
    // queue is full...

    if (timeout == 0) {
        DEBUG_PRINT("port is full (non-blocking write)\n");
        ret = POK_ERRNO_FULL;
        goto end;
    }   

    if (POK_CURRENT_PARTITION.lock_level > 0) {
        DEBUG_PRINT("can't wait with preemption locked\n");
        ret = POK_ERRNO_MODE;
        goto end;
    }
    
    if (pok_thread_is_error_handling(&POK_CURRENT_THREAD)) {
        DEBUG_PRINT("can't wait in error handler\n");
        ret = POK_ERRNO_MODE;
        goto end;
    }

    {
        pok_port_queueing_wait_list_t wait_list_entry;

        if (timeout < 0) {
            wait_list_entry.timeout = -1;
        } else {
            wait_list_entry.timeout = POK_GETTICK() + timeout;
            wait_list_entry.result = POK_ERRNO_TIMEOUT;
        }
        wait_list_entry.thread = POK_SCHED_CURRENT_THREAD;

        wait_list_entry.sending.data_ptr = data;
        wait_list_entry.sending.data_size = len;

        port_wait_list_append(port, &wait_list_entry);

        pok_lockobj_eventwait(&port->header.lock, timeout>0 ? timeout : 0);

        // by now, we're either 
        // - timed out 
        // - someone else delivered us a message
            
        port_wait_list_remove(port, &wait_list_entry);

        ret = wait_list_entry.result;
        goto end;
    }

end:
    pok_lockobj_unlock(&port->header.lock, NULL);
    return ret;
}

pok_ret_t pok_port_queueing_status(
    pok_port_id_t               id,
    pok_port_queueing_status_t  *status)
{
    if (id >= POK_CONFIG_NB_QUEUEING_PORTS) {
        return POK_ERRNO_PORT;
    }
    pok_port_queueing_t *port = &pok_queueing_ports[id];
   
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        return POK_ERRNO_EINVAL;
    }

    if (!port->header.created) {
        return POK_ERRNO_EINVAL;
    }

    status->nb_message = port->nb_message;
    status->max_nb_message = port->max_nb_messages;
    status->max_message_size = port->max_message_size;
    status->direction = port->header.direction;
    status->waiting_processes = port_wait_list_length(port);

    return POK_ERRNO_OK;
}

pok_ret_t pok_port_queueing_id(
    const char      *name,
    pok_port_id_t   *id)
{
    int index = find_queueing_port(name, POK_SCHED_CURRENT_PARTITION);
    if (index < 0) {
        return POK_ERRNO_PORT;
    }
    pok_port_queueing_t *port = &pok_queueing_ports[index];
   
    if (!port->header.created) {
        return POK_ERRNO_EINVAL;
    }

    *id = index;

    return POK_ERRNO_OK;
}

#endif // POK_NEEDS_PORTS_SAMPLING
