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
#ifndef __POK_KERNEL_PORT_UTILS_H__
#define __POK_KERNEL_PORT_UTILS_H__

#include <middleware/port.h>

#ifdef POK_NEEDS_PORTS_QUEUEING

static inline pok_bool_t pok_port_utils_queueing_empty(pok_port_queueing_t *port)
{
    return port->nb_message == 0;
}

static inline pok_bool_t pok_port_utils_queueing_full(pok_port_queueing_t *port) 
{
    return port->nb_message == port->max_nb_messages;
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

#endif
