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

#include <assert.h>

static void pok_queueing_channel_flush(pok_port_channel_t *chan)
{
    pok_port_queueing_t *src = &pok_queueing_ports[chan->src.local.port_id];

    if (chan->dst.kind == POK_PORT_CONNECTION_LOCAL) {
        pok_port_id_t dst_id = chan->dst.local.port_id;
        pok_port_queueing_t *dst = &pok_queueing_ports[dst_id];

        while (!pok_port_utils_queueing_full(dst) && !pok_port_utils_queueing_empty(src)) {
            pok_port_utils_queueing_transfer(src, dst);
        }

        // wake up processes that possibly wait for messages
        // TODO wake them up in order
        pok_lockobj_eventbroadcast(&dst->header.lock);
    } else {
        assert(0 && "Unknown connection type");
    }
    // wake up processes that possibly wait for port becoming non-full
    // TODO wake them up in order
    pok_lockobj_eventbroadcast(&src->header.lock);

}

static void pok_sampling_channel_flush(pok_port_channel_t *chan)
{
    pok_port_sampling_t *src = &pok_sampling_ports[chan->src.local.port_id];

    if (!src->header.must_be_flushed) {
        return;
    }

    if (chan->dst.kind == POK_PORT_CONNECTION_LOCAL) {
        pok_port_id_t dst_id = chan->dst.local.port_id;
        pok_port_sampling_t *dst = &pok_sampling_ports[dst_id];

        memcpy(&dst->data->data[0], &src->data->data[0], src->data->message_size);
        dst->data->message_size = src->data->message_size;
        dst->not_empty = TRUE;
        dst->last_receive = POK_GETTICK(); // TODO or copy it from src port?

    } else {
        assert(0 && "Unknown connection type");
    }
    
    // TODO if we are going to implement multicast
    //      this should go elsewhere
    src->header.must_be_flushed = FALSE;

}

static void pok_port_flush_partition (pok_partition_id_t pid)
{
    // send all _outgoing_ packets

    int i;
    // queueing ports
    for (i = 0; pok_queueing_port_channels[i].src.kind != POK_PORT_CONNECTION_NULL; i++) {
        pok_port_channel_t *chan = &pok_queueing_port_channels[i];
        
        if (chan->src.kind != POK_PORT_CONNECTION_LOCAL) continue; 
        
        pok_port_queueing_t *port = &pok_queueing_ports[chan->src.local.port_id];

        if (port->header.partition != pid) continue;

        pok_queueing_channel_flush(chan);
    }

    // sampling ports
    for (i = 0; pok_sampling_port_channels[i].src.kind != POK_PORT_CONNECTION_NULL; i++) {
        pok_port_channel_t *chan = &pok_sampling_port_channels[i];
        
        if (chan->src.kind != POK_PORT_CONNECTION_LOCAL) continue; 
        
        pok_port_sampling_t *port = &pok_sampling_ports[chan->src.local.port_id];

        if (port->header.partition != pid) continue;

        pok_sampling_channel_flush(chan);
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
