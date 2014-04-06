#include <middleware/port.h>
#include <middleware/port_utils.h>

#include <libc.h>

static pok_port_data_t * pok_port_utils_queueing_tail(
    pok_port_queueing_t *port)
{
    pok_port_size_t index = (port->queue_head + port->nb_message) % port->max_nb_message;
    return (pok_port_data_t *) port->data + port->data_stride * index;
}

static pok_port_data_t * pok_port_utils_queueing_head(
    pok_port_queueing_t *port)
{
    pok_port_size_t index = port->queue_head;
    return (pok_port_data_t *) port->data + port->data_stride * index;
}


void pok_port_utils_queueing_write(
        pok_port_queueing_t *port, 
        const void *message,
        pok_port_size_t message_size)
{
    pok_port_data_t *place = pok_port_utils_queueing_tail(port);

    place->message_size = message_size;
    memcpy(&place->data[0], message, message_size);

    port->nb_message++;
}

void pok_port_utils_queueing_read(
        pok_port_queueing_t *port,
        void *message,
        pok_port_size_t *message_length)
{
    pok_port_data_t *place = pok_port_utils_queueing_head(port);

    *message_length = place->message_size;
    memcpy(message, &place->data[0], place->message_size);

    port->queue_head = (port->queue_head + 1) % port->max_nb_message;
    port->nb_message--;
}

void pok_port_utils_queueing_transfer(
    pok_port_queueing_t *src,
    pok_port_queueing_t *dst)
{
    pok_port_data_t *src_place = pok_port_utils_queueing_head(src);
    pok_port_data_t *dst_place = pok_port_utils_queueing_tail(dst);

    dst_place->message_size = src_place->message_size;
    memcpy(&dst_place->data[0], &src_place->data[0], src_place->message_size);

    dst->nb_message++;
    src->nb_message--;
    src->queue_head = (src->queue_head + 1) % src->max_nb_message;
}
