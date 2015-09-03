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

#include <config.h>

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


#ifdef POK_NEEDS_PORTS_QUEUEING

#define foreach_queuing_channels(varname) \
    for (pok_port_channel_t *varname = &pok_queueing_port_channels[0]; \
         varname->src.kind != POK_PORT_CONNECTION_NULL; \
         varname++)

static pok_bool_t queueing_src_try_pop_waiting(pok_port_queueing_t *src)
{
    uint64_t time = POK_GETTICK();

    while (src->wait_list != NULL) {
        if (time >= src->wait_list->timeout) {
            src->wait_list = src->wait_list->next;
        } else { 
            src->wait_list->result = POK_ERRNO_OK;

            pok_port_utils_queueing_write(src, src->wait_list->sending.data_ptr, src->wait_list->sending.data_size);
            
            pok_lockobj_eventsignal_thread(&src->header.lock, src->wait_list->thread);

            src->wait_list = src->wait_list->next;

            return TRUE;
        }
    }
    return FALSE;
}

static pok_bool_t queueing_dst_try_pop_waiting(
        pok_port_queueing_t *dst, 
        const char *data,
        pok_size_t len)
{
    uint64_t time = POK_GETTICK();

    while (dst->wait_list != NULL) {
        if (time >= dst->wait_list->timeout) {
            dst->wait_list = dst->wait_list->next;
        } else {
            dst->wait_list->result = POK_ERRNO_OK;

            memcpy(dst->wait_list->receiving.data_ptr, data, len);
            *dst->wait_list->receiving.data_size_ptr = len;

            pok_lockobj_eventsignal_thread(&dst->header.lock, dst->wait_list->thread);

            dst->wait_list = dst->wait_list->next;

            return TRUE;
        }
    }
    return FALSE;
}

static void pok_queueing_transfer(
    pok_port_queueing_t *src,
    pok_port_queueing_t *dst)
{
    assert(src->max_message_size <= dst->max_message_size);

    /*
     * Two points here:
     *
     * 1) If someone is waiting in dest.  port, we need to give message to them directly.
     * 2) If someone is waiting in source port, we need to push their message to queue afterwards.
     */

    pok_port_data_t *src_place = pok_port_utils_queueing_head(src);

    if (!queueing_dst_try_pop_waiting(dst, &src_place->data[0], src_place->message_size)) {
        // nobody's waiting - queue the message instead

        pok_port_data_t *dst_place = pok_port_utils_queueing_tail(dst);

        dst_place->message_size = src_place->message_size;
        memcpy(&dst_place->data[0], &src_place->data[0], src_place->message_size);
    
        dst->nb_message++;
    }

    src->nb_message--;
    src->queue_head = (src->queue_head + 1) % src->max_nb_messages;

    queueing_src_try_pop_waiting(src);
}

