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

#ifdef POK_NEEDS_PORTS_SAMPLING

#include <libc.h>
#include <errno.h>
#include <types.h>
#include <core/lockobj.h>
#include <core/partition.h>
#include <core/time.h>
#include <middleware/port.h>
#include <middleware/queue.h>

#if 0
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/* This file contains entry points from system calls
 * (and most of the logic)
 */

// TODO revise return codes

static int find_sampling_port(const char *name, pok_partition_id_t partid) {
    int i;
    for (i = 0; i < POK_CONFIG_NB_SAMPLING_PORTS; i++) {
        if (pok_sampling_ports[i].header.partition != partid) continue;
            
        if (strcmp(pok_sampling_ports[i].header.name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 *  Creates a sampling port.
 */
pok_ret_t pok_port_sampling_create(
    const char*                            name, 
    const pok_port_size_t                  size, 
    const pok_port_direction_t             direction,
    const uint64_t                         refresh,
    pok_port_id_t*                         id)
{
    // first, find the port in the array
    int index = find_sampling_port(name, POK_SCHED_CURRENT_PARTITION);
    if (index < 0) {
        return POK_ERRNO_PORT;
    }
    pok_port_sampling_t *port = &pok_sampling_ports[index];

    // check that attributes passed match attributes in the configuration
    if (size <= 0 || size != port->max_message_size) {
        DEBUG_PRINT("size doesn't match (%u supplied, %u expected)\n", (unsigned) size, (unsigned) port->max_message_size);
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
    // check that partition mode is not normal
#ifdef POK_NEEDS_PARTITIONS
    if (POK_CURRENT_PARTITION.mode == POK_PARTITION_MODE_NORMAL) {
        DEBUG_PRINT("partition mode is normal\n");
        return POK_ERRNO_MODE;
    }
#endif

    if (port->header.created) {
        DEBUG_PRINT("port already created\n");
        return POK_ERRNO_EXISTS;
    }

    // everything is OK, initialize it
    pok_lockobj_attr_t lockattr;
    lockattr.kind = POK_LOCKOBJ_KIND_EVENT;
    
    // actually, it doesn't matter at all, since sampling ports use mutexes only
    lockattr.queueing_policy = POK_QUEUEING_DISCIPLINE_FIFO;  

    pok_ret_t ret = pok_lockobj_create(&port->header.lock, &lockattr);
    if (ret != POK_ERRNO_OK) return ret;

    *id = index;
    port->header.created = TRUE;
    port->refresh = refresh;
    port->last_receive = 0;
    port->last_validity = FALSE;
    port->not_empty = FALSE;

    return POK_ERRNO_OK;
}

/**
 *  Write a message to specified sampling port.
 */
pok_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void              *data,
    pok_port_size_t         len)
{
    if (id >= POK_CONFIG_NB_SAMPLING_PORTS) {
        return POK_ERRNO_PORT;
    }
    pok_port_sampling_t *port = &pok_sampling_ports[id];
    
    // check that it belongs to the current partition
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        DEBUG_PRINT("port %d doesn't belong to this partition %d\n", (int) id, (int) POK_SCHED_CURRENT_PARTITION);
        return POK_ERRNO_PORT;
    }

    // check that it's created
    if (!port->header.created) {
        DEBUG_PRINT("port is not created\n");
        return POK_ERRNO_PORT;
    }

    if (len <= 0) {
        DEBUG_PRINT("len <= 0\n");
        return POK_ERRNO_EINVAL;
    }

    if (len > port->max_message_size) {
        DEBUG_PRINT("message length is greater than port's max (%u > %u)\n", (unsigned) len, (unsigned) port->max_message_size);
        return POK_ERRNO_EINVAL;
    }

    if (port->header.direction != POK_PORT_DIRECTION_OUT) {
        return POK_ERRNO_DIRECTION;
    }
    
    // parameters are OK, do it
    { 
        pok_lockobj_lock(&port->header.lock, NULL);

        port->data->message_size = len;
        memcpy(&port->data->data[0], data, len);

        port->header.must_be_flushed = TRUE;
        port->last_receive = POK_GETTICK();
        
        pok_lockobj_unlock(&port->header.lock, NULL);
    }
    
    return POK_ERRNO_OK;
}

/*
 * Receive message from specified sampling port.
 */
pok_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void                    *message,
    pok_port_size_t         *len,
    bool_t                  *valid)
{
    if (id >= POK_CONFIG_NB_SAMPLING_PORTS) {
        return POK_ERRNO_PORT;
    }
    pok_port_sampling_t *port = &pok_sampling_ports[id];
    
    // check that it belongs to the current partition
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        DEBUG_PRINT("port %d doesn't belong to this partition %d\n", (int) id, (int) POK_SCHED_CURRENT_PARTITION);
        return POK_ERRNO_PORT;
    }
    
    // check that it's created
    if (!port->header.created) {
        return POK_ERRNO_PORT;
    }

    if (port->header.direction != POK_PORT_DIRECTION_IN) {
        return POK_ERRNO_DIRECTION;
    }

    pok_ret_t ret = POK_ERRNO_OK;

    // parameters are OK, do it
    {
        pok_lockobj_lock(&port->header.lock, NULL);

        if (port->not_empty) {
            *len = port->data->message_size;
            memcpy(message, &port->data->data[0], *len);

            uint64_t age = POK_GETTICK() - port->last_receive;

            port->last_validity = (age < port->refresh);
            *valid = port->last_validity;
        } else {
            *len = 0;
            port->last_validity = FALSE;
            *valid = port->last_validity;
            ret = POK_ERRNO_EMPTY;
        }
        
        pok_lockobj_unlock(&port->header.lock, NULL);
    }
    return ret;
}

/**
 * Returns port id by name.
 * Port must be created and belong to the current partition.
 */
pok_ret_t pok_port_sampling_id(const char *name,
                               pok_port_id_t *id)
{
    int index = find_sampling_port(name, POK_SCHED_CURRENT_PARTITION);
    if (index < 0) {
        return POK_ERRNO_PORT;
    }
    pok_port_sampling_t *port = &pok_sampling_ports[index];

    if (!port->header.created) {
        return POK_ERRNO_EINVAL;
    }

    *id = index;

    return POK_ERRNO_OK;
}

/**
 * Get sampling port status
 */
pok_ret_t pok_port_sampling_status (
    const pok_port_id_t         id,
    pok_port_sampling_status_t  *status)
{
    if (id >= POK_CONFIG_NB_SAMPLING_PORTS) {
        return POK_ERRNO_PORT;
    }
    pok_port_sampling_t *port = &pok_sampling_ports[id];
   
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        return POK_ERRNO_EINVAL;
    }

    if (!port->header.created) {
        return POK_ERRNO_EINVAL;
    }

    status->size = port->max_message_size;
    status->direction = port->header.direction;
    status->refresh = port->refresh;
    status->validity = port->last_validity;

    return POK_ERRNO_OK;
}
 
#endif
