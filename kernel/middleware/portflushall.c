/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
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
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

/**
 * \file    middleware/portflushall.c
 * \brief   Flush the ports and send the data of IN ports to OUT ports
 * \date    2008-2014
 * \author  Julien Delange
 * \author  Laurent Lec
 * \auther  Maxim Malkov
 */

#if defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)

#include <types.h>
#include <libc.h>

#include <core/partition.h>
#include <core/lockobj.h>
#include <core/time.h>

#include <middleware/port.h>
#include <middleware/port_utils.h>

static void flush_sampling_port(pok_port_sampling_t *src) {
    if (!src->header.must_be_flushed) {
        return;
    }

    size_t i;
    for (i = 0; i < src->header.num_channels; i++) {
        pok_port_id_t dst_id = src->header.channels[i];
        pok_port_sampling_t *dst = &pok_sampling_ports[dst_id];

        memcpy(&dst->data->data[0], &src->data->data[0], src->data->message_size);
        dst->data->message_size = src->data->message_size;
        dst->not_empty = TRUE;
        dst->last_receive = POK_GETTICK(); // TODO or copy it from src port?
    }

    src->header.must_be_flushed = FALSE;
}

static void flush_queueing_port(pok_port_queueing_t *src) {
    // XXX multicasting for queuing ports is not supported
    size_t i;
    for (i = 0; i < src->header.num_channels && i < 1; i++) {
        pok_port_id_t dst_id = src->header.channels[i];
        pok_port_queueing_t *dst = &pok_queueing_ports[dst_id];

        while (!pok_port_utils_queueing_full(dst) && !pok_port_utils_queueing_empty(src)) {
            pok_port_utils_queueing_transfer(src, dst);
        }

        // wake up processes that possibly wait for messages
        pok_lockobj_eventbroadcast(&dst->header.lock);
    }
    // wake up processes that possible wait for port becoming non-full
    pok_lockobj_eventbroadcast(&src->header.lock);
}

static void pok_port_flush_partition (int pid)
{
    int i;
    for (i = 0; i < POK_CONFIG_NB_SAMPLING_PORTS; i++) {
        pok_port_sampling_t *port = &pok_sampling_ports[i];
        if (port->header.partition == pid) {
            flush_sampling_port(port);
        }   
    }
    for (i = 0; i < POK_CONFIG_NB_QUEUEING_PORTS; i++) {
        pok_port_queueing_t *port = &pok_queueing_ports[i];
        if (port->header.partition == pid) {
            flush_queueing_port(port);
        }      
    }
}

/**
 * Flush all the ports, write all OUT ports to their destinations.
 * This function is called at each major frame
 */
void pok_port_flushall (void)
{
   uint8_t p;
   for (p = 0 ; p < POK_CONFIG_NB_PARTITIONS ; p++)
   {
      if ((pok_partitions[p].mode == POK_PARTITION_MODE_NORMAL) || (pok_partitions[p].mode == POK_PARTITION_MODE_IDLE))
      {
         pok_port_flush_partition (p);
      }
   }
}

#endif
