#ifndef __POK_KERNEL_PORT_UTILS_H__
#define __POK_KERNEL_PORT_UTILS_H__

#include <middleware/port.h>

static inline pok_bool_t pok_port_utils_queueing_empty(pok_port_queueing_t *port)
{
    return port->nb_message == 0;
}

static inline pok_bool_t pok_port_utils_queueing_full(pok_port_queueing_t *port) 
{
    return port->nb_message == port->max_nb_message;
}

void pok_port_utils_queueing_write(
        pok_port_queueing_t *port, 
        const void *message,
        pok_port_size_t message_length
);

void pok_port_utils_queueing_read(
        pok_port_queueing_t *port,
        void *message,
        pok_port_size_t *message_length
);

void pok_port_utils_queueing_transfer(
    pok_port_queueing_t *src,
    pok_port_queueing_t *dst
);
#endif
