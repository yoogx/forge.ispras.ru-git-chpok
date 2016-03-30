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
 * \file    include/middleware/port.h
 * \date    2008-2009
 * \author  Julien Delange
 * \brief   Describe queueing and sampling ports structures
 */

#ifndef __POK_KERNEL_PORTS_H__
#define __POK_KERNEL_PORTS_H__

#include <config.h>

#if defined(POK_NEEDS_PORTS_QUEUEING) || defined(POK_NEEDS_PORTS_SAMPLING)

#include <types.h>
#include <errno.h>
#include <core/lockobj.h>

#ifdef POK_NEEDS_NETWORKING
#include <net/network.h>
#endif

typedef enum
{
	 POK_PORT_QUEUEING_DISCIPLINE_FIFO      = 1,
	 POK_PORT_QUEUEING_DISCIPLINE_PRIORITY  = 2
} pok_port_queueing_discipline_t;

typedef enum
{
	 POK_PORT_DIRECTION_IN   = 1,
	 POK_PORT_DIRECTION_OUT  = 2
} pok_port_directions_t;

typedef enum
{
	 POK_PORT_KIND_QUEUEING  = 1,
	 POK_PORT_KIND_SAMPLING  = 2,
	 POK_PORT_KIND_INVALID   = 10
} pok_port_kinds_t;

typedef struct
{
    pok_port_kinds_t            kind;
    const char                  *name;
    pok_partition_id_t          partition;
    pok_port_direction_t        direction;
    pok_lockobj_t               lock;

    pok_bool_t                  created;
    pok_bool_t                  must_be_flushed;
} pok_port_header_t;

typedef struct
{
    pok_port_size_t             message_size;
    char                        data[];
} pok_port_data_t;

typedef enum
{
    POK_PORT_CONNECTION_NULL = 0, // used as a sentinel
    POK_PORT_CONNECTION_LOCAL = 1,
#ifdef POK_NEEDS_NETWORKING
    POK_PORT_CONNECTION_UDP = 2,
#endif
} pok_port_connection_kind_t;

#ifdef POK_NEEDS_NETWORKING
typedef struct
{
    uint32_t ip;
    uint16_t port;

    pok_bool_t buffer_being_used;

    char buffer[]; 
} pok_port_connection_sampling_udp_send_t;

typedef struct
{
    uint32_t ip;
    uint16_t port;
} pok_port_connection_sampling_udp_recv_t;

// we use scatter/gather thing here, 
// so network overhead can be separated from
// actual messages
typedef struct
{
    int status;
    void *chan; // pointer to corresponding channel (wastes space, but I can't think of more efficient way right now)
    char overhead[POK_NETWORK_OVERHEAD];
} pok_port_connection_queueing_udp_send_aux_t;

typedef struct
{
    uint32_t ip;
    uint16_t port;

    pok_port_connection_queueing_udp_send_aux_t aux_array[];
} pok_port_connection_queueing_udp_send_t;

typedef struct
{
    uint32_t ip;
    uint16_t port;
} pok_port_connection_queueing_udp_recv_t;
#endif

typedef struct
{
    pok_port_connection_kind_t kind;

    union {
        struct {
            pok_port_id_t port_id;
        } local;
#ifdef POK_NEEDS_NETWORKING
        struct {
            union {
                // actual type depends on whether it's src or dst,
                // and on type of the port
                pok_port_connection_sampling_udp_recv_t *sp_recv_ptr;
                pok_port_connection_sampling_udp_send_t *sp_send_ptr;
                pok_port_connection_queueing_udp_recv_t *qp_recv_ptr;
                pok_port_connection_queueing_udp_send_t *qp_send_ptr;

                void *ptr; // this one for simplicity of code generation
            };
        } udp;
#endif
    };
} pok_port_connection_t;

typedef struct
{
    pok_port_connection_t src, dst;
} pok_port_channel_t;

/*
 * Called when the system is started.
 */
void pok_port_init(void);

#ifdef POK_NEEDS_NETWORKING
void pok_port_network_init(void);
#endif

/*
 * Called when partition is being restarted
 */
void pok_port_reset(pok_partition_id_t);

void pok_port_flush_partition(pok_partition_id_t);

