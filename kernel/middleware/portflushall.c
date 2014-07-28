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

#include <bsp.h>
#include <types.h>
#include <libc.h>

#include <core/partition.h>
#include <core/lockobj.h>
#include <core/time.h>

#include <middleware/port.h>
#include <middleware/port_utils.h>

#include <assert.h>

static void pok_queueing_channel_flush_local(
        pok_port_channel_t *chan)
{
    pok_port_queueing_t *src = &pok_queueing_ports[chan->src.local.port_id];
    pok_port_id_t dst_id = chan->dst.local.port_id;
    pok_port_queueing_t *dst = &pok_queueing_ports[dst_id];

    while (!pok_port_utils_queueing_full(dst) && !pok_port_utils_queueing_empty(src)) {
        pok_port_utils_queueing_transfer(src, dst);
    }

    // wake up processes that possibly wait for messages
    // TODO wake them up in order
    pok_lockobj_eventbroadcast(&dst->header.lock);
}

#ifdef POK_NEEDS_NETWORKING

#define QUEUEING_UDP_STATUS_NONE 0 // message hasn't been touched by network code at all
#define QUEUEING_UDP_STATUS_PENDING 1 // message has been sent to the driver, and its buffer is still in use by that driver
#define QUEUEING_UDP_STATUS_SENT 2 // message sent, buffer is free to use, but place is still occupied (it will be reclaimed soon)

static void pok_queueing_channel_udp_buffer_callback(
        void *arg)
{
    pok_port_connection_queueing_udp_send_aux_t *aux = arg;
    pok_port_channel_t *chan = aux->chan; 
    pok_port_queueing_t *port = &pok_queueing_ports[chan->src.local.port_id];

    pok_port_connection_queueing_udp_send_aux_t *aux_array =
        chan->dst.udp.qp_send_ptr->aux_array;

    aux->status = QUEUEING_UDP_STATUS_SENT;

    // now, pop messages from the head of queue
    // that are already sent

    pok_port_size_t i;
    pok_port_size_t messages_to_check = port->nb_message;

    for (i = 0; i < messages_to_check; i++) {
        pok_port_size_t ring_index = port->queue_head;

        if (aux_array[ring_index].status == QUEUEING_UDP_STATUS_SENT) {
            port->queue_head = (port->queue_head + 1) % port->max_nb_messages;
            port->nb_message--;

            aux_array[ring_index].status = QUEUEING_UDP_STATUS_NONE;
        } else {
            // ignore messages not at the head of the queue
            // even if they're already sent, and buffers are free to use
            break;
        }
    }
}

static void pok_queueing_channel_flush_udp(
        pok_port_channel_t *chan)
{
    pok_port_queueing_t *port = &pok_queueing_ports[chan->src.local.port_id];
    pok_port_connection_queueing_udp_send_t *conn_info = chan->dst.udp.qp_send_ptr;

    pok_port_connection_queueing_udp_send_aux_t *aux = chan->dst.udp.qp_send_ptr->aux_array;

    // send all messages that aren't sent already
    pok_port_size_t i;
    for (i = 0; i < port->nb_message; i++) {
        if (aux[i].status != QUEUEING_UDP_STATUS_NONE) {
            continue;
        }

        pok_port_size_t ring_index = (i + port->queue_head) % port->max_nb_messages;
        pok_port_data_t *data = (pok_port_data_t*) port->data + port->data_stride * ring_index;

        pok_network_sg_list_t sg_list[2];
        sg_list[0].buffer = aux[ring_index].overhead;
        sg_list[0].size = POK_NETWORK_OVERHEAD;
        sg_list[1].buffer = (void *) &data->data[0];
        sg_list[1].size = data->message_size;
        
        if (!pok_network_send_udp_gather(
            sg_list,
            2,
            conn_info->ip,
            conn_info->port,
            pok_queueing_channel_udp_buffer_callback,
            (void*) &aux[ring_index]
        ))
        {
            // try again later
            return;
        }

        aux[ring_index].status = QUEUEING_UDP_STATUS_PENDING;
        aux[ring_index].chan = chan;
    }
}
#endif

static void pok_queueing_channel_flush(pok_port_channel_t *chan)
{
    pok_port_queueing_t *src = &pok_queueing_ports[chan->src.local.port_id];

    if (chan->dst.kind == POK_PORT_CONNECTION_LOCAL) {
        pok_queueing_channel_flush_local(chan);
    }
#ifdef POK_NEEDS_NETWORKING
    else if (chan->dst.kind == POK_PORT_CONNECTION_UDP) {
        pok_queueing_channel_flush_udp(chan);
    }
#endif
    else {
        assert(0 && "Unknown connection type");
    }
    // wake up processes that possibly wait for port becoming non-full
    // TODO wake them up in order
    pok_lockobj_eventbroadcast(&src->header.lock);
}

#ifdef POK_NEEDS_NETWORKING
static void pok_sampling_channel_udp_buffer_callback(void *arg) {
    pok_bool_t *var = (pok_bool_t*) arg;
    *var = FALSE;
}
#endif