static void pok_queueing_channel_flush_local(
        pok_port_channel_t *chan)
{
    pok_port_queueing_t *src = &pok_queueing_ports[chan->src.local.port_id];
    pok_port_id_t dst_id = chan->dst.local.port_id;
    pok_port_queueing_t *dst = &pok_queueing_ports[dst_id];

    while (!pok_port_utils_queueing_full(dst) && !pok_port_utils_queueing_empty(src)) {
        pok_queueing_transfer(src, dst);
    }
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

            // we have free spot, use it, if anyone's waiting
            queueing_src_try_pop_waiting(port);
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
        pok_port_size_t ring_index = (i + port->queue_head) % port->max_nb_messages;
        pok_port_data_t *data = (pok_port_data_t*) (port->data + port->data_stride * ring_index);
        
        if (aux[ring_index].status != QUEUEING_UDP_STATUS_NONE) {
            continue;
        }

        pok_network_sg_list_t sg_list[2];
        sg_list[0].buffer = aux[ring_index].overhead;
        sg_list[0].size = POK_NETWORK_OVERHEAD;
        sg_list[1].buffer = (void *) &data->data[0];
        sg_list[1].size = data->message_size;
        
        if (!pok_network_send_udp_gather(
            sg_list,
            data->message_size > 0 ? 2 : 1,
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
#endif // POK_NEEDS_NETWORKING

static void pok_queueing_channel_flush(pok_port_channel_t *chan)
{
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
}

#endif // POK_NEEDS_PORTS_QUEUEING

#ifdef POK_NEEDS_PORTS_SAMPLING

#define foreach_sampling_channels(varname) \
    for (pok_port_channel_t *varname = &pok_sampling_port_channels[0]; \
         varname->src.kind != POK_PORT_CONNECTION_NULL; \
         varname++)

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

    assert(src->max_message_size <= dst->max_message_size);

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
#endif // POK_NEEDS_NETWORKING

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

#endif // POK_NEEDS_PORTS_SAMPLING

#ifdef POK_NEEDS_SIMULATION
#include <stdint.h>
#endif

void pok_port_flush_partition (pok_partition_id_t pid)
{
#ifdef POK_NEEDS_NETWORKING
#ifdef POK_NEEDS_SIMULATION
    pok_bool_t have_something_to_send = FALSE;
#endif // POK_NEEDS_SIMULATION

    pok_network_reclaim_send_buffers(); 
#endif

#ifdef POK_NEEDS_PORTS_QUEUEING
    foreach_queuing_channels(chan) { 
        if (chan->src.kind != POK_PORT_CONNECTION_LOCAL) continue; 
        
        pok_port_queueing_t *port = &pok_queueing_ports[chan->src.local.port_id];

        if (port->header.partition != pid) continue;

        pok_queueing_channel_flush(chan);
    }
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
    foreach_sampling_channels(chan) {
        if (chan->src.kind != POK_PORT_CONNECTION_LOCAL) continue; 
        
        pok_port_sampling_t *port = &pok_sampling_ports[chan->src.local.port_id];

        if (port->header.partition != pid) continue;

        pok_sampling_channel_flush(chan);
    }
#endif

#ifdef POK_NEEDS_NETWORKING
    pok_network_flush_send();

#ifdef POK_NEEDS_SIMULATION
    if (have_something_to_send && sim_stop_tick != UINT64_MAX) {
        sim_stop_tick = pok_tick_counter;
    }
#endif // POK_NEEDS_SIMULATION
#endif
}

#ifdef POK_NEEDS_NETWORKING
static pok_bool_t udp_callback_f(uint32_t ip, uint16_t port, const char *payload, size_t length)
{
#ifdef POK_NEEDS_PORTS_SAMPLING
    foreach_sampling_channels(chan) {
        if (chan->src.kind == POK_PORT_CONNECTION_UDP &&
            chan->src.udp.sp_recv_ptr->port == port && (
                (chan->src.udp.sp_recv_ptr->ip == 0UL ||
                 chan->src.udp.sp_recv_ptr->ip == ip
                )
            ))
        {
            assert(chan->dst.kind == POK_PORT_CONNECTION_LOCAL);

            pok_port_sampling_t *dst = &pok_sampling_ports[chan->dst.local.port_id];

            if (length > dst->max_message_size) {
                printf("received sampling message is too big");
                return TRUE; // albeit invalid, packet matched: consider it handled
            }

            memcpy(&dst->data->data[0], payload, length);
            dst->data->message_size = length;
            dst->not_empty = TRUE;
            dst->last_receive = POK_GETTICK();  

            return TRUE;
        }
    }
#endif

#ifdef POK_NEEDS_PORTS_QUEUEING
    foreach_queuing_channels(chan) {
        if (chan->src.kind == POK_PORT_CONNECTION_UDP &&
            chan->src.udp.qp_recv_ptr->port == port && (
                (chan->src.udp.qp_recv_ptr->ip == 0UL ||
                 chan->src.udp.qp_recv_ptr->ip == ip
                )
            ))
        {
            assert(chan->dst.kind == POK_PORT_CONNECTION_LOCAL);

            pok_port_queueing_t *dst = &pok_queueing_ports[chan->dst.local.port_id];

            if (length > dst->max_message_size) {
                printf("received queueing message is too big");
                return TRUE; // albeit invalid, packet matched: consider it handled
            }

            if (!queueing_dst_try_pop_waiting(dst, payload, length)) {
                // nobody's waiting - queue the message instead

                if (pok_port_utils_queueing_full(dst)) {
                    // TODO set overflow flag, as mandated by the standard
                    printf("queueing overflow\n");
                } else {
                    pok_port_utils_queueing_write(dst, payload, length);
                }
            }

            return TRUE;
        }
    }
#endif
    return FALSE;
}
static pok_network_udp_receive_callback_t udp_callback = {udp_callback_f, NULL};

void pok_port_network_init(void)
{
    pok_network_register_udp_receive_callback(&udp_callback);
}
#endif // POK_NEEDS_NETWORKING

#endif // defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)
