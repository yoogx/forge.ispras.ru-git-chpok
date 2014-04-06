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


/* This file contains entry points from system calls
 * (and most of the logic)
 */

// TODO revise return codes

static int find_queueing_port(const char *name) {
    int i;
    for (i = 0; i < POK_CONFIG_NB_QUEUEING_PORTS; i++) {
        //DEBUG_PRINT("comparing %s %s\n", pok_queueing_ports[i].header.name, name);
        if (strcmp(pok_queueing_ports[i].header.name, name) == 0) {
            return i;
        }
    }
    return -1;
}


pok_ret_t pok_port_queueing_create(
    const char                      *name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_message,
    pok_port_direction_t            direction,
    pok_port_queueing_discipline_t  discipline,
    pok_port_id_t                   *id)
{
    int index = find_queueing_port(name);
    
    if (index < 0) {
        DEBUG_PRINT("there's no port named %s\n", name);
        return POK_ERRNO_PORT;
    }
    
    pok_port_queueing_t *port = &pok_queueing_ports[index];

    // check that it belongs to the current partition
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        DEBUG_PRINT("port doesn't belong to this partition\n");
        return POK_ERRNO_PORT;
    }
    if (port->header.created) {
        DEBUG_PRINT("port already created\n");
        return POK_ERRNO_EXISTS;
    }

    if (message_size <= 0 || message_size != port->max_message_size) {
        DEBUG_PRINT("size doesn't match (%u supplied, %u expected)\n", (unsigned) message_size, (unsigned) port->max_message_size);
        return POK_ERRNO_EINVAL;
    }
    if (max_nb_message <= 0 || max_nb_message != port->max_nb_message) {
        DEBUG_PRINT("max_nb_message doesn't match (%u supplied, %u expected)\n", (unsigned) max_nb_message, (unsigned) port->max_nb_message);
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
    if (POK_CURRENT_PARTITION.mode == POK_PARTITION_MODE_NORMAL) {
        DEBUG_PRINT("partition mode is normal\n");
        return POK_ERRNO_MODE;
    }

    // everything is OK, initialize it
    pok_lockobj_attr_t lockattr;
    lockattr.kind = POK_LOCKOBJ_KIND_EVENT;
    lockattr.locking_policy = POK_LOCKOBJ_POLICY_STANDARD;
    (void) discipline; // TODO

    pok_ret_t ret = pok_lockobj_create(&port->header.lock, &lockattr);
    if (ret != POK_ERRNO_OK) return ret;
    
    *id = index;
    port->header.created = TRUE;

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

    uint64_t deadline;
    // timeout
    //  <0 - wait infinite
    //  =0 - never wait
    //  >0 - wait specified amount of time
    if (timeout > 0) {
        deadline = POK_GETTICK() + timeout;
    } else {
        deadline = 0;
    }

    while (pok_port_utils_queueing_empty(port)) {
        if (timeout == 0 || POK_CURRENT_PARTITION.lock_level > 0) {
            break; // never wait
        }
        if (deadline && POK_GETTICK() >= deadline) {
            break; // deadline expired
        }
        //DEBUG_PRINT("sleeping for %u ms...\n", (unsigned) timeout);
        pok_lockobj_eventwait(&port->header.lock, deadline);
        //DEBUG_PRINT("awoke\n");
    }

    if (pok_port_utils_queueing_empty(port)) {
        if (POK_CURRENT_PARTITION.lock_level > 0 && timeout != 0) {
            DEBUG_PRINT("can't wait with preemption locked\n");
            ret = POK_ERRNO_MODE;
        } else if (timeout) {
            DEBUG_PRINT("port is empty (blocking read, timeout=%d)\n", (int) timeout);
            ret = POK_ERRNO_TIMEOUT;
        } else {
            DEBUG_PRINT("port is empty (non-blocking read)\n");
            ret = POK_ERRNO_EMPTY;
        }
        *len = 0;
    } else {
        pok_port_utils_queueing_read(port, data, len);
        ret = POK_ERRNO_OK;
    }

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

    if (port->header.direction != POK_PORT_DIRECTION_OUT) {
        DEBUG_PRINT("port is for receiving, not for sending");
        return POK_ERRNO_DIRECTION;
    }

    pok_lockobj_lock(&port->header.lock, NULL);
    
    uint64_t deadline;
    // timeout
    //  <0 - wait infinite
    //  =0 - never wait
    //  >0 - wait specified amount of time
    if (timeout > 0) {
        deadline = POK_GETTICK() + timeout;
    } else {
        deadline = 0;
    }

    while (pok_port_utils_queueing_full(port)) {
        if (timeout == 0 || POK_CURRENT_PARTITION.lock_level > 0) {
            break; // never wait
        }
        if (deadline && POK_GETTICK() >= deadline) {
            break; // deadline expired
        }
        //DEBUG_PRINT("sleeping for %u ms...\n", (unsigned) timeout);
        pok_lockobj_eventwait(&port->header.lock, deadline);
        //DEBUG_PRINT("awoke\n");
    }

    if (pok_port_utils_queueing_full(port)) {
        if (POK_CURRENT_PARTITION.lock_level > 0 && timeout != 0) {
            DEBUG_PRINT("can't wait with preemption locked\n");
            ret = POK_ERRNO_MODE;
        } else if (timeout) {
            DEBUG_PRINT("port is full (blocking write, timeout=%d)\n", (int) timeout);
            ret = POK_ERRNO_TIMEOUT;
        } else {
            DEBUG_PRINT("port is full (non-blocking write)\n");
            ret = POK_ERRNO_FULL;
        }
    } else {
        pok_port_utils_queueing_write(port, data, len);
        ret = POK_ERRNO_OK;
    }

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
    status->max_nb_message = port->max_nb_message;
    status->max_message_size = port->max_message_size;
    status->direction = port->header.direction;
    status->waiting_processes = 0; // TODO

    return POK_ERRNO_OK;
}

pok_ret_t pok_port_queueing_id(
    const char      *name,
    pok_port_id_t   *id)
{
    int index = find_queueing_port(name);
    if (index < 0) {
        return POK_ERRNO_PORT;
    }
    pok_port_queueing_t *port = &pok_queueing_ports[index];
   
    if (port->header.partition != POK_SCHED_CURRENT_PARTITION) {
        return POK_ERRNO_EINVAL;
    }

    if (!port->header.created) {
        return POK_ERRNO_EINVAL;
    }

    *id = index;

    return POK_ERRNO_OK;
}

#endif // POK_NEEDS_PORTS_SAMPLING