static pok_bool_t pok_sampling_channel_flush_local(
        pok_port_channel_t *chan)
{
    pok_port_sampling_t *src = &pok_sampling_ports[chan->src.local.port_id];
    pok_port_id_t dst_id = chan->dst.local.port_id;
    pok_port_sampling_t *dst = &pok_sampling_ports[dst_id];

    memcpy(&dst->data->data[0], &src->data->data[0], src->data->message_size);
    dst->data->message_size = src->data->message_size;
    dst->not_empty = TRUE;
    dst->last_receive = POK_GETTICK(); // TODO or copy it from src port?

    return TRUE;
}

#ifdef POK_NEEDS_NETWORKING
static pok_bool_t pok_sampling_channel_flush_udp(
        pok_port_channel_t *chan)
{
    pok_port_sampling_t *src = &pok_sampling_ports[chan->src.local.port_id];
    pok_port_connection_sampling_udp_send_t *conn_info = chan->dst.udp.sp_send_ptr;

    if (conn_info->buffer_being_used) {
        // it means that our buffer is still in use by
        // the network driver
        //
        // it might mean that network card is overwhelmed by requests
        printf("buffer is still being used\n");
        return FALSE;
    }
    conn_info->buffer_being_used = TRUE;

    char *message_buffer = &conn_info->buffer[0];

    memcpy(message_buffer + POK_NETWORK_OVERHEAD, &src->data->data[0], src->data->message_size);

    if (!pok_network_send_udp(
        message_buffer,
        src->data->message_size,
        conn_info->ip,
        conn_info->port,
        pok_sampling_channel_udp_buffer_callback,
        &conn_info->buffer_being_used)) 
    {
        // some other kind of internal error
        // for virtio, it might mean that virtqueue is out of descriptors
        return FALSE;
    }

    return TRUE;
}
#endif

static void pok_sampling_channel_flush(pok_port_channel_t *chan)
{
    pok_port_sampling_t *src = &pok_sampling_ports[chan->src.local.port_id];

    if (!src->header.must_be_flushed) {
        return;
    }

    if (chan->dst.kind == POK_PORT_CONNECTION_LOCAL) {
        if (!pok_sampling_channel_flush_local(chan)) return;
    } 
#ifdef POK_NEEDS_NETWORKING
    else if (chan->dst.kind == POK_PORT_CONNECTION_UDP) {
        if (!pok_sampling_channel_flush_udp(chan)) return;
    }
#endif
    else {
        assert(0 && "Unknown connection type");
    }
    
    // TODO if we are going to implement multicast
    //      this should go elsewhere
    src->header.must_be_flushed = FALSE;

}

static void pok_port_flush_partition (pok_partition_id_t pid)
{
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

#ifdef POK_NEEDS_NETWORKING
static pok_bool_t udp_callback_f(uint32_t ip, uint16_t port, const char *payload, size_t length)
{
    int i;

    i = 0;
    for (i = 0; pok_sampling_port_channels[i].src.kind != POK_PORT_CONNECTION_NULL; i++) {
        pok_port_channel_t *chan = &pok_sampling_port_channels[i];
    
        if (chan->src.kind == POK_PORT_CONNECTION_UDP &&
            chan->src.udp.sp_recv_ptr->port == port && (
                (chan->src.udp.sp_recv_ptr->ip == 0UL ||
                 chan->src.udp.sp_recv_ptr->ip == ip
                )
            ))
        {
            assert(chan->dst.kind == POK_PORT_CONNECTION_LOCAL);

            pok_port_sampling_t *dst = &pok_sampling_ports[chan->dst.local.port_id];

            memcpy(&dst->data->data[0], payload, length);
            dst->data->message_size = length;
            dst->not_empty = TRUE;
            dst->last_receive = POK_GETTICK();  

            return TRUE;
        }
    }

    i = 0;
    for (i = 0; pok_queueing_port_channels[i].src.kind != POK_PORT_CONNECTION_NULL; i++) {
        // FIXME support networking for queueing ports as well
        pok_port_channel_t *chan = &pok_queueing_port_channels[i];

        if (chan->src.kind == POK_PORT_CONNECTION_UDP &&
            chan->src.udp.qp_recv_ptr->port == port && (
                (chan->src.udp.qp_recv_ptr->ip == 0UL ||
                 chan->src.udp.qp_recv_ptr->ip == ip
                )
            ))
        {
            assert(chan->dst.kind == POK_PORT_CONNECTION_LOCAL);

            pok_port_queueing_t *dst = &pok_queueing_ports[chan->dst.local.port_id];

            if (pok_port_utils_queueing_full(dst)) {
                // TODO set overflow flag, as mandated by the standard
                printf("queueing overflow\n");
            } else {
                pok_port_utils_queueing_write(dst, payload, length);
                // TODO wake processes up in specific order
                pok_lockobj_eventbroadcast(&dst->header.lock);
            }

            return TRUE;
        }
    }

    return FALSE;
}
static pok_network_udp_receive_callback_t udp_callback = {udp_callback_f, NULL};

void pok_port_network_init(void)
{
    pok_network_register_udp_receive_callback(&udp_callback);
}
#endif

/**
 * Flush all the ports, write all OUT ports to their destinations.
 * This function is called at each major frame
 */
void pok_port_flushall (void)
{
#ifdef POK_NEEDS_NETWORKING
   pok_network_reclaim_send_buffers();
   pok_network_reclaim_receive_buffers();
#endif

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
