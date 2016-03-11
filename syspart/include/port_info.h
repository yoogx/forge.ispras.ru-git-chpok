#ifndef __POK_SYSPART_PORTS_H__
#define __POK_SYSPART_PORTS_H__

#include <net/network.h>
#include <middleware/port.h>

typedef struct
{
    pok_port_kinds_t            kind;
    NAME_TYPE                   name;
    PORT_DIRECTION_TYPE         direction;
    unsigned                    overhead;
} sys_port_header_t;

typedef struct
{
    MESSAGE_SIZE_TYPE           message_size;
    pok_bool_t                  busy;
    char                        data[];
} sys_port_data_t;

enum QUEUING_STATUS {
    QUEUING_STATUS_NONE, // message hasn't been touched by network code at all
    QUEUING_STATUS_PENDING, // message has been sent to the driver, and its buffer is still in use by that driver
    QUEUING_STATUS_SENT, // message sent, buffer is free to use, but place is still occupied (it will be reclaimed soon)
};

typedef struct
{
    MESSAGE_SIZE_TYPE message_size;
    enum QUEUING_STATUS status;
    uint32_t port_id;
    char     data[];
} sys_queuing_data_t;

typedef struct
{
    sys_port_header_t           header;

    pok_port_queueing_discipline_t discipline;

    pok_port_size_t             max_message_size;
    pok_port_size_t             max_nb_messages;

    pok_port_size_t             nb_message;
    pok_port_size_t             queue_head;

    size_t                      data_stride;
    sys_queuing_data_t         *data;
    APEX_INTEGER                id;

} sys_queuing_port_t;

typedef struct
{
    sys_port_header_t           header;

    pok_port_size_t             max_message_size;
    uint64_t                    refresh;
    uint64_t                    last_receive;
    pok_bool_t                  last_validity;
    pok_bool_t                  not_empty;
    pok_bool_t                  is_new;//TRUE if data hasn't been read yet
    sys_port_data_t             *data;
    APEX_INTEGER                id;
} sys_sampling_port_t;


static inline sys_queuing_data_t* utils_queue_tail(sys_queuing_port_t *port)
{
    uint32_t index = (port->queue_head + port->nb_message) % port->max_nb_messages;
    return (sys_queuing_data_t *) (port->data + port->data_stride * index);
}

static inline sys_queuing_data_t* utils_queue_head(sys_queuing_port_t *port)
{
    uint32_t index = port->queue_head;
    return (sys_queuing_data_t *) (port->data + port->data_stride * index);
}

static inline pok_bool_t utils_queue_empty(sys_queuing_port_t *port)
{
    return port->nb_message == 0;
}

static inline pok_bool_t utils_queue_full(sys_queuing_port_t *port) 
{
    return port->nb_message == port->max_nb_messages;
}

#endif
