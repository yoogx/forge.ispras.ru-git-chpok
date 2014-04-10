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

#include <types.h>
#include <errno.h>
#include <core/lockobj.h>

typedef enum
{
	 POK_PORT_QUEUEING_DISCIPLINE_FIFO      = 1,
	 POK_PORT_QUEUEING_DISCIPLINE_PRIORITY  = 2
} pok_port_queueing_disciplines_t;

typedef enum
{
	 POK_PORT_DIRECTION_IN   = 1,
	 POK_PORT_DIRECTION_OUT  = 2
} pok_port_directions_t;

typedef pok_queueing_discipline_t pok_port_queueing_discipline_t;

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

    size_t                      num_channels;
    pok_port_id_t               *channels;

    pok_bool_t                  created;
    pok_bool_t                  must_be_flushed;

} pok_port_header_t;

typedef struct
{
    pok_port_size_t             message_size;
    unsigned char               data[];
} pok_port_data_t;

void pok_port_init(void);

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

extern pok_port_sampling_t pok_sampling_ports[POK_CONFIG_NB_SAMPLING_PORTS];
#endif

#ifdef POK_NEEDS_PORTS_QUEUEING
typedef struct
{
    pok_port_header_t           header;

    pok_port_size_t             max_message_size;
    pok_port_size_t             max_nb_message;

    pok_port_size_t             nb_message;
    pok_port_size_t             queue_head;

    size_t                      data_stride;
    char                        *data;

} pok_port_queueing_t;

extern pok_port_queueing_t pok_queueing_ports[POK_CONFIG_NB_QUEUEING_PORTS];
#endif

#ifdef POK_NEEDS_PORTS_QUEUEING
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
    int64_t                timeout, 
    const pok_port_size_t   maxlen, 
    void                    *data, 
    pok_port_size_t         *len
);

pok_ret_t pok_port_queueing_send(
    pok_port_id_t       id, 
    const void          *data,
    pok_port_size_t     len,
    int64_t             timeout
);

pok_ret_t pok_port_queueing_status(
    pok_port_id_t               id,
    pok_port_queueing_status_t  *status
);

pok_ret_t pok_port_queueing_id(
    const char      *name,
    pok_port_id_t   *id
);
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
/* Sampling port functions */

typedef struct
{
    pok_port_size_t         size;
    pok_port_direction_t    direction;
    uint64_t                refresh;
    bool_t                  validity;
} pok_port_sampling_status_t;


pok_ret_t pok_port_sampling_create(
    const char*             name,
    pok_port_size_t         size,
    pok_port_direction_t    direction,
    uint64_t                refresh,
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
#endif

#endif
