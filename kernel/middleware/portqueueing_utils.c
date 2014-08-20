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
#ifdef POK_NEEDS_PORTS_QUEUEING

#include <middleware/port.h>
#include <middleware/port_utils.h>

#include <libc.h>

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

    port->queue_head = (port->queue_head + 1) % port->max_nb_messages;
    port->nb_message--;
}

#endif