#ifdef POK_NEEDS_PORTS_QUEUEING
typedef struct pok_port_queueing_wait_list_t
{
    struct pok_port_queueing_wait_list_t *next;

    pok_thread_id_t thread;
    int priority;
    uint64_t timeout;
    pok_ret_t result;
    
    union {
        struct {
            const char *data_ptr;
            pok_port_size_t data_size;
        } sending;

        struct {
            char *data_ptr;
            pok_port_size_t *data_size_ptr;
        } receiving;
    };
} pok_port_queueing_wait_list_t;

typedef struct
{
    pok_port_header_t           header;

    pok_port_queueing_discipline_t discipline;

    pok_port_size_t             max_message_size;
    pok_port_size_t             max_nb_messages;

    pok_port_size_t             nb_message;
    pok_port_size_t             queue_head;

    pok_port_queueing_wait_list_t *wait_list;

    size_t                      data_stride;
    char                        *data;

} pok_port_queueing_t;

extern pok_port_queueing_t pok_queueing_ports[];
extern pok_port_channel_t pok_queueing_port_channels[];

/* Queueing port functions */
typedef struct
{
   pok_port_size_t      nb_message;
   pok_port_size_t      max_nb_message;
   pok_port_size_t      max_message_size;
   pok_port_direction_t direction;
   uint8_t              waiting_processes;
} pok_port_queueing_status_t;

typedef struct
{
    const char                      *name;
    pok_port_size_t                 message_size;
    pok_port_size_t                 max_nb_message;
    pok_port_direction_t            direction;
    pok_port_queueing_discipline_t  discipline;
} pok_port_queueing_create_arg_t;

pok_ret_t pok_port_queueing_create(
    const char                      *name,
    pok_port_size_t                 message_size,
    pok_port_size_t                 max_nb_message,
    pok_port_direction_t            direction,
    pok_port_queueing_discipline_t  discipline,
    pok_port_id_t                   *id
);

pok_ret_t pok_port_queueing_receive(
    pok_port_id_t           id, 
    const pok_time_t*       timeout, 
    void                    *data, 
    pok_port_size_t         *len
);

pok_ret_t pok_port_queueing_send(
    pok_port_id_t       id, 
    const void          *data,
    pok_port_size_t     len,
    const pok_time_t*   timeout
);

pok_ret_t pok_port_queueing_status(
    pok_port_id_t               id,
    pok_port_queueing_status_t  *status
);

pok_ret_t pok_port_queueing_id(
    const char      *name,
    pok_port_id_t   *id
);
#endif // POK_NEEDS_PORTS_QUEUEING

#ifdef POK_NEEDS_PORTS_SAMPLING
typedef struct
{
    pok_port_header_t           header;

    pok_port_size_t             max_message_size;
    uint64_t                    refresh;
    uint64_t                    last_receive;
    pok_bool_t                  last_validity;
    pok_bool_t                  not_empty;
    pok_port_data_t             *data;
} pok_port_sampling_t;

extern pok_port_sampling_t pok_sampling_ports[];
extern pok_port_channel_t pok_sampling_port_channels[];

/* Sampling port functions */

typedef struct
{
    pok_port_size_t         size;
    pok_port_direction_t    direction;
    pok_time_t              refresh;
    bool_t                  validity;
} pok_port_sampling_status_t;


pok_ret_t pok_port_sampling_create(
    const char*             name,
    pok_port_size_t         size,
    pok_port_direction_t    direction,
    const pok_time_t*       refresh,
    pok_port_id_t           *id
);

pok_ret_t pok_port_sampling_write(
    pok_port_id_t           id,
    const void              *data,
    pok_port_size_t         len
);

pok_ret_t pok_port_sampling_read(
    pok_port_id_t           id,
    void                    *message,
    pok_port_size_t         *len,
    bool_t                  *valid
);

pok_ret_t pok_port_sampling_id(
    const char              *name,
    pok_port_id_t           *id
);

pok_ret_t pok_port_sampling_status (
    const pok_port_id_t         id,
    pok_port_sampling_status_t  *status
);
#endif // POK_NEEDS_PORTS_SAMPLING

#endif // defined(POK_NEEDS_PORTS_QUEUEING) || defined(POK_NEEDS_PORTS_SAMPLING)

#endif // __POK_KERNEL_PORTS_H__
